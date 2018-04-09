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

def create_decode_encode_network(width, height, d_val, stim_mode='both'):
    N = width * height
    net = graph.Network("net")
    decoders = np.ones((1, N)) * d_val
    tap_list = []
    if stim_mode == 'both':
        _w1 = 1
        _w2 = -1
    elif stim_mode == 'exc':
        _w1 = 1
        _w2 = 1
    elif stim_mode == 'inh':
        _w1 = -1
        _w2 = -1
    for y in range(0, height, 2):
        for x in range(0, width, 2):
            idx = y * width + x
            tap_list.append((idx, _w1))  # this should be +1 for EXC
    
    for y in range(0, height, 2):
        for x in range(0, width, 2):
            idx = y * width + x
            tap_list.append((idx, _w2))  # this should be -1 for INH
    
    i1 = net.create_input("i1", 1)
    p1 = net.create_pool("p1", (N, [tap_list]))
    b1 = net.create_bucket("b1", 1)
    o1 = net.create_output("o1", 1)
    
    net.create_connection("i1_to_p1", i1, p1, None)
    net.create_connection("p1_to_b1", p1, b1, decoders)
    net.create_connection("b1_to_o1", b1, o1, None)
    
    return net

# TAT has only 1024 entries. Since we are hitting the same synapse twice, we can only use 1024 somas or 512 synapses.
# Hence use maximum 32x32 somas
WIDTH = 16
HEIGHT = 16
DECIMATION = 100

def setup_exp(stim_type='both'):
    net = create_decode_encode_network(WIDTH, HEIGHT, 1./DECIMATION, stim_type)
    HAL.map(net)

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

    print("[INFO] Explicitly setting DAC values")
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
    return net

def measure_rate(net, input_rate):
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
    t_int = (res[-1, 0] - res[0, 0]) * 1e-9  # in seconds
    base_freq = np.sum(res[:, 3]) / t_int  # in Hz
    
    return base_freq

def run_exp(FREQ_LIST, stim_type='both'):
    net = setup_exp(stim_type)
    results = np.zeros_like(FREQ_LIST, dtype=np.float)
    res_error = np.zeros_like(FREQ_LIST, dtype=np.float)
    for _idx, _freq in enumerate(FREQ_LIST):
        print("[INFO] freq = %d" % _freq)
        _res = measure_rate(net, _freq)
        results[_idx] = _res
        _err = _res / results[0] - 1
        res_error[_idx] = 100 * _err
    return (results, res_error)
    
#FREQ_LIST = np.array([int(_x) for _x in 10**np.linspace(0, 4, 10)], dtype=np.int)
FREQ_LIST = np.linspace(1, 1000, 11, dtype=np.int)
res = dict()
err = dict()
for _stype in ['both', 'exc', 'inh']: 
    print("[INFO] Runing %s" % _stype)
    res[_stype], err[_stype]  = run_exp(FREQ_LIST, _stype)

print("[INFO] max common-mode error = %g" % np.amax(np.abs(res['both'])))

OF = open("syn_balance_test.pickle", "wb")
pickle.dump((FREQ_LIST, res, err), OF)
OF.close()
    
plt.subplot(211)
plt.plot(FREQ_LIST,  res['exc'] - res['both'][0], 'd-', label='exc')
plt.plot(-FREQ_LIST, res['inh'] - res['both'][0], 'o-', label='inh')
plt.ylabel("Output Frequency (Hz)")
plt.xlabel("Input Frequency (Hz)")
plt.grid(True)
plt.legend(loc='best')

plt.subplot(223)
plt.plot(FREQ_LIST, res['both'], '.-', label='both')
plt.ylabel("Output Frequency (Hz)")
plt.xlabel("Input Frequency (Hz)")
plt.grid(True)
plt.legend(loc='best')

plt.subplot(224)
plt.plot(FREQ_LIST, err['both'], '.-', label='both')
plt.plot(FREQ_LIST, err['exc'], 'd-', label='exc')
plt.plot(FREQ_LIST, err['inh'], 'o-', label='inh')
plt.ylabel("% Error")
plt.xlabel("Input Frequency (Hz)")
plt.grid(True)

plt.tight_layout()

plt.savefig("syn_balance_test.pdf")
