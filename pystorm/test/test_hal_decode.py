import numpy as np
import matplotlib.pyplot as plt
import time

from pystorm.hal import HAL
HAL = HAL()

from pystorm.hal import DAC_DEFAULTS, data_utils
from pystorm.hal.run_control import RunControl
from pystorm.hal.net_builder import NetBuilder

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

np.random.seed(0)

###########################################
# pool size parameters

K = 8
width = 16
height = 16
width_height = (width, height)
N = width * height

TOL = .1 # RMSE fraction of fmax that is tolerated
FN = "identity" 
#FN = "square" 
#FN = "sigmoid" 
#FN = "sine" 
#FN = "mult" 

###########################################
# misc driver parameters
downstream_time_res = 10000 # ns
upstream_time_res = 1000000 # ns

HAL.set_time_resolution(downstream_time_res, upstream_time_res)

###########################################
# training parameters

fmax = 1000
num_training_points_per_dim = 20
training_hold_time = .5 # seconds

lams = [1e3, 1e4, 1e5, 1e6]
#lams = [1e5, 1e6, 1e7, 1e8]

###########################################
# target function to decode

if FN == "identity":
    Din = 1
    Dout = 1
    def the_func(x): 
        return x 
elif FN == "square":
    Din = 1
    Dout = 1
    def the_func(x): 
        return np.array(x)**2 / fmax
elif FN == "sigmoid":
    Din = 1
    Dout = 1
    def the_func(x): 
        return 1 / (1 + np.exp(-np.array(x)/fmax*8)) * fmax
elif FN == "sine":
    Din = 1
    Dout = 1
    def the_func(x): 
        return np.sin(np.array(x) / fmax * np.pi) * fmax
elif FN == "mult":
    Din = 2
    Dout = 1
    def the_func(x):
        return x[0] * x[1] / fmax

###########################################
# stim rates

total_training_points = num_training_points_per_dim ** Din
stim_rates_1d = np.linspace(-fmax, fmax, num_training_points_per_dim).astype(int)
if Din > 1:
    stim_rates_mesh = np.meshgrid(*([stim_rates_1d]*Din))
else:
    stim_rates_mesh = [stim_rates_1d]
stim_rates = [r.flatten() for r in stim_rates_mesh]

###########################################
# tap point specification

tap_matrix = np.zeros((N, Din))
if Din == 1:
    # one synapse per 4 neurons
    for x in range(0, width, 2):
        for y in range(0, height, 2):
            n = y * width + x
            if x < width // 2:
                tap_matrix[n, 0] = 1
            else:
                tap_matrix[n, 0] = -1
else:
    print("need to implement reasonable taps for Din > 1")
    assert(False)

###########################################
# specify network using HAL

net = graph.Network("net")

# decoders are initially zero, we remap them later (without touching the rest of the network) using HAL.remap_weights()
decoders = np.zeros((Dout, N))
#decoders = np.ones((Dout, N)) * .2 # sanity check: is accumulator mapping correctly?

i1 = net.create_input("i1", Din)
p1 = net.create_pool("p1", tap_matrix, biases=-3)
b1 = net.create_bucket("b1", Dout)
o1 = net.create_output("o1", Dout)

net.create_connection("c_i1_to_p1", i1, p1, None)
decoder_conn = net.create_connection("c_p1_to_b1", p1, b1, decoders)
net.create_connection("c_b1_to_o1", b1, o1, None)

###########################################
# invoke HAL.map(), make tons of neuromorph/driver calls under the hood

# map network
print("calling map")
HAL.map(net)

###########################################
# compute sweep bins

FUDGE = 2
curr_time = HAL.get_time()
times = np.arange(0, total_training_points) * training_hold_time * 1e9 + curr_time + FUDGE * 1e9
times_w_end = np.hstack((times, times[-1] + training_hold_time * 1e9))
vals = np.array(stim_rates).T
input_vals = {i1 : (times, vals)}

rc = RunControl(HAL, net)

_, spikes_and_bin_times = rc.run_input_sweep(input_vals, get_raw_spikes=True, end_time=times_w_end[-1], rel_time=False)
spikes, spike_bin_times = spikes_and_bin_times
A = data_utils.bins_to_rates(spikes[p1], spike_bin_times, times_w_end, init_discard_frac=.2)
A = A.T

plt.figure()
x = np.linspace(-1, 1, total_training_points)
plt.plot(x, A.T)
    
plt.axis([-1, 1, 0, 1000])
plt.savefig("hal_tuning_curves.pdf")

print("got", np.sum(A), "spikes")

outputs = HAL.get_outputs()
print("got outputs, doing binning")

###########################################
# divide A and y into training and test sets

# XXX disregard beginning and end points
# there's some kind of boundary effect
# this helps the training except on the first and last points
# making the training_hold_time longer also seems to help
ignore_pre = 0
ignore_post = 0

total_stim_points = len(stim_rates[0])
valid_stim_points = total_stim_points - ignore_pre - ignore_post
num_train_idxs = int(.7 * valid_stim_points)
num_test_idxs  = valid_stim_points - num_train_idxs
#train_idxs = np.random.permutation([True] * num_train_idxs + [False] * num_test_idxs)
train_idxs = np.linspace(ignore_pre, total_stim_points - ignore_post, num_train_idxs).astype(int)
train_idxs = [i in train_idxs for i in range(total_stim_points)]
test_idxs  = [not ti for ti in train_idxs]

A_train = A[:, train_idxs]
A_test  = A[:, test_idxs]

