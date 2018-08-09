import argparse
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import time
import pickle

from pystorm.hal import HAL, parse_hal_spikes, bin_tags_spikes

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

###########################################
# pool size parameters

# the whole chip
WIDTH = 64 # array width
HEIGHT = 64 # array height

TILE_XY = 4 # height and width of each tile
TILES_X = WIDTH // TILE_XY # number of tiles in x-direction
TILES_Y = HEIGHT // TILE_XY # number of tiles in y-direction

N = WIDTH * HEIGHT

CORE_ID = 0

###########################################
# calibration parameters

FMAX = 1000 # Hz

FMIN_KEEP = 5 # neurons f(0) > FMIN_KEEP have bifurcated (have to account for clobbering)

# |(f(.5 * FMAX) - f(0)) / (f(FMAX) - f(0)) - .5| < FMAX_TOL have not saturated 
# (have to account for noise)
FMAX_TOL_KEEP = .05

TBASELINE = 2 # how long to measure f(0), f(.95 * max), and f(FMAX)  for each neuron

THOLD0 = .5 # how long to hold 0 value before step
THOLD1 = 1 # how long to hold 1 value after step (waiting for synapse to charge up)

# should be enough to make almost everyone bifurcate
DAC_BIAS_SCALE = 1 # avoid > 10
BIAS_TWIDDLE = 1

###########################################
# misc driver parameters

# rate-based analysis, don't need terribly fine resolution
DOWNSTREAM_RES_NS = 10000 # ns
UPSTREAM_RES_NS = 10000000 # ns = 1 ms, targeting 100 ms tau, so this is 100 times finer

###########################################
# experiment code
# the diffusor cuts 4x4 patches

# we're going to turn on one synapse per tile at a time
# we have to map 4 times, with 1 of the 4 synapses turned on for each mapping
# the synapse will be driven as a step input from the spike generator
# the TAT will fan out from this SG input to target all active synapses (1/4 of total synapses)

# diffusor is set to relatively high spread
# in effect, 16 neurons observe roughly the same signal from the synapse

# we will ignore the neurons that have not bifurcated at 0 input,
# and the neurons that have saturated at 1 input by killing them`

# the idea is to sum the firing rate of these 'good' neurons
# to obtain a linear response over the range of synapse values
# the sum frequency over time should resemble an exponential:
# essentially we are using the summed neurons as a spiking ADC for the synapse waveform
# once the data has been collected, we will fit the resulting (noisy) curves to an exponential to extract tau

def open_all_diff_cuts(HAL):
    # connect diffusor around pools
    for tile_id in range(256):
        HAL.driver.OpenDiffusorAllCuts(CORE_ID, tile_id)

def map_network(HAL, syn_idx, biases, syn_lk):

    HAL.set_time_resolution(DOWNSTREAM_RES_NS, UPSTREAM_RES_NS)
    
    net = graph.Network("net")

    bad_syn = HAL.get_calibration('synapse', 'high_bias_magnitude').values.reshape((HEIGHT//2, WIDTH//2))

    taps = [[]] # one dim, not that it matters
    for tile_x in range(TILES_X):
        for tile_y in range(TILES_Y):
            # set syn position in tile based on i
            y = tile_y * TILE_XY + (syn_idx // 2) * 2
            x = tile_x * TILE_XY + (syn_idx % 2) * 2 
            n_idx = y * WIDTH + x
            
            if not bad_syn[y // 2, x // 2]:
                taps[0].append((n_idx, 1)) # sign is all the same

    # need an even number, just get rid of the last one
    if len(taps[0]) % 2 == 1:
        taps[0] = taps[0][:-1]

    taps = (N, taps) 
    pool = net.create_pool("pool", taps, biases=biases)
    # don't bother with identity trick, flaky spikes from get_spikes are fine

    # create input, hook it up
    inp = net.create_input("input", 1)
    net.create_connection("i_to_p", inp, pool, None)

    # map network
    #print("calling map")
    HAL.map(net)

    # diffusor closed everywhere
    open_all_diff_cuts(HAL)

    # fiddle with diffusor spread
    # the ratio of DAC_DIFF_G / DAC_DIFF_R controls the diffusor spread
    # lower ratio is more spread out
    # be wary below 64 for either of these
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_DIFF_G      , 128)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_DIFF_R      , 1024)

    # go as fast as possible
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SOMA_REF    , 1024)

    # set bias twiddles
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , DAC_BIAS_SCALE)

    # set synaptic tau
    # lower is longer, should be ~100 ms
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_LK , syn_lk)

    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_PD      , 40)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_PU      , 1024)

    return net, inp

