"""Validate the synapse input max rates found by check_max_synapse_rates

Bin the max rates into spike generator rates
Pair-by-pair check that synapses indeed satisfy the max rates predicted
"""
import os
from time import sleep
import argparse
import numpy as np
import matplotlib.pyplot as plt

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph
from pystorm.PyDriver import bddriver as bd

from utils.exp import clear_overflows, compute_spike_gen_rates
from utils.file_io import load_txt_data

np.set_printoptions(precision=2)

CORE = 0
NRN_N = 4096
SYN_N = 1024

RUN_TIME = 2. # time to sample
INTER_RUN_TIME = 0.1 # time between samples
RUN_TIME_NS = int(RUN_TIME*1E9)
INTER_RUN_TIME_NS = int(INTER_RUN_TIME*1E9)

SPIKE_GEN_TIME_UNIT_NS = 10000 # time unit of fpga spike generator
MAX_SPIKE_GEN = 256 # depends on SPIKE_GEN_TIME_UNIT_NS

MIN_RATE = 5000 # minimum rate to test
MAX_RATE = 100000 # maximum rate to test
SPIKE_GEN_RATES = compute_spike_gen_rates(MIN_RATE, MAX_RATE, SPIKE_GEN_TIME_UNIT_NS)

FIFO_BUFFER_SIZE = 512
CAUTION_THRESHOLD = 2 * FIFO_BUFFER_SIZE

GROUP_SIZES = [4, 2] # sizes of groups to test

SYN_PD_PU = 1024 # analog bias setting

DATA_DIR = "./data/" + os.path.basename(__file__)[:-3] + "/"
if not os.path.isdir(DATA_DIR):
    os.makedirs(DATA_DIR, exist_ok=True)

MAX_RATE_MARGIN = 0.00 # margin for binning max rates

def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Characterize the synapse max input firing rates')
    parser.add_argument("-r", action="store_true", dest="use_saved_data", help='reuse cached data')
    parser.add_argument("--max_rates", type=str, dest="max_rates",
                        help='Name of file containing max synapse rates')
    args = parser.parse_args()
    return args

def syn_to_soma_addr(syn_n):
    """Convert synapse flat address to soma flat address"""
    soma_n = syn_n * 4
    sqrt_syn_n = int(np.round(np.sqrt(syn_n)))
    sqrt_soma_n = int(np.round(np.sqrt(soma_n)))
    soma_syn_addrs = np.zeros(syn_n, dtype=int)
    for syn_idx in range(syn_n):
        syn_x = syn_idx % sqrt_syn_n
        syn_y = syn_idx // sqrt_syn_n
        soma_x = syn_x * 2
        soma_y = syn_y * 2
        soma_syn_addrs[syn_idx] = soma_y*sqrt_soma_n + soma_x
    return soma_syn_addrs

def set_analog():
    """Sets the synapse config bits and the bias currents"""
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PD, SYN_PD_PU)
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PU, SYN_PD_PU)
    for n_idx in range(NRN_N):
        HAL.driver.SetSomaEnableStatus(CORE, n_idx, bd.bdpars.SomaStatusId.DISABLED)
    for s_idx in range(SYN_N):
        HAL.driver.SetSynapseEnableStatus(CORE, s_idx, bd.bdpars.SynapseStatusId.ENABLED)
    HAL.flush()

def build_net_groups(u_rates, binned_rates, syn_idxs, group_size):
    """Build a network for testing groups of synapses

    Parameters
    ----------
    u_rates: array
        lists the rate bin values
    binned_max_rates: array of numbers
        array of synapse max rates binned to the rates in u_rates
    syn_idxs: array of ints
        indices of synapses corresponeding to binned_max_rates entries
    group_size: int
        size of groups to bundle synapses into
    """
    dim_rate_idxs = {}
    encoder_dim = 0
    for rate in u_rates:
        idxs = syn_idxs[binned_rates == rate]
        groups = len(idxs) // group_size
        idxs = idxs[:groups*group_size] # clip off remainder
        for g_idx in range(groups):
            dim_rate_idxs[encoder_dim] = (rate, idxs[g_idx*group_size:(g_idx+1)*group_size])
            encoder_dim += 1

    tap_matrix_syn = np.zeros((SYN_N, encoder_dim))
    for dim in dim_rate_idxs:
        rate, idxs = dim_rate_idxs[dim]
        tap_matrix_syn[idxs, dim] = 1
    tap_matrix_soma = np.zeros((NRN_N, encoder_dim))
    soma_idxs = syn_to_soma_addr(SYN_N)
    tap_matrix_soma[soma_idxs] = tap_matrix_syn
    net = graph.Network("net")
    net_pool = net.create_pool("p", tap_matrix_soma)
    net_input = net.create_input("i", encoder_dim)
    net.create_connection("c: i to p", net_input, net_pool, None)
    HAL.map(net)
    set_analog()
    return net_input, dim_rate_idxs

