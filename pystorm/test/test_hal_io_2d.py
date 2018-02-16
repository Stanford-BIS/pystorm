"""2D version of test_hal_io"""
import time
import numpy as np

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.hal.hal import parse_hal_binned_tags

DIM = 2 # 1 dimensional
RATE = 1000 # rate of input spikes
WEIGHTS = np.eye(DIM) # weight of connection from input to output
REL_ERROR_TOLERANCE = 0.01  # tolerable relative error
RUN_TIME = 1. # time to sample
INTER_RUN_TIME = 0.2 # time unit to wait for traffic to clear

TEST_VECTORS = [
    [RATE, RATE],
    [0, RATE],
    [RATE, RATE],
    [RATE, 0],
    [RATE, RATE],
]

def clear_outputs():
    """Clear any remaining overflow counts"""
    time.sleep(INTER_RUN_TIME)
    outputs = HAL.get_outputs()
    while outputs.shape[0] > 0:
        print(
            "Clearing outputs: Consumed {:d} output bins ".format(outputs.shape[0]) +
            "covering times {:d}ns to {:d}ns ".format(outputs[0, 0], outputs[-1, 0]) +
            "from output objects\n{}\n".format(np.unique(outputs[:, 1])) +
            "from output dimensions {} ".format(np.unique(outputs[:, 2])) +
            "with total bin count {:d}".format(np.sum(outputs[:, 3]))
        )
        time.sleep(INTER_RUN_TIME)
        outputs = HAL.get_outputs()

def toggle_hal(net_inputs, dims, rates):
    """Turn the inputs on and off via HAL"""
    HAL.start_traffic(flush=False)
    HAL.enable_output_recording(flush=True)

    """how things should work"""
    # HAL.set_input_rates(net_inputs, dims, rates, time=0, flush=True)
    # time.sleep(RUN_TIME)
    # HAL.set_input_rates(net_inputs, dims, [0 for _ in range(DIM)], time=0, flush=True)

    """for working around buggy double 0"""
    for net_input, dim, rate in zip(net_inputs, dims, rates):
        if rate > 0:
            HAL.set_input_rate(net_input, dim, rate, time=0, flush=True)
    HAL.set_input_rates(net_inputs, dims, rates, time=0, flush=True)
    time.sleep(RUN_TIME)
    for net_input, dim, rate in zip(net_inputs, dims, rates):
        if rate > 0:
            HAL.set_input_rate(net_input, dim, 0, time=0, flush=True)

    HAL.stop_traffic(flush=False)
    HAL.disable_output_recording(flush=True)
    time.sleep(INTER_RUN_TIME)

class ResultPrinter(object):
    header = False
    @staticmethod
    def print_results(input_rates, measured_rates):
        """Format the output nicely"""
        if not ResultPrinter.header:
            print("")
            print("".join(["dim {:<3d}                   ".format(dim) for dim in range(DIM)]))
            print("input_rate measured_rate  "*DIM)
            ResultPrinter.header = True
        report_str  = "".join(["{:<6d}     {:<8.1f}        ".format(
            input_rates[dim], measured_rates[dim]) for dim in range(DIM)])
        print(report_str)

def test_hal_io_2d():
    """Perform the test"""
    net = graph.Network("net")
    net_input = net.create_input("i", DIM)
    bucket = net.create_bucket("b", DIM)
    net_output = net.create_output("o", DIM)
    net.create_connection("i_to_b", net_input, bucket, WEIGHTS)
    net.create_connection("b_to_o", bucket, net_output, None)
    HAL.map(net)

    inputs = [net_input for _ in range(DIM)]
    dims = [dim for dim in range(DIM)]
    for input_rates in TEST_VECTORS:
        clear_outputs()
        toggle_hal(inputs, dims, input_rates)
        dim_binned_tags = parse_hal_binned_tags(HAL.get_outputs())[net_output]
        measured_times = np.zeros(DIM)
        total_tags = np.zeros(DIM)
        for dim in range(DIM):
            dim_data = np.array(dim_binned_tags[dim])
            nonzero_idx = np.nonzero(dim_data[:, 1])[0]
            if nonzero_idx.shape[0] > 1:
                measured_times[dim] = (
                    dim_data[nonzero_idx][-1, 0] - dim_data[nonzero_idx][0, 0])/1e9
            else:
                measured_times[dim] = (dim_data[-1, 0] - dim_data[0, 0])/1e9
            total_tags[dim] = np.sum(dim_data[:, 1])
        measured_rates = total_tags/measured_times
        errors = measured_rates - input_rates
        ResultPrinter.print_results(input_rates, measured_rates)

        for dim in range(DIM):
            if input_rates[dim] > 0:
                relative_error = np.abs(measured_rates[dim]-input_rates[dim])/input_rates[dim]
                assert relative_error < REL_ERROR_TOLERANCE, (
                    "\tExceeded relative error tolerance in test_hal_io_2d\n" +
                    "\t\tExpected output rate to be within {:.2%}% of input rate, ".format(
                        REL_ERROR_TOLERANCE) +
                    "\t\tbut relative error was {:.2%}%".format(relative_error))
            else:
                assert measured_rates[dim] == 0, (
                    "\tDetected output when no output was expected\n" +
                    "\t\tMeasured output rate was {:.2%}%".format(measured_rates[dim]))

if __name__ == "__main__":
    test_hal_io_2d()
