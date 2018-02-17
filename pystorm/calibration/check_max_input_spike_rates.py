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
import pickle

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph
from pystorm.PyDriver import bddriver as bd

from utils import load_txt_data

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

SYN_PD_PU = 1024 # analog bias setting

RUN_TIME = 1.0
INTER_RUN_TIME = 0.2

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
    return fpga_rates, periods_spike_gen_time_units

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
        syn_idxs = max_syn_rates > rate
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

def build_net_grouped_syn(rate_syn):
    """Build a network for testing"""
    syn_n = 0
    encoder_dim = 0
    for rate in rate_syn.keys():
        syn_n += len(rate_syn[rate])
        encoder_dim += 1
    tap_matrix_syn = np.zeros((syn_n, encoder_dim))
    for rate_idx, rate in enumerate(sorted(rate_syn.keys())):
        syns = rate_syn[rate]
        for syn in syns:
            tap_matrix_syn[syn, rate_idx] = 1
    tap_matrix_soma = np.zeros((NRN_N, encoder_dim)) # tap matrix in soma address space
    sqrt_syn_n = int(np.round(np.sqrt(syn_n)))
    sqrt_soma_n = int(np.round(np.sqrt(NRN_N)))
    # print("syn         soma      encoders")
    # print(" idx  x  y  idx  x  y")
    for syn_idx in range(syn_n):
        syn_x = syn_idx % sqrt_syn_n
        syn_y = syn_idx // sqrt_syn_n
        soma_x = syn_x * 2
        soma_y = syn_y * 2
        soma_idx = soma_y*sqrt_soma_n + soma_x
        # print("{:04d} {:02d} {:02d} {:04d} {:02d} {:02d} {}".format(
        #     syn_idx, syn_x, syn_y, soma_idx, soma_x, soma_y, tap_matrix_syn[syn_idx]))
        tap_matrix_soma[soma_idx] = tap_matrix_syn[syn_idx]

    np.savetxt(DATA_DIR + "tap_matrix_syn.txt", tap_matrix_syn, "%.0f")
    np.savetxt(DATA_DIR + "tap_matrix_soma.txt", tap_matrix_soma, "%.0f")
    net = graph.Network("net")
    net_pool = net.create_pool("p", tap_matrix_soma)
    net_input = net.create_input("i", encoder_dim)
    net.create_connection("c: i to p", net_input, net_pool, None)
    HAL.map(net)
    set_analog()
    return net_input

def clear_overflow():
    """Clear any remaining overflow counts"""
    overflow, _ = HAL.driver.GetFIFOOverflowCounts(CORE)
    while overflow:
        sleep(INTER_RUN_TIME)
        overflow, _ = HAL.driver.GetFIFOOverflowCounts(CORE)

def toggle_input_rates(net_input, rates):
    """Toggle the spike generator and check for overflow"""
    print("Applying input rates {}".format(rates))
    clear_overflow()
    HAL.start_traffic(flush=False)
    HAL.enable_output_recording(flush=True)
    for dim, rate in enumerate(rates):
        HAL.set_input_rate(net_input, dim, int(rate), time=0, flush=True)
    sleep(RUN_TIME)
    for dim, _ in enumerate(rates):
        HAL.set_input_rate(net_input, dim, 0, time=0, flush=True)
    HAL.flush()
    sleep(INTER_RUN_TIME)
    HAL.stop_traffic(flush=False)
    HAL.disable_output_recording(flush=True)
    overflow, _ = HAL.driver.GetFIFOOverflowCounts(CORE)
    print("\toverflow {}".format(overflow))
    return overflow