def build_net_ref_syn(syn_idxs, ref_syn_idx):
    """Build a network for testing pairs of synapses

    Parameters
    ----------
    syn_idxs: array of ints
        indices of synapses to test
    ref_syn_idx: int
        index of synapse to use as reference
    """
    encoder_dim = len(syn_idxs)
    tap_matrix_syn = np.zeros((SYN_N, encoder_dim))
    tap_matrix_syn[ref_syn_idx, :] = 1
    for dim in range(encoder_dim):
        tap_matrix_syn[syn_idxs[dim], dim] = 1
    tap_matrix_soma = np.zeros((NRN_N, encoder_dim))
    soma_idxs = syn_to_soma_addr(SYN_N)
    tap_matrix_soma[soma_idxs] = tap_matrix_syn
    net = graph.Network("net")
    net_pool = net.create_pool("p", tap_matrix_soma)
    net_input = net.create_input("i", encoder_dim)
    net.create_connection("c: i to p", net_input, net_pool, None)
    HAL.map(net)
    set_analog()
    return net_input

def toggle_input(net_input, dim, rate):
    """Toggle the spike generator and check for overflow"""
    rate = int(rate)
    clear_overflows(HAL, INTER_RUN_TIME)
    HAL.start_traffic(flush=False)
    HAL.enable_output_recording(flush=True)
    cur_time = HAL.get_time()
    HAL.set_input_rate(net_input, dim, rate, time=cur_time+INTER_RUN_TIME_NS)
    HAL.set_input_rate(net_input, dim, 0, time=cur_time+RUN_TIME_NS+INTER_RUN_TIME_NS)
    HAL.flush()
    sleep(RUN_TIME+2*INTER_RUN_TIME)
    HAL.stop_traffic(flush=False)
    HAL.disable_output_recording(flush=True)
    sleep(INTER_RUN_TIME)
    overflow, _ = HAL.driver.GetFIFOOverflowCounts(CORE)
    print("Applied input rate {} overflow {}".format(rate, overflow))
    return overflow

def test_max_rate(net_input, dim, target_rate):
    """Test the dimension of network input at the given rate"""
    rate_idx = np.argmin(np.abs(SPIKE_GEN_RATES - target_rate))
    overflows = 1
    while overflows:
        rate = SPIKE_GEN_RATES[rate_idx]
        overflows = toggle_input(net_input, dim, rate)
        rate_idx -= 1
    return rate

def bin_max_rates(spike_gen_rates, max_rates, max_rate_margin=0.):
    """Bin the max rates into the spike gen rates"""
    max_rates = max_rates*(1-max_rate_margin)
    binned_max_rates = np.zeros_like(max_rates, dtype=int)
    for rate in spike_gen_rates:
        idxs = max_rates > rate
        binned_max_rates[idxs] = rate
    return binned_max_rates

def test_groups(u_rates, max_rates, binned_max_rates, test_idxs, group_size):
    """Bin synapses by spike gen rates and then test by groups within each bin

    Parameters
    ----------
    u_rates: array of floats
        unique rate values of binned_max_rates
    max_rates: array of floats
        synapse max input firing rates
    binned_max_rates: array of floats
    test_idxs: array of ints
        synapse indeices to test
    """
    print("Testing {} synapses by binning in group_size {}".format(
        len(max_rates[test_idxs]), group_size))
    sort_idxs = np.argsort(max_rates[test_idxs])
    sorted_test_idxs = test_idxs[sort_idxs]
    sorted_binned_max_rates = binned_max_rates[sorted_test_idxs]
    validated_max_rates = np.zeros_like(binned_max_rates)
    net_input, dim_rate_idxs = build_net_groups(
        u_rates, sorted_binned_max_rates, sorted_test_idxs, group_size)
    for dim, rate_idxs in dim_rate_idxs.items():
        rate, idxs = rate_idxs
        print("Testing dim {} of {} bin rate {} synapses {} with max rates {}".format(
            dim, net_input.dimensions-1, rate, idxs, max_rates[idxs]))
        validated_max_rates[idxs] = test_max_rate(net_input, dim, rate)
    return validated_max_rates

