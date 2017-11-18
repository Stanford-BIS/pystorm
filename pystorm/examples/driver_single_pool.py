import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pickle
import sys

from pystorm.PyDriver import bddriver as bd
import driver_util
import time

pool_width = 8
pool_height = pool_width
num_neurons = pool_height * pool_width

fmax = 1000
num_training_points = 6
training_hold_time = 4 # seconds

input_tag = 0

# tap points are + on the left half, - on the right half
tap_point_syn_stride = 1
tap_points = {}
for xsyn in range(0, pool_width//2, tap_point_syn_stride):
    for ysyn in range(0, pool_width//2, tap_point_syn_stride):
        #if xsyn >= 2 or ysyn >= 2:
            if xsyn < pool_width//4 and ysyn < pool_height//4 or xsyn > pool_width//4 and ysyn > pool_height//4:
                tap_points[ysyn, xsyn] = 1 # 1 sign means pos
            else:
                #tap_points[ysyn, xsyn] = 1 # 0 sign means neg
                tap_points[ysyn, xsyn] = 0 # 0 sign means neg

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

D.SetDACValue(CORE , bd.bdpars.BDHornEP.DAC_SYN_EXC     , 512)
D.SetDACValue(CORE , bd.bdpars.BDHornEP.DAC_SYN_DC      , 920)
D.SetDACValue(CORE , bd.bdpars.BDHornEP.DAC_SYN_INH     , 512)
D.SetDACValue(CORE , bd.bdpars.BDHornEP.DAC_SYN_LK      , 10)
D.SetDACValue(CORE , bd.bdpars.BDHornEP.DAC_SYN_PD      , 10)
D.SetDACValue(CORE , bd.bdpars.BDHornEP.DAC_SYN_PU      , 1023)
D.SetDACValue(CORE , bd.bdpars.BDHornEP.DAC_DIFF_G      , 1023)
D.SetDACValue(CORE , bd.bdpars.BDHornEP.DAC_DIFF_R      , 250)
D.SetDACValue(CORE , bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , 1)

print("* Configuring neuron array")

# first, disable everything
for i in range(4096):
    D.DisableSoma(CORE, i)
    D.SetSomaGain(CORE, i, bd.bdpars.SomaGainId.ONE);
    D.SetSomaOffsetSign(CORE, i, bd.bdpars.SomaOffsetSignId.NEGATIVE);
    D.SetSomaOffsetMultiplier(CORE, i, bd.bdpars.SomaOffsetMultiplierId.ONE)

for i in range(1024):
    D.DisableSynapse(CORE, i);
    #D.EnableSynapse(CORE, i);
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
            print("enabling synapse", ysyn, xsyn)
            D.EnableSynapse(CORE, syn_aer_idx)

            xnrn = xsyn*2
            ynrn = ysyn*2
            soma_under_syn_addr = D.GetSomaAERAddr(xnrn, ynrn)
            D.SetSomaGain(CORE, soma_under_syn_addr, bd.bdpars.SomaGainId.ONE_FOURTH);

        for xnrn_off in range(2):
            for ynrn_off in range(2):
                xnrn = xsyn * 2 + xnrn_off
                ynrn = ysyn * 2 + ynrn_off

                # un-kill the soma
                soma_aer_idx = D.GetSomaAERAddr(xnrn, ynrn)
                D.EnableSoma(CORE, soma_aer_idx)
                
                if xnrn < pool_width // 2 and ynrn < pool_height // 2:
                    D.SetSomaOffsetSign(CORE, soma_aer_idx, bd.bdpars.SomaOffsetSignId.NEGATIVE);
                    D.SetSomaOffsetMultiplier(CORE, soma_aer_idx, bd.bdpars.SomaOffsetMultiplierId.THREE)
                if xnrn > pool_width // 2 and ynrn > pool_height // 2:
                    D.SetSomaOffsetSign(CORE, soma_aer_idx, bd.bdpars.SomaOffsetSignId.POSITIVE);
                    D.SetSomaOffsetMultiplier(CORE, soma_aer_idx, bd.bdpars.SomaOffsetMultiplierId.ONE)

                # XXX bias? probably need a second pass

for xtile in range(pool_width//4):
    for ytile in range(pool_width//4):
        mem_aer_idx = D.GetMemAERAddr(xtile, ytile)
        D.CloseDiffusorAllCuts(CORE, mem_aer_idx);
        #D.OpenDiffusorAllCuts(CORE, mem_aer_idx);
D.Flush() 

# set up the TAT
print("* Programming TAT for tap points")
tat_entries = []
for tap_idx, tap in enumerate(tap_points):

    # two tap points per entry
    if tap_idx % 2 == 1:

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

        print("tat entry: stop", stop, "addr0", addr0, "addr1", addr1, "sign0", sign0, "sign1", sign1)
        tat_entry = bd.PackWord([
            (bd.TATSpikeWord.STOP              , stop)  ,
            (bd.TATSpikeWord.SYNAPSE_ADDRESS_0 , addr0) ,
            (bd.TATSpikeWord.SYNAPSE_SIGN_0    , sign0) ,
            (bd.TATSpikeWord.SYNAPSE_ADDRESS_1 , addr1) ,
            (bd.TATSpikeWord.SYNAPSE_SIGN_1    , sign1) ])

        tat_entries.append(tat_entry)

    last_tap = tap
    last_sign = tap_points[tap]

D.SetMem(CORE, bd.bdpars.BDMemId.TAT0, tat_entries, input_tag)

# we won't flush the neuron config stuff until here. Wait a while (it's really slow, the CommOK code needs serious work)
time.sleep(1)

print("* Reading TAT")
dumped_tat = D.DumpMem(CORE, bd.bdpars.BDMemId.TAT0)

if driver_util.compare_TAT_words(tat_entries, dumped_tat) == -1:
    D.Stop()
    sys.exit(-1)


# for some reason, you have to do this a couple times (maybe there's input slack?)
D.SetSpikeDumpState(CORE, True, True)
D.SetSpikeDumpState(CORE, True, True)
D.SetSpikeDumpState(CORE, True, True)
D.SetSpikeDumpState(CORE, True, True)

start_time = ns_to_s(D.GetFPGATime()) + .5 # s
end_time = start_time + num_training_points * training_hold_time # s
times = np.linspace(start_time, end_time, num_training_points)

print("* Running training for", end_time - start_time, "seconds")

#rates = np.array(list(np.linspace(0, 0, num_training_points//2).astype(int)) + list(np.linspace(0, fmax, num_training_points//2).astype(int)))

rates = np.linspace(-fmax, fmax, num_training_points).astype(int)
#rates = np.linspace(0, fmax, num_training_points).astype(int)

#for rate, time_ns in zip(rates, s_to_ns(times)):
#    D.SetSpikeGeneratorRates(CORE, [0], [input_tag], [rate], time_ns, False)
#D.Flush()
for r, t in zip(rates, times):
    num_spikes_in_bin = int(abs(r) * training_hold_time)
    bin_start = t
    bin_end = t + training_hold_time
    tag_times = np.array(s_to_ns(np.linspace(bin_start, bin_end, num_spikes_in_bin)))
    if r > 0:
        count = 1
        sign = 0
    else:
        count = 511
        sign = 1
    tags = [bd.PackWord([(bd.InputTag.TAG, 0), (bd.InputTag.COUNT, count)])] * num_spikes_in_bin
    D.SendTags(CORE, tags, tag_times, False)

    ## inject spikes directly instead
    #for tap in tap_points:
    #    tap_sign = tap_points[tap]
    #    ysyn, xsyn = tap
    #    syn_sign = sign ^ tap_sign
    #    syn_addr = D.GetSynAERAddr(xsyn, ysyn)
    #    spikes = [bd.PackWord([(bd.InputSpike.SYNAPSE_ADDRESS, syn_addr), (bd.InputSpike.SYNAPSE_SIGN, syn_sign)])] * num_spikes_in_bin
    #    D.SendSpikes(CORE, spikes, tag_times, False)

    print("this many tags", len(tags))

D.Flush()


print("this should be 2:", D.GetSynAERAddr(1,1))
for x in range(4):
    for y in range(4):
        print(x, ",", y, ":", D.GetSynAERAddr(x,y))

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
in_bin = 0
print(times)
events_per_bin = [0 for c in counts]

for y, x, t in zip(ys, xs, ns_to_s(np.array(spike_times))):
    if t > start_time and t < end_time + training_hold_time:
        if bin_idx+1 < len(times) and t > times[bin_idx + 1]:
            bin_idx += 1
            print("bin boundary at", t)
            print(" ", in_bin, " in bin")
            in_bin = 0
        counts[bin_idx][y, x] += 1
        in_bin += 1

n_to_plot = 4
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
#plt.plot(A[17,:])
plt.plot(A[:,:-1].T)
plt.savefig("tuning_curves.pdf")
print(A.shape)

print("* Done")
D.Stop()
