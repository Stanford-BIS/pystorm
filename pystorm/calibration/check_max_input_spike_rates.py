"""Test the receiver's max throughput by sending spikes to synapes

Synapses can only take in spike at up to a certain rate
Test the max rates first with calibration and then use the max rate info to drive
the receiver with as much traffic as possible
"""
import os
from time import sleep
import argparse
import numpy as np
import matplotlib.pyplot as plt

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph
from pystorm.PyDriver import bddriver as bd

from utils.file_io import load_pickle_data, save_pickle_data
from utils.exp import clear_overflows, compute_spike_gen_rates

CORE = 0
NRN_N = 4096
SYN_N = 1024

SPIKE_GEN_TIME_UNIT_NS = 10000 # time unit of fpga spike generator
MIN_RATE = 1000
MAX_RATE = 100000

DATA_DIR = "./data/" + os.path.basename(__file__)[:-3] + "/"
if not os.path.isdir(DATA_DIR):
    os.makedirs(DATA_DIR, exist_ok=True)

SYN_MAX_RATE_FILE = DATA_DIR + "max_rates.txt"
SYN_MAX_RATE_FRAC = 1.00

SYN_PD_PU = 1024 # analog bias setting

RUN_TIME = 1.0
INTER_RUN_TIME = 0.2

def compute_init_rate_idx(gen_rates, init_rate):
    """Compute the initial spike generator rate to test"""
    return np.argmin(np.abs(gen_rates - init_rate))

def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(
        description='Characterize the max input spike rate to the synapses')
    parser.add_argument("-r", action="store_true", dest="use_saved_data", help='reuse cached data')
    args = parser.parse_args()
    return args

def plot_input_rates(gen_rates, recv_data):
    """Plot the possible input rates and receiver traffic"""
    n_syns_capable = np.zeros(len(gen_rates), dtype=int)
    clipped_total_rates = np.zeros(len(gen_rates))
    for rate_idx, clip_rate in enumerate(gen_rates):
        for max_rate in recv_data.rate_group:
            if max_rate >= clip_rate:
                n_syns_capable[rate_idx] += recv_data.rate_group[max_rate].syn_n
                clipped_total_rates[rate_idx] += recv_data.rate_group[max_rate].syn_n * clip_rate

    fig, ax_rates = plt.subplots()
    ax_syn = ax_rates.twinx()
    ax_syn.plot(gen_rates, n_syns_capable, '-ob')
    for rate in gen_rates:
        ax_rates.axvline(rate, color='k', alpha=0.2, linewidth=1)
    ax_rates.axhline(recv_data.min_max_total_rate * 1E-6, color='k', alpha=0.4, linewidth=1)
    ax_rates.axhline(recv_data.clipped_max_total_rate * 1E-6, color='k', alpha=0.4, linewidth=1)
    ax_rates.plot(gen_rates, clipped_total_rates * 1E-6, '-ok')
    ax_rates.set_xlabel("Input Spike Gen Rate (spks/s)")
    ax_rates.set_ylabel("Total Receiver Input Rate (Mspks/s)", color='k')
    ax_syn.set_ylabel("Number of Synapses", color='b')
    ax_rates.tick_params('y', colors='k')
    ax_syn.tick_params('y', colors='b')
    ax_rates.set_title(
        "Max Receiver Test Rate {:.1f} Mspks/s ".format(recv_data.min_max_total_rate*1E-6) +
        "(including all synapses)\n" +
        "Max Receiver Test Rate {:.1f} Mspks/s\n".format(recv_data.clipped_max_total_rate*1E-6) +
        "(excluding synapses unable to handle a given rate)"
        )
    fig.savefig(DATA_DIR + "receiver_rates_same_input.pdf")

    fig, ax_syns = plt.subplots(ncols=2, figsize=(14, 5))
    for rate in recv_data.rate_group:
        ax_syns[0].axhline(rate, color='k', alpha=0.3, linewidth=1)
        ax_syns[1].axvline(rate, color='k', alpha=0.3, linewidth=1)
    ax_syns[0].plot(recv_data.syn_max_gen_rate, 'o')
    ax_syns[1].hist(recv_data.syn_max_gen_rate, bins=80)
    ax_syns[0].set_xlabel("Synapse Index")
    ax_syns[0].set_ylabel("Max Spike Gen Rate (spks/s)")
    ax_syns[1].set_xlabel("Max Spike Gen Rate (spks/s)")
    ax_syns[1].set_ylabel("Synapse Counts")
    fig.suptitle("Max Receiver Test Rate with Custom Spike Gen Rates: {:.1f} Mspks/s".format(
        recv_data.max_max_total_rate*1E-6))
    fig.savefig(DATA_DIR + "receiver_rates_custom_inputs.pdf")

