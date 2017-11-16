import numpy as np
import matplotlib.pyplot as plt
from pystorm.PyDriver import bddriver as bd
import driver_util
import time
import numpy as np


pool_width = 16
pool_height = pool_width

input_tag = 0

# tap points are + on the left half, - on the right half
tap_point_syn_stride = 2
tap_points = []
for xsyn in range(0, pool_width//2, tap_point_syn_stride):
    for ysyn in range(0, pool_width//2, tap_point_syn_stride):
        if xsyn < pool_width//4:
            tap_points.append((0, xsyn, ysyn)) # 0 sign means pos
        else:
            tap_points.append((1, xsyn, ysyn)) # 1 sign means neg

time_resolution_us = 10 # us

#########################################

CORE = 0

D = driver_util.standard_startup(time_resolution_us * 1000, time_resolution_us * 1000 * 100)

print("* Init'ing DACs")
time.sleep(2)
D.InitDAC(CORE)

#print("* Configuring neuron array")
#
## first, disable everything
#for i in range(4096):
#    D.DisableSoma(CORE, i)
#    D.SetSomaGain(CORE, i, bd.bdpars.SomaGainId.ONE);
#    D.SetSomaOffsetSign(CORE, i, bd.bdpars.SomaOffsetSignId.POSITIVE);
#    D.SetSomaOffsetMultiplier(CORE, i, bd.bdpars.SomaOffsetMultiplierId.ZERO);
#
#for i in range(1024):
#    D.DisableSynapse(CORE, i);
#    D.DisableSynapseADC(CORE, i);
#
#for i in range(256):
#    D.OpenDiffusorAllCuts(CORE, i);

# enable the neurons and synapses we're using
# diffuser is open but that's fine for one pool
for xsyn in range(pool_width//2):
    for ysyn in range(pool_height//2):

        # un-kill the synapse
        syn_aer_idx = D.GetSynAERAddr(xsyn, ysyn)
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

# set up the TAT
print("* Programming TAT for tap points")
tat_entries = []
for tap_idx, tap in enumerate(tap_points):

    last_tap = tap

    # two tap points per entry
    if tap_idx % 2 == 0:

        if tap_idx == len(tap_points) - 1:
            stop = 1
        else:
            stop = 0

        sign0, x0, y0 = last_tap
        sign1, x1, y1 = tap

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
#time.sleep(60)

print("* Reading TAT")
dumped_tat = D.DumpMem(CORE, bd.bdpars.BDMemId.TAT0)

if driver_util.compare_TAT_words(tat_entries, dumped_tat) == -1:
    D.Stop()
    exit(-1)

print("* Watching spikes")

# for some reason, you have to do this a couple times (maybe there's input slack?)
D.SetSpikeDumpState(CORE, True, True)
D.SetSpikeDumpState(CORE, True, True)
D.SetSpikeDumpState(CORE, True, True)
D.SetSpikeDumpState(CORE, True, True)

time.sleep(2)

print(D.GetOutputQueueCounts())
spikes, times = D.RecvSpikes(CORE)

# spike is a single field, no need to do this:
#bd.GetField(s, bd.OutputSpike.NEURON_ADDRESS)

print("processing spikes")
yxs = D.GetSomaXYAddrs(spikes)
counts = {}
for yx, t in zip(yxs, times):
   if yx not in counts:
       counts[yx] = 0
   counts[yx] += 1
print(counts)

print("* Done")
D.Stop()
