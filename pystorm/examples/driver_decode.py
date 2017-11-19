import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pickle
import sys
import time

from pystorm.PyDriver import bddriver as bd
import driver_util
from driver_util import s_to_ns, ns_to_s

np.random.seed(0)

#########################################
# user parameters

pool_width = 8
pool_height = pool_width
num_neurons = pool_height * pool_width

dims = 1
K = 4 # 2*K = taps per dim

fmax = 1000
num_training_points_per_dim = 10
training_hold_time = .5 # seconds

downstream_time_res = 10 * 1000 # ns
upstream_time_res = downstream_time_res * 10 # ns

#########################################
# derived parameters

# tap points
tap_points = [{} for d in range(dims)]
all_tap_points = []

# tap points are +/- in opposite corners for dims == 1
# ignoring K, using all synapses
if dims == 1:
    tap_point_syn_stride = 1
    for xsyn in range(0, pool_width//2, tap_point_syn_stride):
        for ysyn in range(0, pool_width//2, tap_point_syn_stride):
            if xsyn < pool_width//4 and ysyn < pool_height//4 or xsyn > pool_width//4 and ysyn > pool_height//4:
                sign = 1 # +
            else:
                sign = 0 # -

            tap_points[0][(ysyn, xsyn)] = sign 
            all_tap_points.append((ysyn, xsyn))

# otherwise generate random taps, but don't use the same tap twice
else:
    for d in range(dims):
        # add a + tap and a - tap at a random point
        for sign in [1, 0]:
            while True:
                xsyn = np.random.randint(pool_width//2)
                ysyn = np.random.randint(pool_width//2)
                if (ysyn, xsyn) not in all_tap_points: # can saturate threshold if two tap points collide
                    tap_points[d][(ysyn, xsyn)] = sign
                    all_tap_points.append((ysyn, xsyn))
                    break # found an unused tap, break out of "while True"


#########################################

CORE = 0

D = driver_util.standard_startup(downstream_time_res, upstream_time_res)

print("* Init'ing DACs")
time.sleep(2)
D.InitDAC(CORE)

# magical DAC settings (DC is the most important, with the default, inhibition doesn't work)
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
    D.DisableSynapseADC(CORE, i);

#for i in range(256):
#    D.OpenDiffusorAllCuts(CORE, i);

# then only enable the neurons and synapses we're using
# enabling only the used synapses is very important to improve SNR
for xsyn in range(pool_width//2):
    for ysyn in range(pool_height//2):

        # un-kill the synapse, if it's a tap
        # set gain of neurons under synapses to minimum (1/4)
        syn_aer_idx = D.GetSynAERAddr(xsyn, ysyn)
        if (ysyn, xsyn) in all_tap_points:
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
                
                if dims == 1 and pool_width == 8:
                    # some custom bias twiddling
                    if xnrn < pool_width // 2 and ynrn < pool_height // 2:
                        D.SetSomaOffsetSign(CORE, soma_aer_idx, bd.bdpars.SomaOffsetSignId.NEGATIVE);
                        D.SetSomaOffsetMultiplier(CORE, soma_aer_idx, bd.bdpars.SomaOffsetMultiplierId.THREE)
                    if xnrn > pool_width // 2 and ynrn > pool_height // 2:
                        D.SetSomaOffsetSign(CORE, soma_aer_idx, bd.bdpars.SomaOffsetSignId.POSITIVE);
                        D.SetSomaOffsetMultiplier(CORE, soma_aer_idx, bd.bdpars.SomaOffsetMultiplierId.ONE)

# open diffuser within pool
for xtile in range(pool_width//4):
    for ytile in range(pool_width//4):
        mem_aer_idx = D.GetMemAERAddr(xtile, ytile)
        D.CloseDiffusorAllCuts(CORE, mem_aer_idx);
        #D.OpenDiffusorAllCuts(CORE, mem_aer_idx);

D.Flush() 

# set up the TAT
print("* Programming TAT for tap points")
tat_entries = []
for d in range(dims):
    tap_points_in_dim = tap_points[d]
    for tap_idx, tap in enumerate(tap_points_in_dim):

        # two tap points per entry
        if tap_idx % 2 == 1:

            if tap_idx == len(tap_points_in_dim) - 1:
                stop = 1
            else:
                stop = 0

            y0, x0 = last_tap
            sign0  = last_sign
            sign1  = tap_points_in_dim[tap]
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
        last_sign = tap_points_in_dim[tap]

input_tags = list(range(0, dims*K, K))
print("input tags by dimension", input_tags)
D.SetMem(CORE, bd.bdpars.BDMemId.TAT0, tat_entries, 0)

time.sleep(.1) # wait a little for the TAT to program

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

total_training_points = num_training_points_per_dim ** dims
first_bin_time = ns_to_s(D.GetFPGATime()) + .5 # s
last_bin_time = first_bin_time + total_training_points * training_hold_time # s
bin_times = np.linspace(first_bin_time, last_bin_time, total_training_points)

start_time = first_bin_time
end_time = last_bin_time + training_hold_time

print("* Running training for", end_time - start_time, "seconds")

rates_1d = np.linspace(-fmax, fmax, num_training_points_per_dim).astype(int)
if dims > 1:
    rates = np.meshgrid(*(rates_1d * dims))
else:
    rates = [rates_1d]
rates_flat = [r.flatten() for r in rates]

for bin_idx, time_ns in enumerate(s_to_ns(bin_times)):
    bin_rates = [rf[bin_idx] for rf in rates_flat]
    D.SetSpikeGeneratorRates(CORE, list(range(dims)), input_tags, bin_rates, time_ns, False)
D.Flush()

fudge = 1
time.sleep(end_time - start_time + fudge)

print(D.GetOutputQueueCounts())
spikes, spike_times = D.RecvSpikes(CORE)

print("* Processing spikes")

p_objs = {"spikes":spikes, "spike_times":spike_times, "times":bin_times}
pfile = open("single_pool.pck", "wb")
pickle.dump(p_objs, pfile)
pfile.close()

yxs = np.array(D.GetSomaXYAddrs(spikes)).astype(int)

counts = [np.zeros((pool_height, pool_width)).astype(int) for i in range(total_training_points)]
ys = yxs // 64
xs = yxs % 64

bin_idx = 0
in_bin = 0
print(bin_times)
events_per_bin = [0 for c in counts]

for y, x, t in zip(ys, xs, ns_to_s(np.array(spike_times))):
    if t > start_time and t < end_time:
        if bin_idx+1 < len(bin_times) and t > bin_times[bin_idx + 1]:
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
    plt.imshow(counts[int(total_training_points * pidx / n_to_plot)], cmap='Greys')
    plt.colorbar()
plt.savefig("spikes.pdf")

plt.figure()
plt.hist(spike_times, bins=100)
plt.savefig("spike_times.pdf")

# tuning curves
A = np.zeros((num_neurons, total_training_points))
for train_idx, c in enumerate(counts):
    A[:, train_idx] = c.flatten()

plt.figure()
plt.plot(A.T)
plt.savefig("tuning_curves.pdf")
print(A.shape)

print("* Done")
D.Stop()
