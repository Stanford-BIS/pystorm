"""test whether tags injected into accumulator for weighting behave as expected"""
import time
import numpy as np

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.hal.hal import parse_hal_binned_tags, bin_tags_spikes

# test parameters
RATES = np.array([1000, -1000]) # rates of input spikes
REL_ERROR_THRESHOLD = 0.01  # tolerable relative error
RUN_TIME = 5. # time to sample each weight
TRANSFORMS = [
    np.array([[1., 0.], [0., 1.]]),
    np.array([[0., -0.5], [0.75, 0.]]),
]
DIM = 2

def build_net(transform):
    """Network that forwards input to output with a transform"""
    net = graph.Network("net")
    net_input = net.create_input("i", DIM)
    bucket = net.create_bucket("b", DIM)
    net.output = net.create_output("o", DIM)
    net.create_connection("c_p1_to_b1", net_input, bucket, transform)
    net.create_connection("c_b1_to_o1", bucket, net.output, None)
    HAL.map(net)
    return net

def toggle_hal_tag_traffic(net, rates, sleep_time):
    """Toggle HAL tag traffic

    Parameters
    ----------
    net: neuromorph.graph.network instance
    rate: list of floats
        spike rates for FPGA spike generators
    sleep_time: float
        time between turning HAL tag traffic on and off
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
    for net_in in net.get_inputs():
        for dim in range(net_in.get_num_dimensions()):
            HAL.set_input_rate(net_in, dim, rates[dim])
    time.sleep(sleep_time)
    stop_time = HAL.get_time()
    for net_in in net.get_inputs():
        for dim in range(net_in.get_num_dimensions()):
            HAL.set_input_rate(net_in, dim, 0)
    HAL.stop_traffic(flush=False)
    HAL.disable_spike_recording(flush=False)
    HAL.disable_output_recording(flush=True)
    return start_time, stop_time

def test_hal_transform():
    """Perform the test"""
    for transform in TRANSFORMS:
        target_rate_out = np.dot(transform, RATES)
        net = build_net(transform)
        start, stop = toggle_hal_tag_traffic(net, RATES, RUN_TIME)
        tags = parse_hal_binned_tags(HAL.get_outputs())
        measured_rate_out = bin_tags_spikes(tags, [start, stop])[net.output][:, 0]
        relative_error = np.abs((target_rate_out-measured_rate_out)/target_rate_out)
        print("\nMeasured relative errors of\n{}%\nfor transform of\n{}\napplied to\n{}\n".format(
            relative_error*100, transform, RATES))
        assert np.all(relative_error < REL_ERROR_THRESHOLD), (
            "Relative error on transforms exceeds threshold of {:.2}. ".format(REL_ERROR_THRESHOLD) +
            "For transform of\n{}\ndetected relative errors of\n{}".format(transform, relative_error))
        # flush any remaining traffic
        HAL.get_outputs()
        HAL.get_spikes()

if __name__ == "__main__":
    test_hal_transform()
