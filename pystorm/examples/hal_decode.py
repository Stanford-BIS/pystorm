import numpy as np
from pystorm.hal import HAL
import time

from pystorm.hal.neuromorph import graph

net = graph.Network("net")

Din = 1
Dout = 1
K = 8
N = 64

i1 = net.create_input("i1", Din)
p1 = net.create_pool("p1", N, Din)
#b1 = net.create_bucket("b1", Dout)

# tap matrix is NxD (like a normal weight matrix)
# entries are in [-1, 0, 1]
tap_matrix = np.zeros((N, Din))
for d in range(Din):
    for k in range(K):
        tgt = np.random.randint(N)
        while tap_matrix[tgt, d] in [-1, 1]:
            tgt = np.random.randint(N) # retry if already assigned

        #sign = np.random.randint(2) * 2 - 1 # fully random taps

        y = tgt // p1.x
        x = tgt %  p1.x
        print("tap at y:", y // 2, ", x:", x // 2, " = AER:", HAL.driver.GetSynAERAddr(x // 2, y // 2))
        sign = int(2 * (x > p1.x // 2) - 1) # positive signs on one side

        tap_matrix[tgt, d] = sign

# decoders are initially zero, we remap them later
decoders = np.zeros((Dout, N))

net.create_connection("c_i1_to_p1", i1, p1, tap_matrix)
#net.create_connection("c_p1_to_b1", p1, b1, decoders)

# at this point, we've imported hal, the driver should be on

# map network
HAL.map(net)

# turn on spikes
print("starting")
HAL.enable_spike_recording()
HAL.enable_spike_recording()
HAL.enable_spike_recording()
HAL.enable_spike_recording()

duration = 5
duration_ns = duration * 1e9
bins = 20
fmax = 1000
start_time = HAL.get_time()
end_time = start_time + duration_ns
bin_boundaries = np.round(np.linspace(start_time, end_time, bins + 1)).astype(int)

# queue up rates
rates = np.round(np.linspace(-fmax, fmax, bins)).astype(int)
for r, bin_start in zip(rates, bin_boundaries):
    HAL.set_input_rate(i1, 0, r, time=bin_start)

time.sleep(duration)
print("run over")

def filter_spikes(spikes):
    # start by filtering into pool -> neuron -> times
    times = {}
    for t, p, n in spikes:
        if p not in times:
            times[p] = {}
        if n not in times[p]:
            times[p][n] = []
        times[p][n].append(t)
    return times
            
def bin_spikes(times, bin_boundaries):

    n_bins = len(bin_boundaries) - 1

    # initialize A matrices
    As = {}
    for p in times:
        As[p] = np.zeros((p.n_neurons, n_bins)).astype(int)

    # bin spikes, filling in A
    for p in times:
        for n in times[p]:
            #print("binning spikes in pool", p.label, ", neuron", n)            
            this_neuron_times = times[p][n]
            binned_cts = np.histogram(this_neuron_times, bin_boundaries)[0]
            #print(binned_cts)
            As[p][n, :] = binned_cts
    return As

spikes = HAL.get_spikes()
print("got", len(spikes), "spikes")

filtered_times = filter_spikes(spikes)
As = bin_spikes(filtered_times, bin_boundaries)
print(As[p1])

