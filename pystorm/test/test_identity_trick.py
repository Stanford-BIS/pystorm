import numpy as np
import matplotlib.pyplot as plt
import time

from pystorm.hal import HAL
from pystorm.hal.hal import parse_hal_spikes, parse_hal_binned_tags
HAL = HAL()

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

from multi_pool_util import (
    collect_spikes_and_tags, run_big_raw_spikes_pool, reshape_big_pool_spikes, 
    reconstruct_many_pool_spikes, make_XY_rate_comparison_plots)

np.random.seed(0)

# tolerated mean pct error in spike rate difference between raw spikes and identity trick
# spikes are noisy, and the raw spikes are inherently messed up, so this is a big number
ERROR_TOLERANCE = .005

BIAS_LEVEL = -1 # bias level for neurons
COLLECT_TIME_RAW = 8 # seconds
COLLECT_TIME_IDENTITY = 2 # seconds
FCLIP = 100 # Hz, for the plots

# dimensions of identity trick pools
WIDTH = 16
HEIGHT = 16
NUM_POOLS = 16
N = WIDTH * HEIGHT

# dimensions of raw spikes pools
WIDTH_ALL = 64
HEIGHT_ALL = 64
N_ALL = WIDTH_ALL * HEIGHT_ALL

def run_many_identity_trick_pools():
    ps = []
    os = []
    many_parsed_spikes = {}
    for active_n in range(NUM_POOLS):
        net = graph.Network("net")

        decoders = np.eye(N)
        biases = BIAS_LEVEL * np.ones((N,), dtype=int)

        zero_encs = np.zeros((N, 1))
        for n in range(NUM_POOLS):
            p = net.create_pool("p" + str(n), zero_encs, biases=biases)
            if active_n == n:
                b = net.create_bucket("b" + str(n), N)
                o = net.create_output("o" + str(n), N)
                net.create_connection("p_to_b" + str(n), p, b, decoders)
                net.create_connection("b_to_o" + str(n), b, o, None)
                ps.append(p)
                os.append(o)

        print("calling map for identity trick pool", active_n)
        HAL.map(net)

        _, this_pool_many_parsed_spikes = collect_spikes_and_tags(COLLECT_TIME_IDENTITY, "for identity trick pool " + str(active_n))
        
        for o in this_pool_many_parsed_spikes:
            print(o)
            assert(ps[-1] not in many_parsed_spikes)
            many_parsed_spikes[ps[-1]] = this_pool_many_parsed_spikes[o] # replace o with p!

    return many_parsed_spikes, ps

def test_identity_trick():
    ###########################################
    # misc driver parameters
    downstream_time_res = 10000 # ns
    upstream_time_res = 1000000 # ns

    HAL.set_time_resolution(downstream_time_res, upstream_time_res)

    ###########################################
    # first map the entire array as one pool
    # use spike outputs, as ground truth
    # should get spiking with basic DAC settings
    all_encs = np.zeros((N_ALL, 1), dtype=int)
    all_parsed_spikes, p_all = run_big_raw_spikes_pool(all_encs, BIAS_LEVEL, COLLECT_TIME_RAW)

    ###########################################
    # now create multi-pool network
    # remap as many times as there are pools, use decode on one each time
    many_parsed_spikes, ps_many = run_many_identity_trick_pools()

    ###########################################
    # parse data into 2D arrays
    all_parsed_xy = reshape_big_pool_spikes(all_parsed_spikes, WIDTH_ALL, HEIGHT_ALL, COLLECT_TIME_RAW)
    many_parsed_xy = reconstruct_many_pool_spikes(many_parsed_spikes, WIDTH, HEIGHT, WIDTH_ALL, HEIGHT_ALL, COLLECT_TIME_IDENTITY)

    ###########################################
    # plot
    pct_err = make_XY_rate_comparison_plots("identity_trick", all_parsed_xy, many_parsed_xy, FCLIP)

    ###########################################
    # ensure matching within tolerance
    mean_err = np.mean(pct_err)
    print("mean error was", mean_err)
    if mean_err >= ERROR_TOLERANCE:
        print("  >= tolerance of ", ERROR_TOLERANCE)
    assert(mean_err < ERROR_TOLERANCE)

if __name__ == "__main__":
    test_identity_trick()
