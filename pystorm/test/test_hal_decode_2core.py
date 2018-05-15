import numpy as np
import matplotlib.pyplot as plt
import time

from pystorm.hal import HAL
HAL = HAL()

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

np.random.seed(0)

###########################################
# pool size parameters

K = 8
width = 8
height = 8
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
p1 = net.create_pool("p1", tap_matrix)
b2 = net.create_bucket("b2", Din)
p2 = net.create_pool("p2", tap_matrix)
b1 = net.create_bucket("b1", Dout)
o1 = net.create_output("o1", Dout)
b3 = net.create_bucket("b3", Dout)
o2 = net.create_output("o2", Dout)

net.create_connection("c_b2_to_p1", b2, p1, None)
net.create_connection("c_i1_to_b2", i1, b2, np.identity(Din))
net.create_connection("c_b2_to_p2", b2, p2, None)
net.create_connection("c_p2_to_b3", p2, b3, decoders)
net.create_connection("c_b3_to_o2", b3, o2, None)
decoder_conn = net.create_connection("c_p1_to_b1", p1, b1, decoders)
net.create_connection("c_b1_to_o1", b1, o1, None)

###########################################
# invoke HAL.map(), make tons of neuromorph/driver calls under the hood

# map network
print("calling map")
reqs = [(p1, 1), (p2, 1)]
HAL.map(net, verbose = True, spread = 1, map_reqs = reqs)

###########################################
# compute sweep bins

def get_sweep_bins_starting_now(training_hold_time, total_training_points):

    fudge = .5
    fudge_ns = fudge * 1e9

    duration = training_hold_time * total_training_points
    duration_ns = duration * 1e9

    start_time = HAL.get_time() + fudge_ns
    end_time = start_time + duration_ns
    bin_time_boundaries = np.round(np.linspace(start_time, end_time, total_training_points + 1)).astype(int)

    return duration, bin_time_boundaries

duration, bin_time_boundaries = get_sweep_bins_starting_now(training_hold_time, total_training_points)

###########################################
# call HAL.set_input_rate() to program SGs for each bin/dim

# have to do this before set_input_rates, so we don't get stalled
HAL.start_traffic(flush=False)
HAL.enable_spike_recording(flush=False)
HAL.enable_output_recording(flush=True)

# clear spikes before trial
spikes = HAL.get_spikes()
print(len(spikes), "spikes before trial")


def do_sweep(bin_time_boundaries):
    last_bin_start = None
    for bin_idx in range(len(stim_rates[0])):
        bin_start = bin_time_boundaries[bin_idx]
        if last_bin_start is None:
            pass
        else:
            if bin_start < last_bin_start:
                print("last_bin_start", last_bin_start, "vs bin_start", bin_start)
                assert(False)
        last_bin_start = bin_start

        for d in range(Din):
            r = stim_rates[d][bin_idx]

            #print("at", bin_start, "d", d, "is", r)
            HAL.set_input_rate(i1, d, r, time=bin_start, flush=True)

do_sweep(bin_time_boundaries)

###########################################
# sleep during the training sweep

# turn on spikes
print("starting training:", duration, "seconds")

time.sleep(duration + 1)

print("training over")

# should be unecessary
HAL.stop_traffic(flush=False)
HAL.disable_spike_recording(flush=False)
HAL.disable_output_recording(flush=True)

###########################################
# collect results
# NOTE: we could just set the upstream time resolution to whatever hold_time is
# this would make the FPGA do the binning for us

def filter_spikes(spikes):
    # start by filtering into pool -> neuron -> (times, cts)
    # cts is unecessary, but gives same format as filter_tags
    times_and_cts = {}
    for t, p, n in spikes:
        if p not in times_and_cts:
            times_and_cts[p] = {}
        if n not in times_and_cts[p]:
            times_and_cts[p][n] = []
        times_and_cts[p][n].append((t, 1))
    return times_and_cts

def filter_tags(tags):
    times_and_cts = {}
    for t, output_id, dim, ct in tags:
        if output_id not in times_and_cts:
            times_and_cts[output_id] = {}
        if dim not in times_and_cts[output_id]:
            times_and_cts[output_id][dim] = []
        times_and_cts[output_id][dim].append((t, ct))
    return times_and_cts

def filter_one_dim_raw_tags(tag_cts, times, tag_id_filt, obj):
    tag_cts = np.array(tag_cts)

    cts = tag_cts % (1 << 9)
    tags = (tag_cts >> 9) % (1 << 11)

    times_and_cts = {}
    times_and_cts[obj] = {}
    times_and_cts[obj][0] = []
    for t, tag_id, ct in zip(times, tags, cts):
        if tag_id == tag_id_filt:
            if ct > 255:
                signed_ct = ct - 512
            else:
                signed_ct = ct
            #if abs(signed_ct) > 1:
            #    print("big signed ct", signed_ct)
            times_and_cts[obj][0].append((t, signed_ct))
    return times_and_cts

            
def do_binning(times_and_cts, bin_time_boundaries):

    n_bins = len(bin_time_boundaries) - 1

    # initialize A matrices
    As = {}
    for obj in times_and_cts:
        if isinstance(obj, graph.Pool):
            max_idx = obj.n_neurons
        elif isinstance(obj, graph.Output):
            max_idx = obj.dimensions
        As[obj] = np.zeros((max_idx, n_bins)).astype(int)

    # bin spikes, filling in A
    for obj in times_and_cts:
        for n in times_and_cts[obj]:
            #print("binning spikes in pool", p.label, ", neuron", n)            
            this_neuron_times_and_cts = np.array(times_and_cts[obj][n])
            this_neuron_times = this_neuron_times_and_cts[:,0]
            this_neuron_cts = this_neuron_times_and_cts[:,1]
            binned_cts = np.histogram(this_neuron_times, bin_time_boundaries, weights=this_neuron_cts)[0]
            #print(binned_cts)
            As[obj][n, :] = binned_cts
    return As

