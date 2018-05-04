"""
Measure Synapse's time-constant

- Hit Synapse with multiple-spikes to saturate the synapse
    - Use very high duty-cycle ON pulse for PE
- Stop the input to synapse
- Measure the output spike rate decay
    - Soma should be high-firing rate AND sensitive

Prerequisities

- Get Soma F-F curve to linear input
    - I_syn O{ t_on / (t_on + t_off) = t_on / T_in if (t_on + t_off) < T_in and t_off << T_in
    - Sweep T_in with large EXC-DC to measure F-F
    - Use the same EXC, DC to measure Synapse's time-constant
"""

import time
import pickle
import math
import numpy as np
from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
import cal_helpers as ch

################################################################################
# Get Soma F-F curve to linear input
#
# Set synapse EXC-DC to MAX
# Set synapse t_on, t_off to MIN
# Sweep f_in with 1/f_in >> t_off and 1/f_in > (t_on + t_off)
# Then, I_syn O{ t_on*f_in
# Then, f_spk's I_in is linear to f_in
################################################################################

# Bias values
SYN_LK = 1

ch.BIAS_VALUES['synapse']['exc'] = 1024     # max
ch.BIAS_VALUES['synapse']['dc'] = 1         # min
ch.BIAS_VALUES['synapse']['lk'] = SYN_LK
ch.BIAS_VALUES['synapse']['pd'] = 512       # t_on
ch.BIAS_VALUES['synapse']['pu'] = 1024      # t_off
ch.BIAS_VALUES['soma']['off'] = 4
ch.BIAS_VALUES['soma']['ref'] = 1024
ch.BIAS_VALUES['diffusor']['g'] = 512       # short diffusor
ch.BIAS_VALUES['diffusor']['r'] = 1

# Pool specification
width = 8
height = 8
decimation = 1.
pool_size = width * height
num_pools = 4096 // pool_size

HAL = HAL()
HAL.set_time_resolution(upstream_ns=3500, downstream_ns=3500)
#HAL.set_time_resolution(upstream_ns=1000000, downstream_ns=100000)

net = graph.Network("net")
p_id = 0
pool = ch.create_1xN_enc_NxN_dec_network(net, p_id, width, height, 1./ decimation, 'exc', True)
HAL.map(net)

# Set baseline
ch.set_baseline(HAL, ch.BIAS_VALUES)
            
HAL.enable_output_recording()
HAL.start_traffic()

