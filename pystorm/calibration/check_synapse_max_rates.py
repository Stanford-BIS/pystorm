"""Find the maximum input firing rate of the synapses

For each synapse:
Set up the Tag Action Table to send +1 and -1 spikes to an individual synapse for each input spike
generated from one of the FPGA's spike generators.
Do a binary search over the spike generator's output rate.
For each rate: check if the FIFO buffer overflows, which indicates that spikes are being sent
faster than the synapse can be consumed
"""
import os
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

from utils import load_npy_data

np.set_printoptions(precision=2)

CORE = 0
NRN_N = 4096
# SYN_N = 16
# SYN_N = 64
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

FIFO_BUFFER_SIZE = 512

SYN_PD_PU = 1024 # analog bias setting

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
    fpga_rates = np.floor(fpga_rates).astype(int)
    return fpga_rates

SPIKE_GEN_RATES = compute_fpga_rates(MIN_RATE, MAX_RATE, SPIKE_GEN_TIME_UNIT_NS)

DIFF_TOL = 0.05 # tolerance between overflow differences

DATA_DIR = "./data/" + os.path.basename(__file__)[:-3] + "/"
DETAIL_DATA_DIR = DATA_DIR + "fits/"
if not os.path.isdir(DATA_DIR):
    os.makedirs(DATA_DIR, exist_ok=True)
if not os.path.isdir(DETAIL_DATA_DIR):
    os.makedirs(DETAIL_DATA_DIR, exist_ok=True)

def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Characterize the synapse max input firing rates')
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

def clear_overflow(inter_overflow_sample_time):
    """Clear any remaining overflow counts"""
    overflow, _ = HAL.driver.GetFIFOOverflowCounts(CORE)
    while overflow:
        sleep(inter_overflow_sample_time)
        overflow, _ = HAL.driver.GetFIFOOverflowCounts(CORE)

def toggle_spk_generator(rate, run_sleep_time=RUN_TIME, inter_run_sleep_time=INTER_RUN_TIME):
    """Toggle the spike generator and check for overflow"""
    clear_overflow(inter_run_sleep_time)
    HAL.driver.SetSpikeGeneratorRates(
        CORE, [SPIKE_GEN_IDX], [TAT_IDX], [rate], time=0, flush=True)
    sleep(run_sleep_time)
    HAL.driver.SetSpikeGeneratorRates(
        CORE, [SPIKE_GEN_IDX], [TAT_IDX], [0], time=0, flush=True)
    sleep(inter_run_sleep_time)
    overflow_0, _ = HAL.driver.GetFIFOOverflowCounts(CORE)
    print("\tRate {:.1f}, overflow_count:{}".format(rate, overflow_0))
    return overflow_0

def test_rates(syn_idx):
    """Deliver spikes to a synapse to find its spike consumption rate"""
    set_tat(syn_idx)
    overflows = []
    rates = []
    for rate in SPIKE_GEN_RATES[::-1]:
        rates.append(rate)
        overflows.append(toggle_spk_generator(rate))
        if overflows[-1] > 0:
            overflows[-1] += FIFO_BUFFER_SIZE
        if len(overflows) >= 2 and np.all(np.array(overflows[-2:]) == 0):
            break
    rates = rates[::-1]
    overflows = overflows[::-1]
    return rates, overflows

def fit_max_rates(rates, overflows):
    """Infer the max synapse input rate from the overflow data

    Parameters
    ----------
    rates: list of list of floats
        list of list of rates tested for each synapse
    overflows: list of list of ints
        list of list of rates tested for each synapse
    """
    max_rates = np.zeros(SYN_N)
    overflow_slopes = np.zeros(SYN_N)
    for syn_idx in range(SYN_N):
        syn_overflows = np.array(overflows[syn_idx])
        syn_rates = np.array(rates[syn_idx])
        nonzero_idx = syn_overflows>0
        syn_overflows = syn_overflows[nonzero_idx]
        syn_rates = syn_rates[nonzero_idx]
        dodrs = np.diff(syn_overflows) / np.diff(syn_rates)
        print("\nFitting synapse {}".format(syn_idx))
        dodr_clusters = {} # {slope: [indices], ...}
        for dodr_idx, dodr in enumerate(dodrs):
            if dodr > 0:
                if not dodr_clusters:
                    dodr_clusters[dodr] = [dodr_idx]
                else:
                    np_dodr_clusters = np.array(list(dodr_clusters.keys()))
                    relative_diffs = np.abs(dodr-np_dodr_clusters)/np_dodr_clusters
                    if np.all(relative_diffs > DIFF_TOL):
                        dodr_clusters[dodr] = [dodr_idx]
                    else:
                        key = list(dodr_clusters.keys())[np.argmin(relative_diffs)]
                        dodr_clusters[key].append(dodr_idx)
        do_idxs = dodr_clusters[np.max(list(dodr_clusters.keys()))]
        o_idxs = [do_idxs[0]] + list(np.array(do_idxs)+1)
        print("d_overflows/d_rates {}".format(dodrs))
        print("d_overflow/d_rate clusters:\n{}".format(dodr_clusters.keys()))
        print("overflow rates for fitting: {}".format(syn_rates[o_idxs]))
        print("overflows for fitting:      {}".format(syn_overflows[o_idxs]))
        y = syn_overflows[o_idxs]
        A = np.array([syn_rates[o_idxs], np.ones(len(o_idxs))]).T
        x = np.dot(np.dot(np.linalg.inv(np.dot(A.T, A)), A.T), y) # x = (A_T*A)^-1*A_T*y
        overflow_slopes[syn_idx] = x[0]
        max_rates[syn_idx] = -x[1]/x[0]
    return max_rates, overflow_slopes