class RecvData(object):
    """Group synapse data"""
    def __init__(self, rate_group):
        self.rate_group = rate_group

        # number of synapses
        self.syn_n = np.sum(list(map(lambda rate: rate_group[rate].syn_n, rate_group)))

        # possible maximum rates
        min_max_rate = np.min(list(rate_group))
        self.min_max_total_rate = self.syn_n * min_max_rate
        self.max_max_total_rate = np.sum(
            list(map(lambda rate: rate_group[rate].syn_n*rate_group[rate].gen_max_rate,
                     rate_group)))
        clipped_max_total_rates = np.zeros(len(rate_group))
        for idx, min_max_rate in enumerate(sorted(rate_group)):
            for rate in rate_group:
                if rate >= min_max_rate:
                    clipped_max_total_rates[idx] += rate_group[rate].syn_n * min_max_rate
        clipped_max_idx = np.argmax(clipped_max_total_rates)
        self.clipped_max_total_rate = clipped_max_total_rates[clipped_max_idx]
        self.clipped_max_rate = list(sorted(rate_group))[clipped_max_idx]

        self.syn_max_gen_rate = np.zeros(self.syn_n)
        for rate in rate_group:
            self.syn_max_gen_rate[rate_group[rate].idxs] = rate

        self.clipped_syn_n = np.sum(self.syn_max_gen_rate >= self.clipped_max_rate)

class SynGroup(object):
    """Group of synapses sharing the same max spike generator rate"""
    def __init__(self, gen_rates, gen_max_rate, syn_idxs):
        self.syn_n = len(syn_idxs)
        self.idxs = syn_idxs
        self.gen_max_rate = gen_max_rate
        self.gen_max_rate_idx = np.nonzero(gen_rates == gen_max_rate)[0][0]
        self.total_max_rate = self.syn_n * self.gen_max_rate

def make_tat_compatible(syn_spike_gen_rates):
    """Check for compatibility between the synapse spike rates and the Tag Action Table

    The TAT structure requires an even number of synapses for each dimension
    """
    rates = np.unique(syn_spike_gen_rates)
    rate_syn = {}
    for rate in rates:
        rate_syn[rate] = list(np.nonzero(syn_spike_gen_rates == rate)[0])
    # print("Before even-number restriction applied:")
    # print("    rate: synapses")
    # for rate in rates:
    #     print("{:8.1f}: {:d}".format(rate, len(rate_syn[rate])))

    move_syn = None
    for rate in sorted(rates)[::-1]:
        if len(rate_syn[rate])%2 != 0:
            if move_syn:
                rate_syn[rate].append(move_syn)
                move_syn = None
            else:
                move_syn = rate_syn[rate][-1]
                rate_syn[rate] = rate_syn[rate][:-1]
    # print("After even-number restriction applied:")
    # print("    rate: synapses")
    # for rate in rates:
    #     print("{:8.1f}: {:d}".format(rate, len(rate_syn[rate])))
    return rate_syn

def compute_receiver_rates(gen_rates, syn_max_rate_file):
    """Compute the overall input rates given the synapse max input rates

    capable of receiving the
    Parameters
    ----------
    gen_rates: array of floats
        possible rates output by the spike generators
    syn_max_rate_file: string filename
        data file containing the maximum input rate of the synapses

    Returns
    -------
    rate_group: dictionary of [rate] : SynGroup object
        mapping between spike generator rates and synapse groups
    """
    max_syn_rates = np.loadtxt(syn_max_rate_file)
    syn_spike_gen_rates = np.zeros(max_syn_rates.shape)
    for rate in gen_rates:
        syn_idxs = max_syn_rates*SYN_MAX_RATE_FRAC > rate
        syn_spike_gen_rates[syn_idxs] = rate
    rate_syn = make_tat_compatible(syn_spike_gen_rates)
    rate_group = {}
    for rate in rate_syn:
        rate_group[rate] = SynGroup(gen_rates, rate, rate_syn[rate])
    recv_data = RecvData(rate_group)
    plot_input_rates(gen_rates, recv_data)
    return recv_data

def set_analog():
    """Sets the synapse config bits and the bias currents"""
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PD, SYN_PD_PU)
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PU, SYN_PD_PU)
    for n_idx in range(NRN_N):
        HAL.driver.SetSomaEnableStatus(CORE, n_idx, bd.bdpars.SomaStatusId.DISABLED)
    for s_idx in range(SYN_N):
        HAL.driver.SetSynapseEnableStatus(CORE, s_idx, bd.bdpars.SynapseStatusId.ENABLED)
    HAL.flush()

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

