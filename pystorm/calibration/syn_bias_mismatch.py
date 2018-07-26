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

# the whole chip
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

TCOLLECT = 1 # how long to measure spikes
BAD_NRN_THR = .1 # diffusor_box_median (> or <) mean_spike_rate * BAD_NRN_THR determines good/bad

###########################################
# misc driver parameters

downstream_time_res = 10000 # ns
upstream_time_res = 1000000 # ns

HAL.set_time_resolution(downstream_time_res, upstream_time_res)


###########################################
# main loop
# the diffusor cuts 4x4 patches

# we're going to turn on one synapse per tile at a time
# diffusor is set to maximum spread
# in effect, 16 neurons observe roughly the same signal from the synapse

# we have to map 4 times, with 1 of the 4 synapses turned on for each mapping
# others are off and contribute nothing

# the neurons each have their own mismatch, but collectively, 16 should provide some
# picture of each synapses' overall bias

# we will modify the bias twiddle bits on the fly to assess overall bias
# basically, if we can't silence the neurons with a lot of negative bias, 
# the synapse makes too much positive bias
# conversely, if we can't get the neurons to spike with a lot of positive bias,
# the synapse makes too much negative bias

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

def collect_data_once(taps, biases):

    net = graph.Network("net")

    pool = net.create_pool("pool", taps, biases=biases)

    # don't bother with identity trick, flaky spikes from get_spikes are fine

    # map network
    print("calling map")
    HAL.map(net)

    # diffusor closed everywhere
    open_all_diff_cuts()

    # fiddle with diffusor spread
    # the ratio of DAC_DIFF_G / DAC_DIFF_R controls the diffusor spread
    # lower ratio is more spread out
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_DIFF_G      , 128)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_DIFF_R      , 1024)

    # keep em kinda slow
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SOMA_REF    , 2)

    print("starting data collection")
    HAL.start_traffic(flush=False)
    HAL.enable_spike_recording(flush=True)

    time.sleep(TCOLLECT) # rough timing

    HAL.stop_traffic(flush=False)
    HAL.disable_spike_recording(flush=True)
    print("done collecting data")

    spikes = HAL.get_spikes()
    tile_cts = count_spikes_by_tile(spikes)
    nrn_cts = count_spikes_by_nrn(spikes)
    print("done counting spikes")

    return tile_cts, nrn_cts

def compute_tile_medians(nrn_cts):
    med = np.zeros((tiles_y, tiles_x), dtype=int)
    for tile_x in range(tiles_x):
        for tile_y in range(tiles_y):
            ymin = tile_y * tile_height
            xmin = tile_x * tile_width
            ymax = (tile_y + 1) * tile_height
            xmax = (tile_x + 1) * tile_width

            med[tile_y, tile_x] = np.median(nrn_cts[ymin:ymax, xmin:xmax])
    return med

def make_imshow(nrn_cts, idx, bias):
    fig, axes = plt.subplots(2, 2, figsize=(10, 10))
    axes[0,0].imshow(nrn_cts)
    #axes[0].colorbar()

    axes[1,0].set_title('f > mean*.01')
    axes[1,0].imshow(nrn_cts > np.mean(nrn_cts) * .01)

    med = compute_tile_medians(nrn_cts)
    axes[0,1].set_title('med f')
    axes[0,1].imshow(med)

    axes[1,1].set_title('med f > mean*' + str(BAD_NRN_THR))
    axes[1,1].imshow(med > np.mean(nrn_cts) * BAD_NRN_THR)

    plt.savefig('data/syn_mm_nrn_cts_' + str(idx) + '_' + str(bias) + '.png')

# sweep which of the 4 synapses in the tile we use
all_bad_syn = np.zeros((32, 32), dtype=bool)
for i in [0, 1, 2, 3]:
    ###########################################
    # specify network using HAL

    taps = [[]] # one dim, not that it matters
    for tile_x in range(tiles_x):
        for tile_y in range(tiles_y):
            # set syn position in tile based on i
            y = tile_y * tile_height + (i // 2) * 2
            x = tile_x * tile_width + (i % 2) * 2 
            n_idx = y * width + x
            
            taps[0].append((n_idx, 1)) # sign doesn't matter, we're not sending input
    taps = (N, taps) 

    # collect with positive bias, we want people to spike
    bias = 3
    tile_cts, nrn_cts = collect_data_once(taps, bias)
    make_imshow(nrn_cts, i, bias)
    med = compute_tile_medians(nrn_cts)
    bad_syn = med < (np.mean(nrn_cts) * BAD_NRN_THR)

    bias = -3
    tile_cts, nrn_cts = collect_data_once(taps, bias)
    make_imshow(nrn_cts, i, bias)
    med = compute_tile_medians(nrn_cts)
    bad_syn |= (med > np.mean(nrn_cts) * BAD_NRN_THR)

    dx = i % 2
    dy = i // 2
    all_bad_syn[dy::2,dx::2] |= bad_syn

plt.figure()
plt.imshow(all_bad_syn)
plt.savefig('data/all_bad_syn.png')

HAL.add_calibration('synapse', 'high_bias_magnitude', all_bad_syn)
