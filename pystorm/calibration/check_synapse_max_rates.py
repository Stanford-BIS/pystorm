"""Find the maximum firing rate of the synapses as driven by bias

Set synapse offset bias to max
Set synapse refractory bias to max (minimum refractory)

Iterate through the synapses, and check for FIFO overflow
Plot results
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

CORE = 0
NRN_N = 4096
# SYN_N = 64
SYN_N = 1024

RUN_TIME = 1.0
INTER_RUN_TIME = 0.2
RATE_TOL = 0.001
BOUND_FINDING_SCALE = 2

SPIKE_GEN_TIME_UNIT_NS = 10000 # time unit of fpga spike generator
SPIKE_GEN_IDX = 0 # FPGA spike generator index
# Tag Action Table settings
TAT_IDX = 0
TAT_START_ADDR = 0
TAT_STOP_BIT = 1
TAT_SIGN_0 = 0
TAT_SIGN_1 = 1

SYN_PD_PU = 1024 # analog bias setting

INIT_RATE = 10000 # initial rate to test

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
    HAL.set_time_resolution(downstream_ns=SPIKE_GEN_TIME_UNIT_NS)
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

def toggle_spk_generator(rate):
    """Toggle the spike generator and check for overflow"""
    HAL.driver.SetSpikeGeneratorRates(
        CORE, [SPIKE_GEN_IDX], [TAT_IDX], [rate], time=0, flush=True)
    sleep(RUN_TIME)
    HAL.driver.SetSpikeGeneratorRates(
        CORE, [SPIKE_GEN_IDX], [TAT_IDX], [0], time=0, flush=True)
    sleep(INTER_RUN_TIME)
    overflow_0, _ = HAL.driver.GetFIFOOverflowCounts(CORE)
    return overflow_0

def find_syn_max_rate_bounds():
    """Find the minimum and maximum bounds of the max rate"""
    rate = INIT_RATE
    upper_rate = None
    lower_rate = None

    overflow = toggle_spk_generator(rate)
    if overflow:
        upper_rate = rate
    else:
        lower_rate = rate

    while not lower_rate:
        rate //= BOUND_FINDING_SCALE
        overflow = toggle_spk_generator(rate)
        if overflow:
            upper_rate = rate
        else:
            lower_rate = rate

    while not upper_rate:
        rate *= BOUND_FINDING_SCALE
        overflow = toggle_spk_generator(rate)
        if overflow:
            upper_rate = rate
        else:
            lower_rate = rate
    return lower_rate, upper_rate

def measure_syn_max_rate():
    """Deliver spikes to a pair of synapses to find the minimum of their maximum input rates"""
    lower_rate, upper_rate = find_syn_max_rate_bounds()

    while (upper_rate-lower_rate)/lower_rate > RATE_TOL:
        rate = int((upper_rate+lower_rate)/2.)
        overflow = toggle_spk_generator(rate)
        if overflow:
            upper_rate = rate
        else:
            lower_rate = rate

    # check again
    of_upper = toggle_spk_generator(upper_rate)
    of_lower = toggle_spk_generator(lower_rate)
    bound_check = of_upper > 0 and of_lower == 0
    # due to TAT configuration, 2 spikes sent to synapse per spike to TAT
    max_rate = upper_rate+lower_rate
    return max_rate, bound_check

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

    fig_hist = plt.figure()
    bins = min(max(10, synapses), 80)
    plt.hist(max_rates, bins=bins)
    plt.axvline(max_rates_mean, color="k", label="mean")
    plt.axvline(max_rates_median, color="r", label="median")
    for fpga_rate in fpga_rates:
        plt.axvline(fpga_rate, color=(0.8, 0.8, 0.8), linewidth=1)
    plt.xlabel("Max firing Rate (Hz)")
    plt.ylabel("Counts")
    plt.title("Mean:{:,.0f} Median:{:,.0f} Min:{:,.0f} Max:{:,.0f}".format(
        max_rates_mean, max_rates_median, max_rates_min, max_rates_max))
    plt.legend()

    fig_1d.savefig(DATA_DIR + "syn_idx_vs_max_rate.pdf")
    fig_2d_heatmap.savefig(DATA_DIR + "2d_heatmap.pdf")
    fig_2d_surf.savefig(DATA_DIR + "2d_surface.pdf")
    fig_hist.savefig(DATA_DIR + "histogram.pdf")

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
        build_net()
        set_analog()
        set_hal()
        max_rates = np.zeros(SYN_N)
        bound_checks = np.zeros(SYN_N, dtype=bool)
        start_time = get_time()
        for syn_idx in range(SYN_N):
            set_tat(syn_idx)
            rate, check = measure_syn_max_rate()
            max_rates[syn_idx] = rate
            bound_checks[syn_idx] = check
            print("syn:{} max_rate:{:.0f} bounds_consistent:{}".format(syn_idx, rate, check))
            if syn_idx%4 == 0:
                n_syn_completed = syn_idx+1
                delta_time = get_time()-start_time
                est_time_remaining = delta_time/(syn_idx+1) * (SYN_N-n_syn_completed)
                print("estimated time remaining: {:.0f} s = {:.1f} min = {:.2f} hr...".format(
                    est_time_remaining, est_time_remaining/60., est_time_remaining/60./60.))
        print("Max firing rates:")
        print(max_rates)
        print("Min synapse spike consumption times:")
        print(1./max_rates)
        print("Bound consistency checks:")
        print(bound_checks)

    plot_max_rates(max_rates)
    if not use_saved_data:
        np.savetxt(DATA_DIR + "max_rates.txt", max_rates)
    plt.show()

if __name__ == "__main__":
    check_synapse_max_rates(parse_args())
