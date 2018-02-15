import time
import pickle
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt

from pystorm.PyDriver import bddriver as bd
from pystorm.hal import HAL # HAL is a singleton, importing immediately sets up a HAL and its C Driver
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

def create_decode_encode_network(width, height, d_val):
    
    N = width * height
    net = graph.Network("net")
    decoders = np.ones((1, N)) * d_val
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
bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_DC     , 544)
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
BALANCE_TOL = 0.005  # 0.5%

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
    t_int = (res[-1, 0] - res[0, 0]) * 1e-9  # in seconds
    base_freq = np.sum(res[:, 3]) / t_int  # in Hz
    
    return base_freq

def search_bias(bias_type):
    found_value = False
    sweep_bound = np.array([1, 1024], dtype=np.uint)
    while True:
        # new VAL is middle of bound
        dac_val = int(np.floor(np.mean(sweep_bound)))
        # if new VAL is at bounds, exit
        if sweep_bound[0] == dac_val:
            found_value = True
        # Set DAC value
        bddriver.SetDACCount(CORE, bias_type, dac_val)
        HAL.flush()

        # Measure output at 1Hz
        lo_freq = measure_rate(1)
        # Measure output at 1000Hz
        hi_freq = measure_rate(10000)
        
        err_freq = (hi_freq - lo_freq) / lo_freq
        # if difference within tolerance, done
        if abs(err_freq) <= BALANCE_TOL:
            print("[INFO] error tolerance achieved")
            found_value = True

        # if f(1000) > f(0), excitation is stronger => UP DC
        if hi_freq > lo_freq:
            # if UP, new bound is (VAL, PREV_VAL)
            sweep_bound[0] = dac_val
        # else if f(1000) < f(0), inhibition is stronger => DOWN DC
        elif hi_freq < lo_freq:
            # if DOWN, new bound is (PREV_MAX, VAL)
            sweep_bound[1] = dac_val
        else:
            print("[INFO] hi-freq = lo-freq")
            found_value = True
            
        print("err: %g, lo: %g(Hz), hi: %g(Hz), current-val: %d, new-bounds: (%d, %d)" %
              (err_freq, lo_freq, hi_freq, dac_val, sweep_bound[0], sweep_bound[1]))
        
        if found_value:
            break
    
    return dac_val
        
dc_val = search_bias(bd.bdpars.BDHornEP.DAC_SYN_DC)
print("DC Value: %d" % dc_val)

#bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_EXC, 1024)
#bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_DC , 1)
#bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_INH, 1)
#HAL.flush()

# validation
print("Validating...")
FREQ_LIST = np.array([int(_x) for _x in 10**np.linspace(0, 4, 10)], dtype=np.int)
results = np.zeros_like(FREQ_LIST, dtype=np.float)
res_error = np.zeros_like(FREQ_LIST, dtype=np.float)
for _idx, _freq in enumerate(FREQ_LIST):
    _res = measure_rate(_freq)
    results[_idx] = _res
    _err = _res / results[0] - 1
    res_error[_idx] = 100 * _err
    print("IN: %d, OUT: %g, ERR: %g" % (_freq, _res, _err))
    
if np.amax(np.abs(res_error)) <= (100 * BALANCE_TOL):
    print("INFO: [DC = %d], Validation successful. Max Error %g perc." % (dc_val, np.amax(np.abs(res_error))))
else:
    print("INFO: Validation failed. Max Error %g perc." % (np.amax(np.abs(res_error))))
    
plt.subplot(211)
plt.semilogx(FREQ_LIST, results)
plt.ylabel("Output Frequency (Hz)")

plt.subplot(212)
plt.semilogx(FREQ_LIST, res_error)
plt.ylabel("100 * (f(in) - f(0)) / f(0)")
plt.xlabel("Input Frequency (Hz)")

plt.tight_layout()

plt.savefig("syn_balance_validation.pdf")

OF = open("syn_balance_validation.pickle", "wb")
pickle.dump((FREQ_LIST, results), OF)
OF.close()