def start_collection(HAL):

    #print("starting data collection")
    HAL.start_traffic(flush=False)
    HAL.enable_spike_recording(flush=True)

def end_collection(HAL):

    HAL.stop_traffic(flush=False)
    HAL.disable_spike_recording(flush=True)
    #print("done collecting data")

    binned_spikes, _ = HAL.get_binned_spikes(UPSTREAM_RES_NS)

    # transpose everything, since that's how the script originally worked
    pool_id = next(iter(binned_spikes)) # there's only one pool
    nrn_cts = np.sum(binned_spikes[pool_id].T, axis=1).reshape((HEIGHT, WIDTH))

    #return tile_cts, nrn_cts
    return nrn_cts

def end_collection_bin_spikes(HAL, start_time, end_time):
    # windows the data to the relevant time range
    # returns numpy array

    HAL.stop_traffic(flush=False)
    HAL.disable_spike_recording(flush=True)
    #print("done collecting data")

    #starttime = time.time()
    binned_spikes, bin_times = HAL.get_binned_spikes(UPSTREAM_RES_NS)
    #print("getting spikes took", time.time() - starttime)

    start_idx = np.searchsorted(bin_times, start_time)
    end_idx = np.searchsorted(bin_times, end_time)

    pool_id = next(iter(binned_spikes)) # there's only one pool

    # transpose everything, since that's how the script originally worked
    window = binned_spikes[pool_id].T[:, start_idx:end_idx]

    return window

def compute_tile_medians(nrn_cts):
    med = np.zeros((TILES_Y, TILES_X), dtype=int)
    for tile_x in range(TILES_X):
        for tile_y in range(TILES_Y):
            ymin = tile_y * TILE_XY
            xmin = tile_x * TILE_XY
            ymax = (tile_y + 1) * TILE_XY
            xmax = (tile_x + 1) * TILE_XY

            med[tile_y, tile_x] = np.median(nrn_cts[ymin:ymax, xmin:xmax])
    return med

def process_baseline(f0, fhigh, fmax):
    fig, axes = plt.subplots(2, 3, figsize=(10, 10))

    # f0 stuff
    ax = axes[0, 0]
    ax.set_title("log10(f0 + 1)")
    im = ax.imshow(np.log(f0 + 1))
    fig.colorbar(im, ax=ax)

    ax = axes[1, 0]
    fired = f0 > FMIN_KEEP
    fired_frac = np.sum(fired) / N
    ax.set_title("{0:.4f}".format(fired_frac) + " of neurons f0 > " + str(FMIN_KEEP) + "Hz")
    im = ax.imshow(fired)


    # fhigh/fmax stuff
    # only pay attention to bifurcated neurons
    ax = axes[0, 1]
    ax.set_title("(fhigh - f0) / (fmax - f0)")
    
    fdiff_frac = (fhigh - f0) / (fmax - f0) # just eat the arithmetic errors for fmax=f0=0

    # clip data for the plot
    fdiff_frac[fdiff_frac > .5 + .05] = .5 + .05
    fdiff_frac[fdiff_frac < .5 - .05] = .5 - .05

    im = ax.imshow(fdiff_frac)
    fig.colorbar(im, ax=ax)

    ax = axes[1, 1]
    not_sat = abs(fdiff_frac - .5) < FMAX_TOL_KEEP
    not_sat_frac = np.sum(not_sat) / N
    ax.set_title("{0:.4f}".format(not_sat_frac) + " of neurons not saturated")
    im = ax.imshow(not_sat)

    # fmax stuff
    ax = axes[0, 2]
    ax.set_title("log10(fmax + 1)")
    im = ax.imshow(np.log(fmax + 1))
    fig.colorbar(im, ax=ax)

    ax = axes[1, 2]
    ax.set_title("log10(fmax- f0 + 1)")
    im = ax.imshow(np.log(fmax - f0 + 1))
    fig.colorbar(im, ax=ax)

    total_good = np.sum(fired & not_sat)
    total_weird = np.sum(~fired & not_sat)
    print("frac. fired = ", "{0:.4f}".format(np.sum(fired) / N))
    print("frac. fired and not saturated = ", "{0:.4f}".format(total_good / N))
    print("frac. NOT fired and not saturated = ", "{0:.4f}".format(total_weird / N))

    plt.savefig('data/syn_tau_baseline.png')

    return fired & not_sat

