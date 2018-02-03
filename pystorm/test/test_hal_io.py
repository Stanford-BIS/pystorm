"""test whether we can inject and tags and loop them back
"""
import time
import numpy as np

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

def test_hal_io():
    """Perform the test"""
    DIM = 1 # 1 dimensional
    RATE = 1000 # rate of input spikes
    WEIGHT = 1 # weight of connection from input to output
    REL_ERROR_TOLERANCE = 0.01  # tolerable relative error
    RUN_TIME = 5. # time to sample

    net = graph.Network("net")
    net.input = net.create_input("i", DIM)
    bucket = net.create_bucket("b", DIM)
    net.output = net.create_output("o", DIM)
    net.create_connection("i_to_b", net.input, bucket, WEIGHT)
    net.create_connection("b_to_o", bucket, net.output, None)
    HAL.map(net)

    HAL.start_traffic(flush=False)
    HAL.disable_spike_recording(flush=False)
    HAL.enable_output_recording(flush=True)
    HAL.set_input_rate(net.input, 0, RATE, time=0, flush=True)
    time.sleep(RUN_TIME)
    HAL.set_input_rate(net.input, 0, 0, time=int(RUN_TIME*1e9), flush=True)
    HAL.stop_traffic(flush=False)
    HAL.disable_spike_recording(flush=False)
    HAL.disable_output_recording(flush=True)
    binned_tags = HAL.get_outputs()

    measured_time = (binned_tags[-1, 0] - binned_tags[0, 0])/1e9
    total_tags = np.sum(binned_tags[:, 3])
    measured_rate = total_tags/measured_time
    relative_error = np.abs(measured_rate-RATE)/RATE
    print("hal_test_io measured {:.2%}% relative error between ".format(relative_error) +
          "target rate {} and measured rate {:.2}".format(RATE, measured_rate))
    assert relative_error < REL_ERROR_TOLERANCE, (
        "\tExceeded relative error tolerance in test_hal_io\n" +
        "\t\tExpected output rate to be within {:.2%}% of input rate, ".format(
            REL_ERROR_TOLERANCE) +
        "\t\tbut relative error was {:.2%}%".format(relative_error))

if __name__ == "__main__":
    test_hal_io()
