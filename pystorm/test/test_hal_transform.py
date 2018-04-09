"""test whether tags injected into accumulator for weighting behave as expected"""
import time
import numpy as np

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.hal.hal import parse_hal_binned_tags, bin_tags_spikes
from pystorm.calibration.utils.exp import clear_outputs

# test parameters
RATES = np.array([10000, 10000]) # rates of input spikes
REL_ERROR_THRESHOLD = 0.01  # tolerable relative error
RUN_TIME = 2. # time to sample each weight
SLOP_TIME = 0.3 # time to wait for traffic to clear
RUN_TIME_NS = int(RUN_TIME*1E9)
SLOP_TIME_NS = int(SLOP_TIME*1E9)

TRANSFORMS = [
    np.array([[1., 0.], [0., 1.]]),
    np.array([[0., -0.5], [0.75, 0.]]),
]
DIM = 2

def build_net(transform):
    """Network that forwards input to output with a transform"""
    net = graph.Network("net")
    net_in = net.create_input("i", DIM)
    bucket = net.create_bucket("b", DIM)
    net_out = net.create_output("o", DIM)
    net.create_connection("c_p1_to_b1", net_in, bucket, transform)
    net.create_connection("c_b1_to_o1", bucket, net_out, None)
    hal = HAL()
    hal.map(net)
    hal.disable_spike_recording(flush=False)
    return hal, net_in, net_out

def toggle_hal_tag_traffic(hal, net_in, rates):
    """Toggle HAL tag traffic

    Parameters
    ----------
    net_in: neuromorph.graph.network Input instance
    rate: list of floats
        spike rates for FPGA spike generators
    """
    inputs = [net_in for dim in range(net_in.get_num_dimensions())]
    dims = [dim for dim in range(net_in.get_num_dimensions())]
    hal.start_traffic(flush=False)
    hal.enable_output_recording(flush=True)
    start_time = hal.get_time()
    hal.set_input_rates(inputs, dims, rates, time=0)
    time.sleep(RUN_TIME)
    hal.set_input_rates(inputs, dims, [0 for dim in range(net_in.get_num_dimensions())], time=0)
    stop_time = hal.get_time()
    hal.stop_traffic(flush=False)
    hal.disable_output_recording(flush=True)
    return start_time, stop_time

def test_hal_transform():
    """Perform the test"""
    for transform in TRANSFORMS:
        target_rate_out = np.dot(transform, RATES)
        hal, net_in, net_out = build_net(transform)
        clear_outputs(hal, SLOP_TIME)
        start, stop = toggle_hal_tag_traffic(hal, net_in, RATES)
        hal_outputs = hal.get_outputs()
        tags = parse_hal_binned_tags(hal_outputs)
        measured_rate_out = bin_tags_spikes(tags, [start, stop])[net_out][:, 0]
        relative_error = np.abs((target_rate_out-measured_rate_out)/target_rate_out)
        print("\nMeasured relative errors of\n{}%\nfor transform of\n{}\napplied to\n{}\n".format(
            relative_error*100, transform, RATES))
        assert np.all(relative_error < REL_ERROR_THRESHOLD), (
            "Relative error on transforms exceeds threshold of {:.2}. ".format(
                REL_ERROR_THRESHOLD) +
            "For transform of\n{}\ndetected relative errors of\n{}".format(
                transform, relative_error))
        # flush any remaining traffic
        hal.get_outputs()
        hal.get_spikes()

if __name__ == "__main__":
    test_hal_transform()
