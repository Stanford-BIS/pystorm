"""Find the maximum firing rate of the somas as driven by bias

Set soma offset bias to max
Set soma refractory bias to max (minimum refractory)

Iterate through the somas, and collect spikes
Plot results
"""
import os
from time import sleep
import argparse
import numpy as np
import matplotlib.pyplot as plt

from pystorm.hal import HAL
from pystorm.hal.hal import parse_hal_spikes
from pystorm.hal.neuromorph import graph
from pystorm.PyDriver import bddriver as bd

from utils.exp import clear_spikes
from utils.file_io import load_txt_data

CORE = 0
MAX_NEURONS = 4096
BIAS_REF = 1024
BIAS_OFFSET = 1024
TIME_SCALE = 1E-9

NEURONS = 4096
RUN_TIME = 0.1
INTER_RUN_TIME = 0.1

DATA_DIR = "./data/" + os.path.basename(__file__)[:-3] + "/"
if not os.path.isdir(DATA_DIR):
    os.makedirs(DATA_DIR, exist_ok=True)

def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Characterize the soma max firing rates')
    parser.add_argument("-r", action="store_true", dest="use_saved_data", help='reuse cached data')
    args = parser.parse_args()
    return args

def build_net():
    """Builds the HAL-level network for testing"""
    dim = 1
    tap_matrix = np.zeros((MAX_NEURONS, dim))
    net = graph.Network("net")
    pool = net.create_pool("pool", tap_matrix)
    hal = HAL()
    hal.map(net)
    return hal, pool

def set_analog(hal):
    """Sets the soma config bits and the bias currents"""
    for nrn_idx in range(MAX_NEURONS):
        hal.driver.SetSomaGain(CORE, nrn_idx, bd.bdpars.SomaGainId.ONE)
        hal.driver.SetSomaOffsetSign(CORE, nrn_idx, bd.bdpars.SomaOffsetSignId.POSITIVE)
        hal.driver.SetSomaOffsetMultiplier(CORE, nrn_idx, bd.bdpars.SomaOffsetMultiplierId.THREE)
        hal.driver.SetSomaEnableStatus(CORE, nrn_idx, bd.bdpars.SomaStatusId.DISABLED)
    hal.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_REF, BIAS_REF)
    hal.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, BIAS_OFFSET)
    hal.flush()

def set_hal(hal):
    """Sets the HAL settings that remain constant throughout the experiment"""
    hal.disable_output_recording(flush=True)

def toggle_hal(hal, nrn_idx):
    """Start and stop HAL traffic"""
    # clear queues
    aer_nrn_idx = hal.driver.BDPars.GetSomaAERAddr(nrn_idx)
    hal.driver.SetSomaEnableStatus(CORE, aer_nrn_idx, bd.bdpars.SomaStatusId.ENABLED)
    hal.set_time_resolution(upstream_ns=10000)
    hal.start_traffic(flush=False)
    hal.enable_spike_recording(flush=True)
    sleep(RUN_TIME)
    hal.driver.SetSomaEnableStatus(CORE, aer_nrn_idx, bd.bdpars.SomaStatusId.DISABLED)
    hal.stop_traffic(flush=False)
    hal.disable_spike_recording(flush=True)
    hal.set_time_resolution(upstream_ns=10000000)
    hal.flush()

def measure_soma_max_rate(hal, pool, nrn_idx):
    """Collect spikes to find a single soma's max firing rate"""
    clear_spikes(hal, INTER_RUN_TIME)
    toggle_hal(hal, nrn_idx)
    hal_spikes = parse_hal_spikes(hal.get_spikes())
    # print("\nTesting nrn {}. Detected the following spikes".format(nrn_idx))
    # for idx in hal_spikes[pool]:
    #     print("nrn_idx {} spikes {}".format(idx, len(hal_spikes[pool][idx])))
    soma_spikes = np.array(hal_spikes[pool][nrn_idx])[:, 0]
    soma_spikes -= soma_spikes[0]
    soma_spikes = soma_spikes
    n_spks = len(soma_spikes)-1
    time_period = (soma_spikes[-1]- soma_spikes[0])*TIME_SCALE
    max_rate = n_spks/time_period
    clear_spikes(hal, INTER_RUN_TIME)
    return max_rate

def plot_max_rates(max_rates):
    """Plot the data"""
    neurons = len(max_rates)

    fig_1d = plt.figure()
    plt.plot(max_rates, 'o', markersize=1)
    plt.xlim(0, neurons-1)
    plt.xlabel("Soma Index")
    plt.ylabel("Max Firing Rate (Hz)")

    max_rates_2d = max_rates.reshape((int(np.sqrt(neurons)), -1))
    fig_2d_heatmap = plt.figure()
    ims = plt.imshow(max_rates_2d)
    plt.colorbar(ims)
    plt.xlabel("Soma X Coordinate")
    plt.ylabel("Soma Y Coordinate")
    plt.title("Max Firing Rate (Hz)")

    fig_hist = plt.figure()
    bins = min(max(10, neurons), 80)
    max_rates_mean = np.mean(max_rates)
    max_rates_median = np.median(max_rates)
    max_rates_min = np.min(max_rates)
    max_rates_max = np.max(max_rates)
    plt.hist(max_rates, bins=bins)
    plt.axvline(max_rates_mean, color="k", label="mean")
    plt.axvline(max_rates_median, color="r", label="median")
    plt.xlabel("Max firing Rate (Hz)")
    plt.ylabel("Counts")
    plt.title("Mean:{:,.0f} Median:{:,.0f} Min:{:,.0f} Max:{:,.0f}".format(
        max_rates_mean, max_rates_median, max_rates_min, max_rates_max))
    plt.legend()

    fig_1d.savefig(DATA_DIR + "nrn_idx_vs_max_rate.pdf")
    fig_2d_heatmap.savefig(DATA_DIR + "2d_heatmap.pdf")
    fig_hist.savefig(DATA_DIR + "histogram.pdf")

def check_soma_max_rates(parsed_args):
    """Run the check"""
    use_saved_data = parsed_args.use_saved_data
    if use_saved_data:
        max_rates = load_txt_data(DATA_DIR + "max_rates.txt")
    else:
        hal, pool = build_net()
        set_analog(hal)
        set_hal(hal)
        max_rates = np.zeros(NEURONS)
        for nrn_idx in range(NEURONS):
            max_rates[nrn_idx] = measure_soma_max_rate(hal, pool, nrn_idx)
        np.savetxt(DATA_DIR + "max_rates.txt", max_rates)

    plot_max_rates(max_rates)
    print("Max firing rates:")
    print(max_rates)
    plt.show()

if __name__ == "__main__":
    check_soma_max_rates(parse_args())