def test_with_ref(max_rates, binned_max_rates, test_idxs, ref_syn_idx):
    """Test synapses against a reference synapse with validated, binned max rate

    Parameters
    ----------
    ref_idx: int
        index of reference synapse in neuron array
    max_rates: array of floats
        synapse max input firing rates
    binned_max_rates: array of floats
        max_rates binned to the spike generator rate within each synapes' capacity
    ref_syn_idx: int
        index of synapse to use as reference
    """
    validated_max_rates = np.zeros_like(binned_max_rates)
    net_input = build_net_ref_syn(test_idxs, ref_syn_idx)
    for dim in range(net_input.dimensions):
        idx = test_idxs[dim]
        rate = binned_max_rates[idx]
        print("Reference testing dim {} of {} ".format(dim, net_input.dimensions-1) +
              "with rate {} for synapse {} max rate {:.0f} ".format(rate, idx, max_rates[idx]) +
              "against synapse {} with max rate {:.0f}".format(ref_syn_idx, max_rates[ref_syn_idx]))
        validated_max_rates[idx] = test_max_rate(net_input, dim, rate)
    return validated_max_rates

def run_tests(max_rates):
    """Check the max rates"""
    binned_max_rates = bin_max_rates(SPIKE_GEN_RATES, max_rates, MAX_RATE_MARGIN)
    validated_max_rates = np.zeros_like(binned_max_rates, dtype=int)
    u_rates = np.unique(binned_max_rates)
    # Test groups of synapses according to their binned rate
    for group_size in GROUP_SIZES:
        print("\nRunning tests with group_size {}".format(group_size))
        test_idxs = np.nonzero(binned_max_rates != validated_max_rates)[0]
        test_group_results = test_groups(
            u_rates, max_rates, binned_max_rates, test_idxs, group_size=group_size)
        validated_max_rates[test_idxs] = test_group_results[test_idxs]
        skipped_syns = np.nonzero(validated_max_rates == 0)[0]
        mismatched_syns = np.setdiff1d(
            np.nonzero(validated_max_rates != binned_max_rates)[0],
            skipped_syns)
        print("\nTests with group_size {} ".format(group_size) +
              "detected {} mismatched rates ".format(len(mismatched_syns)) +
              "and skipped {} synapses".format(len(skipped_syns)))

    # Assume that there is at least 1 validated synapse in the fastest max rate bin
    # Find bins where max_rates_binned != max_rates_validated and do test with good synapse
    valid_idxs = validated_max_rates == binned_max_rates
    max_idx = np.argmax(max_rates[valid_idxs])
    ref_idx = np.arange(SYN_N)[valid_idxs][max_idx]
    test_idxs = np.nonzero(binned_max_rates != validated_max_rates)[0]
    print("\nRunning tests on {} synapse(s) ".format(len(test_idxs)) +
          "with synapse {} with max rate {:.2f} as reference".format(ref_idx, max_rates[ref_idx]))
    subsets = len(test_idxs) // MAX_SPIKE_GEN + int(len(test_idxs) % MAX_SPIKE_GEN != 0)
    for subset_idx in range(subsets):
        sub_test_idxs = test_idxs[subset_idx*MAX_SPIKE_GEN:(subset_idx+1)*MAX_SPIKE_GEN]
        test_ref_results = test_with_ref(max_rates, binned_max_rates, sub_test_idxs, ref_idx)
        validated_max_rates[sub_test_idxs] = test_ref_results[sub_test_idxs]
    return binned_max_rates, validated_max_rates