#print((HAL.driver.GetOutputQueueCounts())) # debug: see what we got

# call HAL to get spikes
# remember these are corrupted because of the output weirdness
# the "noise" fortunately seems to be relatively unbiased, and we
# can use the spikes directly to create a decoder
spikes = HAL.get_spikes()

# pull out A for our pool, plot it
filtered_times = filter_spikes(spikes)
As = do_binning(filtered_times, bin_time_boundaries)
for p in As:
    A = As[p]
    print('total count in bounds, in exp duration:', np.sum(As[p]))

    plt.figure()
    plt.plot(A.T)
    plt.savefig("hal_tuning_curves_" + p.get_label() +".pdf")

print("got", len(spikes), "spikes")

outputs = HAL.get_outputs()
print("got outputs, doing binning")
filtered_outputs = filter_tags(outputs)
binned_outputs = do_binning(filtered_outputs, bin_time_boundaries)

#print(len(outputs)) # nonzero if SF is working
#print(binned_outputs[o1])
print("got", np.sum(binned_outputs[o1]), "tags from filters. Should be 0 during training (if decoders are 0)")

###########################################
# divide A and y into training and test sets

# XXX disregard beginning and end points
# there's some kind of boundary effect
# this helps the training except on the first and last points
# making the training_hold_time longer also seems to help
ignore_pre = 2
ignore_post = 1

total_stim_points = len(stim_rates[0])
valid_stim_points = total_stim_points - ignore_pre - ignore_post
num_train_idxs = int(.7 * valid_stim_points)
num_test_idxs  = valid_stim_points - num_train_idxs
#train_idxs = np.random.permutation([True] * num_train_idxs + [False] * num_test_idxs)
train_idxs = np.linspace(ignore_pre, total_stim_points - ignore_post, num_train_idxs).astype(int)
train_idxs = [i in train_idxs for i in range(total_stim_points)]
test_idxs  = [not ti for ti in train_idxs]

print(A.shape)
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

plt.figure
plt.plot(A.T)
plt.savefig("hal_decode_tuning_curves.pdf")

assert(impl_rmse < fmax * TOL)

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


#########################################
# set decode weights, call HAL.remap_weights()

# all we have to is change the decoders
decoder_conn.reassign_weights(impl_d.T)

# then call remap
print("remapping weights")
HAL.remap_weights()

duration, bin_time_boundaries = get_sweep_bins_starting_now(training_hold_time, total_training_points)

# clear spikes before trial
spikes = HAL.get_spikes()
print(len(spikes), "spikes before trial")

HAL.start_traffic(flush=False)
HAL.enable_spike_recording(flush=False)
HAL.enable_output_recording(flush=True)

do_sweep(bin_time_boundaries)

###########################################
# sleep during the testing sweep

# turn on spikes
print("starting testing:", duration, "seconds")

time.sleep(duration + 1) 

print("testing over")

###########################################
# check accumulator decode

spikes = HAL.get_spikes()
outputs = HAL.get_outputs()
print("output shape", outputs.shape)
raw_tag_cts, raw_tag_ct_times = HAL.driver.RecvTags(0)

print("got outputs, doing binning")
filtered_outputs = filter_tags(outputs)
binned_outputs = do_binning(filtered_outputs, bin_time_boundaries)
acc_decode = binned_outputs[o1]

print("got", np.sum(binned_outputs[o1]), "tags from filters. Should be nonzero for testing")


###########################################
# sanity check, compare get_outputs to raw tags from Driver
#filtered_raw_tags = filter_one_dim_raw_tags(raw_tag_cts, raw_tag_ct_times, 0, o1)
#binned_raw_tags = do_binning(filtered_raw_tags, bin_time_boundaries)
#raw_decode = binned_raw_tags[o1]

###########################################
# plot decode
if Din == 1 and Dout == 1:
    #plot_x = stim_rates_1d[2:-2]
    #ytgt = (np.array(y_rates).T)[2:-2].flatten()
    #yhat = acc_decode[0][2:-2].flatten()
    #yhat2 = raw_decode[0][2:-2].flatten()
    plot_x = stim_rates_1d
    ytgt = (np.array(y_rates).T).flatten()
    yhat = acc_decode[0].flatten()
    #yhat2 = raw_decode[0].flatten()
    test_rmse = rmse(ytgt, yhat)
    print("rmse for accumulator decode:", test_rmse)
    assert(test_rmse <= 1.5 * impl_rmse) # we want the training RMSE to be similar to the test RMSE
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
else:
    assert(False and "need to make plot for Din/Dout > 1")

###########################################
# sanity check, plot tuning curves during testing

# pull out A for our pool, plot it
filtered_times = filter_spikes(spikes)
As = do_binning(filtered_times, bin_time_boundaries)
if p1 in As:
    A = As[p1]
    print('total count in bounds, in exp duration:', np.sum(As[p1]))

    plt.figure()
    plt.plot(A.T)
    plt.savefig("hal_testing_tuning_curves.pdf")

print("got", len(spikes), "spikes")

print("* Done")