def toggle_input_rates(net_input, rates):
    """Toggle the spike generator and check for overflow"""
    clear_overflows(HAL, INTER_RUN_TIME)
    HAL.start_traffic(flush=False)
    HAL.enable_output_recording(flush=True)
    net_inputs = [net_input for _ in rates]
    dims = [dim for dim in range(len(rates))]
    HAL.set_input_rates(net_inputs, dims, rates, time=0, flush=True)
    sleep(RUN_TIME)
    HAL.set_input_rates(net_inputs, dims, [0 for _ in rates], time=0, flush=True)
    HAL.flush()
    sleep(INTER_RUN_TIME)
    HAL.stop_traffic(flush=False)
    HAL.disable_output_recording(flush=True)
    overflow, _ = HAL.driver.GetFIFOOverflowCounts(CORE)
    print("Applied input rates {} overflow {}".format(rates, overflow))
    return overflow

def find_max_rate(net_input, gen_rates, init_rate, update_idxs=None, rate_limits=None):
    """Find where FIFO starts overflowing

    Parameters
    ----------
    net_input: hal neuormorph Input object
    gen_rates: 1D array of possible spike generator rates
    init_rate: desired initial rate to test
        actual initial rate with be closest rate in gen_rates to init_rate
    update_idxs: array indices indicating which dimension(s) to update
    rate_limits: None, number, or net_input.dimensions long array of numbers
         If supplied, limits the rates supplied to each dimension
    """
    if isinstance(update_idxs, int):
        update_idxs = np.array([update_idxs])
    elif not update_idxs:
        update_idxs = np.arange(net_input.dimensions)
    assert np.max(update_idxs) < net_input.dimensions
    if isinstance(rate_limits, (int, float)):
        rate_limits = np.array([rate_limits for _ in range(net_input.dimensions)])
    elif rate_limits is None:
        rate_limits = np.array([np.max(gen_rates) for _ in range(net_input.dimensions)])
    assert len(rate_limits) == net_input.dimensions

    rates = np.zeros(net_input.dimensions, dtype=int)

    base_rates = []
    test_rates = []
    overflows = []

    idx = compute_init_rate_idx(gen_rates, init_rate)
    base_rates.append(gen_rates[idx])
    rates[update_idxs] = np.clip(base_rates[-1], None, rate_limits[update_idxs])
    test_rates.append(rates.copy())
    overflows.append(toggle_input_rates(net_input, rates))

    if overflows[0] > 0:
        base_rates.append(gen_rates[idx+1])
        rates[update_idxs] = np.clip(base_rates[-1], None, rate_limits[update_idxs])
        test_rates.append(rates.copy())
        overflows.append(toggle_input_rates(net_input, rates))
        while overflows[-1] > 0 or len(overflows) < 5:
            idx -= 1
            if idx >= 0:
                base_rates.append(gen_rates[idx])
                rates[update_idxs] = np.clip(base_rates[-1], None, rate_limits[update_idxs])
                test_rates.append(rates.copy())
                overflows.append(toggle_input_rates(net_input, rates))
            else:
                break
    elif overflows[0] == 0:
        base_rates.append(gen_rates[idx-1])
        rates[update_idxs] = np.clip(base_rates[-1], None, rate_limits[update_idxs])
        test_rates.append(rates.copy())
        overflows.append(toggle_input_rates(net_input, rates))
        while overflows[-1] == 0 or len(overflows) < 5:
            idx += 1
            if idx < len(gen_rates):
                base_rates.append(gen_rates[idx])
                rates[update_idxs] = np.clip(base_rates[-1], None, rate_limits[update_idxs])
                test_rates.append(rates.copy())
                overflows.append(toggle_input_rates(net_input, rates))
            else:
                break

    base_rates = np.array(base_rates)
    test_rates = np.array(test_rates)
    overflows = np.array(overflows)

    sort_idx = np.argsort(base_rates)
    base_rates = base_rates[sort_idx]
    for dim in range(net_input.dimensions):
        test_rates[:, dim] = test_rates[:, dim][sort_idx]
    overflows = overflows[sort_idx]
    return base_rates, test_rates, overflows