def measure_soma_FF():
    # Input frequency list
    FIN_LIST = np.hstack([np.linspace(1, 9, 9, dtype=np.int), np.linspace(10, 1000, 100, dtype=np.int)])
    FIN_LIST = FIN_LIST[[0, -1]]
    SPK_DATA = {_fid: dict({}) for _fid in FIN_LIST}

    _trunning = 0.
    for _fidx, _fin in enumerate(FIN_LIST):
        _t1 = time.time()

        # Set spike generator rate
        print("Recording f_spk for f_in=%g" % _fin)
        HAL.set_input_rate(net.get_inputs()[0], 0, _fin, 0)
        # Discard first few seconds
        time.sleep(2)
        HAL.get_outputs()
        # if rate of accumulator tripping / decode rate < 20M, else TX probably saturated
        ovf = HAL.get_overflow_counts()
        if ovf > 0:
            print("WARNING: Overflow detected")

        # Record spikes for some time
        SPK_DATA[_fin] = ch.record_spikes(HAL, net, meas_int=10)

        _t2 = time.time()
        _trunning += (_t2 - _t1)
        _t_est = (FIN_LIST.shape[0] - 1 - _fidx) * (_trunning / (_fidx + 1))
        _milsec, _sec = math.modf(_t_est)
        print("[INFO] Estimated remaining time: %d:%d (min)" % (int(_t_est / 60), int(_t_est % 60)))

    nspikes = dict({})
    fspk_avg = np.zeros((pool_size // 4, FIN_LIST.shape[0]))
    tisi_mean = np.zeros_like(fspk_avg)
    tisi_std = np.zeros_like(fspk_avg)
    for nid in range(pool_size):
        nspikes[nid] = dict({})
        for fid, fin in enumerate(FIN_LIST):
            _sdata = np.array(SPK_DATA[fin][nid])
            nspikes[nid][fin] = _sdata
            if len(_sdata) > 1:
                fspk_avg[nid][fid] = (len(_sdata) - 1) / (_sdata[-1] - _sdata[0]) * 1e9
                tisi_mean[nid][fid] = np.mean(np.diff(_sdata * 1e-9))
                tisi_std[nid][fid] = np.std(np.diff(_sdata * 1e-9))

    OF = open("./syn_soma_F_F.dat", "wb")
    pickle.dump([FIN_LIST, nspikes, fspk_avg, tisi_mean, tisi_std], OF)
    OF.close()

################################################################################
# Measure Soma transient response to Synaptic output
#
# Start measurement
# Saturate Synapse with input=FMAX
# Stop Synaptic input
# Measure transient
################################################################################

def measure_synapse_transient():
    FMAX = 500
    FMIN = 1
    # Discard first few seconds
    time.sleep(2)
    HAL.get_outputs()
    
    HAL.set_input_rate(net.get_inputs()[0], 0, FMAX, 0)
    time.sleep(5)
    HAL.set_input_rate(net.get_inputs()[0], 0, FMIN, 0)

    # Record spikes for some time
    SPK_DATA = ch.record_spikes(HAL, net, meas_int=10)

    tisi_data = dict({})
    for _nid, _tvals in SPK_DATA.items():
        _tarr = np.array(_tvals)
        if len(_tarr) > 1:
            tisi_data[_nid] = np.array([(_tarr[1:] - _tarr[0]) * 1e-9, np.diff(_tarr) * 1e-9])

    OF = open("./syn_soma_tau_FOUT.dat", "wb")
    pickle.dump([SPK_DATA, tisi_data], OF)
    OF.close()

def estimate_synapse_transient():
    IF = open("./syn_soma_F_F.dat", "rb")
    FIN_LIST, _, fspk_avg, tisi_mean, _ = pickle.load(IF)
    IF.close()

    IF = open("./syn_soma_tau_FOUT.dat", "rb")
    _, tisi_data = pickle.load(IF)
    IF.close()

    syn_out = dict({})
    for _nid, _tdata in tisi_data.items():
        syn_out[_nid] = np.zeros((2, _tdata.shape[1]))
        syn_out[_nid][0, :] = _tdata[0, :]
        #_tisi = tisi_mean[_nid, :]
        _tisi = 1./fspk_avg[_nid, :]
        if np.all(_tisi <= 0.):
            print("[WARNING]: referene F-I curve is empty")
            continue
        for _sid in range(_tdata.shape[1]):
            _tcur = _tdata[1, _sid]
            if _tcur <= 0.:
                print("[WARNING]: ISI is zero")
            if (_tisi[1] >= _tcur) & (_tisi[-1] <= _tcur):
                _refid = np.where(_tisi >= _tcur)[0][0]
                _rmin = _refid - 1
                _rmax = _refid + 1
                if _rmin < 1:
                    _rmin = 1
                if _rmax >= _tisi.shape[0]:
                    _rmax = -1

                _coeff = np.polyfit(1./_tisi[_rmin:_rmax], FIN_LIST[_rmin:_rmax], 1)
                _fexp = 1./_tcur * _coeff[0] + _coeff[1]
                syn_out[_nid][1, _sid] = _fexp

    OF = open("./syn_soma_tau_constructed.dat", "wb")
    pickle.dump(syn_out, OF)
    OF.close()

    return(syn_out)

measure_soma_FF()
measure_synapse_transient()
syn_res = estimate_synapse_transient()

HAL.disable_output_recording()
HAL.stop_traffic()

