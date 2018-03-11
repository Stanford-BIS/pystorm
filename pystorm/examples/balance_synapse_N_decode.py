import time
import pickle
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt

from pystorm.PyDriver import bddriver as bd
from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
HAL = HAL()

def create_decode_encode_network(width, height, d_val):
    
    N = width * height
    net = graph.Network("net")
    decoders = np.eye(N) * d_val
    tap_list = []
    for y in range(0, height, 2):
        for x in range(0, width, 2):
            idx = y * width + x
            tap_list.append((idx, 1))  # this should be +1 for EXC
    
    for y in range(0, height, 2):
        for x in range(0, width, 2):
            idx = y * width + x
            tap_list.append((idx, -1))  # this should be -1 for INH
    
    i1 = net.create_input("i1", 1)
    p1 = net.create_pool("p1", (N, [tap_list]))
    b1 = net.create_bucket("b1", N)
    o1 = net.create_output("o1", N)
    
    net.create_connection("i1_to_p1", i1, p1, None)
    net.create_connection("p1_to_b1", p1, b1, decoders)
    net.create_connection("b1_to_o1", b1, o1, None)
    
    return net

# TAT has only 1024 entries. Since we are hitting the same synapse twice, we can only use 1024 somas or 512 synapses.
# Hence use maximum 32x32 somas
WIDTH = 8
HEIGHT = 8
DECIMATION = 10

net = create_decode_encode_network(WIDTH, HEIGHT, 1./DECIMATION)
HAL.map(net)

# Sweep DAC
bddriver = HAL.driver

CORE = 0
for addr in range(256):
    bddriver.OpenDiffusorAllCuts(CORE, addr)
    
# Disable all soma that are not under a synapse
xaddr, yaddr = [_x.flatten() for _x in np.meshgrid(np.arange(0, 64), np.arange(0, 64), indexing='xy')]
for _x, _y in zip(xaddr, yaddr):
    soma_addr = bddriver.GetSomaAERAddr(_x, _y)
    syn_row = int(np.floor(_y / 2))
    # Odd row somas are not under a synapse
    if (_y % 2) == 1:
        bddriver.DisableSoma(CORE, soma_addr)
    # Odd row synapses have odd column somas under
    if (syn_row % 2) == 0:
       if (_x % 2)  == 1:
           bddriver.DisableSoma(CORE, soma_addr)
    # Even row synapses have even column somas under
    else:
        if (_x % 2)  == 0:
            bddriver.DisableSoma(CORE, soma_addr)
            
    bddriver.SetSomaGain(CORE, soma_addr, bd.bdpars.SomaGainId.ONE)
    bddriver.SetSomaOffsetSign(CORE, soma_addr, bd.bdpars.SomaOffsetSignId.POSITIVE)
    bddriver.SetSomaOffsetMultiplier(CORE, soma_addr, bd.bdpars.SomaOffsetMultiplierId.ONE)

bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_EXC    , 512)
bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_DC     , 638)
bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_INH    , 512)
bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_LK     , 10)
bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PD     , 50)
bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PU     , 1024)
bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_DIFF_G     , 512)
bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_DIFF_R     , 1)
bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, 10)
bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_REF   , 512)

HAL.flush()

HAL.enable_output_recording()
HAL.start_traffic()

MIN_RATE = 0
MAX_RATE = 1000
BALANCE_TOL = 0.01  # 5%

def measure_rate(input_rate):
    # Set spike generator rate
    HAL.set_input_rate(net.get_inputs()[0], 0, input_rate, 0)
    # Discard first 2 seconds
    time.sleep(2)
    HAL.get_outputs()
    # if rate of accumulator tripping / decode rate < 20M, else TX probably saturated
    ovf = HAL.get_overflow_counts()
    if ovf > 0:
        print("WARNING: Overflow detected")
    
    # Keep next two seconds
    time.sleep(5)
    res = HAL.get_outputs()  # [[t_ns, id_op(o1), dim(0), count], ...]
    ovf = HAL.get_overflow_counts()
    if ovf > 0:
        print("WARNING: Overflow detected")
    dims = np.array(np.unique(res[:, 2]), dtype=np.int)
    res_N = np.zeros((dims.shape[0], 3))  # [tini, tend, count]
    for _idx, _ent in enumerate(res):
        _didx = _ent[2]
        if res_N[_didx, 0] <= 0:
            res_N[_didx, 0] = _ent[0]
        else:
            res_N[_didx, 1] = _ent[0]
        res_N[_didx, 2] += _ent[3]
    t_int = (res_N[:, 1] - res_N[:, 0]) * 1e-9  # in seconds
    base_freq = res_N[:, 2] / t_int  # in Hz
    
    return (base_freq, dims)
        
FREQ_LIST = np.array([int(_x) for _x in 10**np.linspace(0, 4, 10)], dtype=np.int)
results = np.zeros((FREQ_LIST.shape[0], WIDTH * HEIGHT), dtype=np.float)
res_error = np.zeros_like(results, dtype=np.float)
for _idx, _freq in enumerate(FREQ_LIST):
    _res, _dims = measure_rate(_freq)
    results[_idx, _dims] = _res
    _err = _res / results[0, _dims] - 1
    res_error[_idx, _dims] = 100 * _err
    print("IN: %d, ERR: %g" % (_freq, np.amax(np.abs(_err))))

print(HAL.driver.GetOutputQueueCounts())
    
if np.amax(np.abs(res_error)) <= BALANCE_TOL:
    print("INFO: [DC = %d], Validation successful. Max Error %g perc." % (dc_val, np.amax(np.abs(res_error))))
else:
    print("INFO: Validation failed. Max Error %g perc." % (np.amax(np.abs(res_error))))

resnz = results[:, results[0, :] > 1]
errnz = res_error[:, results[0, :] > 1]
_sidx = np.argsort(resnz[0, :])
neuron_cmap = [plt.get_cmap('inferno')(idx / (resnz[0, :].shape[0])) for idx in range(resnz[0, :].shape[0] + 1)]

plt.subplot(211)
plt.gca().set_prop_cycle(plt.cycler('color', neuron_cmap)) 
plt.loglog(FREQ_LIST, resnz[:, _sidx], linewidth=1)
plt.ylabel("Output Frequency (Hz)")
plt.title("Soma > 1Hz = %d/%d" % (resnz[0, :].shape[0], results[0, :].shape[0]))

plt.subplot(212)
plt.gca().set_prop_cycle(plt.cycler('color', neuron_cmap)) 
plt.semilogx(FREQ_LIST, errnz[:, _sidx], linewidth=1)
plt.ylabel("100 * (f(in) - f(0)) / f(0)")
plt.xlabel("Input Frequency (Hz)")
plt.ylim((-10, 10))

plt.tight_layout()

plt.savefig("syn_balance_validation_N_decode.pdf")

OF = open("syn_balance_validation_N_decode.pickle", "wb")
pickle.dump((FREQ_LIST, results, res_error), OF)
OF.close()
