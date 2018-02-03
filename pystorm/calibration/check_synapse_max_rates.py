"""Find the maximum input firing rate of the synapses

For each synapse:
Set up the Tag Action Table to send +1 and -1 spikes to an individual synapse for each input spike
generated from one of the FPGA's spike generators.
Do a binary search over the spike generator's output rate.
For each rate: check if the FIFO buffer overflows, which indicates that spikes are being sent
faster than the synapse can be consumed
"""
import os
import sys
from time import sleep
from time import time as get_time
import argparse
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm
from mpl_toolkits.mplot3d import Axes3D

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph
from pystorm.PyDriver import bddriver as bd

np.set_printoptions(precision=2)

CORE = 0
NRN_N = 4096
# SYN_N = 16
SYN_N = 1024

RUN_TIME = 1.0
INTER_RUN_TIME = 0.2

SPIKE_GEN_TIME_UNIT_NS = 10000 # time unit of fpga spike generator
SPIKE_GEN_IDX = 0 # FPGA spike generator index
# Tag Action Table settings
TAT_IDX = 0
TAT_START_ADDR = 0
TAT_STOP_BIT = 1
TAT_SIGN_0 = 0
TAT_SIGN_1 = 1

SYN_PD_PU = 1024 # analog bias setting

INIT_RATE = 12500 # initial rate to test
MIN_RATE = 1000 # minimum rate to test
MAX_RATE = 1000000 # maximum rate to test

def compute_fpga_rates(min_rate, max_rate, spike_gen_time_unit_ns):
    """Compute the fpga spike generator rates between the min and max rates"""
    max_spike_gen_time_units = 1./min_rate*1E9/spike_gen_time_unit_ns
    min_spike_gen_time_units = 1./max_rate*1E9/spike_gen_time_unit_ns
    max_spike_gen_time_units = int(np.ceil(max_spike_gen_time_units))
    min_spike_gen_time_units = max(int(np.floor(min_spike_gen_time_units)), 1)
    periods_spike_gen_time_units = np.arange(
        max_spike_gen_time_units, min_spike_gen_time_units-1, -1)
    fpga_rates = 1./(periods_spike_gen_time_units*SPIKE_GEN_TIME_UNIT_NS*1E-9)
    fpga_rates = np.ceil(fpga_rates).astype(int)
    return fpga_rates

SPIKE_GEN_RATES = compute_fpga_rates(MIN_RATE, MAX_RATE, SPIKE_GEN_TIME_UNIT_NS)
INIT_RATE_IDX = np.argmin(np.abs(SPIKE_GEN_RATES-INIT_RATE))
N_RATES = len(SPIKE_GEN_RATES)

DATA_DIR = "./data/" + os.path.basename(__file__)[:-3] + "/"
if not os.path.isdir(DATA_DIR):
    os.makedirs(DATA_DIR, exist_ok=True)

def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Characterize the synapse max firing rates')
    parser.add_argument("-r", action="store_true", dest="use_saved_data", help='reuse cached data')
    args = parser.parse_args()
    return args

def build_net():
    """Builds the HAL-level network for testing"""
    dim = 1
    tap_matrix = np.zeros((NRN_N, dim))
    net = graph.Network("net")
    net.create_pool("p", tap_matrix)
    HAL.map(net)

def set_analog():
    """Sets the synapse config bits and the bias currents"""
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PD, SYN_PD_PU)
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PU, SYN_PD_PU)
    for n_idx in range(NRN_N):
        HAL.driver.SetSomaEnableStatus(CORE, n_idx, bd.bdpars.SomaStatusId.DISABLED)
    for s_idx in range(SYN_N):
        HAL.driver.SetSynapseEnableStatus(CORE, s_idx, bd.bdpars.SynapseStatusId.ENABLED)
    HAL.flush()

def set_hal():
    """Set the HAL traffic settings"""
    # clear queues
    HAL.set_time_resolution(downstream_ns=SPIKE_GEN_TIME_UNIT_NS, upstream_ns=10000000)
    HAL.start_traffic(flush=False)
    HAL.disable_spike_recording(flush=False)
    HAL.disable_output_recording(flush=True)

def set_tat(syn_idx):
    """Set up the tag action table to send spikes to an individual synapse"""
    addr = HAL.driver.GetSynAERAddr(syn_idx)
    tat_entry = bd.PackWord([
        (bd.TATSpikeWord.STOP, TAT_STOP_BIT),
        (bd.TATSpikeWord.SYNAPSE_ADDRESS_0, addr),
        (bd.TATSpikeWord.SYNAPSE_SIGN_0, TAT_SIGN_0),
        (bd.TATSpikeWord.SYNAPSE_ADDRESS_1, addr),
        (bd.TATSpikeWord.SYNAPSE_SIGN_1, TAT_SIGN_1)])
    HAL.driver.SetMem(CORE, bd.bdpars.BDMemId.TAT0, [tat_entry], TAT_START_ADDR)
    HAL.flush()

