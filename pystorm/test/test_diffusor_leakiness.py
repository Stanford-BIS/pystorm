import numpy as np
import matplotlib.pyplot as plt
import time

from pystorm.hal import HAL
HAL = HAL()

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

np.random.seed(0)

###########################################
# pool size parameters

# the whole chip, why not
width = 64
height = 64

tile_height = 4
tile_width = 4
tiles_x = width // tile_width
tiles_y = height // tile_height

N = width * height

CORE_ID = 0

###########################################
# calibration parameters

BIAS = 1
TCOLLECT = 10 # how long to measure spikes
FMAX = 5000

###########################################
# misc driver parameters

downstream_time_res = 10000 # ns
upstream_time_res = 1000000 # ns

HAL.set_time_resolution(downstream_time_res, upstream_time_res)


###########################################
# main loop
# the diffusor cuts 4x4 patches

# we're going to look at the activity of an tile when its adjacent tile has:
# 1. all synapses disabled
# 2. synapes enabled
# 3. syapses enabled and driven

def open_all_diff_cuts():
    # connect diffusor around pools
    for tile_id in range(256):
        HAL.driver.OpenDiffusorAllCuts(CORE_ID, tile_id)

def count_spikes_by_tile(spikes):
    cts = np.zeros((tiles_y, tiles_x), dtype=int)
    for t, p, n in spikes:
        # discard p, just one pool
        y = n // width
        x = n % height
        ty = y // tile_height
        tx = x // tile_width
        cts[ty, tx] += 1 
    return cts

def count_spikes_by_nrn(spikes):
    cts = np.zeros((height, width), dtype=int)
    for t, p, n in spikes:
        # discard p, just one pool
        y = n // width
        x = n % height
        cts[y, x] += 1 
    return cts

def collect_data_once(taps, biases, input_fmax):

    net = graph.Network("net")

    pool = net.create_pool("pool", taps, biases=biases)

    inp = net.create_input("input", 1)
    net.create_connection("i_to_p", inp, pool, None)

    # don't bother with identity trick, flaky spikes from get_spikes are fine

    # map network
    print("calling map")
    HAL.map(net)

    # diffusor cut everywhere
    open_all_diff_cuts()

    # fiddle with diffusor spread
    # the ratio of DAC_DIFF_G / DAC_DIFF_R controls the diffusor spread
    # lower ratio is more spread out
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_DIFF_G      , 128)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_DIFF_R      , 1024)

    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SOMA_REF    , 1024)

    print("starting data collection")
    HAL.start_traffic(flush=False)
    HAL.enable_spike_recording(flush=True)

    HAL.set_input_rate(inp, 0, input_fmax, time=0) 

    time.sleep(TCOLLECT) # rough timing

    HAL.stop_traffic(flush=False)
    HAL.disable_spike_recording(flush=True)
    print("done collecting data")

    spikes = HAL.get_spikes()
    tile_cts = count_spikes_by_tile(spikes)
    nrn_cts = count_spikes_by_nrn(spikes)
    print("done counting spikes")

    return tile_cts, nrn_cts

def xy_to_nrn_idx(x, y):
    return y * width + x

################################
# run exp
    
taps_off = [[]]

taps_on = [[]]
syns_xy = [(0, 0),
           (0, 2),
           (2, 0),
           (2, 2)]
for x, y in syns_xy:
    n_idx = xy_to_nrn_idx(x, y)
    taps_on[0].append((n_idx, 1)) # sign doesn't matter, we're not sending input

taps_on = (N, taps_on) 
taps_off = (N, taps_off) 

tile_cts_off, nrn_cts_off = collect_data_once(taps_off, BIAS, 0)

tile_cts_on, nrn_cts_on = collect_data_once(taps_on, BIAS, 0)

tile_cts_driven, nrn_cts_driven = collect_data_once(taps_on, BIAS, FMAX)

fig, axes = plt.subplots(3, 2, figsize=(10, 15))

PLT_RANGE = 8
off_clipped = nrn_cts_off[:PLT_RANGE, :PLT_RANGE] / TCOLLECT
on_clipped = nrn_cts_on[:PLT_RANGE, :PLT_RANGE] / TCOLLECT
driven_clipped = nrn_cts_driven[:PLT_RANGE, :PLT_RANGE] / TCOLLECT
vmax = np.max(driven_clipped)

def make_imshow_row(ax_row, data, vmax, to_sub, titles):
    ax = axes[ax_row, 0]
    im = ax.imshow(data, vmin=0, vmax=vmax)
    fig.colorbar(im, ax=ax)
    ax.set_title(titles[0])

    ax = axes[ax_row, 1]
    diff = data - to_sub 
    diff[:4, :4] = 0
    im = ax.imshow(diff, vmin=0, vmax=np.max(diff))
    fig.colorbar(im, ax=ax)
    ax.set_title(titles[1])

make_imshow_row(0, off_clipped, vmax, off_clipped, ['top-left synapses disabled', '(nothing)'])
make_imshow_row(1, on_clipped, vmax, off_clipped, ['top-left synapses enabled', '(1, 0) - (0, 0)'])
make_imshow_row(2, driven_clipped, vmax, off_clipped, ['top-left synapses driven', '(2, 0) - (0, 0)'])

plt.savefig('test_diffusor_leakiness.png')
