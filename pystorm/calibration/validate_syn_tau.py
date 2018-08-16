import argparse
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import time
import pickle

from pystorm.hal import HAL, parse_hal_spikes, bin_tags_spikes
from pystorm.hal import RunControl # helpers for experiment control
from pystorm.hal import NetBuilder # helpers for builing a net

from pystorm.hal.neuromorph import graph
from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

if __name__ == "__main__":
    Y = 64
    X = 64
    N = X * Y
    THOLD = 1e9 # in ns
    TFUDGE = 1e8 # in ns
    NUM_TRIALS = 20

    # settings copied from syn_tau.py that matter for one reason or another
    # mostly so the linearity results for the neurons match
    FMAX = 1000 # hz
    BIAS_TWIDDLE = 1 
    DAC_BIAS_SCALE = 1
    DOWNSTREAM_RES_NS = 10000 # ns
    UPSTREAM_RES_NS = 10000000 # ns = 1 ms, targeting 100 ms tau, so this is 100 times finer

    NUM_TAPS_TO_TEST = 4
    DAC_SYN_LK = 2

    HAL = HAL()

    # set up network

    # look up tau calibration
    tau_syn = HAL.get_calibration('synapse', 'tau_dac_' + str(DAC_SYN_LK))

    # find the indices of some valid taus
    syn_yxs = []
    while len(syn_yxs) < NUM_TAPS_TO_TEST:
        #try_yx = (np.random.randint(Y//2), np.random.randint(X//2))
        try_y = np.random.randint(32)
        try_x = np.random.randint(32)
        try_yx = try_y, try_x
        in_last_corner = (try_y in [30, 31]) and (try_x in [30, 31])
        if not np.isnan(tau_syn.loc[try_y, try_x]) and not in_last_corner:
            syn_yxs.append(try_yx)

    # create tap matrix using these taus
    tap_matrix = np.zeros((X*Y, NUM_TAPS_TO_TEST), dtype=int)
    for tap_idx, (idxy, idxx) in enumerate(syn_yxs):
        tap_yx_addr = (2*idxy) * X + 2*idxx
        print('tap_yx_addr', tap_yx_addr)
        tap_matrix[tap_yx_addr, tap_idx] = 1
        tap_matrix[30*2*X + 30] = 1 # need even tap #, so hit topmost corner

    net_builder = NetBuilder(HAL)
    net = net_builder.create_single_pool_net(Y, X, tap_matrix, biases=BIAS_TWIDDLE)
    inp = net.get_inputs()[0] # there will be a single input

    # initialize RunControl, which gives us a way to run the sweeps we want
    run = RunControl(HAL, net)

    # map network
    HAL.map(net)

    # settings from syn_tau.py
    HAL.set_time_resolution(DOWNSTREAM_RES_NS, UPSTREAM_RES_NS)
    CORE_ID = 0
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_DIFF_G      , 128)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_DIFF_R      , 1024)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SOMA_REF    , 1024)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , DAC_BIAS_SCALE)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_LK      , DAC_SYN_LK)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_PD      , 40)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_PU      , 1024)

    # open diffusor cuts
    net_builder.open_all_diff_cuts()

    # don't want to redo neuron linearity test, open the pickle
    try:
        pck_fname = 'data/all_step_response_spikes.pck'
        _, all_linear = pickle.load(open(pck_fname, 'rb'))
    except IOError:
        print("you need to run the syn_tau calibration to get the information about" +
            " which synapses are linear in the input range")
              

    # drive synapses one at a time
    old_taus = []
    new_taus = []
    for syn_idx, syn_yx in enumerate(syn_yxs):

        all_spikes_coll = None

        for trial_idx in range(NUM_TRIALS):

            # construct input pattern
            curr_time = HAL.get_time()
            times = np.array([0, THOLD, THOLD*2], dtype=int) + TFUDGE + curr_time
            vals = np.zeros((len(times), NUM_TAPS_TO_TEST))
            vals[:, syn_idx] = [0, FMAX, 0]

            input_vals = {inp : (times, vals)}

            starttime = time.time()
            print('starting sweep')

            _, spikes = run.run_input_sweep(input_vals, get_raw_spikes=True, get_outputs=False)
            spikes, bin_times = spikes

            print('ending sweep, elapsed ' + str(time.time() - starttime))

            pool = net.get_pools()[0]
            spikes = spikes[pool]

            # collapse the spikes
            syn_y, syn_x = syn_yx
            y_block = ((syn_y // 2) * 2) * 2 # floor to block corner, convert to nrn space
            x_block = ((syn_x // 2) * 2) * 2
            print("block", y_block, x_block)
            spikes_yx = spikes.reshape(spikes.shape[0], Y, X)

            #zero out non-linear neurons
            my_linear = all_linear[syn_y % 2, syn_x % 2]
            spikes_lin = spikes_yx * my_linear

            spikes_blk = spikes_lin[:, y_block:y_block+4, x_block:x_block+4]

            spikes_blk = spikes_blk.reshape(spikes_blk.shape[0], 16)
            spikes_coll = np.sum(spikes_blk, axis=1)

            if all_spikes_coll is None:
                all_spikes_coll = spikes_coll
            else:
                all_spikes_coll += spikes_coll

        plot_fname_pre = "data/validate_syn_tau" + str(syn_y) + "_" + str(syn_x)
                
        plt.figure()
        plt.plot(all_spikes_coll)
        plt.title('spikes_coll')
        plt.savefig(plot_fname_pre + '_spikes_coll.png')

        assert(len(all_spikes_coll.shape) == 1)
        print(all_spikes_coll.shape)

        # do curve fitting
        from scipy.optimize import curve_fit

        idx_start = len(all_spikes_coll) // 2

        # window and renormalize Z so it looks like a standard
        # saturating exponential going 0 -> 1

        # window
        Z = all_spikes_coll
        Z_on = Z[idx_start:]

        t = np.linspace(0, THOLD/1e9, len(Z_on))

        # shift and scale
        Z_min = np.min(Z_on)
        Z_scaled = Z_on - Z_min
        Z_max = np.mean(Z_scaled[Z_scaled.shape[0] // 2:])
        Z_scaled = Z_scaled / Z_max
        
        mean_off = np.mean(np.abs(Z[:idx_start]))
        mean_on = np.mean(np.abs(Z[idx_start:]))

        # do fit
        if np.abs(mean_on - mean_off) > .05 * mean_off: 

            def respfunc(t, tau):
                return 1 - np.exp(-t / tau)

            popt, pcov = curve_fit(respfunc, t, Z_scaled)
            tau = popt[0]
            
            fit_curves = Z_max * respfunc(t, tau) + Z_min
        else:
            tau = np.nan

        # plot step response curve fits
        if not np.isnan(tau):
            plt.figure()
            plt.plot(t, Z_on)
            plt.plot(t, fit_curves)
            plt.title('scaled spikes and fit curve')
            plt.savefig(plot_fname_pre + '_curve_fits.png')

        new_tau = tau
        old_tau = tau_syn.loc[syn_y, syn_x]
        print('old =', old_tau, 'new =', new_tau)

        old_taus.append(old_tau)
        new_taus.append(new_tau)

    print('summary:')
    print('old_taus, followed by new_taus')
    print(np.array([old_taus, new_taus]))