def toggle_spk_generator(rate_idx, sleep_time=RUN_TIME, inter_sleep_time=INTER_RUN_TIME):
    """Toggle the spike generator and check for overflow"""
    HAL.driver.GetFIFOOverflowCounts(CORE) # clear any remaining overflow
    rate = SPIKE_GEN_RATES[rate_idx]
    HAL.driver.SetSpikeGeneratorRates(
        CORE, [SPIKE_GEN_IDX], [TAT_IDX], [rate], time=0, flush=True)
    sleep(sleep_time)
    HAL.driver.SetSpikeGeneratorRates(
        CORE, [SPIKE_GEN_IDX], [TAT_IDX], [0], time=0, flush=True)
    sleep(inter_sleep_time)
    overflow_0, _ = HAL.driver.GetFIFOOverflowCounts(CORE)
    # print("Toggled {}, overflow_count:{}".format(rate_idx, overflow_0))
    return overflow_0

def find_syn_max_rate_bounds():
    """Find the minimum and maximum bounds of the max rate"""
    # print("\nFinding bounds=============")
    idx = INIT_RATE_IDX
    upper_idx = None
    lower_idx = None
    d_idx = 1

    overflow = toggle_spk_generator(idx)
    if overflow:
        upper_idx = idx
    else:
        lower_idx = idx
    # print("Initial bounds {}, {}".format(lower_idx, upper_idx))
    while not lower_idx:
        prev_idx = idx
        idx -= d_idx
        if idx < 0:
            if prev_idx > 0:
                idx = 0
            else:
                break
        overflow = toggle_spk_generator(idx)
        if overflow:
            upper_idx = idx
            d_idx *= 2
        else:
            lower_idx = idx

    while not upper_idx:
        prev_idx = idx
        idx += d_idx
        if idx >= N_RATES:
            if prev_idx < N_RATES-1:
                idx = N_RATES-1
            else:
                break
        overflow = toggle_spk_generator(idx)
        if overflow:
            upper_idx = idx
        else:
            lower_idx = idx
            d_idx *= 2
    # print("Found bounds {}, {} =============".format(lower_idx, upper_idx))

    return lower_idx, upper_idx

def check_rate(idx):
    """Check that the upper and lower bounds are correct"""
    sleep(2*INTER_RUN_TIME)
    overflow = toggle_spk_generator(idx, RUN_TIME*2, INTER_RUN_TIME*5)
    return overflow == 0

def check_syn_max_rate():
    """Deliver spikes to a pair of synapses to find the minimum of their maximum input rates"""
    lower_idx, upper_idx = find_syn_max_rate_bounds()

    if not lower_idx:
        return -1, True
    elif not upper_idx:
        return N_RATES-1, True

    # print("\nRefining bounds...=============")
    while upper_idx-lower_idx > 1:
        idx = int(np.round((upper_idx+lower_idx)/2.))
        overflow = toggle_spk_generator(idx)
        if overflow:
            upper_idx = idx
        else:
            lower_idx = idx

    # print("\nDouble checking bounds...=============")
    # check again
    checked = False
    while not checked:
        checked = check_rate(lower_idx)
        if not checked:
            lower_idx -= 1
    # print("Found bounds {}, {} =============".format(lower_idx, upper_idx))
    # due to TAT configuration, 2 spikes sent to synapse per spike to TAT
    return lower_idx, upper_idx

