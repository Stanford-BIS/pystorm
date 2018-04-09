"""2D version of test_hal_io"""
import time
import numpy as np

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.hal.hal import parse_hal_binned_tags

from pystorm.calibration.utils.exp import clear_outputs

DIM = 2 # 1 dimensional
RATE = 10000 # rate of input spikes
WEIGHTS = np.eye(DIM) # weight of connection from input to output
REL_ERROR_TOLERANCE = 0.01  # tolerable relative error
RUN_TIME = 1. # time to sample
SLOP_TIME = 0.3 # time to wait for traffic to clear
RUN_TIME_NS = int(RUN_TIME*1E9)
SLOP_TIME_NS = int(SLOP_TIME*1E9)

TEST_VECTORS = [
    [RATE, RATE],
    [0, RATE],
    [RATE, RATE],
    [RATE, 0],
    [RATE, RATE],
]

def toggle_hal(hal, net_inputs, dims, rates):
    """Turn the inputs on and off via HAL"""
    hal.start_traffic(flush=False)
    hal.enable_output_recording(flush=True)
    cur_time = hal.get_time()
    hal.set_input_rates(net_inputs, dims, rates, time=cur_time + SLOP_TIME_NS, flush=False)
    hal.set_input_rates(
        net_inputs, dims, [0 for _ in range(DIM)],
        time=cur_time+ SLOP_TIME_NS + RUN_TIME_NS, flush=True)
    time.sleep(2*SLOP_TIME + RUN_TIME)
    hal.stop_traffic(flush=False)
    hal.disable_output_recording(flush=True)

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
    hal = HAL()
    hal.map(net)

    inputs = [net_input for _ in range(DIM)]
    dims = [dim for dim in range(DIM)]
    for input_rates in TEST_VECTORS:
        clear_outputs(hal, SLOP_TIME)
        toggle_hal(hal, inputs, dims, input_rates)
        dim_binned_tags = parse_hal_binned_tags(hal.get_outputs())[net_output]
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
