"""Find the maximum output spike rate through the transmitter, accumulator, and io

Send all soma traffic through single accumulator dimension
Set soma refractory bias to max (minimum refractory)

Vary soma bias
Vary weighting

Show observed output spike rate
Show effective output spike rate = observed / weighting
"""
import os
from time import sleep
import argparse
import numpy as np
import matplotlib.pyplot as plt

from pystorm.hal import HAL
from pystorm.hal.hal import parse_hal_binned_tags
from pystorm.hal.neuromorph import graph
from pystorm.PyDriver import bddriver as bd

from utils.file_io import load_txt_data
HAL = HAL()

CORE = 0
DIM = 1
N_NRN = 4096
BIAS_REF = 1024

# check transmitter limit
BIAS_OFFSETS = np.array([2, 4, 8, 16, 32, 64, 128])
WEIGHTS = np.array([1./1024, 1./512, 1./256, 1./128, 1./64])

# check the FPGA IO limit
# BIAS_OFFSETS = np.array([2, 4, 8, 10, 11, 12, 13, 14, 16, 18, 20])
# WEIGHTS = np.array([1./16])

TIME_SCALE = 1E-9

RUN_TIME = 0.2
INTER_RUN_TIME = 1.5
INTER_SAMPLE_TIME = 0.3

# when the fpga gets saturated, timing info gets corrupted
MAX_REASONABLE_RATE = 50E6

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
    hal_output_samples = []
    sleep(INTER_RUN_TIME)
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, offset)
    HAL.flush()
    toggle_hal_recording()
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, 1)
    sleep(INTER_SAMPLE_TIME)
    hal_output_samples.append(HAL.get_outputs())
    while hal_output_samples[-1].shape[0] > 0:
        sleep(INTER_SAMPLE_TIME)
        hal_output_samples.append(HAL.get_outputs())

    total_hal_output = []
    for output_sample in hal_output_samples[:-1]:
        total_hal_output += list(output_sample)
    total_hal_output = np.array(total_hal_output)

    output_dim_binned_tags = parse_hal_binned_tags(total_hal_output)
    output_counts = np.array(output_dim_binned_tags[net_out][0])
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
        counts = 0
        time_period = 0.
    return rate, counts, time_period