def plot_max_rates(max_rates):
    """Plot the data"""
    synapses = len(max_rates)

    max_rates_mean = np.mean(max_rates)
    max_rates_median = np.median(max_rates)
    max_rates_min = np.min(max_rates)
    max_rates_max = np.max(max_rates)

    max_period_ns = 1E9/max_rates_min
    min_period_ns = 1E9/max_rates_max

    min_period_fpga_units = int(np.floor(min_period_ns/SPIKE_GEN_TIME_UNIT_NS))
    max_period_fpga_units = int(np.ceil(max_period_ns/SPIKE_GEN_TIME_UNIT_NS))
    fpga_periods = 1E-9*np.arange(
        min_period_fpga_units, max_period_fpga_units+1)*SPIKE_GEN_TIME_UNIT_NS
    fpga_rates = 1./fpga_periods

    fig_1d = plt.figure()
    plt.plot(max_rates, 'o', markersize=1)
    plt.xlim(0, synapses-1)
    plt.xlabel("Soma Index")
    plt.ylabel("Max Firing Rate (Hz)")

    if synapses == NRN_N//4: # all synapses tested
        max_rates_2d = max_rates.reshape((int(np.sqrt(synapses)), -1))
        fig_2d_heatmap = plt.figure()
        ims = plt.imshow(max_rates_2d)
        plt.colorbar(ims)
        plt.xlabel("Soma X Coordinate")
        plt.ylabel("Soma Y Coordinate")
        plt.title("Max Firing Rate (Hz)")

        fig_2d_surf = plt.figure()
        axs = fig_2d_surf.add_subplot(111, projection='3d')
        xy_idx = np.arange(int(np.sqrt(synapses)))
        x_mesh, y_mesh = np.meshgrid(xy_idx, xy_idx)
        surf = axs.plot_surface(
            x_mesh, y_mesh, max_rates_2d, linewidth=0, cmap=cm.viridis, antialiased=False)
        axs.set_xlabel("Soma X Coordinate")
        axs.set_ylabel("Soma Y Coordinate")
        axs.set_zlabel("Soma Max Firing Rate (Hz)")
        fig_2d_surf.colorbar(surf, shrink=0.5, aspect=5)

        fig_2d_heatmap.savefig(DATA_DIR + "2d_heatmap.pdf")
        fig_2d_surf.savefig(DATA_DIR + "2d_surface.pdf")

    fig_hist = plt.figure()
    for fpga_rate in fpga_rates:
        plt.axvline(fpga_rate, color=(0.8, 0.8, 0.8), linewidth=1)
    plt.hist(max_rates, bins=80)
    plt.axvline(max_rates_mean, color="k", alpha=0.6, linewidth=1, label="mean")
    plt.axvline(max_rates_median, color="r", alpha=0.4, linewidth=1, label="median")
    plt.xlabel("Max firing Rate (Hz)")
    plt.ylabel("Counts")
    plt.title("Mean:{:,.0f} Median:{:,.0f} Min:{:,.0f} Max:{:,.0f}".format(
        max_rates_mean, max_rates_median, max_rates_min, max_rates_max))
    plt.legend()

    fig_1d.savefig(DATA_DIR + "syn_idx_vs_max_rate.pdf")
    fig_hist.savefig(DATA_DIR + "histogram.pdf")

def report_time_remaining(start_time, syn_idx):
    """Occasionally estimate and report the remaining time"""
    if syn_idx%4 == 0 and SYN_N > 1:
        n_syn_completed = syn_idx+1
        delta_time = get_time()-start_time
        est_time_remaining = delta_time/(syn_idx+1) * (SYN_N-n_syn_completed)
        print("estimated time remaining: {:.0f} s = {:.1f} min = {:.2f} hr...".format(
            est_time_remaining, est_time_remaining/60., est_time_remaining/60./60.))

def check_synapse_max_rates(parsed_args):
    """Run the check"""
    use_saved_data = parsed_args.use_saved_data
    if use_saved_data:
        try:
            max_rates = np.loadtxt(DATA_DIR + "max_rates.txt")
        except FileNotFoundError:
            print("\nError: Could not find saved data {}\n".format(DATA_DIR + "max_rates.txt"))
            sys.exit(1)
    else:
        print("Checking maximum synaptic input rates over possible input rates \n" +
              str(SPIKE_GEN_RATES) +
              "\nStarting with rate {}".format(SPIKE_GEN_RATES[INIT_RATE_IDX]))
        build_net()
        set_analog()
        set_hal()
        max_rates = np.zeros(SYN_N)
        lower_idxs = np.zeros(SYN_N, dtype=int)
        upper_idxs = np.zeros(SYN_N, dtype=int)
        start_time = get_time()
        for syn_idx in range(SYN_N):
            set_tat(syn_idx)
            lower_idx, upper_idx = check_syn_max_rate()
            lower_idxs[syn_idx] = lower_idx
            upper_idxs[syn_idx] = upper_idx
            max_rates[syn_idx] = SPIKE_GEN_RATES[lower_idx]
            print("syn:{} max_rate:{:.0f}".format(
                syn_idx, max_rates[syn_idx]))
            report_time_remaining(start_time, syn_idx)
        print("Max firing rates:")
        print(max_rates)
        print("Min synapse spike consumption times:")
        print(1./max_rates)

    if not use_saved_data:
        np.savetxt(DATA_DIR + "max_rates.txt", max_rates)
        np.savetxt(DATA_DIR + "lower_idxs.txt", lower_idxs, fmt="%d")
        np.savetxt(DATA_DIR + "upper_idxs.txt", upper_idxs, fmt="%d")
    plot_max_rates(max_rates)
    plt.show()

if __name__ == "__main__":
    check_synapse_max_rates(parse_args())