def plot_data(max_rates, binned_max_rates, validated_max_rates):
    """Plot the results"""
    u_rates = np.unique(binned_max_rates)
    high_gen_idx = np.nonzero(SPIKE_GEN_RATES > u_rates[-1])[0][0]
    low_gen_idx = np.nonzero(SPIKE_GEN_RATES < u_rates[0])[0][-1]
    gen_rates_to_plot = SPIKE_GEN_RATES[low_gen_idx:high_gen_idx+1]

    mismatched_idxs = np.nonzero(binned_max_rates != validated_max_rates)[0]

    fig, axs = plt.subplots(figsize=(12, 6))
    for rate in gen_rates_to_plot:
        axs.axhline(rate, color='k', alpha=0.5, linewidth=1)
    for idx in mismatched_idxs:
        axs.plot([idx, idx], [max_rates[idx], validated_max_rates[idx]], 'r', linewidth=1)
    axs.plot(max_rates, 'o', markersize=1.5, label="max rates")
    axs.plot(binned_max_rates, 'o', markersize=1.5, label="binned max rates")
    axs.plot(validated_max_rates, 'o', markersize=1.5, label="validated max rates")
    axs.set_xlabel("Synapse Index")
    axs.set_ylabel("Rate (spks/s)")
    axs.legend()
    fig.savefig(DATA_DIR + "max_rates_flat_idx.pdf")

    fig, axs = plt.subplots(figsize=(12, 6))
    for rate in gen_rates_to_plot[:-1]:
        axs.axhline(rate, color='k', alpha=0.5, linewidth=1)
    for rate in gen_rates_to_plot:
        axs.axvline(rate, color='k', alpha=0.5, linewidth=1)
    for idx in mismatched_idxs:
        axs.plot([max_rates[idx], max_rates[idx]],
                    [binned_max_rates[idx], validated_max_rates[idx]],
                    'r', linewidth=1)
    axs.plot(max_rates, binned_max_rates, 'o', markersize=1.5, label='predicted')
    axs.plot(max_rates, validated_max_rates, 'o', markersize=1.5, label='validated')
    axs.set_xlabel("Estimated Max Rate (spks/s)")
    axs.set_ylabel("Binned Max Rate (spks/s)")
    axs.legend()
    fig.savefig(DATA_DIR + "predicted_vs_validated_rates.pdf")

    fig, axs = plt.subplots(nrows=2, sharex=True, figsize=(8, 10))
    axs[0].hist(max_rates, bins=80)
    axs[1].hist(binned_max_rates, bins=80)
    for rate in gen_rates_to_plot:
        axs[0].axvline(rate, color='k', alpha=0.5, linewidth=1)
        axs[1].axvline(rate, color='k', alpha=0.5, linewidth=1)
    axs[0].set_title("Max Rates")
    axs[1].set_title("Binned Max Rates")
    axs[1].set_xlabel("Spikes / Second")
    axs[0].set_ylabel("Synapse Count")
    axs[1].set_ylabel("Synapse Count")
    fig.savefig(DATA_DIR + "max_rates_histogram.pdf")

    def plot_heatmap(data, title, fname):
        """heatmap for 2d data"""
        sqrt_n = int(np.ceil(np.sqrt(SYN_N)))
        data_2d = data.reshape((sqrt_n, -1))
        fig_2d_heatmap, axs = plt.subplots()
        ims = axs.imshow(data_2d)
        plt.colorbar(ims)
        axs.set_xlabel("Synapse X Coordinate")
        axs.set_ylabel("Synapse Y Coordinate")
        axs.set_title(title)
        fig_2d_heatmap.savefig(DATA_DIR + fname)
    plot_heatmap(max_rates, "Max Input Rate (spks/s)", "max_rates_heatmap.pdf")
    plot_heatmap(binned_max_rates, "Binned Max Input Rate (spks/s)",
                 "max_rates_binned_heatmap.pdf")

def validate_max_synapse_rates(parsed_args):
    """Run the experiment"""
    use_saved_data = parsed_args.use_saved_data
    max_rates_fname = parsed_args.max_rates

    if not max_rates_fname:
        max_rates_fname = DATA_DIR + "max_rates.txt"
    max_rates = load_txt_data(max_rates_fname)

    if use_saved_data:
        binned_max_rates = load_txt_data(DATA_DIR + "binned_max_rates.txt")
        validated_max_rates = load_txt_data(DATA_DIR + "validated_max_rates.txt")
    else:
        binned_max_rates, validated_max_rates = run_tests(max_rates)
        np.savetxt(DATA_DIR + "binned_max_rates.txt", binned_max_rates)
        np.savetxt(DATA_DIR + "validated_max_rates.txt", validated_max_rates)

    plot_data(max_rates, binned_max_rates, validated_max_rates)
    plt.show()

if __name__ == "__main__":
    validate_max_synapse_rates(parse_args())
