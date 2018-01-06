import numpy as np
from pystorm.hal import HAL
import time

from pystorm.hal.neuromorph import graph

net = graph.Network("net")

np.random.seed(0)

Din = 1
Dout = 2
K = 8
x = 8
y = 8
xy = (x, y)
N = x * y


# tap matrix is NxD (like a normal weight matrix)
# entries are in [-1, 0, 1]
tap_matrix = np.zeros((N, Din))
for d in range(Din):
    for k in range(K):
        tgt = np.random.randint(N)
        while tap_matrix[tgt, d] in [-1, 1]:
            tgt = np.random.randint(N) # retry if already assigned

        #sign = np.random.randint(2) * 2 - 1 # fully random taps

        y = tgt // xy[0]
        x = tgt %  xy[0]
        print("tap at y:", y // 2, ", x:", x // 2, " = AER:", HAL.driver.GetSynAERAddr(x // 2, y // 2))
        sign = int(2 * (x > xy[0] // 2) - 1) # positive signs on one side

        tap_matrix[tgt, d] = sign

# decoders are initially zero, we remap them later
decoders = np.zeros((Dout, N))
#decoders = np.ones((Dout, N)) * .2

i1 = net.create_input("i1", Din)
p1 = net.create_pool("p1", tap_matrix)
b1 = net.create_bucket("b1", Dout)
o1 = net.create_output("o1", Dout)

net.create_connection("c_i1_to_p1", i1, p1, None)
net.create_connection("c_p1_to_b1", p1, b1, decoders)
net.create_connection("c_b1_to_o1", b1, o1, None)

# at this point, we've imported hal, the driver should be on

# map network
HAL.map(net)

from pystorm.PyDriver import bddriver as bd

# turn on spikes
print("starting")
HAL.start_traffic()
HAL.start_traffic()
HAL.start_traffic()
HAL.start_traffic()
HAL.enable_spike_recording()
HAL.enable_spike_recording()
HAL.enable_spike_recording()
HAL.enable_spike_recording()
HAL.enable_output_recording()
HAL.enable_output_recording()
HAL.enable_output_recording()
HAL.enable_output_recording()

duration = 5
duration_ns = duration * 1e9
bins = 10
fmax = 1000
start_time = HAL.get_time()
end_time = start_time + duration_ns
bin_boundaries = np.round(np.linspace(start_time, end_time, bins + 1)).astype(int)

# clear spikes before trial
spikes = HAL.get_spikes()
print(len(spikes), "spikes before trial")

# queue up rates
rates = np.round(np.linspace(-fmax, fmax, bins)).astype(int)
for r, bin_start in zip(rates, bin_boundaries):
    HAL.set_input_rate(i1, 0, r, time=bin_start)

time.sleep(duration)
print("run over")

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

def filter_tags(tags):
    times_and_cts = {}
    for t, output_id, dim, ct in tags:
        if output_id not in times_and_cts:
            times_and_cts[output_id] = {}
        if dim not in times_and_cts[output_id]:
            times_and_cts[output_id][dim] = []
        times_and_cts[output_id][dim].append((t, ct))
    return times_and_cts
            
def do_binning(times_and_cts, bin_boundaries):

    n_bins = len(bin_boundaries) - 1

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
            binned_cts = np.histogram(this_neuron_times, bin_boundaries, weights=this_neuron_cts)[0]
            #print(binned_cts)
            As[obj][n, :] = binned_cts
    return As

print((HAL.driver.GetOutputQueueCounts()))
spikes = HAL.get_spikes()

filtered_times = filter_spikes(spikes)
As = do_binning(filtered_times, bin_boundaries)
if p1 in As:
    print(As[p1])
    print('total count in bounds, in exp duration:', np.sum(As[p1]))
print("got", len(spikes), "spikes")

#tags, times = HAL.driver.RecvTags(0)
#print("got", len(tags), "tags")
#
#def count_tags(tags):
#    tag_cts = {}
#    total_tag_cts = {}
#    for t in tags:
#        ct = bd.GetField(t, bd.TATOutputTag.COUNT)
#        tag = bd.GetField(t, bd.TATOutputTag.TAG)
#        route = bd.GetField(t, bd.TATOutputTag.GLOBAL_ROUTE)
#        k = (tag, route, ct)
#        if k not in tag_cts:
#            tag_cts[k] = 0
#        tag_cts[k] += 1
#        k = (tag, route)
#        if k not in total_tag_cts:
#            total_tag_cts[k] = 0
#        total_tag_cts[k] += ct
#    print(tag_cts)    
#    print(total_tag_cts)
#
#count_tags(tags)

outputs = HAL.get_outputs()
print("got outputs, doing binning")
filtered_outputs = filter_tags(outputs)
binned_outputs = do_binning(filtered_outputs, bin_boundaries)
print("got", len(outputs), "from filters")
print(binned_outputs[o1])
print(np.sum(binned_outputs[o1]))

