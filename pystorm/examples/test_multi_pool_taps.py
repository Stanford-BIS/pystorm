import numpy as np
import matplotlib.pyplot as plt
import time

from pystorm.hal import HAL
from pystorm.hal.hal import parse_hal_spikes
HAL = HAL()

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

np.random.seed(0)

###########################################
# pool size parameters

width = 16
height = 16
N = width * height
Din = 1

num_pools = 16

width_all = 64
height_all = 64
N_all = width_all * height_all

fin = 1000
collect_time = .5

###########################################
# misc driver parameters
downstream_time_res = 10000 # ns
upstream_time_res = 1000000 # ns

HAL.set_time_resolution(downstream_time_res, upstream_time_res)

###########################################
# tap point specification

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

###########################################

def collect_data(inps, rate):

    # drive some inputs to see difference in taps
    for i in inps:
        HAL.set_input_rate(i, 0, rate, 0, True)

    # flush anything not collected
    _ = HAL.get_spikes()

    print("starting data collection")
    HAL.start_traffic(flush=False)
    HAL.enable_spike_recording(flush=True)

    time.sleep(collect_time)

    print("stopping data collection")
    HAL.stop_traffic(flush=False)
    HAL.disable_spike_recording(flush=True)

    spikes = HAL.get_spikes()
    parsed_spikes = parse_hal_spikes(spikes)
    return parsed_spikes

###########################################
# first map the entire array as one pool
net = graph.Network("net")

# some misc encoders

assert(Din == 1) # this doesn't work otherwise

small_tap_matrix = make_taps(width, height, Din).flatten()
small_tap_matrix_xy = small_tap_matrix.reshape((height, width))

all_encs_xy = np.zeros((height_all, width_all))
for y_base in range(0, width_all, width):
    for x_base in range(0, width_all, width):
        all_encs_xy[y_base:y_base + height, x_base:x_base + width] = small_tap_matrix_xy
all_encs = all_encs_xy.reshape((N_all, Din))

i_all = net.create_input("i_all", Din)
p_all = net.create_pool("p_all", all_encs)
net.create_connection("i_to_p_all", i_all, p_all, None)

# map network
print("calling map for all neurons")
HAL.map(net)
HAL.driver.SetDACCount(0 , bd.bdpars.BDHornEP.DAC_SYN_PD      , 20)
HAL.driver.SetDACCount(0 , bd.bdpars.BDHornEP.DAC_DIFF_G      , 1024)
HAL.driver.SetDACCount(0 , bd.bdpars.BDHornEP.DAC_DIFF_R      , 10)


all_off_parsed_spikes = collect_data([i_all], 0)
all_parsed_spikes = collect_data([i_all], fin)

###########################################
# create multi-pool network

net = graph.Network("net")

tap_matrix = make_taps(width, height, Din)

ps = []
inps = []
for n in range(num_pools):
    i = net.create_input("i" + str(n), Din)
    p = net.create_pool("p" + str(n), tap_matrix)
    net.create_connection("c" + str(n), i, p, None)
    inps.append(i)
    ps.append(p)

# map network
print("calling map")
HAL.map(net)
HAL.driver.SetDACCount(0 , bd.bdpars.BDHornEP.DAC_SYN_PD      , 20)
HAL.driver.SetDACCount(0 , bd.bdpars.BDHornEP.DAC_DIFF_G      , 1024)
HAL.driver.SetDACCount(0 , bd.bdpars.BDHornEP.DAC_DIFF_R      , 10)

many_parsed_spikes = collect_data(inps, fin)
many_off_parsed_spikes = collect_data(inps, 0)

######################################

# get many pools data into same frame as big pool data

def all_to_array(all_parsed_spikes):
    all_parsed_arr = np.zeros((N_all,))
    for n in range(N_all):
        if n in all_parsed_spikes[p_all]:
            all_parsed_arr[n] = len(all_parsed_spikes[p_all][n])
    all_parsed_xy = all_parsed_arr.reshape((height_all, width_all))
    return all_parsed_xy

all_parsed_xy = all_to_array(all_parsed_spikes)
all_off_parsed_xy = all_to_array(all_off_parsed_spikes)

###########

def many_to_array(many_parsed_spikes):
    many_parsed_xy = np.zeros((height_all, width_all))
    for p, spikes in many_parsed_spikes.items():
        this_parsed_arr = np.zeros((N,))
        for n in range(N):
            if n in spikes:
                this_parsed_arr[n] = len(spikes[n])
        this_p_parsed_xy = this_parsed_arr.reshape((height, width))
        pool_x, pool_y = p.mapped_xy
        many_parsed_xy[pool_y:pool_y + height, pool_x:pool_x + width] = this_p_parsed_xy
    return many_parsed_xy

many_parsed_xy = many_to_array(many_parsed_spikes)
many_off_parsed_xy = many_to_array(many_off_parsed_spikes)

###########

#def scale(x):
#    return np.log10(x + 1)
def scale(x):
    return x

vmin = np.min(scale(many_parsed_xy))
vmax = np.max(scale(many_parsed_xy))

plt.figure(figsize=(10,10))
plt.title("spikes for whole array")
plt.imshow(scale(all_parsed_xy), vmin=vmin, vmax=vmax)
plt.colorbar()
plt.savefig("big_pool_taps.png")

plt.figure(figsize=(10,10))
plt.title("spikes for array reconstructed from pools")
plt.imshow(scale(many_parsed_xy), vmin=vmin, vmax=vmax)
plt.colorbar()
plt.savefig("multi_pool_taps.png")

plt.figure(figsize=(10,10))
plt.title("diff on/off spikes for whole array")
plt.imshow(scale(all_parsed_xy - all_off_parsed_xy))
plt.colorbar()
plt.savefig("big_pool_on_off_diff_taps.png")

plt.figure(figsize=(10,10))
plt.title("diff on/off spikes for array_reconstructed from pools")
plt.imshow(scale(many_parsed_xy - many_off_parsed_xy))
plt.colorbar()
plt.savefig("multi_pool_on_off_diff_taps.png")

