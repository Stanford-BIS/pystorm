import numpy as np
import matplotlib.pyplot as plt
import time

from pystorm.hal import HAL # HAL is a singleton, importing immediately sets up a HAL and its C Driver

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

np.random.seed(0)

###########################################
# pool size parameters

K = 8
width = 16
height = 16
width_height = (width, height)
N = width * height

###########################################
# misc driver parameters
downstream_time_res = 10000 # ns
upstream_time_res = 1000000 # ns

HAL.set_time_resolution(downstream_time_res, upstream_time_res)

###########################################
# sweep

fmax = 1000
num_training_points_per_dim = 3
training_hold_time = 2 # seconds

Din = 1

###########################################
# stim rates

total_training_points = num_training_points_per_dim ** Din
stim_rates_1d = np.linspace(-fmax, fmax, num_training_points_per_dim).astype(int)
if Din > 1:
    stim_rates_mesh = np.meshgrid(*([stim_rates_1d]*Din))
else:
    stim_rates_mesh = [stim_rates_1d]
stim_rates = [r.flatten() for r in stim_rates_mesh]

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
# biases and gains

biases = np.zeros((N,), dtype='int')
gain_divisors = np.ones((N,), dtype='int')

# make bias higher on top half, lower on bottom half
# gains higher in 2nd and 4th horizontal quartiles
for x in range(0, width):
    for y in range(0, height):
        n = y * width + x

        if y < height // 2:
            biases[n] = 3
        else:
            biases[n] = -2

        if x > width // 4 and x <= width // 2 or x > width * 3 // 4:
            gain_divisors[n] = 1
        else:
            gain_divisors[n] = 4

dac_offset_value = 10 # size of bias unit

###########################################
# specify network using HAL

net = graph.Network("net")

i1 = net.create_input("i1", Din)
p1 = net.create_pool("p1", tap_matrix, biases=biases, gain_divisors=gain_divisors)

net.create_connection("c_i1_to_p1", i1, p1, None)

###########################################
# invoke HAL.map(), make tons of neuromorph/driver calls under the hood

# map network
print("calling map")
HAL.map(net)

###########################################
# compute sweep bins

def get_sweep_bins_starting_now(training_hold_time, total_training_points):

    fudge = .5
    fudge_ns = fudge * 1e9

    duration = training_hold_time * total_training_points
    duration_ns = duration * 1e9

    start_time = HAL.get_time() + fudge_ns
    end_time = start_time + duration_ns
    bin_time_boundaries = np.round(np.linspace(start_time, end_time, total_training_points + 1)).astype(int)

    return duration, bin_time_boundaries

duration, bin_time_boundaries = get_sweep_bins_starting_now(training_hold_time, total_training_points)

###########################################
# call HAL.set_input_rate() to program SGs for each bin/dim

# have to do this before set_input_rates, so we don't get stalled
HAL.start_traffic(flush=False)
HAL.enable_spike_recording(flush=False)
HAL.enable_output_recording(flush=True)

# clear spikes before trial
spikes = HAL.get_spikes()
print(len(spikes), "spikes before trial")

# make soma bias high to see effect of bits
HAL.driver.SetDACCount(0, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, dac_offset_value)

def do_sweep(bin_time_boundaries):
    last_bin_start = None
    for bin_idx in range(len(stim_rates[0])):
        bin_start = bin_time_boundaries[bin_idx]
        if last_bin_start is None:
            pass
        else:
            if bin_start < last_bin_start:
                print("last_bin_start", last_bin_start, "vs bin_start", bin_start)
                assert(False)
        last_bin_start = bin_start

        for d in range(Din):
            r = stim_rates[d][bin_idx]

            #print("at", bin_start, "d", d, "is", r)
            HAL.set_input_rate(i1, d, r, time=bin_start, flush=True)

do_sweep(bin_time_boundaries)

###########################################
# sleep during the training sweep

# turn on spikes
print("starting training:", duration, "seconds")

time.sleep(duration + 1)

print("training over")

# should be unecessary
HAL.stop_traffic(flush=False)
HAL.disable_spike_recording(flush=False)
HAL.disable_output_recording(flush=True)

###########################################
# collect results
# NOTE: we could just set the upstream time resolution to whatever hold_time is
# this would make the FPGA do the binning for us

def filter_spikes(spikes):
    # start by filtering into pool -> neuron -> (times, cts)
    # cts is unecessary, but gives same format as filter_tags
    times_and_cts = {}
    for t, p, n in spikes:
        if p not in times_and_cts:
            times_and_cts[p] = {}
        if n not in times_and_cts[p]:
            times_and_cts[p][n] = []
        times_and_cts[p][n].append((t, 1))
    return times_and_cts
            
def do_binning(times_and_cts, bin_time_boundaries):

    n_bins = len(bin_time_boundaries) - 1

    # initialize A matrices
    As = {}
    for obj in times_and_cts:
        if isinstance(obj, graph.Pool):
            max_idx = obj.n_neurons
        elif isinstance(obj, graph.Output):
            max_idx = obj.dimensions
        As[obj] = np.zeros((max_idx, n_bins)).astype(int)

    # bin spikes, filling in A
    for obj in times_and_cts:
        for n in times_and_cts[obj]:
            #print("binning spikes in pool", p.label, ", neuron", n)            
            this_neuron_times_and_cts = np.array(times_and_cts[obj][n])
            this_neuron_times = this_neuron_times_and_cts[:,0]
            this_neuron_cts = this_neuron_times_and_cts[:,1]
            binned_cts = np.histogram(this_neuron_times, bin_time_boundaries, weights=this_neuron_cts)[0]
            #print(binned_cts)
            As[obj][n, :] = binned_cts
    return As

#print((HAL.driver.GetOutputQueueCounts())) # debug: see what we got

# call HAL to get spikes
# remember these are corrupted because of the output weirdness
# the "noise" fortunately seems to be relatively unbiased, and we
# can use the spikes directly to create a decoder
spikes = HAL.get_spikes()

# pull out A for our pool, plot it
filtered_times = filter_spikes(spikes)
As = do_binning(filtered_times, bin_time_boundaries)
if p1 in As:
    A = As[p1]
    print('total count in bounds, in exp duration:', np.sum(As[p1]))

    plt.figure()
    plt.plot(A.T)
    plt.title("tuning curves")
    plt.savefig("hal_tuning_curves.pdf")

    plt.figure()
    plt.imshow(A[:,total_training_points // 2].reshape((height, width)))
    plt.title("neuron activity at zero input")
    plt.savefig("zero_input_xy_counts.pdf")

