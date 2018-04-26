import numpy as np
import time
import math
from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from cal_helpers import create_1xN_enc_NxN_dec_network, find_hifi_soma, find_insensitive_soma

p_id = 0
width = 8
height = 8
decimation = 1.
pool_size = width * height
num_pools = 4096 // pool_size

fast_soma_idx = np.array([[], []])
insens_soma_idx = np.array([[], []])

HAL = HAL()

_trunning = 0.
for p_act in range(num_pools):
    _t1 = time.time()
    net = graph.Network("net")
    pools = []
    for p_id in range(num_pools):
        _isconn = False
        if p_id == p_act:
            _isconn = True
        pools.append(create_1xN_enc_NxN_dec_network(net, p_id, width, height, 1./ decimation, 'exc', _isconn))
    HAL.map(net)
                
    HAL.enable_output_recording()
    HAL.start_traffic()

    _isidx = find_hifi_soma(HAL, net, pools[p_act], width, height)[0]
    if fast_soma_idx.shape[1] == 0:
        fast_soma_idx = _isidx
    if _isidx.shape[1] > 0:
        fast_soma_idx = np.hstack((fast_soma_idx, _isidx))

    _isidx = find_insensitive_soma(HAL, net, pools[p_act], width, height)[0]
    if insens_soma_idx.shape[1] == 0:
        insens_soma_idx = _isidx
    if _isidx.shape[1] > 0:
        insens_soma_idx = np.hstack((insens_soma_idx, _isidx))

    HAL.stop_traffic()
    HAL.disable_output_recording()

    _t2 = time.time()
    _trunning += (_t2 - _t1)
    _t_est = (num_pools - 1 - p_act) * (_trunning / (p_act + 1))
    _milsec, _sec = math.modf(_t_est)
    print("[INFO] Estimated remaining time: %d:%d (min)" % (int(_t_est / 60), int(_t_est % 60)))

np.savetxt("fast_idx.dat", fast_soma_idx)
np.savetxt("insens_idx.dat", insens_soma_idx)
