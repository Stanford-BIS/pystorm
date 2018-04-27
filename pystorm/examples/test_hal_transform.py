import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import time

from pystorm.hal import HAL
HAL = HAL()

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

np.random.seed(0)

###########################################
# network parameters

Din = 1
Dout = 2
fmax = 1000
num_training_points_per_dim = 20
training_hold_time = .1


###########################################
# misc driver parameters
downstream_time_res = 100000 # ns
upstream_time_res = 100000 # ns

HAL.set_time_resolution(downstream_time_res, upstream_time_res)

D = HAL.driver;

# D.ResetFPGATime()
# old_time = D.GetFPGATime()
# for n in range(0, 3):
#     time.sleep(1)
#     new_time = D.GetFPGATime()
#     print(new_time)


###########################################
# stim rates

total_training_points = num_training_points_per_dim ** Din
stim_rates_1d = np.linspace(-fmax, fmax, num_training_points_per_dim).astype(int)
if Din > 1:
    stim_rates_mesh = np.meshgrid(*([stim_rates_1d]*Din))
else:
    stim_rates_mesh = [stim_rates_1d]
stim_rates = [r.flatten() for r in stim_rates_mesh]
print("stims")
print(stim_rates)

###########################################
# specify network using HAL

net = graph.Network("net")

if Din == 1 and Dout == 1:
    transform = np.array([[1.0]])
elif Din == 2 and Dout == 1:
    transform = np.array([[1.0, .25]])
elif Din == 1 and Dout == 2:
    transform = np.array([[1.0], [0.5]])
elif Din == 1 and Dout == 3:
    transform = np.array([[1.0], [0.5], [0.25]])
elif Din == 2 and Dout == 2:
    transform = np.array([[1.0, .25], [.1, .2]])
elif Din == 2 and Dout == 4:
    transform = np.array([[1.0, .25], [.1, .4], [.1, .1], [.25, 1.0]])
elif Din == 1 and Dout > 3:
    transform = np.linspace(0, 1, Dout).reshape((Dout, Din))
else:
    assert(False and "write a matrix for this Din/Dout combo")


i1 = net.create_input("i1", Din)
b1 = net.create_bucket("b1", Dout)
b2 = net.create_bucket("b2", Dout)
o1 = net.create_output("o1", Dout)

net.create_connection("c_i1_to_b1", i1, b1, transform)
net.create_connection("c_b1_to_b2", b1, b2, np.identity(Dout))
net.create_connection("c_b2_to_o1", b2, o1, None)

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

# old_time = D.GetFPGATime()
# for n in range(0, 3):
#     time.sleep(1)
#     new_time = D.GetFPGATime()
#     print(new_time)

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

HAL.driver.SetSpikeFilterDebug(0, True)

# have to do this before set_input_rates, so we don't get stalled
HAL.start_traffic(flush=False)
HAL.enable_spike_recording(flush=False)
HAL.enable_output_recording(flush=True)

# clear spikes before trial
spikes = HAL.get_spikes()
print(len(spikes), "spikes before trial")

def do_sweep(bin_time_boundaries):
    last_bin_start = None
    for bin_idx in range(len(stim_rates[0])):
        inputs = []
        dims = []
        rates = []

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
            #HAL.set_input_rate(i1, d, r, time=bin_start, flush=True)
            inputs.append(i1)
            dims.append(d)
            rates.append(r)

        HAL.set_input_rates(inputs, dims, rates, time=bin_start, flush=True)

for core in range(0, 2):
    D.SetSpikeFilterDebug(core, en=True)

do_sweep(bin_time_boundaries)
# input_time = int(HAL.get_time() + 10000e9)
# input_time_2 = int(HAL.get_time() + 2e9)
# print(HAL.get_time())


# print("input high at: ",input_time)
# print("input 2 high at: ",input_time_2)
# HAL.set_input_rate(i1, 0, 100, time=input_time, flush=True)
# HAL.set_input_rate(i1, 0, 200, time=input_time_2, flush=True)


###########################################
# sleep during the training sweep

# turn on spikes
# duration = duration*2
print("starting training:", duration, "seconds")


print("start time: ",HAL.get_time())
time.sleep((duration + 1))
# time.sleep((duration + 1)/4)
# HAL.set_input_rate(i1, 0, 400, time=0, flush=True)
# time.sleep((duration + 1)/4)
# HAL.set_input_rate(i1, 0, 600, time=0, flush=True)
# time.sleep((duration + 1)/4)
# HAL.set_input_rate(i1, 0, -300, time=0, flush=True)
# time.sleep((duration + 1)/4)

print("end time: ",HAL.get_time())

print("training over")

# should be unecessary
HAL.stop_traffic(flush=False)
HAL.disable_spike_recording(flush=False)
HAL.disable_output_recording(flush=True)


for core in range(0,2):
    print(core)
    tags = D.RecvUnpackedTags(core)
    tagmap = {}
    for i in range(0, len(tags[0])):
        if tags[1][i] != 2047:
            if tags[1][i] not in tagmap:
                tagmap[tags[1][i]] = 0
            tagmap[tags[1][i]] += tags[0][i]

    print(tagmap)


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


plt.figure(figsize=(10, 10))
legend = []
#colors = plt.get_cmap("Oranges")(np.linspace(0, 1, Dout))
plt.gca().set_color_cycle(None)
for dim_idx, dim_out in enumerate(binned_outputs[o1]):
    #color = colors[dim_idx]
    plt.plot(dim_out, marker='.', linestyle='None')
    legend.append("measured dim " + str(dim_idx))
plt.gca().set_color_cycle(None)
for dim_idx, dim_out in enumerate(binned_outputs[o1]):
    #color = colors[dim_idx]
    plt.plot(ideal_outputs[dim_idx, :])
    legend.append("expected dim " + str(dim_idx))
plt.legend(legend)
plt.show()


#filt_idxs, filt_states, times = HAL.driver.RecvSpikeFilterStates(0, 1000)

#counts, tags, routes, times = HAL.driver.RecvUnpackedTags(0)
#tags = np.array(tags)
#plt.figure()
#plt.hist(tags[tags != 2047], bins=9)