def test_syn_groups(net_input, gen_rates, rate_syn):
    """Test the synapses as clustered by their quantized max rates

    Returns a dictionary
    max_rate : R x 3 array
        for each rate R, record R, the total input rate, and the overflow count
    """
    rate_data = {} # return data
    for max_rate_idx, max_rate in enumerate(rate_syn):
        syn_n = len(rate_syn[max_rate])
        # input_rates = np.zeros(len(rate_syn))
        input_rates = np.ones(len(rate_syn))
        idxs = np.logical_and(
            gen_rates <= max_rate * 4,
            gen_rates >= max_rate / 2)
        test_rates = gen_rates[idxs]
        print("\nTesting rates within consumption rates of single synapse cluster\n{}".format(
            test_rates))
        rate_data[max_rate] = np.zeros((len(test_rates), 3))
        for test_idx, test_rate in enumerate(test_rates):
            input_rates[max_rate_idx] = test_rate
            total_rate = test_rate * syn_n
            overflow = toggle_input_rates(net_input, input_rates)
            rate_data[max_rate][test_idx, 0] = test_rate
            rate_data[max_rate][test_idx, 1] = total_rate
            rate_data[max_rate][test_idx, 2] = overflow

    fig, axs = plt.subplots(nrows=2)
    for max_rate in sorted(rate_data):
        input_rates = rate_data[max_rate][:, 0]
        total_rates = rate_data[max_rate][:, 1]
        overflows = rate_data[max_rate][:, 2]

        total_rates_mhz = total_rates * 1E-6
        line = axs[0].plot(input_rates, overflows, '-o')[0]
        axs[0].axvline(max_rate, color=line.get_color(), linewidth=0.5)
        axs[1].plot(total_rates_mhz, overflows, '-o')[0]
        axs[0].set_xlabel("Spike Generator Rate (spks/s)")
        axs[1].set_xlabel("Total Input Rate (Mspks/s)")
        axs[0].set_ylabel("Overflow Counts")
        axs[1].set_ylabel("Overflow Counts")
    fig.savefig(DATA_DIR + "syn_group_by_group_test.pdf")

    return rate_data

def clip_input_rates(test_rate, rate_syn):
    """Clip the test rate to rates synapses can handle"""
    applied_rates = np.zeros(len(rate_syn))
    for dim, rate in enumerate(sorted(rate_syn)):
        if rate >= test_rate:
            applied_rates[dim] = test_rate
        else:
            applied_rates[dim] = rate
    return applied_rates

def test_custom_rates(net_input, gen_rates, rate_syn):
    """Test input to all synapses grouped by their quantized max rates

    Clip input to each groups qantized max rate as needed
    """
    min_max_rate = np.min(list(rate_syn))
    max_max_rate = np.max(list(rate_syn))
    idxs = np.logical_and(
        gen_rates <= max_max_rate * 4,
        gen_rates >= min_max_rate / 3)
    test_rates = gen_rates[idxs]
    overflows = np.zeros(len(test_rates))
    total_rates = np.zeros(len(test_rates))
    n_syns = np.array(list(map(lambda x: len(rate_syn[x]), sorted(rate_syn))))
    print("\nTesting rates around synapse max consumption rates\n{}".format(test_rates))
    for idx, test_rate in enumerate(test_rates):
        input_rates = clip_input_rates(test_rate, rate_syn)
        total_rates[idx] = np.sum(input_rates*n_syns)
        overflows[idx] = toggle_input_rates(net_input, input_rates)

    min_total_rate = np.sum(np.min(list(rate_syn)) * n_syns)
    max_total_rate = np.sum(np.array(sorted(rate_syn.keys()))*n_syns)

    total_rates_mhz = total_rates * 1E-6
    min_total_rate_mhz = min_total_rate * 1E-6
    max_total_rate_mhz = max_total_rate * 1E-6
    fig, ax = plt.subplots()
    ax.axvline(min_total_rate_mhz, color='k', linewidth=1, label="{:.1f}".format(
        min_total_rate_mhz))
    ax.axvline(max_total_rate_mhz, color='r', linewidth=1, label="{:.1f}".format(
        max_total_rate_mhz))
    ax.plot(total_rates_mhz, overflows, '-o')
    ax.set_xlabel("Total Input Rate (Mspks/s)")
    ax.set_ylabel("Overflow Count")
    ax.legend()
    fig.savefig(DATA_DIR + "custom_rates_test.pdf")

    return total_rates, overflows

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

def find_max_rate(net_input, gen_rates, init_rate):
    """Find where FIFO starts overflowing"""
    idx = compute_init_rate_idx(gen_rates, init_rate)

    test_rates = []
    overflows = []

    rate = gen_rates[idx]
    test_rates.append(rate)
    overflows.append(toggle_input_rates(net_input, [gen_rates[idx]]))

    if overflows[0] > 0:
        rate = gen_rates[idx+1]
        test_rates.append(rate)
        overflows.append(toggle_input_rates(net_input, [rate]))
        while overflows[-1] > 0 or len(overflows) < 5:
            idx -= 1
            rate = gen_rates[idx]
            test_rates.append(rate)
            overflows.append(toggle_input_rates(net_input, [rate]))
    elif overflows[0] == 0:
        rate = gen_rates[idx-1]
        test_rates.append(rate)
        overflows.append(toggle_input_rates(net_input, [rate]))
        while overflows[-1] == 0 or len(overflows) < 5:
            idx += 1
            rate = gen_rates[idx]
            test_rates.append(rate)
            overflows.append(toggle_input_rates(net_input, [rate]))

    test_rates = np.array(test_rates)
    overflows = np.array(overflows)

    sort_idx = np.argsort(test_rates)
    test_rates = test_rates[sort_idx]
    overflows = overflows[sort_idx]
    return test_rates, overflows

