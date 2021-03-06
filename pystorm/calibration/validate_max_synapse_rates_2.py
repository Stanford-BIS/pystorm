"""Validate the synapse input max rates found by check_max_synapse_rates"""
import os
from time import sleep
from time import time as get_time
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

DEFAULT_TEST_TIME = 2.0 # time to collect overflow data
DEFAULT_SLOP_TIME = 0.2 # time to allow traffic to flush at the start and end of an experiment

SPIKE_GEN_TIME_UNIT_NS = 10000 # time unit of fpga spike generator
SPIKE_GEN_IDX = 0 # FPGA spike generator index
# Tag Action Table settings
TAT_IDX = 0
TAT_START_ADDR = 0
TAT_STOP_BIT = 1
TAT_SIGN_0 = 0
TAT_SIGN_1 = 1

FIFO_BUFFER_SIZE = 255
VALIDATE_HIGH_RATE_BUF = 500 # upper bound padding to test high side of max_rate

SYN_PD_PU = 1024 # analog bias setting

MIN_RATE = 1000 # minimum rate to test
MAX_RATE = 100000 # maximum rate to test

SPIKE_GEN_RATES = compute_spike_gen_rates(MIN_RATE, MAX_RATE, SPIKE_GEN_TIME_UNIT_NS)

DATA_DIR = "./data/" + os.path.basename(__file__)[:-3] + "/"
DETAIL_DATA_DIR = DATA_DIR + "fits/"
if not os.path.isdir(DATA_DIR):
    os.makedirs(DATA_DIR, exist_ok=True)
if not os.path.isdir(DETAIL_DATA_DIR):
    os.makedirs(DETAIL_DATA_DIR, exist_ok=True)

def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Characterize the synapse max input firing rates')
    parser.add_argument(
        "-r", action="store_true", dest="use_saved_data", help='reuse saved data')
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
    addr = HAL.driver.BDPars.GetSynAERAddr(syn_idx)
    tat_entry = bd.PackWord([
        (bd.TATSpikeWord.STOP, TAT_STOP_BIT),
        (bd.TATSpikeWord.SYNAPSE_ADDRESS_0, addr),
        (bd.TATSpikeWord.SYNAPSE_SIGN_0, TAT_SIGN_0),
        (bd.TATSpikeWord.SYNAPSE_ADDRESS_1, addr),
        (bd.TATSpikeWord.SYNAPSE_SIGN_1, TAT_SIGN_1)])
    HAL.driver.SetMem(CORE, bd.bdpars.BDMemId.TAT0, [tat_entry], TAT_START_ADDR)
    HAL.flush()

def toggle_spk_generator(rate, test_time, slop_time):
    """Toggle the spike generator and check for overflow"""
    clear_overflows(HAL, slop_time)
    test_time_ns = int(test_time*1E9)
    slop_time_ns = int(slop_time*1E9)
    cur_time_ns = HAL.get_time()
    HAL.driver.SetSpikeGeneratorRates(
        CORE, [SPIKE_GEN_IDX], [TAT_IDX], [rate], time=cur_time_ns+slop_time_ns)
    HAL.driver.SetSpikeGeneratorRates(
        CORE, [SPIKE_GEN_IDX], [TAT_IDX], [0], time=cur_time_ns+test_time_ns+slop_time_ns)
    sleep(test_time + 2*slop_time)
    overflow_0, _ = HAL.driver.GetFIFOOverflowCounts(CORE)
    print("\tRate {:.1f}, overflow_count {} overflow rate {:.1f}".format(
        rate, overflow_0, overflow_0/test_time))
    return overflow_0

def validate_syn(syn_idx, max_rate_2, test_time, slop_time):
    """Deliver spikes to a synapse to find its spike consumption rate"""
    set_tat(syn_idx)
    high_rate_target = max_rate_2 + FIFO_BUFFER_SIZE / test_time + VALIDATE_HIGH_RATE_BUF
    low_rate = SPIKE_GEN_RATES[np.nonzero(SPIKE_GEN_RATES <= max_rate_2)[0][-1]]
    high_rate = SPIKE_GEN_RATES[np.nonzero(SPIKE_GEN_RATES > high_rate_target)[0][0]]
    low_overflows = toggle_spk_generator(low_rate, test_time, slop_time)
    high_overflows = toggle_spk_generator(high_rate, test_time, slop_time)
    return low_rate, low_overflows, high_rate, high_overflows