def plot_rates(rates, weights, offsets):
    """Plot the data"""
    effective_rates = rates/weights
    valid_idxs = np.logical_and(effective_rates < MAX_REASONABLE_RATE, effective_rates >= 0)
    max_idxs = np.array([rates.shape[0] for _ in weights])
    for idx in range(len(weights)):
        invalid_idxs = np.nonzero(~valid_idxs[:, idx])[0]
        if invalid_idxs.shape[0] > 0:
            max_idxs[idx] = invalid_idxs[0]
            valid_idxs[invalid_idxs[0]:, idx] = False

    max_rate = np.max(rates[valid_idxs])
    max_effective_rate = np.max(effective_rates[valid_idxs])
    rates_mhz = rates*1E-6
    effective_rates_mhz = effective_rates*1E-6
    max_effective_rate_mhz = max_effective_rate*1E-6
    max_rate_mhz = max_rate*1E-6

    if np.any(~valid_idxs):
        fig_unscreened_rates, axs = plt.subplots(nrows=2, ncols=1, sharex=True, figsize=(8, 8))
        axs[0].axhline(max_effective_rate_mhz, color='k', linewidth=1)
        axs[1].axhline(max_rate_mhz, color='k', linewidth=1)
        for w_idx, weight in enumerate(weights):
            axs[1].plot(offsets, rates_mhz[:, w_idx], '-o',
                        label="{:.1f}".format(np.log2(weight)))
            axs[0].plot(offsets, effective_rates_mhz[:, w_idx], '-o',
                        label="{:.1f}".format(np.log2(weight)))
        axs[1].set_xlabel("Offset Bias")
        axs[0].set_ylabel("Output Rate / Decode Weight (MHz)")
        axs[1].set_ylabel("Output Rate (MHz)")
        axs[0].set_title("Max(Output Rate / Decode Weight)={:.2f} MHz".format(
            max_effective_rate_mhz))
        axs[1].set_title("Max(Output Rate)={:.2f} MHz".format(
            max_rate_mhz))
        handles, labels = axs[0].get_legend_handles_labels()
        axs[0].legend(handles[::-1], labels[::-1], title="log2(decoders)")
        fig_unscreened_rates.suptitle(
            "Including data collected while FPGA IO buffer was saturated")
        fig_unscreened_rates.savefig(DATA_DIR + "rates_prescreened.pdf")
    else:
        if os.path.exists(DATA_DIR + "rates_prescreened.pdf"):
            os.remove(DATA_DIR + "rates_prescreened.pdf")


    fig_rates, axs = plt.subplots(nrows=2, ncols=1, sharex=True, figsize=(8, 8))
    axs[0].axhline(max_effective_rate_mhz, color='k', linewidth=1)
    axs[1].axhline(max_rate_mhz, color='k', linewidth=1)
    for w_idx, weight in enumerate(weights):
        axs[1].plot(offsets[:max_idxs[w_idx]], rates_mhz[:max_idxs[w_idx], w_idx], '-o',
                    label="{:.1f}".format(np.log2(weight)))
        axs[0].plot(offsets[:max_idxs[w_idx]], effective_rates_mhz[:max_idxs[w_idx], w_idx], '-o',
                    label="{:.1f}".format(np.log2(weight)))
    axs[1].set_xlabel("Offset Bias")
    axs[0].set_ylabel("Output Rate / Decode Weight (MHz)")
    axs[1].set_ylabel("Output Rate (MHz)")
    axs[0].set_title("Max(Output Rate / Decode Weight)={:.2f} MHz".format(max_effective_rate_mhz))
    axs[1].set_title("Max(Output Rate)={:.2f} MHz".format(max_rate_mhz))
    handles, labels = axs[0].get_legend_handles_labels()
    axs[0].legend(handles[::-1], labels[::-1], title="log2(decoders)")
    fig_rates.savefig(DATA_DIR + "rates.pdf")

def print_data(rates, weights, offsets, counts, time_periods):
    """Print out the results"""
    for w_idx, weight in enumerate(weights):
        print("\nlog2(decode weight)={:.1f}".format(np.log2(weight)))
        print("Offset Effective_Rate Measured_Rate      Count Time_Period")
        for o_idx, offset in enumerate(offsets):
            rate = rates[o_idx, w_idx]
            count = counts[o_idx, w_idx]
            time_period = time_periods[o_idx, w_idx]
            print("{:6d}      {:5.3e}     {:5.3e} {:10d}      {:6.4f}".format(
                offset, rate/weight, rate, count, time_period))

def check_max_output_spike_rates(parsed_args):
    """Run the check"""
    use_saved_data = parsed_args.use_saved_data
    if use_saved_data:
        rates = load_txt_data(DATA_DIR + "rates.txt")
        weights = load_txt_data(DATA_DIR + "weights.txt")
        offsets = load_txt_data(DATA_DIR + "offsets.txt", dtype=int)
    else:
        weights = WEIGHTS
        offsets = BIAS_OFFSETS
        rates = np.zeros((len(offsets), len(weights)))
        counts = np.zeros(rates.shape, dtype=int)
        time_periods = np.zeros(rates.shape)
        for w_idx, weight in enumerate(weights):
            net_out = build_net(weight)
            set_analog()
            for o_idx, offset in enumerate(offsets):
                rate, count, time_period = measure_output_rate(net_out, offset)
                rates[o_idx, w_idx] = rate
                counts[o_idx, w_idx] = count
                time_periods[o_idx, w_idx] = time_period

    plot_rates(rates, weights, offsets)
    if not use_saved_data:
        np.savetxt(DATA_DIR + "rates.txt", rates)
        np.savetxt(DATA_DIR + "weights.txt", weights)
        np.savetxt(DATA_DIR + "offsets.txt", offsets, fmt="%d")
    print_data(rates, weights, offsets, counts, time_periods)
    plt.show()

if __name__ == "__main__":
    check_max_output_spike_rates(parse_args())
