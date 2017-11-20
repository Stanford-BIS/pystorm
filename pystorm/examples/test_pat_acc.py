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

pool_width = 8
pool_height = pool_width
num_neurons = pool_height * pool_width

# not actually using taps, this is just copy-paste
# want to have neuron config that we know spikes
dims = 1
tap_points = [{} for d in range(dims)]
all_tap_points = []
tap_point_syn_stride = 1
for xsyn in range(0, pool_width//2, tap_point_syn_stride):
    for ysyn in range(0, pool_width//2, tap_point_syn_stride):
        if xsyn < pool_width//4 and ysyn < pool_height//4 or xsyn > pool_width//4 and ysyn > pool_height//4:
            sign = 1 # +
        else:
            sign = 0 # -

        tap_points[0][(ysyn, xsyn)] = sign 
        all_tap_points.append((ysyn, xsyn))

def configure_pool(D):
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
                        # some custom bias twiddling, this is specific to a particular chip
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


###########################

D = driver_util.standard_startup()
CORE = 0
driver_util.standard_DAC_settings(D, CORE)
input_tags = configure_pool(D)

# kill all neurons again
for i in range(4096):
    D.DisableSoma(CORE, i)

# enable the two in the corner
D.EnableSoma(CORE, 0)
D.EnableSoma(CORE, 1)
D.Flush()

###########################
# program a positive and a negative decoder
# one neuron emits positive tags, the other negative tags

print("* Programming AM")
acc_entries = [
    bd.PackWord([
      (bd.AMWord.STOP, 0),
      (bd.AMWord.THRESHOLD, 2),
      (bd.AMWord.NEXT_ADDRESS, 2**11)]),
    bd.PackWord([
      (bd.AMWord.STOP, 1),
      (bd.AMWord.THRESHOLD, 2),
      (bd.AMWord.NEXT_ADDRESS, 2**11+1)])]
D.SetMem(CORE, bd.bdpars.BDMemId.AM, acc_entries, 0)

print("* Programming MM")
D.SetMem(CORE, bd.bdpars.BDMemId.MM, [64,     0], 0)
D.SetMem(CORE, bd.bdpars.BDMemId.MM, [0, 255-64], 256)

print("* Programming PAT")
PAT_entry = bd.PackWord([
  (bd.PATWord.AM_ADDRESS, 0),
  (bd.PATWord.MM_ADDRESS_HI, 0),
  (bd.PATWord.MM_ADDRESS_LO, 0)])

D.SetMem(CORE, bd.bdpars.BDMemId.PAT, [PAT_entry], 0)

time.sleep(.5)

D.SetSpikeTrafficState(CORE, True, True)
D.SetSpikeTrafficState(CORE, True, True)
D.SetSpikeTrafficState(CORE, True, True)
D.SetSpikeTrafficState(CORE, True, True)

D.SetSpikeDumpState(CORE, True, True)
D.SetSpikeDumpState(CORE, True, True)
D.SetSpikeDumpState(CORE, True, True)
D.SetSpikeDumpState(CORE, True, True)

print("* turning on neurons, Waiting")

time.sleep(4)

D.SetSpikeTrafficState(CORE, False, True)
D.SetSpikeTrafficState(CORE, False, True)
D.SetSpikeTrafficState(CORE, False, True)
D.SetSpikeTrafficState(CORE, False, True)
D.SetSpikeDumpState(CORE, False, True)
D.SetSpikeDumpState(CORE, False, True)
D.SetSpikeDumpState(CORE, False, True)
D.SetSpikeDumpState(CORE, False, True)

print('these should match')
print(D.GetOutputQueueCounts())
time.sleep(1)
print(D.GetOutputQueueCounts())

tags_out, tags_out_times = D.RecvTags(CORE)
cts = {}
for tag in tags_out:
  tag_id = bd.GetField(tag, bd.AccOutputTag.TAG)
  ct = bd.GetField(tag, bd.AccOutputTag.COUNT)
  if ct > 255:
      ct -= 512
  if tag_id not in cts:
    cts[tag_id] = 0
  cts[tag_id] += ct

spk_cts = {}
spikes, spike_times = D.RecvSpikes(CORE)
for spike in spikes:
    if spike not in spk_cts:
        spk_cts[spike] = 0
    spk_cts[spike] += 1

print("spikes by neuron")
print(spk_cts)
print("tag cts, neuron 0 is .5, neuron 1 is -.5")
print(cts)

print("* Done")
D.Stop()
