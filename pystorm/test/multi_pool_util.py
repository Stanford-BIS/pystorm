import numpy as np
import matplotlib.pyplot as plt
import time

from pystorm.hal import HAL
from pystorm.hal.hal import parse_hal_spikes, parse_hal_binned_tags
HAL = HAL()

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

def collect_spikes_and_tags(collect_time, info_str=""):
    _ = HAL.get_spikes()
    _ = HAL.get_outputs()

    print("starting data collection " + info_str)
    HAL.start_traffic(flush=False)
    HAL.enable_output_recording(flush=False)
    HAL.enable_spike_recording(flush=True)

    time.sleep(collect_time)

    print("  stopping data collection")
    HAL.stop_traffic(flush=False)
    HAL.disable_output_recording(flush=False)
    HAL.disable_spike_recording(flush=True)

    spikes = HAL.get_spikes()
    parsed_spikes = parse_hal_spikes(spikes)

    outputs = HAL.get_outputs()
    parsed_outputs = parse_hal_binned_tags(outputs)

    return parsed_spikes, parsed_outputs

def run_big_raw_spikes_pool(encs, bias_level, collect_time, use_inp=False):
    net = graph.Network("net")

    # some misc encoders

    N = encs.shape[0]
    biases = bias_level * np.ones((N,), dtype=int)

    p_all = net.create_pool("p_all", encs, biases=biases)
    if use_inp:
        i_all = net.create_input("i_all", 1)
        net.create_connection("", i_all, p_all, None)

    # map network
    print("calling map for all neurons")
    HAL.map(net)

    spikes, _ = collect_spikes_and_tags(collect_time, "for big raw spikes pool")

    if use_inp:
        return spikes, p_all, i_all
    else:
        return spikes, p_all

def run_many_raw_spike_pools(num_pools, encs, bias_level, collect_time, use_inp=False):
    net = graph.Network("net")

    N = encs.shape[0]
    biases = bias_level * np.ones((N,), dtype=int)

    ps = []
    inps = []
    for n in range(num_pools):
        p = net.create_pool("p" + str(n), encs, biases=biases)
        ps.append(p)
        if use_inp:
            i = net.create_input("i" + str(n), 1)
            net.create_connection("", i, p, None)
            inps.append(i)

    # map network
    print("calling map")
    HAL.map(net)

    spikes, _ = collect_spikes_and_tags(collect_time, "for big raw spikes pool")

    if use_inp:
        return spikes, ps, inps
    else:
        return spikes, ps

def reshape_big_pool_spikes(all_parsed_spikes, width, height, collect_time):
    # reshape raw spike data
    N_all = width * height
    all_parsed_arr = np.zeros((N_all,))
    assert(len(all_parsed_spikes) == 1) # should just have p_all
    for k, spikes in all_parsed_spikes.items():
        for n in range(N_all):
            if n in spikes:
                all_parsed_arr[n] = sum([el[1] for el in spikes[n]]) # els are (time, ct)

    all_parsed_xy = all_parsed_arr.reshape((height, width)) / collect_time
    return all_parsed_xy


def reconstruct_many_pool_spikes(many_parsed_spikes, width, height, height_all, width_all, collect_time):
    N = width * height
    # reconstruct many pool spikes into one dataset
    many_parsed_xy = np.zeros((height_all, width_all))
    for p, spikes in many_parsed_spikes.items():
        this_parsed_arr = np.zeros((N,))
        for n in range(N):
            if n in spikes:
                this_parsed_arr[n] = sum([el[1] for el in spikes[n]]) # els are (time, ct)
        this_p_parsed_xy = this_parsed_arr.reshape((height, width))
        pool_x, pool_y = p.mapped_xy
        many_parsed_xy[pool_y:pool_y + height, pool_x:pool_x + width] = this_p_parsed_xy / collect_time
    return many_parsed_xy

def make_XY_rate_comparison_plots(fname, all_parsed_xy, many_parsed_xy, fclip):
    plt.figure()
    plt.title("spike rate histogram")
    plt.hist(all_parsed_xy, bins=20)

    print("clipping", np.sum(all_parsed_xy > fclip), "very fast neurons")
    all_parsed_xy[all_parsed_xy > fclip] = fclip
    many_parsed_xy[many_parsed_xy > fclip] = fclip

    def scale(x):
        #return np.log10(x + 1)
        return x

    vmin = np.min(scale(all_parsed_xy))
    vmax = np.max(scale(all_parsed_xy))

    plt.figure(figsize=(10,10))
    plt.subplot(221)
    plt.title("rates for whole array")
    plt.imshow(scale(all_parsed_xy), vmin=vmin, vmax=vmax)
    plt.colorbar()

    plt.subplot(223)
    plt.title("rates for array reconstructed from pools")
    plt.imshow(scale(many_parsed_xy), vmin=vmin, vmax=vmax)
    plt.colorbar()

    plt.subplot(222)
    pct_err = np.abs(all_parsed_xy - many_parsed_xy) / fclip
    plt.title("error fraction in terms of max spike rate")
    plt.imshow(pct_err)
    plt.colorbar()
    plt.savefig(fname + ".png")

    return pct_err