y_rates = the_func(stim_rates)
ytrain = np.array(y_rates)[:, train_idxs].T
ytest = np.array(y_rates)[:, test_idxs].T

#########################################
# train population
# y = A'd
# Ay = AA'd
# inv(AA')Ay = d
# inv(AA' + lam*I)Ay = d
#
# try different lambdas, pick the best one
# increase if necessary to keep weights < 1

def rmse(x, y):
    return np.sqrt(np.mean((x - y)**2))

# sweep lambda
decoders = []
rmses = []
for lam in lams:
    d = np.dot(np.dot(np.linalg.inv(np.dot(A_train, A_train.T) + lam * np.eye(N)), A_train), ytrain)
    yhat = np.dot(A_test.T, d)
    cv_rmse = rmse(ytest, yhat)

    decoders.append(d)
    rmses.append(cv_rmse)

# pick the best decode for all lambdas
best_idx = np.argmin(rmses)
print("best lambda was", lams[best_idx])
print("  with RMSE", np.min(rmses))

# decoders have to be less than 1, so pick a higher lambda if this condition is not met

best_d = decoders[best_idx]

impl_idx = best_idx
impl_d = decoders[impl_idx]
while np.max(np.abs(impl_d)) > 1:
    impl_idx += 1
    impl_d = decoders[impl_idx]
    print("had weights > 1, using suboptimal decode with higher lambda")
    print("  new RMSE:", rmses[impl_idx])

best_yhat = np.dot(A_test.T, best_d)
best_rmse = rmse(best_yhat, ytest)
impl_yhat = np.dot(A_test.T, impl_d)
impl_rmse = rmse(impl_yhat, ytest)

plt.figure()
plt.hist(impl_d, bins=40)
plt.title("decoder distribution")
plt.savefig("hal_decode_weight_dist.pdf")

#########################################
## sanity check, what should the weights be per driver_decode.py
## can compare to remapped_core.txt after HAL.remap_weights()
#import driver_util
#print("sanity check ones' c decoders")
#_, _, _, discretized_d = driver_util.weight_to_mem(impl_d.T)
#discretized_yhat = np.dot(A_test.T, discretized_d)

#########################################
# plot decode

if Din == 1 and Dout == 1:
    plot_x = stim_rates_1d[test_idxs]
    plt.figure()
    plt.plot(plot_x, ytest, color='b')
    plt.plot(plot_x, best_yhat, color='r')
    plt.plot(plot_x, impl_yhat, color='b')
    #plt.plot(plot_x, discretized_yhat, color='k')
    #plt.legend(["target", "best decoders", "implementable decoders (|d|<1)", "discretized decoders"])
    plt.legend(["target", "best decoders", "implementable decoders (|d|<1)"])
    plt.title("CV Decode\nRMSE: " + str(best_rmse) + "/" + str(impl_rmse))
    plt.xlabel("input rate")
    plt.ylabel("output rate")
    plt.savefig("hal_decode.pdf")
else:
    assert(False and "need to make plot for Din/Dout > 1")
    plt.figure()
    plt.plot(ytest, color='b')
    plt.plot(best_yhat, color='r')
    plt.title("CV Decode\nRMSE: " + str(best_rmse))
    plt.xlabel("input idx")
    plt.ylabel("output rate")
    plt.savefig("hal_decode.pdf")

assert(impl_rmse < fmax * TOL)

#########################################
# set decode weights, call HAL.remap_weights()

# all we have to is change the decoders
decoder_conn.reassign_weights(impl_d.T)

# then call remap
print("remapping weights")
HAL.remap_weights()

FUDGE = 2
curr_time = HAL.get_time()
times = np.arange(0, total_training_points) * training_hold_time * 1e9 + curr_time + FUDGE * 1e9
times_w_end = np.hstack((times, times[-1] + training_hold_time * 1e9))
vals = np.array(stim_rates).T
input_vals = {i1 : (times, vals)}

rc = RunControl(HAL, net)

outputs_and_bin_times, _ = rc.run_input_sweep(input_vals, get_raw_spikes=False, end_time=times_w_end[-1], rel_time=False)
tags, tag_bin_times = outputs_and_bin_times
yhat = data_utils.bins_to_rates(tags[o1], tag_bin_times, times_w_end, init_discard_frac=.2)
yhat = yhat.flatten()

###########################################
# plot decode
if Din == 1 and Dout == 1:
    #plot_x = stim_rates_1d[2:-2]
    #ytgt = (np.array(y_rates).T)[2:-2].flatten()
    #yhat = acc_decode[0][2:-2].flatten()
    #yhat2 = raw_decode[0][2:-2].flatten()
    plot_x = stim_rates_1d
    ytgt = (np.array(y_rates).T).flatten()
    #yhat2 = raw_decode[0].flatten()
    test_rmse = rmse(ytgt, yhat)
    plt.figure()
    plt.plot(plot_x, ytgt, color='b')
    plt.plot(plot_x, yhat, color='r')
    #plt.plot(plot_x, yhat2, color='g')
    plt.title("Accumulator Decode\nRMSE: " + str(test_rmse))
    #plt.legend(["target", "FPGA SF outputs", "raw tag outputs"])
    plt.legend(["target", "FPGA SF outputs"])
    plt.xlabel("input rate")
    plt.ylabel("output rate")
    plt.savefig("hal_acc_decode.pdf")
    print("rmse for accumulator decode:", test_rmse)
    assert(test_rmse <= 1.5 * impl_rmse) # we want the training RMSE to be similar to the test RMSE
else:
    assert(False and "need to make plot for Din/Dout > 1")


print("* Done")