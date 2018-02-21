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
import matplotlib

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph
from pystorm.PyDriver import bddriver as bd

from utils.exp import clear_overflows, compute_spike_gen_rates
from utils.file_io import load_npy_data, load_txt_data

np.set_printoptions(precision=2)

CORE = 0
NRN_N = 4096
# SYN_N = 1
# SYN_N = 4
# SYN_N = 16
# SYN_N = 64
SYN_N = 1024

RUN_TIME = 1.0
INTER_RUN_TIME = 0.2

SPIKE_GEN_TIME_UNIT_NS = 5000 # time unit of fpga spike generator
SPIKE_GEN_IDX = 0 # FPGA spike generator index
# Tag Action Table settings
TAT_IDX = 0
TAT_START_ADDR = 0
TAT_STOP_BIT = 1
TAT_SIGN_0 = 0
TAT_SIGN_1 = 1

FIFO_BUFFER_SIZE = 512

SYN_PD_PU = 1024 # analog bias setting

MIN_RATE = 5000 # minimum rate to test
MAX_RATE = 100000 # maximum rate to test

SPIKE_GEN_RATES = compute_spike_gen_rates(MIN_RATE, MAX_RATE, SPIKE_GEN_TIME_UNIT_NS)

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
    parser.add_argument("-nf", action="store_true", dest="no_fit_plots",
                        help="Skip plotting of fits. " +
                        "Plotting and saving the fits of many synapses can take a long time")
    parser.add_argument("--ammend", type=str, dest="ammend_file",
                        help="Redo measurements on the synapse indices in the ammend file")
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

