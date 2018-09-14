import matplotlib.pyplot as plt
import numpy as np

from pystorm.hal.net_builder import NetBuilder
from pystorm.hal import HAL
from pystorm.hal.run_control import RunControl

hal = HAL() 
print(hal.SIR_count)
net_builder = NetBuilder(hal)

np.random.seed(1)

IO_FREQ = 1000 # Hz
T = 10 # s

Y = 16
X = 16
N = X * Y
D = 20 # want lots of dims to crank the traffic
BIAS = 0

SY = Y // 2
SX = X // 2

# not important where the traffic goes
# make some weird-ass taps (but that don't saturate)
bad_syns, _ = net_builder.determine_bad_syns()
tap_matrix = np.zeros((N, D))
num_taps = 0
for y in range(0, Y, 2):
    for x in range(0, X, 4):
        if not bad_syns[y, x] and num_taps < D:
            idx = y * X + x
            tap_matrix[idx, num_taps] = 1
            tap_matrix[idx + 2, num_taps] = 1
            num_taps += 1
        
net = net_builder.create_single_pool_net(Y, X, tap_matrix, biases=BIAS)
hal.map(net)
inp_obj = net.get_inputs()[0]
pool_obj = net.get_pools()[0]

fmaxes = net_builder.determine_safe_fmaxes()
fmax = fmaxes[pool_obj]

# default downstream resolution is 10 us

run = RunControl(hal, net)

# make inputs at IO_FREQ

IO_times = np.linspace(0, T, T*IO_FREQ) * 1e9 + 10e9
rates = np.zeros((len(IO_times), D))

import time
t0 = time.time()
print('now look for warnings')
run.run_input_sweep({inp_obj: (IO_times, rates)}) 
print('ran for', time.time() - t0, 's')
print('hit SIR', hal.SIR_count, 'times')

# and just watch the output
