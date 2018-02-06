"""Find the maximum output spike rate through the transmitter, accumulator, and io

Send all soma traffic through single accumulator dimension
Set soma refractory bias to max (minimum refractory)

Vary soma bias
Vary weighting

Show observed output spike rate
Show effective output spike rate = observed / weighting
"""
import os
import sys
from time import sleep
import argparse
import numpy as np
import matplotlib.pyplot as plt

from pystorm.hal import HAL
from pystorm.hal.hal import parse_hal_binned_tags
from pystorm.hal.neuromorph import graph
from pystorm.PyDriver import bddriver as bd

CORE = 0
DIM = 1
N_NRN = 4096
BIAS_REF = 1024
BIAS_OFFSETS = np.array([2, 4, 8, 16, 32, 64, 128])
WEIGHTS = np.array([1./512, 1./256, 1./128, 1./64, 1./32])
TIME_SCALE = 1E-9

RUN_TIME = 0.2
INTER_RUN_TIME = 3.0

DATA_DIR = "./data/" + os.path.basename(__file__)[:-3] + "/"
if not os.path.isdir(DATA_DIR):
    os.makedirs(DATA_DIR, exist_ok=True)

def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Characterize the soma max firing rates')
    parser.add_argument("-r", action="store_true", dest="use_saved_data", help='reuse cached data')
    args = parser.parse_args()
    return args

def build_net(weight):
    """Builds the HAL-level network for testing"""
    decoders = np.ones((DIM, N_NRN)) * weight
    tap_matrix = np.zeros((N_NRN, DIM))

    net = graph.Network("net")
    pool = net.create_pool("pool", tap_matrix)
    bucket = net.create_bucket("b", DIM)
    net_out = net.create_output("o", DIM)
    net.create_connection("p_to_b", pool, bucket, decoders)
    net.create_connection("b_to_o", bucket, net_out, None)
    HAL.map(net)
    return net_out

def set_analog():
    """Sets the soma config bits and the bias currents"""
    for n_idx in range(N_NRN):
        HAL.driver.SetSomaGain(CORE, n_idx, bd.bdpars.SomaGainId.ONE)
        HAL.driver.SetSomaOffsetSign(CORE, n_idx, bd.bdpars.SomaOffsetSignId.POSITIVE)
        HAL.driver.SetSomaOffsetMultiplier(CORE, n_idx, bd.bdpars.SomaOffsetMultiplierId.THREE)
        HAL.driver.SetSomaEnableStatus(CORE, n_idx, bd.bdpars.SomaStatusId.ENABLED)
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_REF, BIAS_REF)
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, 1)
    HAL.flush()

def toggle_hal_recording():
    """Start and stop HAL traffic"""
    HAL.get_outputs()
    HAL.set_time_resolution(upstream_ns=100000)
    HAL.start_traffic(flush=False)
    HAL.disable_spike_recording(flush=False)
    HAL.enable_output_recording(flush=True)
    sleep(RUN_TIME)
    HAL.stop_traffic(flush=False)
    HAL.disable_spike_recording(flush=False)
    HAL.disable_output_recording(flush=True)
    HAL.set_time_resolution(upstream_ns=10000000)

def measure_output_rate(net_out, offset):
    """Collect binned tags and calcaulte the overall firing rate"""
    sleep(INTER_RUN_TIME)
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, offset)
    HAL.flush()
    toggle_hal_recording()
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, 1)
    raw_hal_outputs = HAL.get_outputs()
    hal_outputs = parse_hal_binned_tags(raw_hal_outputs)
    output_counts = np.array(hal_outputs[net_out][0])
    nonzero_idxs = np.nonzero(output_counts[:, 1])[0]
    if len(nonzero_idxs) > 1:
        min_idx = nonzero_idxs[0]
        max_idx = nonzero_idxs[-1]
        min_time = output_counts[min_idx][0]
        max_time = output_counts[max_idx][0]
        counts = np.sum(output_counts[min_idx:max_idx+1, 1])
        time_period = (max_time - min_time)*TIME_SCALE
        rate = counts/time_period
    else:
        rate = 0.
    return rate

def plot_rates(rates, weights, offsets):
    """Plot the data"""
    effective_rates = rates/weights
    max_effective_rate = np.max(effective_rates)
    rates_mhz = rates*1E-6
    effective_rates_mhz = effective_rates*1E-6
    max_effective_rate_mhz = max_effective_rate*1E-6

    fig_rates, axs = plt.subplots(nrows=2, ncols=1, sharex=True, figsize=(8, 8))
    axs[0].axhline(max_effective_rate_mhz, color='k', linewidth=1)
    for w_idx, weight in enumerate(weights):
        axs[1].plot(offsets, rates_mhz[:, w_idx], '-o',
                    label="{:.1f}".format(np.log2(weight)))
        axs[0].plot(offsets, effective_rates_mhz[:, w_idx], '-o',
                    label="{:.1f}".format(np.log2(weight)))
    axs[1].set_xlabel("Offset Bias")
    axs[0].set_ylabel("Output Rate / Decode Weight (MHz)")
    axs[1].set_ylabel("Output Rate (MHz)")
    axs[0].set_title("Max(Output Rate / Decode Weight)={:.2f} MHz".format(max_effective_rate_mhz))
    handles, labels = axs[0].get_legend_handles_labels()
    axs[0].legend(handles[::-1], labels[::-1], title="log2(decoders)")

    fig_rates.savefig(DATA_DIR + "rates.pdf")

def load_data(fname, dtype=None):
    """Load data from fname"""
    try:
        if dtype:
            data = np.loadtxt(fname, dtype=dtype)
        else:
            data = np.loadtxt(fname)
    except FileNotFoundError:
        print("\nError: Could not find saved data {}\n".format(fname))
        sys.exit(1)
    return data

def print_data(rates, weights, offsets):
    """Print out the results"""
    for w_idx, weight in enumerate(weights):
        print("\nlog2(decode weight)={:.1f}".format(np.log2(weight)))
        print("Offset Measured_Rate Effective_Rate")
        for o_idx, offset in enumerate(offsets):
            rate = rates[o_idx, w_idx]
            print("{:6d}     {:5.3e}      {:5.3e}".format(offset, rate, rate/weight))

def check_max_output_spike_rates(parsed_args):
    """Run the check"""
    use_saved_data = parsed_args.use_saved_data
    if use_saved_data:
        rates = load_data(DATA_DIR + "rates.txt")
        weights = load_data(DATA_DIR + "weights.txt")
        offsets = load_data(DATA_DIR + "offsets.txt", dtype=int)
    else:
        weights = WEIGHTS
        offsets = BIAS_OFFSETS
        rates = np.zeros((len(offsets), len(weights)))
        for w_idx, weight in enumerate(weights):
            net_out = build_net(weight)
            set_analog()
            for o_idx, offset in enumerate(offsets):
                rates[o_idx, w_idx] = measure_output_rate(net_out, offset)

    plot_rates(rates, weights, offsets)
    if not use_saved_data:
        np.savetxt(DATA_DIR + "rates.txt", rates)
        np.savetxt(DATA_DIR + "weights.txt", weights)
        np.savetxt(DATA_DIR + "offsets.txt", offsets, fmt="%d")
    print_data(rates, weights, offsets)
    plt.show()

if __name__ == "__main__":
    check_max_output_spike_rates(parse_args())
