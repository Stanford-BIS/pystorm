import argparse
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import time
import pickle

from pystorm.hal import HAL, parse_hal_spikes, parse_hal_binned_tags, bin_tags_spikes

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

WIDTH = 32
HEIGHT = 32
N = WIDTH * HEIGHT

# dimensions to collect tags from
D = 8

CORE_ID = 0

SOMA_BIAS = 1
DAC_BIAS_SCALE = 1 # avoid > 10
TCOLLECT = 1 # how long to turn on spikes for
NTRIAL = 4 # how many trials to average stats over

NS_RES = 1000000

def map_network(HAL):

    downstream_time_res = 10000 # ns
    upstream_time_res = NS_RES # ns = 1 ms

    HAL.set_time_resolution(downstream_time_res, upstream_time_res)
    
    net = graph.Network("net")

    # get bad_syn, but make even
    bad_syn = HAL.get_calibration('synapse', 'high_bias_magnitude').values.reshape((32, 32))
    bad_syn = bad_syn[:HEIGHT//2, :WIDTH//2].flatten()
    bad_idx = np.arange(N//4)[bad_syn]
    if np.sum(bad_syn) % 2 == 1:
        bad_syn[bad_idx[-1]] = 0
    bad_syn = bad_syn.reshape((HEIGHT//2, WIDTH//2))

    taps = np.zeros((HEIGHT, WIDTH), dtype=int)
    taps[::2, ::2] = ~bad_syn
    taps = taps.reshape((N,1))

    pool = net.create_pool("pool", taps, biases=SOMA_BIAS)
    bucket = net.create_bucket("bucket", D)
    output = net.create_output("output", D)

    decoders = np.array([[1 / 2**d for d in range(D)] for n in range(N)]).T

    net.create_connection("p2b", pool, bucket, decoders)
    net.create_connection("b2o", bucket, output, None)

    # map network
    #print("calling map")
    HAL.map(net)

    # set bias twiddles
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , DAC_BIAS_SCALE)

    return net

def start_collection(HAL):

    #print("starting data collection")
    HAL.start_traffic(flush=False)
    HAL.enable_output_recording(flush=True)

def end_collection_bin_spikes_old_way(HAL, boundaries):

    HAL.stop_traffic(flush=False)
    HAL.disable_output_recording(flush=True)
    #print("done collecting data")

    starttime = time.time()
    raw_tags = HAL.get_outputs()
    print(len(raw_tags))
    print("  get_outputs took (total)", time.time() - starttime)

    starttime = time.time()
    parsed_tags = parse_hal_binned_tags(raw_tags)
    print("parse_hal_binned_tags took", time.time() - starttime)

    starttime = time.time()
    binned_tags = bin_tags_spikes(parsed_tags, boundaries)
    for obj in binned_tags:
        print(np.sum(binned_tags[obj], axis=1))
    print("bin_tags_spikes took", time.time() - starttime)
    #print("done binning spikes")

    return binned_tags, raw_tags

def end_collection_bin_spikes_new_way(HAL):

    HAL.stop_traffic(flush=False)
    HAL.disable_output_recording(flush=True)
    #print("done collecting data")

    starttime = time.time()
    binned_tags, bin_times = HAL.get_array_outputs()
    print("  get_array_outputs took (total)", time.time() - starttime)
    o0 = next(iter(binned_tags))
    print(np.sum(binned_tags[o0], axis=1))

    return binned_tags, bin_times
    
if __name__ == "__main__":
    HAL = HAL() # HAL is a global, used by run_tau_exp, assign it here

    net = map_network(HAL)

    times = []
    for i in range(NTRIAL):
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
    for i in range(NTRIAL):
        starttime = time.time()

        fpga_time = HAL.get_time()
        start_collection(HAL)

        time.sleep(TCOLLECT * 1.05)

        binned, bin_times = end_collection_bin_spikes_new_way(HAL)

        times.append(time.time() - starttime)

    print(times)
    print("mean cycle time NEW", np.mean(times))
