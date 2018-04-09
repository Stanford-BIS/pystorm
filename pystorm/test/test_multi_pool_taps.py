import numpy as np
import matplotlib.pyplot as plt
import time

from pystorm.hal import HAL
from pystorm.hal.hal import parse_hal_spikes, parse_hal_binned_tags
HAL = HAL()

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

from multi_pool_util import *

np.random.seed(0)

# tolerated mean pct error in spike rate difference between raw spikes and identity trick
# spikes are noisy, and the raw spikes are inherently messed up, so this is a big number
ERROR_TOLERANCE = .1

BIAS_LEVEL = -3 # bias level for neurons
COLLECT_TIME_RAW = .5 # seconds, with synapse, the spiking is a lot faster
FCLIP = 1000 # Hz, for the plots
FIN = 1000 # fmax for inputs

# dimensions of identity trick pools
WIDTH = 16
HEIGHT = 16
NUM_POOLS = 16
N = WIDTH * HEIGHT

# dimensions of raw spikes pools
WIDTH_ALL = 64
HEIGHT_ALL = 64
N_ALL = WIDTH_ALL * HEIGHT_ALL

def make_taps(width, height, Din):
    N = width * height
    tap_matrix = np.zeros((N, Din))
    if Din == 1:
        # one synapse per 4 neurons
        for x in range(0, width, 2):
            for y in range(0, height, 2):
                n = y * width + x
                if x < width // 2:
                    tap_matrix[n, 0] = 1
                else:
                    tap_matrix[n, 0] = -1
    else:
        print("need to implement reasonable taps for Din > 1")
        assert(False)
    return tap_matrix


def test_multi_pool_taps():
    ###########################################
    # misc driver parameters
    downstream_time_res = 10000 # ns
    upstream_time_res = 1000000 # ns

    HAL.set_time_resolution(downstream_time_res, upstream_time_res)

    ###########################################
    # first map the entire array as one pool
    # use spike outputs, as ground truth
    # should get spiking with basic DAC settings

    # build big taps out of a bunch of small taps
    small_tap_matrix = make_taps(WIDTH, HEIGHT, 1).flatten()
    small_tap_matrix_xy = small_tap_matrix.reshape((HEIGHT, WIDTH))

    all_encs_xy = np.zeros((HEIGHT_ALL, WIDTH_ALL))
    for y_base in range(0, WIDTH_ALL, WIDTH):
        for x_base in range(0, WIDTH_ALL, WIDTH):
            all_encs_xy[y_base:y_base + HEIGHT, x_base:x_base + WIDTH] = small_tap_matrix_xy
    all_encs = all_encs_xy.reshape((N_ALL, 1))

    all_parsed_spikes, p_all, i_all = run_big_raw_spikes_pool(all_encs, BIAS_LEVEL, COLLECT_TIME_RAW, use_inp=True)

    # run again with input
    HAL.set_input_rate(i_all, 0, FIN)
    all_parsed_spikes_on, _ = collect_spikes_and_tags(COLLECT_TIME_RAW, "for big raw spikes pool with input on")

    ###########################################
    # now create multi-pool network
    # remap as many times as there are pools, use decode on one each time
    small_encs = make_taps(WIDTH, HEIGHT, 1)
    many_parsed_spikes, ps_many, is_many = run_many_raw_spike_pools(NUM_POOLS, small_encs, BIAS_LEVEL, COLLECT_TIME_RAW, use_inp=True)

    # run again with input
    for i in is_many:
        HAL.set_input_rate(i, 0, FIN)
    many_parsed_spikes_on, _ = collect_spikes_and_tags(COLLECT_TIME_RAW, "for many pools with input on")

    ###########################################
    # parse data into 2D arrays
    all_parsed_xy = reshape_big_pool_spikes(all_parsed_spikes, WIDTH_ALL, HEIGHT_ALL, COLLECT_TIME_RAW)
    all_parsed_xy_on = reshape_big_pool_spikes(all_parsed_spikes_on, WIDTH_ALL, HEIGHT_ALL, COLLECT_TIME_RAW)

    many_parsed_xy = reconstruct_many_pool_spikes(many_parsed_spikes, WIDTH, HEIGHT, WIDTH_ALL, HEIGHT_ALL, COLLECT_TIME_RAW)
    many_parsed_xy_on = reconstruct_many_pool_spikes(many_parsed_spikes_on, WIDTH, HEIGHT, WIDTH_ALL, HEIGHT_ALL, COLLECT_TIME_RAW)

    ###########################################
    # plot difference of input on and input off

    diff_many = many_parsed_xy - many_parsed_xy_on
    diff_all = all_parsed_xy - all_parsed_xy_on
    pct_err = make_XY_rate_comparison_plots("multi_pool_taps", diff_all, diff_many, FCLIP)

    ###########################################
    # ensure matching within tolerance
    mean_err = np.mean(pct_err)
    print("mean error was", mean_err)
    if mean_err >= ERROR_TOLERANCE:
        print("  >= tolerance of ", ERROR_TOLERANCE)
    assert(mean_err < ERROR_TOLERANCE)

if __name__ == "__main__":
    test_multi_pool_taps()
