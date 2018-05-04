BIAS_VALUES = {
    'synapse': {
        'exc' : 512,
        'dc'  : 544,
        'inh' : 512,
        'lk'  : 10,
        'pd'  : 50,
        'pu'  : 1024
        },
    'soma': {
        'off' : 10,
        'ref' : 512
        },
    'diffusor': {
        'g'   : 512,
        'r'   : 1
        }
    }
SPK_WEIGHT = {
    'exc': [1, 1],
    'inh': [-1, -1],
    'mixed': [1, -1]
}
DECIMATION = 1.
F_MAX = 5e3  # max nominal firing rate
SENS_TOL = 0.05

def set_baseline(HAL, bias_values):
    import numpy as np
    from pystorm.PyDriver import bddriver as bd

    bddriver = HAL.driver

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
        bddriver.SetSomaOffsetMultiplier(CORE, soma_addr, bd.bdpars.SomaOffsetMultiplierId.ZERO)
    
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_EXC    , bias_values['synapse']['exc'])
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_DC     , bias_values['synapse']['dc'])
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_INH    , bias_values['synapse']['inh'])
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_LK     , bias_values['synapse']['lk'])
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PD     , bias_values['synapse']['pd'])
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PU     , bias_values['synapse']['pu'])
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_DIFF_G     , bias_values['diffusor']['g'])
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_DIFF_R     , bias_values['diffusor']['r'])
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, bias_values['soma']['off'])
    bddriver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_REF   , bias_values['soma']['ref'])
    
    HAL.flush()

def create_1xN_enc_NxN_dec_network(net, pool_id, width, height, d_weight, spk_type='mixed', use_pool=True):
    import numpy as np
    from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

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
        b1 = net.create_bucket("b%d" % pool_id, N // 4)
        o1 = net.create_output("o%d" % pool_id, N // 4)
        
        net.create_connection("i%d_to_p%d" % (pool_id, pool_id), i1, p1, None)
        #decoders = np.eye(N) * d_weight
        decoders = np.zeros((N // 4, N))
        for iy, y in enumerate(range(0, height, 2)):
            for ix, x in enumerate(range(0, width, 2)):
                icol = y * width + x
                irow = iy * (width // 2) + ix
                decoders[irow, icol] = d_weight
        net.create_connection("p%d_to_b%d" % (pool_id, pool_id), p1, b1, decoders)
        net.create_connection("b%d_to_o%d" % (pool_id, pool_id), b1, o1, None)
    else:
        zero_encs = np.zeros((N, 1))
        p1 = net.create_pool("p%d" % pool_id, zero_encs)
    
    return(p1)

def record_spikes(HAL, net, meas_int=5):
    import time
    import numpy as np

    # Keep next two seconds
    time.sleep(meas_int)
    HAL.disable_output_recording()
    HAL.stop_traffic()
    res = HAL.get_outputs()  # [[t_ns, id_op(o1), dim(0), count], ...]
    print(HAL.driver.GetOutputQueueCounts())
    ovf = HAL.get_overflow_counts()
    if ovf > 0:
        print("WARNING: Overflow detected")
    dims = np.array(np.unique(res[:, 2]), dtype=np.int)
    print("[INFO]: Output from '%d' neurons, num spikes=%d" % (len(dims), res.shape[0]))

    spk_data = {_dim:[] for _dim in dims}  # dict of dimensions
    for _idx, _ent in enumerate(res):
        _did = _ent[2]
        if _ent[3] > 1:
            print("[WARNING]: %d spikes in one time bin" % _ent[3])
        for _sid in range(_ent[3]):
            spk_data[_did].append(_ent[0])
        
    HAL.enable_output_recording()
    HAL.start_traffic()
    return (spk_data)

def measure_rate(HAL, net, input_rate, sil_int=2, meas_int=5):
    import time
    import numpy as np

    # Set spike generator rate
    HAL.set_input_rate(net.get_inputs()[0], 0, input_rate, 0)
    # Discard first 2 seconds
    time.sleep(sil_int)
    HAL.get_outputs()
    # if rate of accumulator tripping / decode rate < 20M, else TX probably saturated
    ovf = HAL.get_overflow_counts()
    if ovf > 0:
        print("WARNING: Overflow detected")
    
    # Keep next two seconds
    time.sleep(meas_int)
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
    freq_res = res_N[:, 2] / t_int  # in Hz
    
    return (freq_res, dims)

def find_hifi_soma(HAL, net, pool, width, height, sil_int=1, meas_int=1, draw_plot=False):
    import time
    import numpy as np
    import matplotlib as mpl
    mpl.use('Agg')
    import matplotlib.pyplot as plt

    f_res = np.zeros((height, width))

    _xi, _yi = pool.mapped_xy
    set_baseline(HAL, BIAS_VALUES)

    _res, _dims = measure_rate(HAL, net, 1, sil_int=sil_int, meas_int=meas_int)
    f_res[[_dims // width, _dims % width]] = _res
    _fidx = np.where(f_res > F_MAX)

    _v0 = np.nanmin(f_res)
    _v1 = np.nanmax(f_res)

    if draw_plot:
        plt.figure(figsize=(6., 3.))
        plt.subplot(121)
        plt.imshow(f_res, interpolation='none')
        plt.colorbar()
        plt.subplot(122)
        _ffilt = f_res * 1.0
        _ffilt[_fidx] = np.nan
        plt.imshow(_ffilt, interpolation='none')
        plt.colorbar()
        plt.tight_layout()
        plt.show()
        plt.savefig("soma_fnom.pdf")

    return(np.array([_yi + _fidx[0], _xi + _fidx[1]], dtype=np.int), f_res)

def find_insensitive_soma(HAL, net, pool, width, height, sil_int=1, meas_int=5, draw_plot=False):
    import time
    import numpy as np
    import matplotlib as mpl
    mpl.use('Agg')
    import matplotlib.pyplot as plt

    f_A = np.zeros((height, width))
    f_B = np.zeros_like(f_A)

    _xi, _yi = pool.mapped_xy
    set_baseline(HAL, BIAS_VALUES)

    _res, _dims = measure_rate(HAL, net, 1, sil_int=sil_int, meas_int=meas_int)
    f_A[[_dims // width, _dims % width]] = _res

    _res, _dims = measure_rate(HAL, net, 1000, sil_int=sil_int, meas_int=meas_int)
    f_B[[_dims // width, _dims % width]] = _res

    _sidx = np.where((f_B <= 1e-6) & (f_A <= 1e-6))

    f_D = f_B - f_A
    _ferr = f_D / f_B
    _fidx = np.where(_ferr < SENS_TOL)
    f_B[_sidx] = np.nan
    f_A[_sidx] = np.nan
    f_D[_sidx] = np.nan

    _v0 = np.nanmin(f_B)
    _v1 = np.nanmax(f_B)

    if draw_plot:
        plt.figure(figsize=(6., 6.))
        plt.subplot(221)
        plt.imshow(f_B, interpolation='none', vmin=_v0, vmax=_v1)
        plt.colorbar()
        plt.subplot(222)
        plt.imshow(f_A, interpolation='none', vmin=_v0, vmax=_v1)
        plt.colorbar()
        plt.subplot(223)
        plt.imshow(f_D, interpolation='none')
        plt.colorbar()
        plt.subplot(224)
        plt.imshow(_ferr, interpolation='none')
        plt.colorbar()
        plt.tight_layout()
        plt.show()
        plt.savefig("soma_sensitivity.pdf")

    return(np.array([_yi + _fidx[0], _xi + _fidx[1]], dtype=np.int), (f_B, f_A, f_D))
