import numpy as np
import time

import matplotlib as mpl
from matplotlib.pyplot import savefig, figure, subplot, hist, xlabel, ylabel, tight_layout, ion, show, suptitle, plot, sca, gca, semilogy

from pystorm.hal import HAL # HAL is a singleton, importing immediately sets up a HAL and its C Driver

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

np.random.seed(0)

###########################################
# pool size parameters

width = 60
height = 60
width_height = (width, height)
N = width * height

###########################################
# misc driver parameters
downstream_time_res = 10000 # ns
upstream_time_res = 100000 # ns

HAL.set_time_resolution(downstream_time_res, upstream_time_res)

###########################################
# tap point specification

Din = 1
tap_matrix = np.zeros((N, Din), dtype=np.int)

###########################################
# specify network using HAL

net = graph.Network("net")

Dout = 1
decoders = np.zeros((Dout, N), dtype=np.float)
decoders[0, N-1] = 0.5
#decoders[0, 0] = 0.5

p1 = net.create_pool("p1", tap_matrix)
b1 = net.create_bucket("b1", Dout)
o1 = net.create_output("o1", Dout)

decoder_conn = net.create_connection("c_p1_to_b1", p1, b1, decoders)
net.create_connection("c_b1_to_o1", b1, o1, None)

###########################################
# invoke HAL.map(), make tons of neuromorph/driver calls under the hood

# map network
print("calling map")
HAL.map(net)

CORE = 0
driver = HAL.driver
for addr in range(256):
    driver.OpenDiffusorAllCuts(CORE, addr)

for addr in range(1024):
    driver.DisableSynapse(CORE, addr)

for addr in range(4096):
    driver.DisableSoma(CORE, addr)
    driver.SetSomaGain(CORE, addr, bd.bdpars.SomaGainId.ONE)
    driver.SetSomaOffsetSign(CORE, addr, bd.bdpars.SomaOffsetSignId.NEGATIVE)
    driver.SetSomaOffsetMultiplier(CORE, addr, bd.bdpars.SomaOffsetMultiplierId.ZERO)
    
insurgent_idx = (0, 0)
incumbent_idx = (width - 1, height - 1)

driver.EnableSomaXY(CORE, insurgent_idx[0], insurgent_idx[1])
driver.EnableSomaXY(CORE, incumbent_idx[0], incumbent_idx[1])
#driver.EnableSynapseXY(CORE, incumbent_idx[0] // 2, incumbent_idx[1] // 2)
driver.SetSomaGainXY(CORE, incumbent_idx[0], incumbent_idx[1], bd.bdpars.SomaGainId.ONE_FOURTH)
driver.SetSomaOffsetSignXY(CORE, incumbent_idx[0], incumbent_idx[1], bd.bdpars.SomaOffsetSignId.POSITIVE)
driver.SetSomaOffsetMultiplierXY(CORE, incumbent_idx[0], incumbent_idx[1], bd.bdpars.SomaOffsetMultiplierId.ONE)

driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_EXC     , 512)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_DC      , 550)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_INH     , 512)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_LK      , 10)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_PD      , 100)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_PU      , 1024)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_DIFF_G      , 1024)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_DIFF_R      , 1)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , 4)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SOMA_REF    , 512)

driver.Flush()

def filter_tags(tags):
    times_and_cts = {}
    for t, output_id, dim, ct in tags:
        if output_id not in times_and_cts:
            times_and_cts[output_id] = {}
        if dim not in times_and_cts[output_id]:
            times_and_cts[output_id][dim] = []
        times_and_cts[output_id][dim].append((t, ct))
    return times_and_cts

# Experiment

GAIN = bd.bdpars.SomaGainId
OFFSET = bd.bdpars.SomaOffsetSignId
OFF_MULT = bd.bdpars.SomaOffsetMultiplierId

HAL.start_traffic(flush=False)
HAL.enable_spike_recording(flush=False)
HAL.enable_output_recording(flush=True)

HAL.get_outputs()

data = {}
ion()
figure()
_ax1 = subplot(211)
_ax2 = subplot(212)
for _sign in [OFFSET.POSITIVE, OFFSET.NEGATIVE]:
    for _mult in [OFF_MULT.ZERO, OFF_MULT.ONE, OFF_MULT.TWO, OFF_MULT.THREE]:
        for _gain in [GAIN.ONE, GAIN.ONE_HALF, GAIN.ONE_THIRD, GAIN.ONE_FOURTH]:
            driver.SetSomaGainXY(CORE, insurgent_idx[0], insurgent_idx[1],_gain)
            driver.SetSomaOffsetSignXY(CORE, insurgent_idx[0], insurgent_idx[1], _sign)
            driver.SetSomaOffsetMultiplierXY(CORE, insurgent_idx[0], insurgent_idx[1], _mult)
            print((_sign, _mult, _gain))
            driver.Flush()
            
            time.sleep(1)
            HAL.get_outputs()
            
            time.sleep(2)
            _outputs = HAL.get_outputs()
            if _outputs.shape[0] > 0:
                if not np.all(_outputs[:, 1] == o1):
                    print("WARNING! output_id wrong")
                if not np.all(_outputs[:, 2] == 0):
                    print("WARNING! dims wrong")
                if not np.all((_outputs[:, 3] == 0) | (_outputs[:, 3] == 1)):
                    print("WARNING! counts wrong")
                _res = _outputs[_outputs[:, 3] == 1, 0]
                _isi = np.diff(_res) * 1e-9
                data[(_sign, _mult, _gain)] = _isi
                print("INFO! Number of output events: %d" % _outputs.shape[0])
                print("INFO! Sum of counts: %d" % np.sum(_outputs[:, 3]))
                print("INFO! Number of ISI: %d" % len(_isi))
                sca(_ax1)
                semilogy(np.sort(_isi))
                sca(_ax2)
                semilogy(_isi)
                show()
            else:
                print("WARNING! no events")

            _spikes = HAL.get_spikes()
            print("INFO! Number of spikes of '0' neuron: %d" % np.sum(_spikes[:, 2] == 0))
            print("INFO! Number of spikes of '3599' neuron: %d" % np.sum(_spikes[:, 2] == 3599))
tight_layout()
show()