def build_net_1d(clip_rate, rate_group):
    """Build a network with 1 spike generator for testing

    Parameters
    ----------
    clip_rate: float
        only target synapses capable of receiving at least clip_rate input
    rate_group: dict {float rate: SynGroup syn_group}
    """
    encoder_dim = 1
    tap_matrix_syn = np.zeros((SYN_N, encoder_dim))
    for rate in rate_group:
        if rate >= clip_rate:
            tap_matrix_syn[rate_group[rate].idxs] = 1
    tap_matrix_soma = np.zeros((NRN_N, encoder_dim)) # tap matrix in soma address space
    soma_idxs = syn_to_soma_addr(SYN_N)
    tap_matrix_soma[soma_idxs] = tap_matrix_syn
    net = graph.Network("net")
    net_pool = net.create_pool("p", tap_matrix_soma)
    net_input = net.create_input("i", encoder_dim)
    net.create_connection("c: i to p", net_input, net_pool, None)
    HAL.map(net)
    set_analog()
    return net_input

def test_1d(gen_rates, recv_data):
    """Test 1D pools

    First test all synapses and search across input rates
    Second drop slow synapses and search across input rates
    """
    net_input = build_net_1d(0, recv_data.rate_group)
    init_rate = np.min(list(recv_data.rate_group))
    _, min_test_rates, min_overflows = find_max_rate(
        net_input, gen_rates, init_rate)
    min_total_rates = min_test_rates * recv_data.syn_n

    net_input = build_net_1d(recv_data.clipped_max_rate, recv_data.rate_group)
    _, clip_test_rates, clip_overflows = find_max_rate(
        net_input, gen_rates, init_rate=recv_data.clipped_max_rate)
    clip_total_rates = clip_test_rates * recv_data.clipped_syn_n
    return min_total_rates, min_overflows, clip_total_rates, clip_overflows

def plot_test_1d(recv_data, test_1d_data):
    """Plot the data from test_1d"""
    min_rates, min_overflows, clip_rates, clip_overflows = test_1d_data
    fig, axs = plt.subplots()
    axs.axhline(0, color='k', linewidth=1)
    min_max_line = axs.axvline(recv_data.min_max_total_rate*1E-6, linewidth=1)
    clip_max_line = axs.axvline(recv_data.clipped_max_total_rate*1E-6, linewidth=1)
    min_line = axs.plot(min_rates*1E-6, min_overflows, '-o', label="target all syn")[0]
    clip_line = axs.plot(clip_rates*1E-6, clip_overflows, '-o', label="without slow syn")[0]
    min_max_line.set_color(min_line.get_color())
    clip_max_line.set_color(clip_line.get_color())
    axs.set_xlabel("Total Input Rate (Mspks/s)")
    axs.set_ylabel("Overflow Count")
    axs.legend(loc="best")
    axs.set_title("1 Spike Generator\nMax Observed Zero-Overflow Input Rate {:.1f} Mspks/s".format(
        np.max(clip_rates[clip_overflows == 0])*1E-6))
    fig.savefig(DATA_DIR + "test_1d.pdf")

def build_net_group(recv_data):
    """Build a network for testing with synapses grouped by their binned max spike rates

    Returns a neuromorph Network Input object. Its dimensions correspond to the
    synapse groups sorted by their binned rate.
    """
    encoder_dim = len(recv_data.rate_group)
    tap_matrix_syn = np.zeros((SYN_N, encoder_dim))
    for rate_idx, rate in enumerate(sorted(recv_data.rate_group)):
        tap_matrix_syn[recv_data.rate_group[rate].idxs, rate_idx] = 1
    tap_matrix_soma = np.zeros((NRN_N, encoder_dim)) # tap matrix in soma address space
    soma_idxs = syn_to_soma_addr(SYN_N)
    tap_matrix_soma[soma_idxs] = tap_matrix_syn
    net = graph.Network("net")
    net_pool = net.create_pool("p", tap_matrix_soma)
    net_input = net.create_input("i", encoder_dim)
    net.create_connection("c: i to p", net_input, net_pool, None)
    HAL.map(net)
    set_analog()
    return net_input

def test_group(gen_rates, recv_data):
    """Test the synapses as clustered by their quantized max rates

    Returns
    -------
    {max_rate: (test_rates, overflows)}
    """
    net_input = build_net_group(recv_data)
    rate_data = {} # return data
    for rate_idx, max_rate in enumerate(recv_data.rate_group):
        _, test_rates, overflows = find_max_rate(
            net_input, gen_rates, init_rate=max_rate, update_idxs=rate_idx)
        rate_data[max_rate] = (test_rates[:, rate_idx], overflows)
    return rate_data

