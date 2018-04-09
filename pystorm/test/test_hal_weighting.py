"""test whether tags injected into accumulator for weighting behave as expected"""
import time
import numpy as np

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.hal.hal import parse_hal_binned_tags, bin_tags_spikes
HAL = HAL()

# test parameters
RATE = 10000 # rate of input spikes
REL_ERROR_THRESHOLD = 0.01  # tolerable relative error
RUN_TIME = 5. # time to sample each weight
WEIGHTS = np.array([1., 0.5, 0.25, -0.25, -0.5, -1.0]) # weights to test
DIM = 1

def build_net(weight):
    """Network that forwards input to output with weighting"""
    net = graph.Network("net")
    net.input = net.create_input("i", DIM)
    bucket = net.create_bucket("b", DIM)
    net.output = net.create_output("o", DIM)
    net.create_connection("c_p1_to_b1", net.input, bucket, weight)
    net.create_connection("c_b1_to_o1", bucket, net.output, None)
    HAL.map(net)
    return net

def toggle_recording(net, sleep_time):
    """Start and stop HAL traffic

    Parameters
    ----------
    sleep_time: float
    """
    n_spurious_tags = len(HAL.get_outputs())
    assert n_spurious_tags == 0, (
        "no tags expected before hal output recording enabled. Got {} tags".format(
            n_spurious_tags))
    n_spurious_spikes = len(HAL.get_spikes())
    assert n_spurious_spikes == 0, (
        "no spikes expected before spike recording enabled. Got {} spikes".format(
            n_spurious_spikes))
    HAL.start_traffic(flush=False)
    HAL.disable_spike_recording(flush=False)
    HAL.enable_output_recording(flush=True)
    start_time = HAL.get_time()
    HAL.set_input_rate(net.input, 0, RATE)
    time.sleep(sleep_time)
    stop_time = HAL.get_time()
    HAL.set_input_rate(net.input, 0, 0)
    HAL.stop_traffic(flush=False)
    HAL.disable_spike_recording(flush=False)
    HAL.disable_output_recording(flush=True)
    return start_time, stop_time

def test_hal_weighting():
    """Perform the test"""
    for weight in WEIGHTS:
        net = build_net(weight)
        start, stop = toggle_recording(net, RUN_TIME)
        tags = parse_hal_binned_tags(HAL.get_outputs())
        rate_out = bin_tags_spikes(tags, [start, stop])[net.output][:][0][0]
        observed_weight = rate_out / RATE
        relative_error = np.abs((weight-observed_weight)/weight)
        print("\nMeasured relative error of {:.2%} for weight of {}\n".format(
            relative_error, weight))
        assert np.all(relative_error < REL_ERROR_THRESHOLD), (
            "Relative error on weights exceeds threshold of {:.2%}. ".format(REL_ERROR_THRESHOLD) +
            "For weight of {}, detected relative error of {:.2%}".format(weight, relative_error))
        # flush any remaining traffic
        HAL.get_outputs()
        HAL.get_spikes()

if __name__ == "__main__":
    test_hal_weighting()
