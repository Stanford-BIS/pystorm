"""
We want to send one spike to all Synapses and decode from all Somas.
This is equivalent to 1x1024 encode and 4096x1 decode.
The idea is that, ISI jitter will be averaged out by decoding across multiple Somas.
Then, we send EXC, INH or MIX spikes to this input node to calibrate the Synapse.
"""
import sys
import os
import time
import pickle
from numpy import diff, sort, median, array, zeros, savetxt
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
from matplotlib.pyplot import savefig, figure, subplot, hist, xlabel, ylabel, tight_layout, ion, show, suptitle, plot, sca, gca
from matplotlib.ticker import EngFormatter

from pystorm.PyDriver import bddriver as bd
from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
HAL = HAL()

def create_decode_encode_network(width, height, d_val):
    
    N = width * height
    net = graph.Network("net")
    decoders = np.ones((1, N)) * d_val
    tap_list = []
    for y in range(0, height, 2):
        for x in range(0, width, 2):
            idx = y * width + x
            tap_list.append((idx, 1))

    for y in range(0, height, 2):
        for x in range(0, width, 2):
            idx = y * width + x
            tap_list.append((idx, -1))
        
    i1 = net.create_input("i1", 1)
    p1 = net.create_pool("p1", (N, [tap_list]))
    b1 = net.create_bucket("b1", 1)
    o1 = net.create_output("o1", 1)
    
    net.create_connection("i1_to_p1", i1, p1, None)
    net.create_connection("p1_to_b1", p1, b1, decoders)
    net.create_connection("b1_to_o1", b1, o1, None)
    
    return net

# TAT has only 1024 entries. Since we are hitting the same synapse twice, we can only use 1024 somas or 512 synapses.
# Hence use 32x32 somas
#net = create_decode_encode_network(32, 32, 1.)
net = create_decode_encode_network(8, 8, 1.)
HAL.map(net)

#HAL.set_input_rate(net.get_inputs()[0], 0, 1000, 0)

# Sweep DAC
bddriver = HAL.driver

"""
CORE = 0
for addr in range(4096):
    bddriver.SetSomaGain(CORE, addr, bd.bdpars.SomaGainId.ONE)
    bddriver.SetSomaOffsetSign(CORE, addr, bd.bdpars.SomaOffsetSignId.NEGATIVE)
    bddriver.SetSomaOffsetMultiplier(CORE, addr, bd.bdpars.SomaOffsetMultiplierId.ZERO)

bddriver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_EXC     , 512)
bddriver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_DC      , 544)
bddriver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_INH     , 512)
bddriver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_LK      , 10)
bddriver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_PD      , 100)
bddriver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_PU      , 1024)
bddriver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_DIFF_G      , 1024)
bddriver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_DIFF_R      , 1)
bddriver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , 25)
bddriver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SOMA_REF    , 512)
"""

HAL.flush()

HAL.enable_output_recording()
HAL.start_traffic()

for idx in range(10):
    # Measure
    time.sleep(2)
    res = HAL.get_outputs()  # [[t_ns, id_op(o1), dim(0), count], ...]
    # if rate of accumulator tripping / decode rate < 20M, else TX probably saturated
    ovf = HAL.get_overflow_counts()
    t_int = (res[-1, 0] - res[0, 0]) * 1e-9

    print(sum(res[:, 3] / t_int), ovf)