def plot_test_group(recv_data, test_group_data):
    """Plot the data from test_groups"""
    max_total_rate = 0
    fig, axs = plt.subplots(ncols=2, figsize=(14, 6))
    axs[0].axhline(0, color='k', linewidth=0.5)
    axs[1].axhline(0, color='k', linewidth=0.5)
    axs[1].axvline(recv_data.min_max_total_rate*1E-6, color='k', linewidth=0.5)
    axs[1].axvline(recv_data.clipped_max_total_rate*1E-6, color='k', linewidth=0.5)
    for rate in sorted(recv_data.rate_group):
        input_rates = test_group_data[rate][0]
        overflows = test_group_data[rate][1]
        total_rates = input_rates * recv_data.rate_group[rate].syn_n
        max_total_rate = max(max_total_rate, np.max(total_rates))
        axvline = axs[0].axvline(rate, linewidth=0.5)
        line = axs[0].plot(input_rates, overflows, '-o', linewidth=1)[0]
        axvline.set_color(line.get_color())
        axs[1].plot(total_rates*1E-6, overflows, '-o')
        axs[0].set_xlabel("Spike Generator Rate (spks/s)")
        axs[1].set_xlabel("Total Input Rate (Mspks/s)")
        axs[0].set_ylabel("Overflow Counts")
    fig.suptitle("Test Synapse Groups Individually\nMax Observed Total Input Rate {:.1f}".format(
        ))
    fig.savefig(DATA_DIR + "test_group.pdf")

def test_all_groups(gen_rates, recv_data):
    """Test all synapses simultaneously as grouped by their quantized max rates

    Returns
    -------
    {test_rate: ([clipped_rates], overflows)}
    """
    net_input = build_net_group(recv_data)

    rate_limits = np.sort(list(recv_data.rate_group))
    init_rate = np.min(rate_limits)
    base_rates, test_rates, overflows = find_max_rate(
        net_input, gen_rates, init_rate, rate_limits=rate_limits)
    return base_rates, test_rates, overflows

def plot_test_all_groups(recv_data, test_all_groups_data):
    """Plot data from test_all_groups"""
    base_rates, test_rates, overflows = test_all_groups_data
    n_syns = np.array(
        [recv_data.rate_group[rate].syn_n for rate in sorted(list(recv_data.rate_group))])
    total_rates = np.sum(test_rates*n_syns, axis=1)

    fig, axs = plt.subplots(ncols=2, figsize=(14, 6))
    axs[0].plot(base_rates, overflows, '-o')
    axs[1].plot(total_rates*1E-6, overflows, '-o')
    axs[0].axhline(0, color='k', linewidth=0.5)
    axs[1].axhline(0, color='k', linewidth=0.5)
    axs[1].axvline(recv_data.min_max_total_rate*1E-6, color='k', linewidth=1)
    axs[1].axvline(recv_data.clipped_max_total_rate*1E-6, color='k', linewidth=1)
    axs[1].axvline(recv_data.max_max_total_rate*1E-6, color='k', linewidth=1)
    axs[0].set_xlabel("Base Input Rates (spks/s)")
    axs[0].set_ylabel("Overflow Count")
    axs[1].set_xlabel("Total Input Rates (Mspks/s")
    fig.suptitle("Test All Synapses Groups Together")
    fig.savefig(DATA_DIR + "test_all_groups.pdf")

def check_max_input_spike_rates(parsed_args):
    """Run the test"""
    use_saved_data = parsed_args.use_saved_data
    if use_saved_data:
        recv_data = load_pickle_data(DATA_DIR + "recv_data.p")
        test_1d_data = load_pickle_data(DATA_DIR + "test_1d_data.p")
        test_group_data = load_pickle_data(DATA_DIR + "test_group_data.p")
        test_all_group_data = load_pickle_data(DATA_DIR + "test_all_group_data.p")
    else:
        gen_rates = compute_spike_gen_rates(MIN_RATE, MAX_RATE, SPIKE_GEN_TIME_UNIT_NS)
        recv_data = compute_receiver_rates(gen_rates, SYN_MAX_RATE_FILE)
        test_1d_data = test_1d(gen_rates, recv_data)
        test_group_data = test_group(gen_rates, recv_data)
        test_all_group_data = test_all_groups(gen_rates, recv_data)

        save_pickle_data(DATA_DIR + "recv_data.p", recv_data)
        save_pickle_data(DATA_DIR + "test_1d_data.p", test_1d_data)
        save_pickle_data(DATA_DIR + "test_group_data.p", test_group_data)
        save_pickle_data(DATA_DIR + "test_all_group_data.p", test_all_group_data)

    plot_test_1d(recv_data, test_1d_data)
    plot_test_group(recv_data, test_group_data)
    plot_test_all_groups(recv_data, test_all_group_data)

    plt.show()

if __name__ == "__main__":
    check_max_input_spike_rates(parse_args())
