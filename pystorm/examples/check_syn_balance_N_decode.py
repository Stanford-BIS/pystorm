import time
import pickle
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.cbook import flatten

from pystorm.PyDriver import bddriver as bd
from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
HAL = HAL()

SPK_WEIGHT = {
    'exc': [1, 1],
    'inh': [-1, -1],
    'mixed': [1, -1]
}

def create_decode_encode_network(net, pool_id, width, height, d_val, spk_type='mixed', use_pool=False):
    N = width * height

    if use_pool:
        _sign1, _sign2 = SPK_WEIGHT[spk_type]

        tap_list = []
        for y in range(0, height, 2):
            for x in range(0, width, 2):
                idx = y * width + x
                tap_list.append((idx, _sign1))  # this should be +1 for EXC
        
        for y in range(0, height, 2):
            for x in range(0, width, 2):
                idx = y * width + x
                tap_list.append((idx, _sign2))  # this should be -1 for INH

        p1 = net.create_pool("p%d" % pool_id, (N, [tap_list]))

        i1 = net.create_input("i%d" % pool_id, 1)
        b1 = net.create_bucket("b%d" % pool_id, N)
        o1 = net.create_output("o%d" % pool_id, N)
        
        net.create_connection("i%d_to_p%d" % (pool_id, pool_id), i1, p1, None)
        decoders = np.eye(N) * d_val
        net.create_connection("p%d_to_b%d" % (pool_id, pool_id), p1, b1, decoders)
        net.create_connection("b%d_to_o%d" % (pool_id, pool_id), b1, o1, None)
    else:
        zero_encs = np.zeros((N, 1))
        p1 = net.create_pool("p%d" % pool_id, zero_encs)
    
    return(p1)

# TAT has only 1024 entries. Since we are hitting the same synapse twice, we can only use 2048 somas or 512 synapses.
# Hence use maximum 32x64 somas
WIDTH = 8
HEIGHT = 8
DECIMATION = 10
NUM_POOLS = 4096 // (WIDTH * HEIGHT)

def create_multi_pools(active_pool, spk_type):
    net = graph.Network("net")
    
    _pools = []
    for pid in range(NUM_POOLS):
       _pen = (pid == active_pool)
       _p = create_decode_encode_network(net, pid, WIDTH, HEIGHT, 1./DECIMATION, spk_type, _pen)
       _pools.append(_p)

    HAL.map(net)

    return (net, _pools)

bddriver = HAL.driver

def set_baseline():
    CORE = 0
    for addr in range(256):
        bddriver.OpenDiffusorAllCuts(CORE, addr)
        
    # Disable all soma that are not under a synapse
    xaddr, yaddr = [_x.flatten() for _x in np.meshgrid(np.arange(0, 64), np.arange(0, 64), indexing='xy')]
    for _x, _y in zip(xaddr, yaddr):
        soma_addr = bd.Driver.BDPars.GetSomaAERAddr(_x, _y)
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
        bddriver.SetSomaOffsetMultiplier(CORE, soma_addr, bd.bdpars.SomaOffsetMultiplierId.THREE)
    
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_EXC    , 512)
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_DC     , 576)
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_INH    , 512)
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_LK     , 10)
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PD     , 48)
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PU     , 1024)
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_DIFF_G     , 512)
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_DIFF_R     , 1)
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, 12)
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_REF   , 1024)
    
    HAL.flush()

MIN_RATE = 0
MAX_RATE = 1000
BALANCE_TOL = 0.01  # 5%

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
        
#FREQ_LIST = np.array([int(_x) for _x in 10**np.linspace(0, 3, 11)], dtype=np.int)
FREQ_LIST = np.array([int(_x) for _x in flatten([[1], np.linspace(100, 1000, 10)])], dtype=np.int)

spk_result = {
    'exc':np.zeros((FREQ_LIST.shape[0], 4096)),
    'inh':np.zeros((FREQ_LIST.shape[0], 4096)),
    'mixed':np.zeros((FREQ_LIST.shape[0], 4096))
}

SUB_P = 1
NUM_NEURONS = int(NUM_POOLS / SUB_P) * WIDTH * HEIGHT
def measure_pool_responses():
    for _pid, _pact in enumerate(range(int(NUM_POOLS / SUB_P))):
        print("[INFO] active pool: %d/%d" % (_pid + 1, NUM_POOLS))
        for _stype in SPK_WEIGHT.keys():
            print("[INFO] Measuring '%s' input response [%d/%d]" % (_stype, _pid + 1, NUM_POOLS))
            _net, _pools = create_multi_pools(_pact, _stype)

            HAL.enable_output_recording()
            HAL.start_traffic()

            set_baseline()

            for _idx, _freq in enumerate(FREQ_LIST):
                print("[INFO] Setting input frequency to %g" % _freq)
                _res, _dims = measure_rate(_net, _freq)
                spk_result[_stype][_idx, _pid * WIDTH * HEIGHT + _dims] = _res

                _nspikes = _res[_res > 0.1]
                print("[INFO] Measured output frequency range: (%g, %g)" % (np.amin(_nspikes), np.amax(_nspikes)))
                print("[INFO] Measured output frequency med/sig: (%g, %g)" % (np.median(_nspikes), np.std(_nspikes)))
                print(HAL.driver.GetOutputQueueCounts())

                OF = open("check_syn_balance_N_decode.pickle", "wb")
                pickle.dump((FREQ_LIST, spk_result, NUM_NEURONS), OF)
                OF.close()

            HAL.stop_traffic()
            HAL.disable_output_recording()