# sweep which of the 4 synapses in the tile we use
def run_tau_exp(HAL, num_trials, syn_lk):
    all_binned_spikes = {}
    all_linear = {}
    for syn_y, syn_x in [(0,0), (0,1), (1,0), (1,1)]:
        
        syn_idx = syn_y * 2 + syn_x

        #############################################
        # assess linearity of neuron responses:
        # measure at 0, FMAX/2, and FMAX
        net, inp = map_network(HAL, syn_idx, BIAS_TWIDDLE, syn_lk)

        # f(0)
        start_collection(HAL)

        time.sleep(TBASELINE)

        nrn_cts_f0 = end_collection(HAL)

        # f(.5 * FMAX)
        HAL.set_input_rate(inp, 0, FMAX // 2, time=0) 
        start_collection(HAL)

        time.sleep(TBASELINE)
        nrn_cts_fhigh = end_collection(HAL)

        HAL.set_input_rate(inp, 0, 0, time=0) 
        time.sleep(.1)

        # f(FMAX)
        HAL.set_input_rate(inp, 0, FMAX, time=0) 
        start_collection(HAL)

        time.sleep(TBASELINE)
        nrn_cts_fmax = end_collection(HAL)

        HAL.set_input_rate(inp, 0, 0, time=0) 
        time.sleep(.1)

        # compute which responses were linear within tolerance
        f0 = nrn_cts_f0 / TBASELINE
        fhigh = nrn_cts_fhigh / TBASELINE
        fmax = nrn_cts_fmax / TBASELINE
        linear = process_baseline(f0, fhigh, fmax)

        all_binned_spikes[(syn_y, syn_x)] = []
        all_linear[(syn_y, syn_x)] = linear
        
        #############################################
        # using only the linear neurons, measure synapse step responses
        for trial_num in range(num_trials):
            fpga_time = HAL.get_time()
            TFUDGE = .1
            
            start_collection(HAL)
            HAL.set_input_rate(inp, 0, 0,    time=int(fpga_time + TFUDGE * 1e9), flush=False)
            HAL.set_input_rate(inp, 0, FMAX, time=int(fpga_time + (TFUDGE + THOLD0) * 1e9), flush=False)
            HAL.set_input_rate(inp, 0, 0,    time=int(fpga_time + (TFUDGE + THOLD0 + THOLD1) * 1e9), flush=True)

            time.sleep(THOLD0 + THOLD1 + 2*TFUDGE)
            start_ns = fpga_time + TFUDGE * 1e9
            end_ns = fpga_time + (TFUDGE + THOLD0 + THOLD1) * 1e9
            print('start_s', start_ns / 1e9)
            print('end_s', end_ns / 1e9)

            binned_spikes = end_collection_bin_spikes(HAL, start_ns, end_ns)
            all_binned_spikes[(syn_y, syn_x)].append(binned_spikes)

    return all_binned_spikes, all_linear

############################################
# data processing/tau fitting code

def collapse_multitrial(As):
    A = np.zeros_like(As[0])
    for A_single in As:
        A += A_single
    return A

def get_syn_responses(A, linear):
    S_yx = np.zeros((TILES_Y, TILES_X, A.shape[1]))
    A_yx = A.reshape((TILES_Y * TILE_XY, TILES_X * TILE_XY, A.shape[1]))
    A_yx_lin = (A_yx.transpose(2, 0, 1) * linear).transpose(1, 2, 0)
    for ty in range(TILES_Y):
        for tx in range(TILES_X):
            for sample_idx in range(A_yx_lin.shape[2]):
                S_yx[ty, tx, sample_idx] = np.sum(A_yx_lin[ty*TILE_XY : (ty+1)*TILE_XY, tx*TILE_XY : (tx+1)*TILE_XY, sample_idx])
    return S_yx

def combine_quadrant_responses(S_yxs, syn_yxs):
    assert(S_yxs[0].shape[0] == TILES_Y)
    assert(S_yxs[0].shape[1] == TILES_X)
    assert(len(syn_yxs) == len(S_yxs))
    assert(len(syn_yxs) == 4)
    Sall_yx = np.zeros((TILES_Y * 2, TILES_X * 2, S_yxs[0].shape[2]))
    for syn_yx, S_yx in zip(syn_yxs, S_yxs):
        syn_y, syn_x = syn_yx
        Sall_yx[syn_y::2, syn_x::2, :] = S_yx
    return Sall_yx

def respfunc(t, tau):
    return 1 - np.exp(-t / tau)

def fit_taus(S_yxs, plot=False, plot_fname_pre=None, pyx=8):

    from scipy.optimize import curve_fit

    taus = np.zeros((S_yxs.shape[0], S_yxs.shape[1]))
    Z_mins = np.zeros_like(taus)
    Z_maxs = np.zeros_like(taus)
    
    idx_start = int(np.round(THOLD0 / (THOLD0 + THOLD1) * S_yxs.shape[2]))
    len_Z_on = S_yxs.shape[2] - idx_start
    
    Z_ons = np.zeros((S_yxs.shape[0], S_yxs.shape[1], len_Z_on))
    curves = np.zeros_like(Z_ons)
    
    for ty in range(S_yxs.shape[0]):
        for tx in range(S_yxs.shape[1]):
            Z = S_yxs[ty,tx,:]
            
            # window and renormalize Z so it looks like a standard
            # saturating exponential going 0 -> 1

            # window
            idx_start = int(np.round(THOLD0 / (THOLD0 + THOLD1) * len(Z)))
            Z_on = Z[idx_start:]
    
            t = np.linspace(0, THOLD1, len(Z_on))

            # shift and scale
            Z_min = np.min(Z_on)
            Z_scaled = Z_on - Z_min
            Z_max = np.mean(Z_scaled[Z_scaled.shape[0] // 2:])
            Z_scaled = Z_scaled / Z_max
            Z_mins[ty, tx] = Z_min
            Z_maxs[ty, tx] = Z_max
            
            mean_off = np.mean(np.abs(Z[:idx_start]))
            mean_on = np.mean(np.abs(Z[idx_start:]))
            if np.abs(mean_on - mean_off) > .05 * mean_off: 

                popt, pcov = curve_fit(respfunc, t, Z_scaled)
                taus[ty, tx] = popt[0]
                
                curves[ty, tx, :] = Z_max * respfunc(t, taus[ty, tx]) + Z_min
                Z_ons[ty, tx] = Z_on
            else:
                taus[ty, tx] = np.nan
            
    if plot:
        # histogram of taus
        plt.figure()
        taus_hist = taus[~np.isnan(taus)]
        plt.hist(taus_hist.flatten(), bins=20)
        plt.title('tau distribution\nmean = ' + str(np.mean(taus_hist)) + ' std = ' + str(np.std(taus_hist)))
        plt.savefig(plot_fname_pre + '_tau_hist.png')

        # imshow of tau locations
        plt.figure()
        plt.imshow(taus)
        plt.savefig(plot_fname_pre + '_tau_locations.png')
        plt.colorbar()

        # step response curve fits
        plot_yx_data([Z_ons[:pyx, :pyx, :], curves[:pyx, :pyx, :]], mask=~np.isnan(taus), t=t)
        plt.savefig(plot_fname_pre + '_curve_fits.png')

    return taus


def plot_yx_data(yx_datas, mask=None, t=None):
    PY = yx_datas[0].shape[0]
    PX = yx_datas[0].shape[1]
    
    fs = (PX*3, PY*3)
    fig, axes = plt.subplots(PY, PX, figsize=fs, sharex=True)
    plt.tight_layout(w_pad=.1, h_pad=.1)
        
    for py in range(PY):
        for px in range(PX):
            for yx_data in yx_datas:
                if mask is not None and mask[py, px]:
                    if t is not None:
                        axes[py, px].plot(t, yx_data[py, px, :])
                    else:
                        axes[py, px].plot(yx_data[py, px, :])
    
    return axes
    

if __name__ == "__main__":

    np.random.seed(0)
    matplotlib.rcParams['interactive'] == False

    parser = argparse.ArgumentParser(description='calibrate synaptic taus, for a particular DAC_SYN_LK value')
    parser.add_argument('DAC_SYN_LK_value', type=int, help='the value of DAC_SYN_LK to calibrate for. Only certain values are tracked by the calibration DB')
    parser.add_argument('--num_trials', type=int, default=8, help='the number of trials to average responses over')
    parser.add_argument('--fit_only', action='store_true', help='use pickled data, just fit tau')
    parser.add_argument('--no_plots', action='store_true', help='suppress generation of plots')
    parser.add_argument('--plot_range', type=int, default=8, help='plot only [:plot_range, :plot_range] corner')

    args = parser.parse_args()

    # read pickle or collect data
    pck_fname = 'data/all_step_response_spikes.pck'
    if args.fit_only:
        all_binned_spikes, all_linear = pickle.load(open(pck_fname, 'rb'))
    else: 
        HAL = HAL() # HAL is a global, used by run_tau_exp, assign it here
        all_binned_spikes, all_linear = run_tau_exp(HAL, args.num_trials, args.DAC_SYN_LK_value)
        pickle.dump([all_binned_spikes, all_linear], open(pck_fname, 'wb'))
        
    plt.figure()
    pid = next(iter(all_binned_spikes[0,0]))
    oneA = all_binned_spikes[0,0][0]
    plt.plot(oneA.T)
    plt.savefig('data/Atest.png')

    plt.figure()
    plt.plot(np.sum(oneA.T, axis=1))
    plt.savefig('data/Asum.png')

    # collapse multitrial data
    collapsed_As = [collapse_multitrial(all_binned_spikes[k]) for k in all_binned_spikes]

    plt.figure()
    oneA = collapsed_As[0]
    plt.plot(oneA.T)
    plt.savefig('data/Acoll.png')

    plt.figure()
    plt.plot(np.sum(oneA.T, axis=1))
    plt.savefig('data/Acollsum.png')

    syn_yx_list = [k for k in all_binned_spikes]
    linear_list = [all_linear[k] for k in all_binned_spikes]

    # combine data from quadrants
    S_yxs = [get_syn_responses(A, linear) for A, linear in zip(collapsed_As, linear_list)]
    Sall_yx = combine_quadrant_responses(S_yxs, syn_yx_list)

    print(Sall_yx.shape)

    plt.figure()
    plt.plot(Sall_yx[0, 0, :])
    plt.savefig('data/syn_responses_00.png')

    plt.figure()
    plt.plot(Sall_yx[0, 1, :])
    plt.savefig('data/syn_responses_01.png')

    # fit tau
    taus = fit_taus(Sall_yx, plot=~args.no_plots, plot_fname_pre='data/tau_step_response', pyx=args.plot_range)

    # record to cal_db
    synapse_cal = 'tau_dac_' + str(args.DAC_SYN_LK_value)
    #HAL = HAL()
    HAL.add_calibration('synapse', synapse_cal, taus)

