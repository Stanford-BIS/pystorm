import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pickle
import sys
import time

from pystorm.PyDriver import bddriver as bd
import driver_util
from driver_util import s_to_ns, ns_to_s

np.random.seed(0)

#########################################
# user parameters

collect_curves = True

pool_width = 8
pool_height = pool_width
num_neurons = pool_height * pool_width

#dims = 1
#def the_func(x): 
#    return x 

#dims = 1
#def the_func(x): 
#    return np.array(x)**2 / fmax

dims = 1
def the_func(x): 
    return 1 / (1 + np.exp(-np.array(x)/fmax*8)) * fmax

#dims = 1
#def the_func(x): 
#    return np.sin(np.array(x) / fmax * np.pi) * fmax

#dims = 2
#def the_func(x):
#    return x[0] * x[1] / fmax

K = 8 # K = taps per dim

fmax = 1000
num_training_points_per_dim = 100
training_hold_time = .5 # seconds

lams = [1e3, 1e4, 1e5, 1e6]
#lams = [2e5]

downstream_time_res = 10 * 1000 # ns
upstream_time_res = downstream_time_res * 10 # ns

#########################################
# derived parameters

# tap points
tap_points = [{} for d in range(dims)]
all_tap_points = []

# tap points are +/- in opposite corners for dims == 1
# ignoring K, using all synapses
if dims == 1:
    tap_point_syn_stride = 1
    for xsyn in range(0, pool_width//2, tap_point_syn_stride):
        for ysyn in range(0, pool_width//2, tap_point_syn_stride):
            if xsyn < pool_width//4 and ysyn < pool_height//4 or xsyn > pool_width//4 and ysyn > pool_height//4:
                sign = 1 # +
            else:
                sign = 0 # -

            tap_points[0][(ysyn, xsyn)] = sign 
            all_tap_points.append((ysyn, xsyn))

# encode radially, sort of, sign based on quadrant
elif dims == 2:
    for d in range(dims):
        for k in range(K):
            while True:
                xsyn = np.random.randint(pool_width//2)
                ysyn = np.random.randint(pool_height//2)
                if (ysyn, xsyn) not in all_tap_points: # can saturate threshold if two tap points collide
                    # -- +-
                    # -+ ++
                    xpos = xsyn >= pool_width // 4
                    ypos = ysyn >= pool_height // 4
                    if d == 0:
                        if xpos:
                            sign = 0
                        else:
                            sign = 1
                    else:
                        if ypos:
                            sign = 0
                        else:
                            sign = 1
                         
                    tap_points[d][(ysyn, xsyn)] = sign
                    all_tap_points.append((ysyn, xsyn))
                    break # found an unused tap, break out of "while True"


# otherwise generate random taps, but don't use the same tap twice
else:
    for d in range(dims):
        # add a + tap and a - tap at a random point
        for k in range(K):
            for sign in [1, 0]:
                while True:
                    xsyn = np.random.randint(pool_width//2)
                    ysyn = np.random.randint(pool_width//2)
                    if (ysyn, xsyn) not in all_tap_points: # can saturate threshold if two tap points collide
                        tap_points[d][(ysyn, xsyn)] = sign
                        all_tap_points.append((ysyn, xsyn))
                        break # found an unused tap, break out of "while True"

taps = np.zeros((pool_height, pool_width))
for d, tap_points_in_dim in enumerate(tap_points):
    for tap in tap_points_in_dim:
        taps[2*tap[0], 2*tap[1]] += (d+1) * (2 * tap_points[d][tap] - 1)
plt.figure()
plt.imshow(taps)
plt.colorbar()
plt.savefig("tap_points.pdf")

# input rates
total_training_points = num_training_points_per_dim ** dims
rates_1d = np.linspace(-fmax, fmax, num_training_points_per_dim).astype(int)
if dims > 1:
    rates = np.meshgrid(*([rates_1d]*dims))
else:
    rates = [rates_1d]
rates_flat = [r.flatten() for r in rates]

# output rates to approximate
rates_out = the_func(rates_flat)

#########################################
# collect tuning curves

def configure_pool(D):
    print("* Configuring neuron array")
    
    # first, disable everything
    for i in range(4096):
        D.DisableSoma(CORE, i)
        D.SetSomaGain(CORE, i, bd.bdpars.SomaGainId.ONE);
        D.SetSomaOffsetSign(CORE, i, bd.bdpars.SomaOffsetSignId.NEGATIVE);
        D.SetSomaOffsetMultiplier(CORE, i, bd.bdpars.SomaOffsetMultiplierId.ONE)

    for i in range(1024):
        D.DisableSynapse(CORE, i);
        D.DisableSynapseADC(CORE, i);

    #for i in range(256):
    #    D.OpenDiffusorAllCuts(CORE, i);

    # then only enable the neurons and synapses we're using
    # enabling only the used synapses is very important to improve SNR
    for xsyn in range(pool_width//2):
        for ysyn in range(pool_height//2):

            # un-kill the synapse, if it's a tap
            # set gain of neurons under synapses to minimum (1/4)
            syn_aer_idx = D.GetSynAERAddr(xsyn, ysyn)
            if (ysyn, xsyn) in all_tap_points:
                print("enabling synapse", ysyn, xsyn)
                D.EnableSynapse(CORE, syn_aer_idx)

                xnrn = xsyn*2
                ynrn = ysyn*2
                soma_under_syn_addr = D.GetSomaAERAddr(xnrn, ynrn)
                D.SetSomaGain(CORE, soma_under_syn_addr, bd.bdpars.SomaGainId.ONE_FOURTH);

            for xnrn_off in range(2):
                for ynrn_off in range(2):
                    xnrn = xsyn * 2 + xnrn_off
                    ynrn = ysyn * 2 + ynrn_off

                    # un-kill the soma
                    soma_aer_idx = D.GetSomaAERAddr(xnrn, ynrn)
                    D.EnableSoma(CORE, soma_aer_idx)
                    
                    if dims == 1 and pool_width == 8:
                        # some custom bias twiddling, this is specific to a particular chip
                        if xnrn < pool_width // 2 and ynrn < pool_height // 2:
                            D.SetSomaOffsetSign(CORE, soma_aer_idx, bd.bdpars.SomaOffsetSignId.NEGATIVE);
                            D.SetSomaOffsetMultiplier(CORE, soma_aer_idx, bd.bdpars.SomaOffsetMultiplierId.THREE)
                        if xnrn > pool_width // 2 and ynrn > pool_height // 2:
                            D.SetSomaOffsetSign(CORE, soma_aer_idx, bd.bdpars.SomaOffsetSignId.POSITIVE);
                            D.SetSomaOffsetMultiplier(CORE, soma_aer_idx, bd.bdpars.SomaOffsetMultiplierId.ONE)

    # open diffuser within pool
    for xtile in range(pool_width//4):
        for ytile in range(pool_width//4):
            mem_aer_idx = D.GetMemAERAddr(xtile, ytile)
            D.CloseDiffusorAllCuts(CORE, mem_aer_idx);
            #D.OpenDiffusorAllCuts(CORE, mem_aer_idx);

    D.Flush() 

    # set up the TAT
    print("* Programming TAT for tap points")
    tat_entries = []
    for d in range(dims):
        tap_points_in_dim = tap_points[d]
        for tap_idx, tap in enumerate(tap_points_in_dim):

            # two tap points per entry
            if tap_idx % 2 == 1:

                if tap_idx == len(tap_points_in_dim) - 1:
                    stop = 1
                else:
                    stop = 0

                y0, x0 = last_tap
                sign0  = last_sign
                sign1  = tap_points_in_dim[tap]
                y1, x1 = tap

                addr0 = D.GetSynAERAddr(x0, y0)
                addr1 = D.GetSynAERAddr(x1, y1)

                print("tat entry: stop", stop, "addr0", addr0, "addr1", addr1, "sign0", sign0, "sign1", sign1)
                tat_entry = bd.PackWord([
                    (bd.TATSpikeWord.STOP              , stop)  ,
                    (bd.TATSpikeWord.SYNAPSE_ADDRESS_0 , addr0) ,
                    (bd.TATSpikeWord.SYNAPSE_SIGN_0    , sign0) ,
                    (bd.TATSpikeWord.SYNAPSE_ADDRESS_1 , addr1) ,
                    (bd.TATSpikeWord.SYNAPSE_SIGN_1    , sign1) ])

                tat_entries.append(tat_entry)

            last_tap = tap
            last_sign = tap_points_in_dim[tap]

    input_tags = list(range(0, dims*K, K))
    print("input tags by dimension", input_tags)
    D.SetMem(CORE, bd.bdpars.BDMemId.TAT0, tat_entries, 0)

    time.sleep(.1) # wait a little for the TAT to program

    print("* Reading TAT")
    dumped_tat = D.DumpMem(CORE, bd.bdpars.BDMemId.TAT0)

    if driver_util.compare_TAT_words(tat_entries, dumped_tat) == -1:
        D.Stop()
        sys.exit(-1)

    # for some reason, you have to do this a couple times (maybe there's input slack?)
    D.SetSpikeDumpState(CORE, True, True)
    D.SetSpikeDumpState(CORE, True, True)
    D.SetSpikeDumpState(CORE, True, True)
    D.SetSpikeDumpState(CORE, True, True)

    return input_tags

def do_sweep(D, input_tags):
    first_bin_time = ns_to_s(D.GetFPGATime()) + .5 # s
    last_bin_time = first_bin_time + total_training_points * training_hold_time # s
    bin_times = np.linspace(first_bin_time, last_bin_time, total_training_points)

    start_time = first_bin_time
    end_time = last_bin_time + training_hold_time

    print("* Running sweep for", end_time - start_time, "seconds")

    for bin_idx, time_ns in enumerate(s_to_ns(bin_times)):
        bin_rates = [rf[bin_idx] for rf in rates_flat]
        for idx, (d, t, r) in enumerate(zip(list(range(dims)), input_tags, bin_rates)):
            D.SetSpikeGeneratorRates(CORE, [d], [t], [r], time_ns, False)
            D.Flush()

        # XXX theres's a bug (probably w/ event time sorting). You gotta do the above for now
        # the flush is necessary

        #D.SetSpikeGeneratorRates(CORE, list(range(dims)), input_tags, bin_rates, time_ns, False)

    D.Flush()

    fudge = 1
    time.sleep(end_time - start_time + fudge)

    print(D.GetOutputQueueCounts())
    spikes, spike_times = D.RecvSpikes(CORE)

    return bin_times, spikes, spike_times

# we need some utility functions even if we're not doing training
D = driver_util.standard_startup(downstream_time_res, upstream_time_res)
CORE = 0

if collect_curves:

    driver_util.standard_DAC_settings(D, CORE)

    input_tags = configure_pool(D)

    # do training sweep (collect tuning curves)
    bin_times, spikes, spike_times = do_sweep(D, input_tags)

    # save results
    p_objs = {"spikes":spikes, 
              "spike_times":spike_times, 
              "bin_times":bin_times,
              "input_tags":input_tags}
    pfile = open("single_pool.pck", "wb")
    pickle.dump(p_objs, pfile)
    pfile.close()
    
else:
    pfile = open("single_pool.pck", "rb")
    p_objs = pickle.load(pfile)
    pfile.close()

#########################################
# bin spikes

spikes      = p_objs["spikes"]
spike_times = p_objs["spike_times"]
bin_times   = p_objs["bin_times"]
input_tags  = p_objs["input_tags"]

def get_binned_counts(ids, num_ids, times, bin_times):

    hold_time = bin_times[1] - bin_times[0]

    counts = [np.zeros((num_ids)).astype(int) for i in range(len(bin_times))]

    bin_idx = 0
    in_bin = 0

    for idx, t in zip(ids, times):
        if t > bin_times[0] and t < bin_times[-1] + hold_time:
            if bin_idx+1 < len(bin_times) and t > bin_times[bin_idx + 1]:
                bin_idx += 1
                print("bin boundary at", t)
                print(" ", in_bin, " in bin")
                in_bin = 0
            counts[bin_idx][idx] += 1
            in_bin += 1

    return counts

D.SetSpikeTrafficState(CORE, False, True)
D.SetSpikeTrafficState(CORE, False, True)
D.SetSpikeTrafficState(CORE, False, True)
D.SetSpikeTrafficState(CORE, False, True)

print("* Processing spikes")

yxs = np.array(D.GetSomaXYAddrs(spikes)).astype(int)
ys = yxs // 64
xs = yxs % 64
ysxs = [y*pool_width + x for y, x in zip(ys, xs)]
ysxs_valid = [n < num_neurons for n in ysxs]

ysxs        = np.array(ysxs)[ysxs_valid]
spike_times = np.array(spike_times)[ysxs_valid]
print(len(spike_times))
print(len(ysxs))

counts_flat = get_binned_counts(ysxs, num_neurons, ns_to_s(spike_times), bin_times)
counts = [cf.reshape((pool_height, pool_width)) for cf in counts_flat]

n_to_plot = 4
plt.figure(figsize=(4*n_to_plot, 4))
for pidx in range(n_to_plot):
    plt.subplot(1, n_to_plot, pidx+1)
    plt.imshow(counts[int(total_training_points * pidx / n_to_plot)], cmap='Greys')
    plt.colorbar()
plt.savefig("spikes.pdf")

plt.figure()
plt.hist(spike_times, bins=100)
plt.savefig("spike_times.pdf")

#########################################
# assemble tuning curves
A = np.zeros((num_neurons, total_training_points))
for train_idx, c in enumerate(counts):
    A[:, train_idx] = c.flatten()

plt.figure()
plt.plot(A.T)
plt.savefig("tuning_curves.pdf")
print(A.shape)

num_train_idxs = int(.7 * total_training_points)
num_test_idxs  = total_training_points - num_train_idxs
#train_idxs = np.random.permutation([True] * num_train_idxs + [False] * num_test_idxs)
train_idxs = np.linspace(0, total_training_points, num_train_idxs).astype(int)
train_idxs = [i in train_idxs for i in range(total_training_points)]
test_idxs  = [not ti for ti in train_idxs]

A_train = A[:, train_idxs]
A_test  = A[:, test_idxs]

ytrain = np.array(rates_out)[:, train_idxs].T
ytest = np.array(rates_out)[:, test_idxs].T

#########################################
# train population
# y = A'd
# Ay = AA'd
# inv(AA')Ay = d
# inv(AA' + lam*I)Ay = d

def rmse(x, y):
    return np.sqrt(np.mean((x - y)**2))

# sweep lambda
decoders = []
rmses = []
for lam in lams:
    d = np.dot(np.dot(np.linalg.inv(np.dot(A_train, A_train.T) + lam * np.eye(num_neurons)), A_train), ytrain)
    yhat = np.dot(A_test.T, d)
    cv_rmse = rmse(ytest, yhat)

    decoders.append(d)
    rmses.append(cv_rmse)

# pick the best decode for all lambdas
best_idx = np.argmin(rmses)
best_d = decoders[best_idx]
print("best lambda was", lams[best_idx])

plot_yhat = np.dot(A_test.T, best_d)
plot_rmse = rmse(plot_yhat, ytest)

if dims == 1:
    plot_x = rates_1d[test_idxs]
    plt.figure()
    plt.plot(plot_x, ytest, color='b')
    plt.plot(plot_x, plot_yhat, color='r')
    plt.title("CV Decode\nRMSE: " + str(plot_rmse))
    plt.xlabel("input rate")
    plt.ylabel("output rate")
    plt.savefig("decode.pdf")
else:
    plt.figure()
    plt.plot(ytest, color='b')
    plt.plot(plot_yhat, color='r')
    plt.title("CV Decode\nRMSE: " + str(plot_rmse))
    plt.xlabel("input idx")
    plt.ylabel("output rate")
    plt.savefig("decode.pdf")

############################################
# set up accumulator for decode

mm_vals, thr_idxs, thr_vals, d_eff = driver_util.weight_to_mem(best_d.T) # DxN

print("* Programming AM")
acc_entries = []
for d in range(dims):
    stop = (d == dims-1)
    thr = thr_idxs[d]
    print("programming AM. stop", stop, "thr", thr)
    acc_entry = bd.PackWord([
      (bd.AMWord.STOP, 0),
      (bd.AMWord.THRESHOLD, thr),
      (bd.AMWord.NEXT_ADDRESS, (1 << 11) + d)])
    acc_entries.append(acc_entry)
    acc_entry = bd.PackWord([
      (bd.AMWord.STOP, 0),
      (bd.AMWord.THRESHOLD, thr),
      (bd.AMWord.NEXT_ADDRESS, (1 << 11) + d + 1)])
    acc_entries.append(acc_entry)
    acc_entry = bd.PackWord([
      (bd.AMWord.STOP, 0),
      (bd.AMWord.THRESHOLD, thr),
      (bd.AMWord.NEXT_ADDRESS, (1 << 11) + d + 2)])
    acc_entries.append(acc_entry)
    acc_entry = bd.PackWord([
      (bd.AMWord.STOP, stop),
      (bd.AMWord.THRESHOLD, thr),
      (bd.AMWord.NEXT_ADDRESS, (1 << 11) + d + 3)])
    acc_entries.append(acc_entry)
D.SetMem(CORE, bd.bdpars.BDMemId.AM, acc_entries, 0)

print("* Programming MM")
for y in range(pool_height):
    for x in range(pool_width):
        nrn_aer_addr = D.GetSomaAERAddr(x, y)
        flat_xy_addr = y * pool_width + x
        decoders = list(mm_vals[:, flat_xy_addr]) * 4

        # MM is 256x256, effectively
        MM_addr_y = nrn_aer_addr  % 256
        MM_addr_x = nrn_aer_addr // 256 * 16 # assuming we won't go over 16 dims
        print("MM entry for", (y, x), "at", (MM_addr_y, MM_addr_x), "is", decoders)
        start_addr = MM_addr_y * 256 + MM_addr_x 

        D.SetMem(CORE, bd.bdpars.BDMemId.MM, decoders, start_addr)
D.Flush()

print("* Programming PAT")
# need to fix this for pool not in corner
#for y in range(pool_height//8):
#    for x in range(pool_width//8):
#        nrn_aer_addr = D.GetSomaAERAddr(8*x, 8*y)
#        MM_addr_y = nrn_aer_addr  % 256
#        MM_addr_x = nrn_aer_addr // 256 * 16 # assuming we won't go over 16 dims
#
#        PAT_entry = bd.PackWord([
#          (bd.PATWord.AM_ADDRESS, 0),
#          (bd.PATWord.MM_ADDRESS_HI, MM_addr_y // 64),
#          (bd.PATWord.MM_ADDRESS_LO, MM_addr_x)])
#
#        D.SetMem(CORE, bd.bdpars.BDMemId.PAT, [PAT_entry], nrn_aer_addr // 64)
PAT_entry = bd.PackWord([
  (bd.PATWord.AM_ADDRESS, 0),
  (bd.PATWord.MM_ADDRESS_HI, 0),
  (bd.PATWord.MM_ADDRESS_LO, 0)])

# sleep a bit
time.sleep(4)

# turn spikes -> accumulator on
D.SetSpikeTrafficState(CORE, True, True)
D.SetSpikeTrafficState(CORE, True, True)
D.SetSpikeTrafficState(CORE, True, True)
D.SetSpikeTrafficState(CORE, True, True)

# testing sweep
bin_times, spikes, spike_times = do_sweep(D, input_tags)

tags_out, tag_out_times = D.RecvTags(CORE)
tags_out = np.array(tags_out)
tag_out_times = np.array(tag_out_times)
print("* got", len(tags_out), "tags back")

tag_cts = tags_out % (1 << 9)
tag_ids = (tags_out >> 9) % (1 << 11)

pos_tags = (tag_cts == 1) | (tag_cts == (1<<3) + 1)
neg_tags = (tag_cts == 511) | (tag_cts == 511 - (1<<3))

plt.figure()
plt.hist(tag_ids, bins=100)
plt.savefig("tag_ids.pdf")
plt.figure()
plt.hist(tag_cts, bins=100)
plt.savefig("tag_cts.pdf")
plt.figure()
plt.hist(tag_out_times, bins=100)
plt.savefig("tag_times.pdf")

tag_ids_pos = tag_ids[pos_tags]
tag_times_pos = tag_out_times[pos_tags]
tag_ids_neg = tag_ids[neg_tags]
tag_times_neg = tag_out_times[neg_tags]

pos_counts = get_binned_counts(tag_ids_pos, dims+3, ns_to_s(tag_times_pos), bin_times)
neg_counts = get_binned_counts(tag_ids_neg, dims+3, ns_to_s(tag_times_neg), bin_times)
acc_decode = np.array(pos_counts) - np.array(neg_counts)
print("POS")
print(pos_counts)
print("NEG")
print(neg_counts)
print("decode")
print(acc_decode)

if dims == 1:
    plot_x = rates_1d[2:-2]
    ytgt = (np.array(rates_out).T)[2:-2].flatten()
    yhat = acc_decode[2:-2,1].flatten()
    #print(yhat)
    #print(ytgt)
    plot_rmse = rmse(ytgt, yhat)
    plt.figure()
    plt.plot(plot_x, ytgt, color='b')
    plt.plot(plot_x, yhat, color='r')
    plt.title("Accumulator Decode\nRMSE: " + str(plot_rmse))
    plt.xlabel("input rate")
    plt.ylabel("output rate")
    plt.savefig("acc_decode.pdf")

#################################################
# plot tuning curves from during decode
yxs = np.array(D.GetSomaXYAddrs(spikes)).astype(int)
ys = yxs // 64
xs = yxs % 64
ysxs = [y*pool_width + x for y, x in zip(ys, xs)]
ysxs_valid = [n < num_neurons for n in ysxs]

print(len(spike_times))
print(len(ysxs))
print(len(ysxs_valid))
ysxs        = np.array(ysxs)[ysxs_valid]
spike_times = np.array(spike_times)[ysxs_valid]

counts_flat = get_binned_counts(ysxs, num_neurons, ns_to_s(spike_times), bin_times)
counts = [cf.reshape((pool_height, pool_width)) for cf in counts_flat]

A = np.zeros((num_neurons, total_training_points))
for train_idx, c in enumerate(counts):
    A[:, train_idx] = c.flatten()

plt.figure()
plt.plot(A.T)
plt.title("during decode")
plt.savefig("tuning_curves_during_decode.pdf")

#print("best_d")
#print(best_d)
#print("d_eff")
#print(d_eff)
#print("mm_vals")
#print(mm_vals)

# check decode
if dims == 1:
    plot_x = rates_1d[2:-2]
    ytgt = (np.array(rates_out).T)[2:-2]
    yhat = np.dot(A.T, best_d)[2:-2]
    yhat2 = np.dot(A.T, d_eff)[2:-2]
    plot_rmse = rmse(ytgt, yhat)
    plot_rmse2 = rmse(ytgt, yhat2)
    plt.figure()
    plt.plot(plot_x, ytgt, color='b')
    plt.plot(plot_x, yhat, color='r')
    plt.plot(plot_x, yhat2, color='g')
    plt.title("testing decode during acc experiment\nRMSE: " + str(plot_rmse) + "," + str(plot_rmse2))
    plt.xlabel("input rate")
    plt.ylabel("output rate")
    plt.savefig("test_decode_during_acc.pdf")


print("* Done")
D.Stop()
