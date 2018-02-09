"""Check the accuracy of the input rates

Set up the following traffic flow:
spike generator -> accumulator (weight 1) -> fpga -> pc

Vary the input spike rates and measure the output spike rates
"""
import os
import time
import argparse
import numpy as np
import matplotlib.pyplot as plt

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from utils import load_txt_data

DIM = 1 # 1 dimensional
WEIGHT = 1 # weight of connection from input to output
RUN_TIME = 5. # time to sample
INTER_RUN_TIME = 0.2 # time between samples

UNIT_PERIOD = HAL.downstream_ns*1E-9
TGT_RATE_MIN = 1000
TGT_RATE_MAX = 100000

FLOAT_TOL = 0.000001 # for handling floating to integer comparisons

DATA_DIR = "./data/" + os.path.basename(__file__)[:-3] + "/"
if not os.path.isdir(DATA_DIR):
    os.makedirs(DATA_DIR, exist_ok=True)

def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Characterize the soma max firing rates')
    parser.add_argument("-r", action="store_true", dest="use_saved_data", help='reuse cached data')
    args = parser.parse_args()
    return args

def compute_base_rates():
    """Compute the rates to test"""
    period_min_rate = 1./TGT_RATE_MIN
    period_max_rate = 1./TGT_RATE_MAX

    periods_min_rate = int(np.ceil(period_min_rate/UNIT_PERIOD))
    periods_max_rate = np.clip(int(np.floor(period_max_rate/UNIT_PERIOD)), 1, None)
    unit_periods = np.arange(periods_max_rate, periods_min_rate)[::-1]
    rates = 1./(unit_periods*UNIT_PERIOD)
    return rates

BASE_RATES = compute_base_rates() # rate of input spikes

def build_net():
    """Build a network for testing"""
    net = graph.Network("net")
    net_input = net.create_input("i", DIM)
    bucket = net.create_bucket("b", DIM)
    net_output = net.create_output("o", DIM)
    net.create_connection("i_to_b", net_input, bucket, WEIGHT)
    net.create_connection("b_to_o", bucket, net_output, None)
    HAL.map(net)
    return net_input

def toggle_hal(net_input, rate):
    """Toggle the spike input and output recording"""
    HAL.set_time_resolution(upstream_ns=100000)
    HAL.start_traffic(flush=False)
    HAL.enable_output_recording(flush=True)
    HAL.set_input_rate(net_input, 0, rate, time=0, flush=True)
    time.sleep(RUN_TIME)
    HAL.set_input_rate(net_input, 0, 0, time=0, flush=True)
    HAL.stop_traffic(flush=False)
    HAL.disable_output_recording(flush=True)
    time.sleep(INTER_RUN_TIME)
    HAL.set_time_resolution(upstream_ns=1000000)

def compute_test_rates():
    """Compute the exact rates to test
    
    HAL takes in integer rates values
    This function handles the floating point to integer issues
    """
    intermediate_rates = BASE_RATES[:-1] + np.diff(BASE_RATES)/2.
    print(BASE_RATES[:10])
    print(intermediate_rates[:10])
    test_rates = np.sort(list(BASE_RATES) + list(intermediate_rates))
    print(test_rates[:10])
    integer_rates = []
    for rate in test_rates:
        rounded_rate = int(np.round(rate))
        if np.abs(rounded_rate - rate) < FLOAT_TOL:
            integer_rates += [rounded_rate-1, rounded_rate, rounded_rate+1]
        else:
            integer_rates += [int(rate), int(rate)+1]
    integer_rates = np.array(integer_rates)
    integer_rates = integer_rates[integer_rates <= TGT_RATE_MAX]
    integer_rates = integer_rates[integer_rates > 0]
    integer_rates = np.unique(integer_rates)
    return integer_rates

def plot_rates(rates, measured_rates):
    """Plot results"""
    fig, axs = plt.subplots(nrows=2, ncols=1, figsize=(8, 12))
    for ax in axs:
        for base_rate in BASE_RATES:
            ax.axvline(base_rate, color="k", linestyle="-", linewidth=0.5, alpha=0.2)
            ax.axhline(base_rate, color="k", linestyle="-", linewidth=0.5, alpha=0.2)
    axs[0].plot(rates, measured_rates, '-o', label="measured rate")
    axs[0].plot([0, rates[-1]], [0, rates[-1]], 'k-', linewidth=1, label="unity")
    axs[0].legend()
    axs[0].set_xlabel("Target Rate (Hz)")
    axs[0].set_ylabel("Measured Rate (Hz)")
    axs[0].set_xlim((0, axs[0].get_xlim()[1]))
    axs[0].set_ylim((0, axs[0].get_ylim()[1]))

    axs[1].loglog(rates, measured_rates, '-o', label="measured rate")
    axs[1].loglog(rates, rates, 'k-', linewidth=1, label="unity")
    axs[1].set_xlabel("Target Rate (Hz)")
    axs[1].set_ylabel("Measured Rate (Hz)")

    fig.savefig(DATA_DIR + "input_rates.pdf")

def check_input_rates(parsed_args):
    """Perform the test"""
    use_saved_data = parsed_args.use_saved_data
    if use_saved_data:
        data = load_txt_data(DATA_DIR + "rates.txt")
        rates = data[:, 0]
        measured_rates = data[:, 1]
    else:
        HAL.disable_spike_recording(flush=True)
        HAL.set_time_resolution(upstream_ns=100000)

        rates = compute_test_rates()
        n_rates = len(rates)
        print("checking {} rates {}".format(n_rates, rates))
        measured_rates = np.zeros(n_rates)
        net_input = build_net()
        for idx, rate in enumerate(rates):
            print("checking rate {}".format(rate))
            toggle_hal(net_input, rate)
            binned_tags = HAL.get_outputs()
            measured_time = (binned_tags[-1, 0] - binned_tags[0, 0])/1e9
            total_tags = np.sum(binned_tags[:, 3])
            measured_rates[idx] = total_tags/measured_time

    plot_rates(rates, measured_rates)

    if not use_saved_data:
        data = np.zeros((len(rates), 2))
        data[:, 0] = rates
        data[:, 1] = measured_rates
        np.savetxt(DATA_DIR + "rates.txt", data)
    plt.show()

if __name__ == "__main__":
    check_input_rates(parse_args())
