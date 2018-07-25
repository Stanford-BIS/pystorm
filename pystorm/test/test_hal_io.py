"""Test whether we can inject and tags and loop them back"""
import time
import numpy as np

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

DIM = 1 # 1 dimensional
RATE = 10000 # rate of input spikes
WEIGHT = 1 # weight of connection from input to output
REL_ERROR_TOLERANCE = 0.01  # tolerable relative error
RUN_TIME = 2. # time to sample
SLOP_TIME = 0.2 # time for traffic to flush
RUN_TIME_NS = int(RUN_TIME*1E9)
SLOP_TIME_NS = int(SLOP_TIME*1E9)

# import logging
# logging.basicConfig(level=logging.DEBUG)

def test_hal_io():
    """Perform the test"""

    net = graph.Network("net")
    net.input = net.create_input("i", DIM)
    bucket = net.create_bucket("b", DIM)
    net.output = net.create_output("o", DIM)
    net.create_connection("i_to_b", net.input, bucket, WEIGHT)
    net.create_connection("b_to_o", bucket, net.output, None)
    hal = HAL()
    hal.map(net)

    hal.start_traffic(flush=False)
    hal.disable_spike_recording(flush=False)
    hal.enable_output_recording(flush=True)
    cur_time = hal.get_time()
    hal.set_input_rate(net.input, 0, RATE, time=cur_time+SLOP_TIME_NS, flush=False)
    hal.set_input_rate(net.input, 0, 0, time=cur_time+SLOP_TIME_NS+RUN_TIME_NS, flush=True)
    time.sleep(2*SLOP_TIME + RUN_TIME)
    hal.stop_traffic(flush=False)
    hal.disable_output_recording(flush=True)
    binned_tags = hal.get_outputs()

    nonzero_idx = np.nonzero(binned_tags[:, 3])[0]
    binned_tags = binned_tags[nonzero_idx]
    measured_time = (binned_tags[-1, 0] - binned_tags[0, 0])/1e9
    total_tags = np.sum(binned_tags[:, 3])
    measured_rate = total_tags/measured_time
    relative_error = np.abs(measured_rate-RATE)/RATE
    print("hal_test_io measured {:.2%}% relative error between ".format(relative_error) +
          "target rate {} and measured rate {:.2f}".format(RATE, measured_rate))
    assert relative_error < REL_ERROR_TOLERANCE, (
        "\tExceeded relative error tolerance in test_hal_io\n" +
        "\t\tExpected output rate to be within {:.2%}% of input rate, ".format(
            REL_ERROR_TOLERANCE) +
        "\t\tbut relative error was {:.2%}%".format(relative_error))

if __name__ == "__main__":
    test_hal_io()