def report_time_remaining(start_time, syn_idx):
    """Occasionally estimate and report the remaining time"""
    if syn_idx%4 == 0 and SYN_N > 1:
        n_syn_completed = syn_idx+1
        delta_time = get_time()-start_time
        est_time_remaining = delta_time/(syn_idx+1) * (SYN_N-n_syn_completed)
        print("estimated time remaining: {:.0f} s = {:.1f} min = {:.2f} hr...".format(
            est_time_remaining, est_time_remaining/60., est_time_remaining/60./60.))

def plot_data(max_rates_2, low_rates, low_rates_o, high_rates, high_rates_o):
    """Plot the data"""
    fig_check_data, axs = plt.subplots(nrows=2, figsize=(8, 10))
    axs[0].plot(low_rates_o, 'o', label="max non-overflow")
    axs[0].plot(high_rates_o, 'o', label="min overflow")
    for syn_idx, _ in enumerate(max_rates_2):
        axs[1].plot(
            [syn_idx, syn_idx], [low_rates[syn_idx], max_rates_2[syn_idx]],
            'r', linewidth=1)
        axs[1].plot(
            [syn_idx, syn_idx], [max_rates_2[syn_idx], high_rates[syn_idx]],
            'b', linewidth=1)
    axs[0].axhline(0, linewidth=1, color='k')
    axs[0].legend(loc="best", title="input")
    axs[1].set_xlabel("Synapse Index")
    axs[0].set_ylabel("Overflow Counts")
    axs[1].set_ylabel("Rate (spks/s)")
    axs[0].set_title(
        "Validate Max Rate Measurements\n" +
        "no-overflow (overflow) data should all be zero (nonzero)")
    fig_check_data.savefig(DATA_DIR + "check_data.pdf")

def validate_max_synapse_rates(parsed_args):
    """Run the check"""
    use_saved_data = parsed_args.use_saved_data
    max_rates_2 = load_txt_data(DATA_DIR + "max_rates_2.txt") # max rates / 2
    syn_n = len(max_rates_2)
    if use_saved_data:
        low_rates = load_txt_data(DATA_DIR + "low_rates.txt")
        low_rates_o = load_txt_data(DATA_DIR + "low_rates_o.txt")
        high_rates = load_txt_data(DATA_DIR + "high_rates.txt")
        high_rates_o = load_txt_data(DATA_DIR + "high_rates_o.txt")
    else:
        low_rates = np.zeros(syn_n)
        low_rates_o = np.zeros(syn_n)
        high_rates = np.zeros(syn_n)
        high_rates_o = np.zeros(syn_n)
        build_net()
        set_analog()
        set_hal()
        start_time = get_time()
        for syn_idx in range(syn_n):
            print("Testing synapse {}".format(syn_idx))
            vdata = validate_syn(
                syn_idx, max_rates_2[syn_idx], DEFAULT_TEST_TIME, DEFAULT_SLOP_TIME)
            low_rates[syn_idx], low_rates_o[syn_idx] = vdata[0:2]
            high_rates[syn_idx], high_rates_o[syn_idx] = vdata[2:4]
            report_time_remaining(start_time, syn_idx)
        np.savetxt(DATA_DIR + "low_rates.txt", low_rates)
        np.savetxt(DATA_DIR + "low_rates_o.txt", low_rates_o)
        np.savetxt(DATA_DIR + "high_rates.txt", high_rates)
        np.savetxt(DATA_DIR + "high_rates_o.txt", high_rates_o)

    plot_data(max_rates_2, low_rates, low_rates_o, high_rates, high_rates_o)
    plt.show()

if __name__ == "__main__":
    validate_max_synapse_rates(parse_args())
