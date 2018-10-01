import argparse
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import time
import pickle

from pystorm.hal import HAL, parse_hal_spikes, bin_tags_spikes

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

# the whole chip
WIDTH = 64 # array width
HEIGHT = 64 # array height
N = WIDTH * HEIGHT

CORE_ID = 0

SOMA_BIAS = 1
DAC_BIAS_SCALE = 1 # avoid > 10
TCOLLECT = 1 # how long to turn on spikes for

NTRIAL_OLD = 2 # how many trials to average stats over
NTRIAL_NEW = 20 # how many trials to average stats over

NS_RES = 1000000

def map_network(HAL):

    downstream_time_res = 10000 # ns
    upstream_time_res = NS_RES # ns = 1 ms

    HAL.set_time_resolution(downstream_time_res, upstream_time_res)
    
    net = graph.Network("net")

    # get bad_syn, but make even
    bad_syn = HAL.get_calibration('synapse', 'high_bias_magnitude').values
    bad_idx = np.arange(N//4)[bad_syn]
    bad_syn[bad_idx[-1]] = 0
    bad_syn = bad_syn.reshape((HEIGHT//2, WIDTH//2))

    taps = np.zeros((HEIGHT, WIDTH), dtype=int)
    taps[::2, ::2] = ~bad_syn
    taps = taps.reshape((N,1))

    pool = net.create_pool("pool", taps, biases=SOMA_BIAS)

    # map network
    #print("calling map")
    HAL.map(net)

    # set bias twiddles
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , DAC_BIAS_SCALE)

    return net

def start_collection(HAL):

    #print("starting data collection")
    HAL.start_traffic(flush=False)
    HAL.enable_spike_recording(flush=True)

def end_collection_bin_spikes_old_way(HAL, boundaries):

    HAL.stop_traffic(flush=False)
    HAL.disable_spike_recording(flush=True)
    #print("done collecting data")

    starttime = time.time()
    raw_spikes = HAL.get_spikes()
    print("  get_spikes took (total)", time.time() - starttime)

    starttime = time.time()
    parsed_spikes = parse_hal_spikes(raw_spikes)
    print("parse_hal_spikes took", time.time() - starttime)

    starttime = time.time()
    binned_spikes = bin_tags_spikes(parsed_spikes, boundaries)
    print("bin_tags_spikes took", time.time() - starttime)
    #print("done binning spikes")

    return binned_spikes, raw_spikes

def end_collection_bin_spikes_new_way(HAL):

    HAL.stop_traffic(flush=False)
    HAL.disable_spike_recording(flush=True)
    #print("done collecting data")

    starttime = time.time()
    binned_spikes, bin_times = HAL.get_binned_spikes(NS_RES)
    print("  get_binned_spikes took (total)", time.time() - starttime)

    return binned_spikes, bin_times
    
if __name__ == "__main__":
    HAL = HAL() # HAL is a global, used by run_tau_exp, assign it here

    net = map_network(HAL)

    times = []
    for i in range(NTRIAL_OLD):
        starttime = time.time()

        fpga_time = HAL.get_time()
        start_collection(HAL)

        time.sleep(TCOLLECT * 1.05)

        boundaries = np.linspace(fpga_time, fpga_time + TCOLLECT*1e9, 1000)
        binned, raw = end_collection_bin_spikes_old_way(HAL, boundaries)

        times.append(time.time() - starttime)

    print(times)
    print("mean cycle time OLD", np.mean(times))

    times = []
    for i in range(NTRIAL_NEW):
        starttime = time.time()

        fpga_time = HAL.get_time()
        start_collection(HAL)

        time.sleep(TCOLLECT * 1.05)

        binned, bin_times = end_collection_bin_spikes_new_way(HAL)
        p0 = next(iter(binned))

        times.append(time.time() - starttime)

    print(times)
    print("mean cycle time NEW", np.mean(times))
