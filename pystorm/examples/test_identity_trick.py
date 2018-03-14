import numpy as np
import matplotlib.pyplot as plt
import time

from pystorm.hal import HAL
from pystorm.hal.hal import parse_hal_spikes, parse_hal_binned_tags
HAL = HAL()

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

np.random.seed(0)

###########################################
# pool size parameters

width = 8
height = 8
N = width * height
Din = 1

num_pools = 4

width_all = 16
height_all = 16
N_all = width_all * height_all

collect_time = 2

###########################################
# misc driver parameters
downstream_time_res = 10000 # ns
upstream_time_res = 1000000 # ns

HAL.set_time_resolution(downstream_time_res, upstream_time_res)

###########################################
# first map the entire array as one pool
# use spike outputs, as ground truth
net = graph.Network("net")

# some misc encoders

all_encs = np.zeros((N_all, Din))

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
# remap as many times as there are pools, use decode on one each time

ps = []
os = []
many_parsed_spikes = {}
for active_n in range(num_pools):
    net = graph.Network("net")

    decoders = np.eye(N)

    zero_encs = np.zeros((N, Din))
    for n in range(num_pools):
        p = net.create_pool("p" + str(n), zero_encs)
        if active_n == n:
            b = net.create_bucket("b" + str(n), N)
            o = net.create_output("o" + str(n), N)
            net.create_connection("p_to_b" + str(n), p, b, decoders)
            net.create_connection("b_to_o" + str(n), b, o, None)
            ps.append(p)
            os.append(o)

    # map network
    print("calling map")
    HAL.map(net)
    HAL.driver.SetSpikeFilterDebug(0, True)

    print("starting data collection for many pools")
    _ = HAL.driver.RecvUnpackedTags(0)
    HAL.start_traffic(flush=False)
    HAL.enable_spike_recording(flush=True)

    time.sleep(collect_time)

    print("stopping data collection")
    HAL.stop_traffic(flush=False)
    HAL.disable_spike_recording(flush=True)

    # the raw tags are correct, there's a bug in the SF
    counts, tags, routes, times = HAL.driver.RecvUnpackedTags(0)
    tags = np.array(tags)
    counts = np.array(counts) # should be > 0, spike counts
    times = np.array(times)
    if np.sum(counts > 255) != 0:
        print("WARNING: negative count")
    filt_idxs   = tags[tags != 2047] # get rid of junk
    filt_states = counts[tags != 2047]
    times       = times[tags != 2047]

    # hijack translate_tags
    outputs, dims, counts = net.translate_tags(filt_idxs, filt_states)

    this_pool_many_spikes = np.array([times, outputs, dims, counts]).T
    this_pool_many_parsed_spikes = parse_hal_binned_tags(this_pool_many_spikes)
    
    for o in this_pool_many_parsed_spikes:
        print(o)
        assert(ps[-1] not in many_parsed_spikes)
        many_parsed_spikes[ps[-1]] = this_pool_many_parsed_spikes[o] # replace o with p!

# get many pools data into same frame as big pool data

all_parsed_arr = np.zeros((N_all,))
for n in range(N_all):
    if n in all_parsed_spikes[p_all]:
        all_parsed_arr[n] = len(all_parsed_spikes[p_all][n])

all_parsed_xy = all_parsed_arr.reshape((height_all, width_all))

many_parsed_xy = np.zeros_like(all_parsed_xy)
for k in many_parsed_spikes:
    print(k)
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

plt.figure(figsize=(10,10))
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
plt.savefig("identity_trick.png")

#plt.show()