measure_pool_responses()

IF = open("./check_syn_balance_N_decode.pickle", "rb")
FREQ_LIST, spk_result, NUM_NEURONS = pickle.load(IF)
IF.close

FREQ_LOW = 2.  # > 2 spk/s with decimation = 10 => f_real = 20Hz
MIN_DEV = 0.1   # Use only those neurons whose f_max - f_min > (MIN_DEV * f_mid)

################################################################################
# Find neurons that show some difference in behavior beween INH and EXC
################################################################################
_fmax = spk_result['exc'][-1, :]   # Output at maximum excitation
_fmin = spk_result['inh'][-1, :]   # Output at maximum inhibition
_fmid = spk_result['mixed'][0, :]  # Output with zero input
_fdev = np.abs(_fmax - _fmin) / _fmid

VIDX = np.where(_fdev >= MIN_DEV)[0]
SORTVIDX = VIDX[np.argsort(_fmax[VIDX])]

################################################################################
# Find 5th, 50th and 95th percentile neurons
################################################################################
_f5, _f50, _f95 = np.nanpercentile(spk_result['exc'][:, SORTVIDX], (5, 50, 95), axis=1)
IDX5 = np.where(spk_result['exc'][-1, SORTVIDX] >= _f5[-1])[0][0]
IDX50 = np.where(spk_result['exc'][-1, SORTVIDX] >= _f50[-1])[0][0]
IDX95 = np.where(spk_result['exc'][-1, SORTVIDX] <= _f95[-1])[0][-1]

neuron_cmap = [plt.get_cmap('viridis')(idx / VIDX.shape[0]) for idx in range(VIDX.shape[0])]

plt.figure()
_ax1 = plt.subplot(221)
plt.gca().set_prop_cycle(plt.cycler('color', neuron_cmap))
plt.plot(FREQ_LIST, spk_result['exc'][:, SORTVIDX], '.-', markersize=1., alpha=0.3)
plt.gca().set_prop_cycle(plt.cycler('color', neuron_cmap))
plt.plot(-1 * FREQ_LIST, spk_result['inh'][:, SORTVIDX], '.-', markersize=1., alpha=0.3)
plt.title("Raw data (N=%d/%d)" % (len(SORTVIDX), NUM_NEURONS))
plt.ylabel("Output Frequency")
plt.ylim((0., _fmax[SORTVIDX[IDX95 + 1]]))

_ax2 = plt.subplot(222)
_spkres = spk_result['exc'][:, SORTVIDX]
plt.fill_between(FREQ_LIST, _spkres[:, IDX5], _spkres[:, IDX95], color=neuron_cmap[int(0.5 * SORTVIDX.shape[0])],
        alpha=.5)
plt.plot(FREQ_LIST, _spkres[:, IDX50], '.-', color=neuron_cmap[int(0.5 * SORTVIDX.shape[0])], zorder=99)
_spkres = spk_result['inh'][:, SORTVIDX]
plt.fill_between(-1. * FREQ_LIST, _spkres[:, IDX5], _spkres[:, IDX95], color=neuron_cmap[int(0.5 * SORTVIDX.shape[0])],
        alpha=.5)
plt.plot(-1. * FREQ_LIST, _spkres[:, IDX50], '.-', color=neuron_cmap[int(0.5 * SORTVIDX.shape[0])], zorder=99)
plt.title("5th, 50th & 95th percentile")
plt.ylim((0., _fmax[SORTVIDX[IDX95 + 1]]))

plt.subplot(212)
plt.gca().set_prop_cycle(plt.cycler('color', neuron_cmap))
_spr = spk_result['mixed'][:, SORTVIDX]
_perspr = 100. * (_spr / _spr[0, :] - 1)
plt.plot(FREQ_LIST, _perspr, '.-', markersize=1., alpha=0.3)
plt.xlabel("Input Frequency")
plt.ylabel(r"$100 * (f/f_0 - 1)$")
plt.ylim((-10., 10.))

plt.tight_layout()

plt.savefig("balance_synapse_N_decode.pdf")
