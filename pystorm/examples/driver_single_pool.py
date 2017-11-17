import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pickle
import sys

from pystorm.PyDriver import bddriver as bd
import driver_util
import time

pool_width = 16
pool_height = pool_width
num_neurons = pool_height * pool_width

fmax = 10000
num_training_points = 20
training_hold_time = 4 # seconds

input_tag = 0

# tap points are + on the left half, - on the right half
tap_point_syn_stride = 2
tap_points = {}
for xsyn in range(0, pool_width//2, tap_point_syn_stride):
    for ysyn in range(0, pool_width//2, tap_point_syn_stride):
        if xsyn < pool_width//4:
            tap_points[ysyn, xsyn] = 0 # 0 sign means pos
        else:
            tap_points[ysyn, xsyn] = 1 # 1 sign means neg

downstream_time_res = 10 * 1000 # ns
upstream_time_res = downstream_time_res * 10 # ns


#########################################

def s_to_ns(s):
    return (1e9 * s).astype(int)

def ns_to_s(ns):
    return ns / 1e9

#########################################

CORE = 0

D = driver_util.standard_startup(downstream_time_res, upstream_time_res)

print("* Init'ing DACs")
time.sleep(2)
D.InitDAC(CORE)

print("* Configuring neuron array")

# first, disable everything
for i in range(4096):
    D.DisableSoma(CORE, i)
#    D.SetSomaGain(CORE, i, bd.bdpars.SomaGainId.ONE);
#    D.SetSomaOffsetSign(CORE, i, bd.bdpars.SomaOffsetSignId.POSITIVE);
#    D.SetSomaOffsetMultiplier(CORE, i, bd.bdpars.SomaOffsetMultiplierId.ZERO);

for i in range(1024):
    D.DisableSynapse(CORE, i);
    D.DisableSynapseADC(CORE, i);

#for i in range(256):
#    D.OpenDiffusorAllCuts(CORE, i);

# enable the neurons and synapses we're using
# diffuser is open but that's fine for one pool
for xsyn in range(pool_width//2):
    for ysyn in range(pool_height//2):

        # un-kill the synapse, if it's a tap
        syn_aer_idx = D.GetSynAERAddr(xsyn, ysyn)
        if (ysyn, xsyn) in tap_points:
            pass
            D.EnableSynapse(CORE, syn_aer_idx)

        for xnrn_off in range(2):
            for ynrn_off in range(2):
                xnrn = xsyn * 2 + xnrn_off
                ynrn = ysyn * 2 + ynrn_off

                # un-kill the soma
                soma_aer_idx = D.GetSomaAERAddr(xnrn, ynrn)
                D.EnableSoma(CORE, soma_aer_idx)

                # XXX should set gain based on tap point proximity
                # XXX bias? probably need a second pass

for xtile in range(pool_width//4):
    for ytile in range(pool_width//4):
        mem_aer_idx = D.GetSynAERAddr(xtile, ytile)
        #D.CloseDiffusorAllCuts(CORE, mem_aer_idx);
        D.OpenDiffusorAllCuts(CORE, mem_aer_idx);
D.Flush() 

# set up the TAT
print("* Programming TAT for tap points")
tat_entries = []
for tap_idx, tap in enumerate(tap_points):

    last_tap = tap
    last_sign = tap_points[tap]

    # two tap points per entry
    if tap_idx % 2 == 0:

        if tap_idx == len(tap_points) - 1:
            stop = 1
        else:
            stop = 0

        y0, x0 = last_tap
        sign0  = last_sign
        sign1  = tap_points[tap]
        y1, x1 = tap

        addr0 = D.GetSynAERAddr(x0, y0)
        addr1 = D.GetSynAERAddr(x1, y1)

        tat_entry = bd.PackWord([
            (bd.TATSpikeWord.STOP              , stop)  ,
            (bd.TATSpikeWord.SYNAPSE_ADDRESS_0 , addr0) ,
            (bd.TATSpikeWord.SYNAPSE_SIGN_0    , sign0) ,
            (bd.TATSpikeWord.SYNAPSE_ADDRESS_1 , addr1) ,
            (bd.TATSpikeWord.SYNAPSE_SIGN_1    , sign1) ])

        tat_entries.append(tat_entry)

D.SetMem(CORE, bd.bdpars.BDMemId.TAT0, tat_entries, input_tag)

# we won't flush the neuron config stuff until here. Wait a while (it's really slow, the CommOK code needs serious work)
time.sleep(1)

#print("* Reading TAT")
#dumped_tat = D.DumpMem(CORE, bd.bdpars.BDMemId.TAT0)
#
#if driver_util.compare_TAT_words(tat_entries, dumped_tat) == -1:
#    D.Stop()
#    sys.exit(-1)


# for some reason, you have to do this a couple times (maybe there's input slack?)
D.SetSpikeDumpState(CORE, True, True)
D.SetSpikeDumpState(CORE, True, True)
D.SetSpikeDumpState(CORE, True, True)
D.SetSpikeDumpState(CORE, True, True)

start_time = ns_to_s(D.GetFPGATime()) + 1 # s
end_time = start_time + num_training_points * training_hold_time # s
times = np.linspace(start_time, end_time, num_training_points)

print("* Running training for", end_time - start_time, "seconds")

#rates = np.linspace(-fmax, fmax, num_training_points).astype(int)
rates = np.linspace(0, fmax, num_training_points).astype(int)

#for rate, time_ns in zip(rates, s_to_ns(times)):
#    D.SetSpikeGeneratorRates(CORE, [0], [input_tag], [rate], time_ns, False)
#D.Flush()
for r, t in zip(rates, times):
    num_spikes_in_bin = int(abs(r) * training_hold_time)
    bin_start = t
    bin_end = t + training_hold_time
    tag_times = np.array(s_to_ns(np.linspace(bin_start, bin_end, num_spikes_in_bin)))
    #if r > 0:
    #    count = 1
    #else:
    #    count = 511
    count = 1
    tags = [bd.PackWord([(bd.InputTag.TAG, 0), (bd.InputTag.COUNT, count)])] * num_spikes_in_bin
    #D.SendTags(CORE, tags, tag_times)
    spikes = [bd.PackWord([(bd.InputSpike.SYNAPSE_ADDRESS, D.GetSomaAERAddr(4,4)), (bd.InputSpike.SYNAPSE_SIGN, 0)])] * num_spikes_in_bin
    D.SendSpikes(CORE, spikes, tag_times)
    print("this many tags", len(tags))

time.sleep(end_time - start_time + 2)

print(D.GetOutputQueueCounts())
spikes, spike_times = D.RecvSpikes(CORE)

# spike is a single field, no need to do this:
#bd.GetField(s, bd.OutputSpike.NEURON_ADDRESS)

print("processing spikes")

p_objs = {"spikes":spikes, "spike_times":spike_times, "times":times}
pfile = open("single_pool.pck", "wb")
pickle.dump(p_objs, pfile)
pfile.close()

yxs = np.array(D.GetSomaXYAddrs(spikes)).astype(int)

counts = [np.zeros((pool_height, pool_width)).astype(int) for i in range(num_training_points)]
ys = yxs // 64
xs = yxs % 64

bin_idx = 0
idx = 0
in_bin = 0
events_per_bin = [0 for c in counts]
for y, x, t in zip(ys, xs, ns_to_s(np.array(spike_times))):
    #if idx % print_every == 0:
    #    print(y,x)
    if bin_idx+1 < len(times) and t > times[bin_idx + 1]:
        bin_idx += 1
        print("bin boundary at", t)
        print(" ", in_bin, " in bin")
        in_bin = 0
    counts[bin_idx][y, x] += 1
    idx += 1
    in_bin += 1

n_to_plot = 10
plt.figure(figsize=(4*n_to_plot, 4))
for pidx in range(n_to_plot):
    plt.subplot(1, n_to_plot, pidx+1)
    plt.imshow(counts[int(num_training_points * pidx / n_to_plot)], cmap='Greys')
    plt.colorbar()
plt.savefig("spikes.pdf")

plt.figure()
plt.hist(spike_times, bins=100)
plt.savefig("spike_times.pdf")

# tuning curves
A = np.zeros((num_neurons, num_training_points))
for train_idx, c in enumerate(counts):
    A[:, train_idx] = c.flatten()

plt.figure()
plt.plot(A[0,:])
plt.savefig("tuning_curves.pdf")
print(A.shape)

print("* Done")
D.Stop()