def report_time_remaining(start_time, syn_idx):
    """Occasionally estimate and report the remaining time"""
    if syn_idx%4 == 0 and SYN_N > 1:
        n_syn_completed = syn_idx+1
        delta_time = get_time()-start_time
        est_time_remaining = delta_time/(syn_idx+1) * (SYN_N-n_syn_completed)
        print("estimated time remaining: {:.0f} s = {:.1f} min = {:.2f} hr...".format(
            est_time_remaining, est_time_remaining/60., est_time_remaining/60./60.))

def plot_data(rates, overflows, max_rates, overflow_slopes):
    """Plot the data"""
    syn_n = len(max_rates)
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
    plt.xlim(0, syn_n-1)
    plt.xlabel("Synapse Index")
    plt.ylabel("Max Input Rate / 2 (spks/s)")
    fig_1d.savefig(DATA_DIR + "syn_idx_vs_max_rate.pdf")

    fig_hist = plt.figure()
    for fpga_rate in fpga_rates:
        plt.axvline(fpga_rate, color=(0.8, 0.8, 0.8), linewidth=1)
    plt.hist(max_rates, bins=80)
    plt.axvline(max_rates_mean, color="k", alpha=0.6, linewidth=1, label="mean")
    plt.axvline(max_rates_median, color="r", alpha=0.4, linewidth=1, label="median")
    plt.xlabel("Max Input Rate / 2 (spks/s)")
    plt.ylabel("Counts")
    plt.title("Mean:{:,.0f} Median:{:,.0f} Min:{:,.0f} Max:{:,.0f}".format(
        max_rates_mean, max_rates_median, max_rates_min, max_rates_max))
    plt.legend()
    fig_hist.savefig(DATA_DIR + "histogram.pdf")

    if syn_n == NRN_N//4: # all syn_n tested
        sqrt_n = int(np.ceil(np.sqrt(syn_n)))
        max_rates_2d = max_rates.reshape((sqrt_n, -1))
        fig_2d_heatmap = plt.figure()
        ims = plt.imshow(max_rates_2d)
        plt.colorbar(ims)
        plt.xlabel("Synapse X Coordinate")
        plt.ylabel("Synapse Y Coordinate")
        plt.title("Max Input Rate / 2 (spks/s)")
        fig_2d_heatmap.savefig(DATA_DIR + "2d_heatmap.pdf")

    fig, axis = plt.subplots(nrows=1, ncols=1)
    for syn_idx in range(syn_n):
        axis.axhline(0, linewidth=0.5, color='k')
        axis.plot(rates[syn_idx], overflows[syn_idx], 'o', label="measured")
        ylims = axis.get_ylim()
        axis.plot(rates[syn_idx], overflow_slopes[syn_idx]*(rates[syn_idx]-max_rates[syn_idx]),
                  "r", linewidth=0.7, label="fit")
        axis.axvline(max_rates[syn_idx], linewidth=0.5, color='k')
        axis.set_ylim(ylims)
        axis.set_xlabel("Input Rate / 2 (spks/s)")
        axis.set_ylabel("FIFO Overflow Count")
        axis.legend()
        axis.set_title("Synapse {:04d} Estimated Max Input Rate {:.1f}".format(
            syn_idx, max_rates[syn_idx]))
        fig.savefig(DETAIL_DATA_DIR + "syn_{:04d}.png".format(syn_idx))
        plt.cla()
    plt.close()

def check_synapse_max_rates(parsed_args):
    """Run the check"""
    use_saved_data = parsed_args.use_saved_data
    if use_saved_data:
        overflows = load_npy_data(DATA_DIR + "overflows.npy")
        rates = load_npy_data(DATA_DIR + "rates.npy")
    else:
        build_net()
        set_analog()
        set_hal()
        rates = [[] for _ in range(SYN_N)]
        overflows = [[] for _ in range(SYN_N)]
        start_time = get_time()
        for syn_idx in range(SYN_N):
            print("Testing synapse {}".format(syn_idx))
            rates[syn_idx], overflows[syn_idx] = test_rates(syn_idx)
            report_time_remaining(start_time, syn_idx)
        rates = np.array(rates)
        overflows = np.array(overflows)
        np.save(DATA_DIR + "overflows.npy", overflows)
        np.save(DATA_DIR + "rates.npy", rates)

    max_rates, overflow_slopes = fit_max_rates(rates, overflows)
    print("\nMax Input rates:")
    print(max_rates)
    # print("Min synapse spike consumption times:")
    # print(1./max_rates)

    plot_data(rates, overflows, max_rates, overflow_slopes)
    plt.show()

if __name__ == "__main__":
    check_synapse_max_rates(parse_args())