def test_1d(gen_rates, recv_data):
    """Test 1D pools"""
    net_input = build_net_1d(0, recv_data.rate_group)
    min_test_rates, min_overflows = find_max_rate(
        net_input, gen_rates, np.min(list(recv_data.rate_group)))
    min_total_rates = min_test_rates * recv_data.syn_n

    net_input = build_net_1d(recv_data.clipped_max_rate, recv_data.rate_group)
    clip_test_rates, clip_overflows = find_max_rate(
        net_input, gen_rates, recv_data.clipped_max_rate)
    clip_total_rates = clip_test_rates * recv_data.clipped_syn_n
    return min_total_rates, min_overflows, clip_total_rates, clip_overflows

def plot_test_1d(recv_data, min_rates, min_overflows, clip_rates, clip_overflows):
    """Plot the data from test_1d"""
    fig, ax = plt.subplots()
    ax.axhline(0, color='k', linewidth=1)
    min_max_line = ax.axvline(recv_data.min_max_total_rate*1E-6, linewidth=1)
    clip_max_line = ax.axvline(recv_data.clipped_max_total_rate*1E-6, linewidth=1)
    min_line = ax.plot(min_rates*1E-6, min_overflows, '-o', label="target all syn")[0]
    clip_line = ax.plot(clip_rates*1E-6, clip_overflows, '-o', label="without slow syn")[0]
    min_max_line.set_color(min_line.get_color())
    clip_max_line.set_color(clip_line.get_color())
    ax.set_xlabel("Total Input Rate (Mspks/s)")
    ax.set_ylabel("Overflow Count")
    ax.legend(loc="best")
    ax.set_title("1 Spike Generator\nMax 0-Overflow Input Rate {:.1f} Mspks/s".format(
        np.max(clip_rates[clip_overflows == 0])*1E-6))
    fig.savefig(DATA_DIR + "test_1d_rates.pdf")

def test_receiver(parsed_args):
    """Run the test"""
    use_saved_data = parsed_args.use_saved_data
    if use_saved_data:
        with open(DATA_DIR + "recv_data.p", "rb") as recv_file:
            recv_data = pickle.load(recv_file)
        min_rates = load_txt_data(DATA_DIR + "min_rates.txt")
        min_overflows = load_txt_data(DATA_DIR + "min_overflows.txt")
        clip_rates = load_txt_data(DATA_DIR + "clip_rates.txt")
        clip_overflows  = load_txt_data(DATA_DIR + "clip_overflows.txt")
    else:
        gen_rates, _ = compute_fpga_rates(MIN_RATE, MAX_RATE, SPIKE_GEN_TIME_UNIT_NS)
        recv_data = compute_receiver_rates(gen_rates, SYN_MAX_RATE_FILE)
        min_rates, min_overflows, clip_rates, clip_overflows = test_1d(gen_rates, recv_data)

        with open(DATA_DIR + "recv_data.p", "wb") as recv_file:
            pickle.dump(recv_data, recv_file)
        np.savetxt(DATA_DIR + "min_rates.txt", min_rates)
        np.savetxt(DATA_DIR + "min_overflows.txt", min_overflows)
        np.savetxt(DATA_DIR + "clip_rates.txt", clip_rates)
        np.savetxt(DATA_DIR + "clip_overflows.txt", clip_overflows)

    plot_test_1d(recv_data, min_rates, min_overflows, clip_rates, clip_overflows)

    # net_input = build_net_grouped_syn(rate_syn)
    # group_data = test_syn_groups(net_input, gen_rates, rate_syn)
    # np.save(DATA_DIR + "group_rates_data.npy", group_data)
    # clip_rate_data = test_custom_rates(net_input, gen_rates, rate_syn)
    # np.save(DATA_DIR + "clip_rates_data.npy", clip_rate_data)

    plt.show()

if __name__ == "__main__":
    test_receiver(parse_args())
