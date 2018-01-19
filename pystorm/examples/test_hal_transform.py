import numpy as np
import matplotlib.pyplot as plt
import time

from pystorm.hal import HAL # HAL is a singleton, importing immediately sets up a HAL and its C Driver

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

np.random.seed(0)

###########################################
# network parameters

Din = 2
Dout = 1
fmax = 1000
num_training_points_per_dim = 5
training_hold_time = .5

###########################################
# misc driver parameters
downstream_time_res = 10000 # ns
upstream_time_res = 1000000 # ns

HAL.set_time_resolution(downstream_time_res, upstream_time_res)

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
# specify network using HAL

net = graph.Network("net")

if Din == 1 and Dout == 1:
    transform = np.array([[1.0]])
elif Din == 2 and Dout == 1:
    transform = np.array([[1.0, .25]])
elif Din == 1 and Dout == 2:
    transform = np.array([[1.0], [.1]])
elif Din == 2 and Dout == 2:
    transform = np.array([[1.0, .25], [.1, .2]])
else:
    assert(False and "write a matrix for this Din/Dout combo")

i1 = net.create_input("i1", Din)
b1 = net.create_bucket("b1", Dout)
o1 = net.create_output("o1", Dout)

net.create_connection("c_i1_to_b1", i1, b1, transform)
net.create_connection("c_b1_to_o1", b1, o1, None)

###########################################
# compute outputs

arr_rates = np.array(stim_rates)
arr_counts = arr_rates * training_hold_time
ideal_outputs = np.dot(transform, arr_counts)

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


def do_sweep(bin_time_boundaries):
    for d in range(Din):
        for r, bin_start in zip(stim_rates[d], bin_time_boundaries[:-1]):
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

def filter_tags(tags):
    times_and_cts = {}
    for t, output_id, dim, ct in tags:
        if output_id not in times_and_cts:
            times_and_cts[output_id] = {}
        if dim not in times_and_cts[output_id]:
            times_and_cts[output_id][dim] = []
        times_and_cts[output_id][dim].append((t, ct))
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

outputs = HAL.get_outputs()
print("got outputs, doing binning")
filtered_outputs = filter_tags(outputs)
binned_outputs = do_binning(filtered_outputs, bin_time_boundaries)

print(binned_outputs[o1])

plt.figure()
legend = []
for dim_idx, dim_out in enumerate(binned_outputs[o1]):
    plt.plot(dim_out)
    legend.append("measured dim " + str(dim_idx))
    plt.plot(ideal_outputs[dim_idx, :])
    legend.append("expected dim " + str(dim_idx))
plt.legend(legend)
plt.show()
