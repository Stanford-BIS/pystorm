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

collect_time = 30

###########################################
# misc driver parameters
downstream_time_res = 10000 # ns
upstream_time_res = 1000000 # ns

HAL.set_time_resolution(downstream_time_res, upstream_time_res)

###########################################
# tap point specification

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

###########################################
# first map the entire array as one pool
net = graph.Network("net")

# some misc encoders

all_encs = np.zeros((N_all, Din))
#all_encs[0, 0] = 1
#all_encs[2, 0] = 1

p_all = net.create_pool("p_all", all_encs)

# map network
print("calling map for all neurons")
HAL.map(net)

# should get some spiking with basic DAC settings

print("starting data collection for big pool")
HAL.start_traffic(flush=False)
HAL.enable_spike_recording(flush=True)

time.sleep(collect_time)

print("stopping data collection")
HAL.stop_traffic(flush=False)
HAL.disable_spike_recording(flush=True)

all_spikes = HAL.get_spikes()
all_parsed_spikes = parse_hal_spikes(all_spikes)

###########################################
# now create multi-pool network

net = graph.Network("net")

#decoders = np.zeros((Dout, N))

zero_encs = np.zeros((N, Din))
ps = []
for n in range(num_pools):
    p = net.create_pool("p" + str(n), zero_encs)
    ps.append(p)

# map network
print("calling map")
HAL.map(net)

print("starting data collection for many pools")
_ = HAL.get_spikes() # throw away
HAL.start_traffic(flush=False)
HAL.enable_spike_recording(flush=True)

time.sleep(collect_time)

print("stopping data collection")
HAL.stop_traffic(flush=False)
HAL.disable_spike_recording(flush=True)

many_spikes = HAL.get_spikes()
many_parsed_spikes = parse_hal_spikes(many_spikes)

# get many pools data into same frame as big pool data

all_parsed_arr = np.zeros((N_all,))
for n in range(N_all):
    if n in all_parsed_spikes[p_all]:
        all_parsed_arr[n] = len(all_parsed_spikes[p_all][n])

all_parsed_xy = all_parsed_arr.reshape((height_all, width_all))

many_parsed_xy = np.zeros_like(all_parsed_xy)
for p, spikes in many_parsed_spikes.items():
    this_parsed_arr = np.zeros((N,))
    for n in range(N):
        if n in spikes:
            this_parsed_arr[n] = len(spikes[n])
    this_p_parsed_xy = this_parsed_arr.reshape((height, width))
    pool_x, pool_y = p.mapped_xy
    many_parsed_xy[pool_y:pool_y + height, pool_x:pool_x + width] = this_p_parsed_xy

plt.figure()
plt.title("spike rate histogram")
plt.hist(all_parsed_xy, bins=20)

fmax = 100 * collect_time / 10
print("clipping", np.sum(all_parsed_xy > fmax), "very fast neurons")
all_parsed_xy[all_parsed_xy > fmax] = fmax
many_parsed_xy[many_parsed_xy > fmax] = fmax

def scale(x):
    return np.log10(x + 1)

vmin = np.min(scale(all_parsed_xy))
vmax = np.max(scale(all_parsed_xy))

plt.figure(10,10)
plt.subplot(221)
plt.title("log spikes for whole array")
plt.imshow(scale(all_parsed_xy), vmin=vmin, vmax=vmax)
plt.colorbar()

plt.subplot(223)
plt.title("log spikes for array reconstructed from pools")
plt.imshow(scale(many_parsed_xy), vmin=vmin, vmax=vmax)
plt.colorbar()

plt.subplot(222)
pct_err = np.abs(all_parsed_xy - many_parsed_xy) / fmax
plt.title("error fraction in terms of max spike rate")
plt.imshow(pct_err)
plt.colorbar()

plt.show()