def toggle_spk_generator(rate, run_sleep_time=RUN_TIME, inter_run_sleep_time=INTER_RUN_TIME):
    """Toggle the spike generator and check for overflow"""
    clear_overflows(HAL, inter_run_sleep_time)
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

    Returns the max rates as estimated by the slope fit
    as well as a conservative estimate from the highest
    zero-overflow condition
    """
    max_rates = np.zeros(SYN_N)
    overflow_slopes = np.zeros(SYN_N)
    max_rates_conservative = np.zeros(SYN_N)
    for syn_idx in range(SYN_N):
        syn_overflows = np.array(overflows[syn_idx])
        syn_rates = np.array(rates[syn_idx])
        nonzero_idx = syn_overflows > 0
        syn_overflows = syn_overflows[nonzero_idx]
        syn_rates_nonzero = syn_rates[nonzero_idx]
        dodrs = np.diff(syn_overflows) / np.diff(syn_rates_nonzero)
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
        print("overflow rates for fitting: {}".format(syn_rates_nonzero[o_idxs]))
        print("overflows for fitting:      {}".format(syn_overflows[o_idxs]))
        y = syn_overflows[o_idxs]
        A = np.array([syn_rates_nonzero[o_idxs], np.ones(len(o_idxs))]).T
        x = np.dot(np.dot(np.linalg.inv(np.dot(A.T, A)), A.T), y) # x = (A_T*A)^-1*A_T*y
        overflow_slopes[syn_idx] = x[0]
        max_rates[syn_idx] = -x[1]/x[0]

        # build a conservative estimate of the max rates
        zero_idx = ~nonzero_idx
        max_zero_overflow_rate = syn_rates[zero_idx][-1]
        idx = SPIKE_GEN_RATES <= max_zero_overflow_rate - FIFO_BUFFER_SIZE / RUN_TIME
        max_rates_conservative[syn_idx] = SPIKE_GEN_RATES[idx][-1]

    return max_rates, overflow_slopes, max_rates_conservative

def report_time_remaining(start_time, syn_idx):
    """Occasionally estimate and report the remaining time"""
    if syn_idx%4 == 0 and SYN_N > 1:
        n_syn_completed = syn_idx+1
        delta_time = get_time()-start_time
        est_time_remaining = delta_time/(syn_idx+1) * (SYN_N-n_syn_completed)
        print("estimated time remaining: {:.0f} s = {:.1f} min = {:.2f} hr...".format(
            est_time_remaining, est_time_remaining/60., est_time_remaining/60./60.))

def plot_data(
        rates, overflows, max_rates_2, overflow_slopes,
        max_rates_conservative_2, syn_idxs, no_fit_plots=False):
    """Plot the data"""
    syn_n = len(max_rates_2)
    max_rates_2_mean = np.mean(max_rates_2)
    max_rates_2_median = np.median(max_rates_2)
    max_rates_2_min = np.min(max_rates_2)
    max_rates_2_max = np.max(max_rates_2)

    low_idx = np.nonzero(SPIKE_GEN_RATES < max_rates_2_min)[0][-1]
    high_idx = np.nonzero(SPIKE_GEN_RATES > max_rates_2_max)[0][0]
    gen_rates_half = SPIKE_GEN_RATES[low_idx:high_idx+1]
    low_idx = np.nonzero(SPIKE_GEN_RATES < max_rates_2_min*2)[0][-1]
    high_idx = np.nonzero(SPIKE_GEN_RATES > max_rates_2_max*2)[0][0]
    gen_rates_full = SPIKE_GEN_RATES[low_idx:high_idx+1]

    fig_1d, axs = plt.subplots(ncols=2, figsize=(14, 6))
    axs[0].plot(max_rates_2, 'o', markersize=1)
    axs[0].set_xlabel("Synapse Index")
    axs[0].set_ylabel("Max Input Rate / 2 (spks/s)")
    for gen_rate in gen_rates_half:
        axs[0].axhline(gen_rate, color=(0.8, 0.8, 0.8), linewidth=1)
    axs[1].plot(max_rates_2*2, 'o', markersize=1)
    axs[1].set_xlabel("Synapse Index")
    axs[1].set_ylabel("Max Input Rate (spks/s)")
    for gen_rate in gen_rates_full:
        axs[1].axhline(gen_rate, color=(0.8, 0.8, 0.8), linewidth=1)
    fig_1d.savefig(DATA_DIR + "syn_idx_vs_max_rate.pdf")

    if syn_n > 1: # make histograms
        fig_hist, axs = plt.subplots(ncols=2, figsize=(16, 6))
        for gen_rate in gen_rates_half:
            axs[0].axvline(gen_rate, color=(0.8, 0.8, 0.8), linewidth=1)
        axs[0].hist(max_rates_2, bins=80)
        axs[0].axvline(max_rates_2_mean, color="k", alpha=0.6, linewidth=1, label="mean")
        axs[0].axvline(max_rates_2_median, color="r", alpha=0.4, linewidth=1, label="median")
        axs[0].set_xlabel("Max Input Rate / 2 (spks/s)")
        axs[0].set_ylabel("Counts")
        axs[0].set_title(
            "Half Rate Histogram\n"+
            "Mean:{:,.0f} Median:{:,.0f} Min:{:,.0f} Max:{:,.0f}".format(
                max_rates_2_mean, max_rates_2_median, max_rates_2_min, max_rates_2_max))
        axs[0].legend()

        for gen_rate in gen_rates_full:
            axs[1].axvline(gen_rate, color=(0.8, 0.8, 0.8), linewidth=1)
        axs[1].hist(max_rates_2*2, bins=80)
        axs[1].axvline(max_rates_2_mean*2, color="k", alpha=0.6, linewidth=1, label="mean")
        axs[1].axvline(max_rates_2_median*2, color="r", alpha=0.4, linewidth=1, label="median")
        axs[1].set_xlabel("Max Input Rate (spks/s)")
        axs[1].set_ylabel("Counts")
        axs[1].set_title(
            "Full Rate Histogram\n"+
            "Mean:{:,.0f} Median:{:,.0f} Min:{:,.0f} Max:{:,.0f}".format(
                max_rates_2_mean*2, max_rates_2_median*2, max_rates_2_min*2, max_rates_2_max*2))
        axs[1].legend()

        fig_hist.suptitle("All Synapse Histogram")
        fig_hist.savefig(DATA_DIR + "histogram.pdf")

    if syn_n == NRN_N//4: # all syn_n tested
        sqrt_n = int(np.ceil(np.sqrt(syn_n)))
        max_rates_2_2d = max_rates_2.reshape((sqrt_n, -1))
        fig_heatmap, axs = plt.subplots()
        ims = axs.imshow(max_rates_2_2d*2)
        plt.colorbar(ims)
        axs.set_xlabel("Synapse X Coordinate")
        axs.set_ylabel("Synapse Y Coordinate")
        axs.set_title("Max Input Rate (spks/s)")
        fig_heatmap.savefig(DATA_DIR + "2d_heatmap.pdf")

        max_rates_2_hex = np.nan*np.ones((sqrt_n*2, sqrt_n*2))
        max_rates_2_hex[0::4, 0::2] = max_rates_2_2d[0::2, :]
        max_rates_2_hex[2::4, 1::2] = max_rates_2_2d[1::2, :]
        fig_hex_heatmap, axs = plt.subplots()
        matplotlib.cm.get_cmap().set_bad(color='w')
        ims = axs.imshow(max_rates_2_hex*2)
        axs.set_xticks([])
        axs.set_yticks([])
        plt.colorbar(ims)
        axs.set_title("Max Input Rate (spks/s)")
        fig_hex_heatmap.savefig(DATA_DIR + "2d_hex_heatmap.pdf")


        tile_max_rates = dict(
            upper_left=max_rates_2_2d[0::2, 0::2].flatten()*2,
            upper_right=max_rates_2_2d[0::2, 1::2].flatten()*2,
            lower_left=max_rates_2_2d[1::2, 0::2].flatten()*2,
            lower_right=max_rates_2_2d[1::2, 1::2].flatten()*2,)
        fig_tile, axs = plt.subplots(ncols=3, figsize=(20, 6))
        for pos in tile_max_rates:
            axs[0].hist(tile_max_rates[pos], bins=40, alpha=0.4, label=pos)
        axs[0].set_xlabel("Max Input Rate (spks/s)")
        axs[0].set_ylabel("Counts")
        axs[0].set_title("Histograms")
        axs[0].legend()
        for pos in tile_max_rates:
            pdf, bin_edges = np.histogram(tile_max_rates[pos], density=True)
            bin_widths = np.diff(bin_edges)
            bin_centers = bin_edges[:-1] + bin_widths/2
            cdf = np.cumsum(pdf*bin_widths)
            axs[1].plot(bin_centers, cdf, alpha=0.4, label=pos)
        axs[1].set_xlabel("Max Input Rate (spks/s)")
        axs[1].set_ylabel("Probability")
        axs[1].set_title("Cumulative Distribution Functions")
        axs[1].legend()
        qq_quantiles = {}
        quantiles = np.linspace(0, 1, 80)
        for pos in tile_max_rates:
            qq_pdf, bin_edges = np.histogram(
                tile_max_rates[pos], range=(max_rates_2_min*2, max_rates_2_max*2), bins=80,
                density=True)
            bin_widths = np.diff(bin_edges)
            bin_centers = bin_edges[:-1] + bin_widths/2
            qq_cdf = np.cumsum(qq_pdf*bin_widths)
            qq_quantiles[pos] = np.interp(quantiles, qq_cdf, bin_centers)
        positions = list(qq_quantiles.keys())
        for idx0, pos0 in enumerate(positions):
            for pos1 in positions[idx0+1:]:
                axs[2].plot(qq_quantiles[pos0], qq_quantiles[pos1],
                            linewidth=1, label="{} : {}".format(pos0, pos1))
        xlim = axs[2].get_xlim()
        ylim = axs[2].get_ylim()
        min_val = np.min(xlim + ylim)
        max_val = np.max(xlim + ylim)
        axs[2].plot([min_val, max_val], [min_val, max_val], 'k', linewidth=1)
        axs[2].set_xlim(xlim)
        axs[2].set_ylim(ylim)
        axs[2].set_xlabel("Position 0's Max Input Rate (spks/s)")
        axs[2].set_ylabel("Position 1's Max Input Rate (spks/s)")
        axs[2].set_title("Quantile-Quantiles")
        axs[2].legend(title="Position 0 : Position 1")
        fig_tile.suptitle("Dividing Synapses by Position in Tile")
        fig_tile.savefig(DATA_DIR + "syn_tile.pdf")

    if not no_fit_plots:
        fig, axis = plt.subplots(nrows=1, ncols=1)
        for syn_idx in syn_idxs:
            axis.axhline(0, linewidth=0.5, color='k')
            axis.plot(rates[syn_idx], overflows[syn_idx], 'o', label="measured")
            ylims = axis.get_ylim()
            axis.plot(
                rates[syn_idx], overflow_slopes[syn_idx]*(rates[syn_idx]-max_rates_2[syn_idx]),
                "r", linewidth=0.7, label="fit")
            axis.axvline(max_rates_2[syn_idx], linewidth=0.5, color='k')
            axis.axvline(max_rates_conservative_2[syn_idx], linestyle=':', linewidth=0.5,
                         color='k')
            axis.set_ylim(ylims)
            axis.set_xlabel("Input Rate / 2 (spks/s)")
            axis.set_ylabel("FIFO Overflow Count + FIFO Depth")
            axis.legend()
            axis.set_title("Synapse {:04d}\nEstimated Max Input Rate = {:.1f} = {:.1f}x2".format(
                syn_idx, max_rates_2[syn_idx]*2, max_rates_2[syn_idx]))
            fig.savefig(DETAIL_DATA_DIR + "syn_{:04d}.png".format(syn_idx))
            plt.cla()
        plt.close()

def check_synapse_max_rates(parsed_args):
    """Run the check"""
    use_saved_data = parsed_args.use_saved_data
    no_fit_plots = parsed_args.no_fit_plots
    ammend_file = parsed_args.ammend_file
    if use_saved_data:
        overflows = load_npy_data(DATA_DIR + "overflows.npy")
        rates = load_npy_data(DATA_DIR + "rates.npy")
        syn_idxs = range(SYN_N)
    else:
        if ammend_file:
            syn_idxs = load_txt_data(ammend_file, dtype=int)
            ref_overflows = load_npy_data(DATA_DIR + "overflows.npy")
            ref_rates = load_npy_data(DATA_DIR + "rates.npy")
        else:
            syn_idxs = range(SYN_N)
        build_net()
        set_analog()
        set_hal()
        rates = [[] for _ in range(SYN_N)]
        overflows = [[] for _ in range(SYN_N)]
        start_time = get_time()
        for syn_idx in syn_idxs:
            print("Testing synapse {}".format(syn_idx))
            rates[syn_idx], overflows[syn_idx] = test_rates(syn_idx)
            report_time_remaining(start_time, syn_idx)
        if ammend_file:
            for syn_idx in syn_idxs:
                ref_rates[syn_idx] = rates[syn_idx]
                ref_overflows[syn_idx] = overflows[syn_idx]
            rates = ref_rates
            overflows = ref_overflows
        else:
            rates = np.array(rates)
            overflows = np.array(overflows)
        np.save(DATA_DIR + "overflows.npy", overflows)
        np.save(DATA_DIR + "rates.npy", rates)

    max_rates_2, overflow_slopes, max_rates_conservative_2 = fit_max_rates(rates, overflows)
    print("\nMax input rates / 2:")
    print(max_rates_2)
    print("Min synapse spike consumption times x 2:")
    print(1./max_rates_2)
    np.savetxt(DATA_DIR + "max_rates.txt", max_rates_2*2)
    np.savetxt(DATA_DIR + "max_rates_conservative.txt", max_rates_conservative_2*2)

    plot_data(
        rates, overflows, max_rates_2, overflow_slopes,
        max_rates_conservative_2, syn_idxs, no_fit_plots=no_fit_plots)
    plt.show()

if __name__ == "__main__":
    check_synapse_max_rates(parse_args())
