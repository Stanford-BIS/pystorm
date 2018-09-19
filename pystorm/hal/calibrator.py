import numpy as np
import pandas as pd
from pystorm.hal import DAC_DEFAULTS, data_utils
from pystorm.hal.net_builder import NetBuilder
from pystorm.hal.run_control import RunControl
from copy import copy

class PoolSpec(object):

    def __init__(self,
            label=None, 
            YX=None, loc_yx=None, 
            D=None, TPM=None, 
            gain_divisors=1, biases=0, 
            diffusor_cuts_yx={},
            fmax=None):
        """PoolSpec is the set of parameters to a Pool object.
        It's an input to many Calibrator functions, and can be used to create a Pool
        via Network.create_pool_from_spec() 
        """

        self.label = label
        self.YX = YX
        self.Y, self.X = YX
        self.loc_y, self.loc_x  = loc_yx
        self.loc_yx = loc_yx
        self.gain_divisors = gain_divisors
        self.biases = biases
        self.diffusor_cuts_yx = diffusor_cuts_yx
        self.fmax = fmax

        self.TPM = TPM

        # if D is not specified, try to get it from TPM
        if TPM is None:
            self.D = D
        elif TPM is not None and D is None:
            self.D = TPM.shape[1]
        elif TPM is not None and D is not None:
            raise ValueError("Both TPM and D specified, but do not agree: "
                    + str(D) + " vs " + str(TPM.shape[1]))
            self.D = D

    def check_specified(self, required_pars):
        """Check if the PoolSpec has parameters required specified"""
        for par_name in required_pars:
            if getattr(self, par_name) is None:
                raise ValueError("required parameter " + par_name + " is not defined in PoolSpec")
    def copy(self):
        return copy(self)

class Calibrator(object):

    def __init__(self, hal): 
        self.hal = hal

    def get_basic_calibration(self, cal_obj, cal_type, return_as_numpy=False):
        """Pulls a 'basic' calibration from the CalibrationDB"""
        # XXX for now, this is just a pass-through to HAL. In the future,
        # the HAL calls will be deprecated for calls here
        return self.hal.get_calibration(cal_obj, cal_type, return_as_numpy=return_as_numpy)

    def get_bad_syns(self, pulse_attrition=.05):
        """Looks at synapse-related calibration data, discards synapses that have 
        high bias offset contributions and very slow synapses. 

        Uses the current value of DAC_SYN_PD when retrieving the synaptic delay calibration.

        Parameters:
        ==========
        pulse_attrition (float, default .05) : fraction of synapses to discard based
            on having long pulse extender values

        Returns:
        =======
        ((width, height) array of bools for which synapses are bad, debug_info)
            debug info is dict with keys 
                {'pulse widths',
                 'pulse widths w/o high_bias syns'}
        """

        # look up high bias synapses
        high_bias = self.get_basic_calibration(
            'synapse', 'high_bias_magnitude', return_as_numpy=True)

        # look up pulse widths
        curr_DAC_SYN_PD = self.hal.get_DAC_value('DAC_SYN_PD')
        pulse_widths = self.get_basic_calibration(
            'synapse', 'pulse_width_dac_' + str(curr_DAC_SYN_PD), return_as_numpy=True)

        # disable synapses below attrition point
        pwflat = pulse_widths.flatten()
        fmaxes = 1 / pwflat
        order = np.argsort(fmaxes)
        cutoff_idx = order[int(pulse_attrition * len(fmaxes))]
        fmax = fmaxes[cutoff_idx] # absolute max possible fmax
        slow_pulse = fmaxes <= fmax
        slow_pulse = slow_pulse.reshape(high_bias.shape)

        all_bad_syn = slow_pulse | high_bias

        dbg = {'pulse_widths': pulse_widths, 'slow_pulse':slow_pulse, 'high_bias': high_bias}
        
        return all_bad_syn, dbg

    @staticmethod
    def crop_calibration(cal_array, pool_loc_yx, pool_size_yx):

        if not isinstance(cal_array, np.ndarray):
            raise ValueError("cal_array is not numpy array, is ", str(type(cal_array)))
        if not cal_array.shape[-2:] in [(32, 32), (64, 64)]:
            raise ValueError("cal_array has weird shape: " + str(cal_array.shape) + 
                "expect one of: (64, 64), (32, 32), " +
                "(N, 64, 64), (N, 32, 32), " +
                "where N is the number of channels in the calibration")

        to_syn = NetBuilder.to_synspace

        # if it's a synapse cal

        if cal_array.shape[-1] == 32:
            is_syn = True
        else:
            is_syn = False

        Y, X = pool_size_yx
        Y_loc, X_loc = pool_loc_yx
        if is_syn:
            Y, X = to_syn(Y, X)
            Y_loc, X_loc = to_syn(Y_loc, X_loc)

        if len(cal_array.shape) == 3:
            return cal_array[:, Y_loc:Y_loc + Y, X_loc:X_loc + X]
        elif len(cal_array.shape) == 2:
            return cal_array[Y_loc:Y_loc + Y, X_loc:X_loc + X]
        else:
            assert(False)

    @staticmethod
    def crop_calibration_to_pool(cal_array, ps):
        ps.check_specified(['YX', 'loc_yx'])
        return Calibrator.crop_calibration(cal_array, (ps.loc_y, ps.loc_x), (ps.Y, ps.X))

    def get_all_bias_twiddles(self, extrapolate=True):
        """collect the different soma bias twiddle calibrations into one data structure
        use the data associated with the current DAC_SOMA_OFFSET setting

        Returns:
        =======
        6x64x64 array of offsets
        """

        # look up current DAC setting
        curr_DAC_SOMA_OFFSET = self.hal.get_DAC_value('DAC_SOMA_OFFSET')

        all_offsets = np.zeros((7, 64, 64))
        for i, p, b in zip([0, 1, 2, 4, 5, 6], ['n']*3 + ['p']*3, [3, 2, 1, 1, 2, 3]):
            all_offsets[i] = self.hal.get_calibration(
                'soma', 'bias_twiddle_' + p + str(b) + '_dac_' + str(curr_DAC_SOMA_OFFSET), 
                return_as_numpy=True)

        if extrapolate:
            all_offsets = all_offsets.reshape((7, 64**2))
            all_offsets = Calibrator.extrapolate_bias_twiddles(all_offsets)
            all_offsets = all_offsets.reshape((7, 64, 64))

        return all_offsets

    @staticmethod
    def extrapolate_bias_twiddles(all_offsets):
        """fill in all_offsets unsampled (NaN) points with reasonable guesses
        based on mean firing rates at each offset and mismatch of reused transistors

        this code works, but is more complicated than it needs to be

        Parameters:
        ==========
        all_offsets (7xN float array, Hz) :
            firing rates of neurons when exposed to the 7 different twiddle bit settings,
            relative to their responses to bias 0 (all_offsets[3,:] should be all 0)

        Returns:
        =======
        all_offsets_est (7xN float array, Hz) :
            same thing with NaNs filled in 
        """

        _, N = all_offsets.shape

        def idx_to_mag(idx):
            return np.abs(idx - 3)

        est_slope_p = np.zeros(N)
        highest_sample_p = np.zeros(N, dtype=int)
        est_slope_n = np.zeros(N)
        highest_sample_n = np.zeros(N, dtype=int)
        for n in range(N):
            nrn_offsets = all_offsets[:, n]
            valid = ~np.isnan(nrn_offsets)
            valid_off = nrn_offsets.copy()
            valid_off[~valid] = 0
            if len(valid_off) > 0:
                biggest = np.max(valid_off)
                biggest_arg = np.argmax(valid_off)
                if biggest > 0 and biggest_arg > 3:
                    est_slope_p[n] = biggest / idx_to_mag(biggest_arg)
                    highest_sample_p[n] = idx_to_mag(biggest_arg)
                    
                smallest = np.min(valid_off)
                smallest_arg = np.argmin(valid_off)
                if smallest < 0 and smallest_arg < 3:
                    est_slope_n[n] = smallest / idx_to_mag(smallest_arg)
                    highest_sample_n[n] = idx_to_mag(smallest_arg)
                    
        # compute global mean offsets
        means = []
        for bias_level in range(7):
            bias_offsets = all_offsets[bias_level, :]
            means.append(np.mean(bias_offsets[~np.isnan(bias_offsets)]))
            
        # weight estimated slopes and global means together to fill in data
        all_offsets_est = all_offsets.copy()
        for n in range(N):
            for bias_level in range(7):
                if np.isnan(all_offsets_est[bias_level, n]):
                    assign = False
                    if bias_level > 3:
                        highest_sample = highest_sample_p[n]
                        est_slope = est_slope_p[n]
                        if idx_to_mag(bias_level) > highest_sample:
                            assign = True
                    elif bias_level < 3:
                        highest_sample = highest_sample_n[n]
                        est_slope = est_slope_n[n]
                        if idx_to_mag(bias_level) > highest_sample:
                            assign = True
                    if assign:
                        nrn_est_offset = idx_to_mag(bias_level) * est_slope
                        all_offsets_est[bias_level, n] = nrn_est_offset * highest_sample / 3 + \
                                                         means[bias_level] * (1 - highest_sample / 3)
        return all_offsets_est

    def optimize_fmax(self, ps, DAC_SYN_PD=None, safety_margin=.85):
        """for all pools in the network, determine the best possible fmax * safety_margin
        given its tap point assignment

        Parameters:
        ===========
        safety_margin (float, default .85) : fmax margin to allow for decode mixing
        ps : (PoolSpec object)
            required pars: YX, loc_yx, TPM
        DAC_SYN_PD (int, default None) : which value of DAC_SYN_PD to use to retrieve the fmax
            calibration data. Supplying none causes the function to use the currently-set value

        Returns:
        =======
        safe_fmax (float, Hz) for the pool
        """
        required_pars = ['YX', 'loc_yx', 'TPM']
        ps.check_specified(required_pars)

        # get pulse width calibration data
        if DAC_SYN_PD is None:
            DAC_SYN_PD = self.hal.get_DAC_value('DAC_SYN_PD')
        pulse_widths = self.get_basic_calibration('synapse', 'pulse_width_dac_' + str(DAC_SYN_PD), return_as_numpy=True)

        # for each pool, figure out which tap points it uses
        # align that set within the global pulse_widths data
        # figure out what the slowest allowed fmax is of all of them
        pool_pulse_widths = pulse_widths[ps.loc_y//2 : (ps.loc_y + ps.Y)//2, 
                                         ps.loc_x//2 : (ps.loc_x + ps.X)//2]

        #syns_used = Pool.syn_use_count_from_TPM(ps.TPM, ps.Y, ps.X) >= 1
        # XXX above requires circular import, fn inlined for now
        used_by_dims = np.sum(np.abs(ps.TPM), axis=1) # used by any dim
        # shaped H//2, 2 , W//2, 2, D
        syn_blocks_used = used_by_dims.reshape((ps.Y//2, 2, ps.X//2, 2)) 
        syns_used = np.sum(syn_blocks_used, axis=(1, 3)) >= 1

        used_pw = pool_pulse_widths[syns_used]
        max_pw = np.max(used_pw)

        return 1 / max_pw * safety_margin

    @staticmethod
    def optimize_bias_twiddles(encoders, offsets_at_b3, pool_tw_offsets, policy='greedy_flat'):
        """Given the measured encoders and offsets of a pool, and the set of changes in 
        firing rates for each twiddle value relative to twiddle 0, determine the offsets that
        result in optimal neuron yield. 

        Parameters:
        ==========
        encoders (NxD array, Hz) : 
            estimated gain * encoders for the pool
        offsets_at_b3 (len N array, Hz) : 
            estimated offsets for the pool WHEN EACH BIAS TWIDDLE IS SET TO +3
            (these first two parameters are the returns of get_encoders_and_offsets())
        pool_tw_offsets (7xN array, Hz) :
            firing rates for each twiddle value relative to twiddle 0
            (this is the return of get_all_bias_twiddles())
        policy (string {'random', 'center', 'greedy_flat'}) :
            policy used to pick between multiple achievable 'good' offset values
                'random' : Choose from possible offsets uniformly randomly
                'center' : Tries to achieve an intercept distribution peaked in the center.
                    Also good if trying to get the highest possible yield (because
                    estimates of gains/biases might be slightly off, and neurons that we try 
                    to place near the edge of the intercept range might actually fall outside)
                    Choose the offset closest to 0.
                'greedy_flat' : Tries to achieve a flat intercept distribution. 
                    Divides the space of intercepts into bins, iterate through neurons, 
                    choosing the offset that puts the neuron in the bin with the 
                    fewest neurons in it currently. 
                'avoid edges' : Tries to achieve a flat distribution, not including upper/lower 5% of the range.
                    A mixture of 'center' and 'greedy_flat'.
                    Meant to achieve mostly flat intercept distribution, 
                    but without sacrificing yield.

        Returns:
        ========
        tw_vals, new_offsets, good, bin_counts, dbg

        tw_vals (len N int array, {-3, -2, 1, 0, 1, 2, 3}) : 
            twiddle values returned by the optimization
        new_offsets (len N float array, units Hz) : 
            expected offsets after twiddling for each neuron
        good : (len N bool array) :
            good/bad flag for each neuron post-twiddling
        bin_counts (array of ints): 
            histogram of intercepts post-twiddling, bins evenly spaced in [-1, 1]
        """

        BIAS_LEVELS = 7
        N_NEURONS = len(offsets_at_b3)

        #                measured at +3  (biases relative to 3)
        offset_options = offsets_at_b3 + pool_tw_offsets - pool_tw_offsets[BIAS_LEVELS - 1]

        gains = np.linalg.norm(encoders, axis=1)
        initial_intercepts = -offsets_at_b3 / gains
        twiddle_intercept_deltas = -(pool_tw_offsets - pool_tw_offsets[BIAS_LEVELS - 1]) / gains

        intercept_options = twiddle_intercept_deltas + initial_intercepts
        good_options_mask = (intercept_options < 1) & (intercept_options > -1)

        # fill these in for each policy
        tw_assignments = np.zeros((N_NEURONS), dtype=int)
        new_offsets = np.zeros(tw_assignments.shape, dtype=float)
        good = np.zeros(tw_assignments.shape, dtype=bool)
        bin_counts = None

        # used to define bin_counts
        num_bins = 20
        bin_edges = np.linspace(-1, 1, num_bins+1)

        if policy == 'random':
            for n in range(N_NEURONS):
                if ~np.isnan(initial_intercepts[n]):
                    good_options_idxs = np.arange(BIAS_LEVELS)[good_options_mask[:, n].flatten()]
                    if len(good_options_idxs) > 0:
                        bias_idx = good_options_idxs[np.random.randint(len(good_options_idxs))]
                        nrn_is_good = True
                    else:
                        # we might be firing all the time, or we might be straddling the 
                        # 'good' range with our intercept options,
                        # pick the closest, err high
                        bias_idx = (np.arange(BIAS_LEVELS)[offset_options[:, n] > 0])[0]
                        nrn_is_good = False
                else:
                    # if the intercept is NaN, this must be a never-fires guy
                    bias_idx = BIAS_LEVELS - 1 # +3
                    nrn_is_good = False

                tw_assignments[n] = bias_idx
                new_offsets[n] = offset_options[bias_idx, n]
                good[n] = nrn_is_good

            bin_counts = np.histogram(-new_offsets / gains, bin_edges)

        elif policy == 'center':
            for n in range(N_NEURONS):
                if ~np.isnan(initial_intercepts[n]):
                    good_options_idxs = np.arange(BIAS_LEVELS)[good_options_mask[:, n].flatten()]
                    this_nrn_offset_options = offset_options[:, n]
                    this_nrn_mask = good_options_mask[:, n]
                    good_options = this_nrn_offset_options[this_nrn_mask]
                    if len(good_options_idxs) > 0:
                        bias_idx = good_options_idxs[np.argmin(np.abs(good_options))]
                        nrn_is_good = True
                    else:
                        # we might be firing all the time, or we might be straddling the 
                        # 'good' range with our intercept options,
                        # pick the closest, err high
                        bias_idx = (np.arange(BIAS_LEVELS)[offset_options[:, n] > 0])[0]
                        nrn_is_good = False
                else:
                    # if the intercept is NaN, this must be a never-fires guy
                    bias_idx = BIAS_LEVELS - 1 # +3
                    nrn_is_good = False

                tw_assignments[n] = bias_idx
                new_offsets[n] = offset_options[bias_idx, n]
                good[n] = nrn_is_good

            bin_counts = np.histogram(-new_offsets / gains, bin_edges)

        elif policy == 'avoid_edges':
            raise NotImplementedError("avoid_edges isn't done yet")
            
        elif policy == 'greedy_flat':
            # divide space of intercepts into bins, iterate through neurons, 
            # take the DAC setting that puts neuron in the least-currently-filled bin

            # f(x) = 0 = f'(x) = x * e_unit * g + b (intercept = x condition)
            # g/b = -1/(x * e_unit)
            # x * e_unit = -b/g

            bin_counts = np.zeros((num_bins,))

            def binify(val):
                return (((val + 1) / 2) * num_bins).astype(int)

            for n in range(N_NEURONS):
                if ~np.isnan(initial_intercepts[n]):
                    good_options_idxs = np.arange(BIAS_LEVELS)[good_options_mask[:, n].flatten()]
                    good_intercepts = intercept_options[good_options_idxs, n]

                    if len(good_options_idxs) > 0:
                        option_bin_idxs = binify(good_intercepts) # bin idxs of [-1, 1] ints
                        option_bin_counts = bin_counts[option_bin_idxs] # counts of those bins

                        # idx to the smallest of those counts
                        # also the idx to the good_options_idxs that produces it
                        best_good_option_idx = np.argmin(option_bin_counts) 
                        
                        # bin idx to increment
                        best_bin_idx = option_bin_idxs[best_good_option_idx] 

                        bin_counts[best_bin_idx] += 1

                        bias_idx = good_options_idxs[best_good_option_idx]
                        nrn_is_good = True

                    else:
                        # we might be firing all the time, or we might be straddling the 
                        # 'good' range with our intercept options,
                        # pick the closest, err high
                        bias_idx = (np.arange(BIAS_LEVELS)[offset_options[:, n] > 0])[0]
                        nrn_is_good = False
                else:
                    # if the intercept is NaN, this must be a never-fires guy
                    bias_idx = BIAS_LEVELS - 1 # +3
                    nrn_is_good = False

                tw_assignments[n] = bias_idx
                new_offsets[n] = offset_options[bias_idx, n]
                good[n] = nrn_is_good
            
        elif policy == 'flat':
            # consider all possible DAC settings across all neurons simulatenously
            # choose the settings that achieve the flattest possible intercept distribution
            raise NotImplementedError("policy not implemented")
            gains = np.linalg.norm(encoders, axis=1)
        else:
            raise NotImplementedError("unkown policy")


        def tw_idx_to_val(idx):
            vals = [-3, -2, -1, 0, 1, 2, 3]
            return vals[idx]

        tw_vals = np.zeros(tw_assignments.shape, dtype=int)
        for n in range(N_NEURONS):
            tw_vals[n] = tw_idx_to_val(tw_assignments[n])
            
        dbg = {'options' : intercept_options}

        return tw_vals, new_offsets, good, bin_counts, dbg

    def get_encoders_and_offsets(self, ps, num_sample_angles=3, solver='scipy_opt', dacs={}):
        """Estimate the gains and biases of each neuron in the network

        An exhaustive (O(2**D)) scanning of the input space is not necessary.

        For each neuron, we need (D+1) data points where the neuron is firing. 
        Assuming that the neuron hasn't saturated (we set the refractory period to the minimum),
        with these points, we can fit a plane whose parameters are the effective encoders and offsets.

        Selecting the (D+1) points for each neurons is achieved by guessing at the encoder of 
        each neuron by looking at its surrounding tap points. The polarities of these tap points
        will be used for the middle point (e.g. [-1, 1, 0, 0, 1, 1] for some neuron in a 6D pool).
        Points within some angle of this point are sampled to obtain the other D points. 

        Data collection proceeds in iterations. All neurons are sampled simultaneously (in case the D+1 points
        for one neuron actually cause some other to fire--this extra data helps the fitting).
        After each iteration is completed, it is decided whether another is needed. All neurons must
        satisfy one of the following:
            - If the neuron does not fire at its "middle" point, it's assumed that it won't ever fire.
            - If it does, and the other D points also induced firing, then it's OK to proceed to the fitting.
        If the middle point induces firing, but not all D extra points did, another set of points
        are sampled at a tighter angle for each unsatisfied neuron. 
        After some number of iterations, if there are still unsatisfied neurons, 
        their responses must be really "wedged" into a corner of the input space. 
        We assume the encoder is the "middle" point, and the offset is the shortest projection
        of any point that did induce firing onto the middle point.

        Note that for NEF purposes, we're oversampling the input space (going all the way to the hypercube corners)
        The returned offsets will be in the range [-sqrt(D), sqrt(D)].

        Fitting yields a plane of the form: dot(a, x) + b, a is the effective encoder, b is the offset.
        Fitting is performed by least-squares: 
            min||X*A - fout|| 
            where A are the encoders, X are the sample points, fout are the observed firing rates
            rows of X and fout are extracted from the overall dataset where fout_i > 0

        Inputs:
        =======
        ps : (PoolSpec object)
            required pars: YX, loc_yx, TPM, fmax
            relevant pars: gain_divisors, biases, diffusor_cuts_yx, 

        Returns:
        =======
        est_encoders, est_offsets, mean_residuals, insufficient_points, dbg
            (same returns as  estimate_encs_from_tuning_curves)

        est_encoders: NxD array of estimated encoders * gains
        est_offsets: N-array of estimated offsets

        encoders * gains are in Hz, converting input value to output firing rate
        offsets are in Hz of output firing rate

        mean_residuals: N-array of fit errors
        insufficient_points: N-array of bools indicating that there weren't
            enough points to perform the fit accurately
        """

        required_pars = ['YX', 'loc_yx', 'TPM', 'fmax']
        ps.check_specified(required_pars)

        SAMPLE_FUDGE = 2 # sample (D + 1) * SAMPLE_FUDGE pts per middle point
        SAMPLE_ANGLES = [np.pi / 2**i for i in range(2, num_sample_angles+2)]
        HOLD_TIME = 1 # seconds
        BASELINE_TIME = 1 # seconds
        LPF_DISCARD_TIME = HOLD_TIME / 2 # seconds

        # estimate "middle points" for each neuron
        # first just compute approximate encoders based on exponential decays
        # then threshold and compute ceil to get something like [-1, 1, 0, 0, 1, 1]
        lam = 2 # XXX constant for now, should probably guess from DAC value
        approx_enc = Calibrator.get_approx_encoders(ps.TPM, ps.Y, ps.X, lam)

        nb = NetBuilder(self.hal)
        net = nb.create_single_pool_net_from_spec(ps)
        pool = net.get_pools()[0]
        inp = net.get_inputs()[0]

        self.hal.map(net)

        for dac, value in dacs.items():
            self.hal.set_DAC_value(dac, value)

        # set up run controller to help us do sweeps
        run = RunControl(self.hal, net)

        done = False
        unsolved_encs = approx_enc
        all_sample_pts = None
        all_spikes = None

        for sample_angle in SAMPLE_ANGLES:
            # generate sample points around unsolved_encs
            num_samples_per = (pool.dimensions + 1) * SAMPLE_FUDGE
            sample_pts, unique_encs = Calibrator.get_sample_points_around_encs(unsolved_encs, sample_angle, num_samples_per)

            # the input sweep of those sample_pts
            print("running sample sweep at sample_angle =", sample_angle, "rad")
            print("  taking", sample_pts.shape[0], "sample points for", unique_encs.shape[0], "unique encs")
            print("  will run for", HOLD_TIME * sample_pts.shape[0] / 60, "min.")
            tnow = self.hal.get_time()
            times = np.arange(sample_pts.shape[0]) * HOLD_TIME * 1e9 + tnow + .1e9
            times_w_end = np.arange(sample_pts.shape[0] + 1) * HOLD_TIME * 1e9 + tnow + .1e9
            sample_freqs = sample_pts * ps.fmax
            input_vals = {inp: (times, sample_freqs)}
            start_time = times[0]
            end_time = times[-1] + HOLD_TIME * 1e9
            _, spikes_and_bin_times = run.run_input_sweep(input_vals, get_raw_spikes=True, get_outputs=False, 
                                            start_time=start_time, end_time=end_time, rel_time=False)
            spikes, spike_bin_times = spikes_and_bin_times

            # each input sweep adds to the dataset that goes into the solver
            # that means that we re-solve for all neurons each sample
            # could make sense because more data is collected each time,
            # but adds potentially unecessary processing time
            if all_sample_pts is None:
                all_sample_pts = sample_pts
            else:
                all_sample_pts = np.vstack((all_sample_pts, sample_pts))

            #print("doing spike processing")
            discard_frac = LPF_DISCARD_TIME / HOLD_TIME
            spike_rates = data_utils.bins_to_rates(spikes[pool], spike_bin_times, times_w_end, init_discard_frac=discard_frac)
            if all_spikes is None:
                all_spikes = spike_rates
            else:
                all_spikes = np.vstack((all_spikes, spike_rates))

            #print("estimating encoders")
            est_encs, est_offsets, mean_residuals, insufficient_samples = \
                Calibrator.estimate_encs_from_tuning_curves(all_sample_pts, all_spikes, solver=solver)

            # whittle away at set of neurons we still need more data for
            unsolved_encs = approx_enc[insufficient_samples]
            print(np.sum(insufficient_samples), "neurons still need more points")

        # neurons without an estimated offset after trying tightest SAMPLE_ANGLE are assumed
        # to have offset < sqrt(D), we have given up on them

        debug = {'mean_residuals': mean_residuals,
                 'all_sample_pts': all_sample_pts,
                 'all_spikes': all_spikes}
        
        return est_encs, est_offsets, insufficient_samples, debug
                
    def validate_est_encs(self, est_encs, est_offsets, ps, sample_pts, dacs={}):
        """Validate the output of get_encoders_and_offsets

        Samples neuron firing rates at supplied sample_pts, compares to 
        est_encs * sample_pts + est_offsets, to directly assess predictive
        quality of est_encs and est_offsets.

        Inputs:
        =======
        est_encs (NxD array) : encoder estimates
        est_offsets (N array) : offset estimates
        ps : (PoolSpec object)
            required pars: YX, loc_yx, TPM, fmax
            relevant pars: gain_divisors, biases, diffusor_cuts_yx, 
        sample_pts (SxD array) : points to sample in the input space

        Returns:
        =======
        rmse_err, meas_rates, est_rates
        rmse_err (float) : RMSE firing rate error
        meas_rates (SxN array) : firing rates of each neuron at each sample_pt
        est_rates (SxN array) : what the est_encoders/offsets predicted
        """ 
        ps.check_specified(['YX', 'loc_yx', 'TPM', 'fmax'])

        HOLD_TIME = 1 # seconds
        LPF_DISCARD_TIME = HOLD_TIME / 2 # seconds

        # set up run controller to help us do sweeps
        nb = NetBuilder(self.hal)
        net = nb.create_single_pool_net_from_spec(ps)
        pool = net.get_pools()[0]
        inp = net.get_inputs()[0]

        self.hal.map(net)

        for dac, value in dacs.items():
            self.hal.set_DAC_value(dac, value)

        run = RunControl(self.hal, net)

        tnow = self.hal.get_time()
        times = np.arange(sample_pts.shape[0]) * HOLD_TIME * 1e9 + tnow + .1e9
        times_w_end = np.arange(sample_pts.shape[0] + 1) * HOLD_TIME * 1e9 + tnow + .1e9
        start_time = times[0]
        end_time = times[-1] + HOLD_TIME * 1e9
        sample_freqs = sample_pts * ps.fmax

        input_vals = {inp: (times, sample_freqs)}

        _, spikes_and_bin_times = run.run_input_sweep(input_vals, get_raw_spikes=True, get_outputs=False, 
                                        start_time=start_time, end_time=end_time, rel_time=False)
        print("done sweeping")
        spikes, spike_bin_times = spikes_and_bin_times

        est_A = np.maximum(0, np.dot(sample_pts, est_encs.T) + est_offsets)
        discard_frac = LPF_DISCARD_TIME / HOLD_TIME
        meas_A = data_utils.bins_to_rates(spikes[pool], spike_bin_times, times_w_end, init_discard_frac=discard_frac)

        RMSE = np.sqrt(np.mean((est_A.flatten() - meas_A.flatten())**2))
        return RMSE, meas_A, est_A
        
    @staticmethod
    def get_gains(encoders):
        """compute effective gains from encoders"""
        return np.linalg.norm(encoders, axis=1)

    @staticmethod
    def get_intercepts(encoders, offsets):
        """get neurons' intercepts"""
        gains = Calibrator.get_gains(encoders)
        intercepts = -offsets / gains
        good_mask = (intercepts < 1) & (intercepts > -1)
        return intercepts, good_mask

    @staticmethod
    def get_good_mask(encoders, offsets):
        """get mask for 'good' neurons (intercept in [-1, 1] range)"""
        _, good_mask = Calibrator.get_intercepts(encoders, offsets)
        return good_mask


    @staticmethod
    def plot_neuron_yield_cone(encoders, offsets, good, old_encs_offsets_and_vals=None, ax=None, xylim=None, title=None, figsize=(15, 15)):

        import matplotlib.pyplot as plt

        gains = Calibrator.get_gains(encoders)

        if ax is None:
            plt.figure(figsize=figsize)
            ax = plt.gca()

        colors = []
        for g in good:
            if g:
                colors.append('g')
            else:
                colors.append('r')
        ax.scatter(gains, offsets, c=colors)
        if xylim is None:
            cone_max = max(gains[~np.isnan(gains)])
        else:
            cone_max = xylim[1]
        x = np.linspace(0, cone_max, 100)
        ax.plot(x, x)
        ax.plot(x, -x)
        if title is None:
            ax.set_title('neuron gain and bias')
        else:
            ax.set_title(title)
        ax.set_xlabel('gain')
        ax.set_ylabel('bias')

        if xylim is not None:
            ax.axis(xylim)

        if old_encs_offsets_and_vals is not None:
            old_encs, old_offsets, vals = old_encs_offsets_and_vals
            old_gains = Calibrator.get_gains(old_encs)
            cmap = plt.get_cmap('RdBu')(np.linspace(0, 1, 7))

            for n_idx, g0, g1, o0, o1, v in zip(range(len(old_gains)), old_gains, gains, old_offsets, offsets, vals):
                if np.isnan(o1):
                    ax.scatter(g0, o0, marker='x', c=colors[n_idx])
                else:
                    vidx = v + 3
                    ax.plot([g0, g1], [o0, o1], c=cmap[vidx])

    @staticmethod
    def get_approx_encoders(tap_list_or_matrix, pooly, poolx, lam):
        """from a tap_list or tap_matrix, infer approximate encoders
        See get_encoders_and_offsets for more detail.
        """

        # we need a tap matrix
        if isinstance(tap_list_or_matrix, list):
            # create tap matrix from tap list
            dimensions = len(tap_list_or_matrix)
            tap_matrix = np.zeros((dimensions, pooly, poolx), dtype=int)
            # user may have specified tap list or tap matrix
            
            for dim_idx, dim_taps in enumerate(tap_list_or_matrix):
                for tap_idx, tap_sign in dim_taps:
                    tap_y = tap_idx // poolx
                    tap_x = tap_idx % poolx
                    tap_matrix[dim_idx, tap_y, tap_x] = tap_sign
        elif isinstance(tap_list_or_matrix, np.ndarray):
            # tap matrix is shaped like an encoder, we need enc dims as the 0th matrix dim
            _, dimensions = tap_list_or_matrix.shape
            tap_matrix = tap_list_or_matrix.T.reshape(tap_list_or_matrix.shape[1], pooly, poolx)
        else:
            raise ValueError("tap_list_or_matrix must be a list of lists or a numpy ndarray")

        num_neurons = pooly * poolx

        # approximate diffusor kernel
        def approx_decay_fn(d, lam):
            return (1 - lam) * np.exp(-d / lam)

        # fill in kernel
        kernel_x, kernel_y = (poolx * 2 - 1, pooly * 2 - 1)
        kernel = np.zeros((kernel_x, kernel_y))
        for x in range(kernel_x):
            for y in range(kernel_y):
                y_coord = y - pooly + 1
                x_coord = x - poolx + 1
                d = np.sqrt(y_coord**2 + x_coord**2)
                kernel[y, x] = approx_decay_fn(d, lam)

        # convolve kernel with tap matrix to obtain approximate encoders
        from scipy.signal import convolve2d

        eff_enc_T = np.zeros((dimensions, pooly, poolx))
        for d in range(dimensions):
            full_window = convolve2d(tap_matrix[d, :, :], kernel)
            eff_enc_T[d, :] = full_window[(pooly - 1):-(pooly - 1), (poolx - 1):-(poolx - 1)]

        return eff_enc_T.reshape((dimensions, pooly * poolx)).T


    @staticmethod
    def get_sample_points_around_encs(approx_enc, angle_away, num_samples_per):
        """from a set of approximate encoders, derive sample points that can be used 
        to determine the actual encoders
        See get_encoders_and_offsets for more detail.
        """
        dimensions = approx_enc.shape[1]

        if dimensions == 1:
            # take a different approach for D == 1
            # there can only be 2 unique encs
            unique_encs = np.array([[1], [-1]])

            sample_points = np.zeros((2 * num_samples_per,))
            min_val = np.sin(angle_away)
            max_val = 1
            sample_points[:num_samples_per] = np.linspace(min_val, max_val, num_samples_per)
            sample_points[num_samples_per:] = np.linspace(-min_val, -max_val, num_samples_per)

            return sample_points.reshape((num_samples_per * 2, 1)), unique_encs

        else:
            # do thresholding/ceil'ing
            threshold = .05
            thresh_enc = approx_enc.copy()
            thresh_enc[np.abs(thresh_enc) <= threshold] = 0
            thresh_enc[thresh_enc > 0] = 1
            thresh_enc[thresh_enc < 0] = -1

            # clear redundant encs
            unique_encs = np.unique(thresh_enc, axis=0)
            num_unique, _ = unique_encs.shape

            def get_point_angle_away(pt, angle):
                # 1. generate random point
                # 2. remove projection onto original point 
                #    (get random point orthogonal to original)
                # 3. add back scaled amount of original point that gives desired angle
                dims = len(pt)
                pt_norm = np.linalg.norm(pt)
                if pt_norm > 0:
                    unit_pt = pt / pt_norm

                    # 1. 
                    rand_pt = np.random.randn(dims)
                    rand_pt /= np.linalg.norm(rand_pt)

                    # 2. 
                    proj = unit_pt * np.dot(unit_pt, rand_pt)
                    rand_orthog_pt = rand_pt - proj
                    rand_orthog_pt /= np.linalg.norm(rand_orthog_pt)
                    assert(np.abs(np.dot(rand_orthog_pt, unit_pt)) <= .0001)

                    # 3. 
                    perp_component = np.cos(angle) * rand_orthog_pt
                    pll_component = np.sin(angle) * unit_pt
                    angle_away_pt = perp_component + pll_component
                    return angle_away_pt
                else:
                    return np.zeros_like(pt)

            sample_points = np.zeros((num_unique * num_samples_per, dimensions))
            for n in range(num_unique):
                sample_points[num_samples_per * n, :] = unique_encs[n] # use thresholded enc as middle point
                for pt_idx in range(num_samples_per - 1): # create D more points around this point
                    unit_vect_angle_away = get_point_angle_away(unique_encs[n], angle_away)
                    # scale unit vector up to unit cube
                    longest_comp = np.max(np.abs(unit_vect_angle_away)) 
                    if longest_comp > 0:
                        scaled_angle_away = unit_vect_angle_away / longest_comp
                    else:
                        scaled_angle_away = np.zeros_like(unit_vect_angle_away)
                    sample_points[num_samples_per * n + pt_idx] = scaled_angle_away

            return sample_points, unique_encs


    @staticmethod
    def estimate_encs_from_tuning_curves(sample_pts, firing_rates, fired_tolerance=35, solver='scipy_opt'):
        """Given firing_rates collected at sample_pts, infer the encoders
        and offsets of each neuron. 

        Inputs:
        ======
        sample_pts (SxD array) : input values used to collect data
        firing_rates (SxN array) : firing rates at those input values
        fired_tolerance (tup(float, float)) : 
            firing_rates > fired_tolerance are considered "on"
        solver (string) : how to solve for encoders
            LS (fit plane only, don't use "didn't fire" pts)
            scipy_opt (fit to relu shape, using "didn't fire" pts)

        Returns:
        =======
        est_encoders: NxD array of estimated encoders * gains
        est_offsets: N-array of estimated offsets
        mean_residuals: N-array of fit errors
        insufficient_points: N-array of bools indicating that there weren't
            enough points to perform the fit accurately

        See get_encoders_and_offsets for more detail.
        """
        num_samples, dims = sample_pts.shape
        num_samples_, neurons = firing_rates.shape
        assert(num_samples == num_samples_)

        est_encs = np.zeros((neurons, dims))
        est_offsets = np.zeros((neurons,))
        mean_residuals = np.zeros((neurons,))
        insufficient_samples = np.zeros((neurons,), dtype=bool)

        for n in range(neurons):
            # derive set of points that are valid for fitting
            on = firing_rates[:, n] > fired_tolerance
            firing_rates_on = firing_rates[on, n]
            num_on_samples = firing_rates_on.shape[0]

            # if we have at least D + 1 on samples, go for it
            if num_on_samples >= dims + 1:
                insufficient_samples[n] = False

                if solver == 'LS':
                    firing_rates_on = firing_rates_on.reshape(num_on_samples, 1)
                    sample_pts_on = sample_pts[on]
                    sample_pts_with_ones = np.ones((num_on_samples, dims+1))
                    sample_pts_with_ones[:, :dims] = sample_pts_on

                    # do LS fit
                    # min||sample_pts_on*A - firing_rates_on|| 
                    #         Sx(D+1)   *   (D+1)x1   ~   Sx1
                    # S*x = f
                    # S.T*S*x = S.T*f
                    # x = (S.T*S)-1 * S.T * f
                    # x = pinv(S) * f
                    pinv = np.linalg.pinv(sample_pts_with_ones)
                    enc_and_offset = np.dot(pinv, firing_rates_on)

                    est_encs[n, :] = enc_and_offset[:dims, :].flatten()
                    est_offsets[n] = enc_and_offset[dims, :].flatten()

                    if len(sample_pts_with_ones > 0):
                        mean_residuals[n] = np.mean(np.dot(sample_pts_with_ones, enc_and_offset) - firing_rates_on)
                    else:
                        mean_residuals[n] = 0

                elif solver == 'scipy_opt':
                    # use all the points, including 0s
                    # should be more robust to having very few sample points

                    from scipy.optimize import leastsq

                    def resp_func(x, p):
                        return np.maximum(0, np.dot(x, p[:dims]) + p[dims])                        
                    def get_min_func(x, y):
                        return lambda p : y - resp_func(x, p)

                    popt, _ = leastsq(get_min_func(sample_pts, firing_rates[:, n]), np.zeros((dims+1, 1)))

                    est_encs[n, :] = popt[:dims]
                    est_offsets[n] = popt[dims]
                    mean_residuals[n] = 0 # unsupported
                    
                else:
                    raise ValueError("unsupported solver: " + solver + 
                        ". Choose from 'LS' or 'scipy_opt'")

            # not enough samples to estimate encoder
            else:
                insufficient_samples[n] = True
                est_encs[n, :] = np.nan
                est_offsets[n] = np.nan
                mean_residuals[n] = np.nan

        return est_encs, est_offsets, mean_residuals, insufficient_samples

    def create_optimized_yx_taps(self, ps):
        LY, LX = ps.loc_yx
        Y, X = ps.YX
        D = ps.D

        # pull out bad_syn calibration for this pool
        bad_syns, _ = self.get_bad_syns()
        pool_bad_syns = Calibrator.crop_calibration_to_pool(bad_syns, ps)

        # create synapse tap point assignments
        SY, SX = NetBuilder.to_synspace(Y, X)
        syn_yx_tap_matrix = NetBuilder.create_default_yx_taps(SY, SX, D, bad_syn=pool_bad_syns)

        # reshape to NxD shape and make even
        tap_matrix = NetBuilder.syn_taps_to_nrn_taps(syn_yx_tap_matrix)
        NetBuilder.make_taps_even(tap_matrix)

        return tap_matrix, syn_yx_tap_matrix

    def optimize_yield(self, ps_orig, dacs={},
            fmax_safety_margin=.85, 
            bias_twiddle_policy='greedy_flat', 
            offset_source='calibration_db'):
        """Runs experiments on the supplied patch of neurons to optimize NEF-style neuron yield
        (number of neurons with intercepts (-bias/gain) in [-1, 1])

        uses the current DAC values

        Parameters:
        ===========
        ps_orig : (PoolSpec object)
            required pars: YX, loc_yx, D
        offset_source : (string {'calibration_db', 'new_sweep'}, or 7xN array)
            where to get the effect of the bias twiddle bits from
            calibration_db : stored calibration db values
                (measured for whole chip in standard setup)
            run_sweep : run a new experiment for this exact pool
                configuration (could be more accurate)
            can also specify twiddle offsets directly as 7xN array
        fmax_safety_margin : 
            safety_margin (float, default .85) : fmax margin (fudge factor) 
            to allow for decode mixing in optimize_fmax()
        bias_twiddle_policy : 
            policy used to pick between multiple achievable 'good' offset values.
            See optimize_bias_twiddles().
 
        Returns:
            ps : PoolSpec filled in with parameters that improve yield
                fills in these parameters not fixed by the user:
                    TPM, fmax, diffusor_cuts_yx
                always fills in biases
                No policy for gains, leaves at default=1
            est_encs, est_offsets : (NxD and len N arrays, Hz)
                estimated encoders and offsets
            dbg : {'before' : (encs, offsets at biases=3),
                   'expected' : (encs, offsets expected from optimization)}
                   'pool_tw_offsets' : pool twiddle offset values that were used
        =======
        """

        ps = ps_orig.copy()
        N = ps.X * ps.Y

        # capture these DAC values and use them throughout
        DAC_values = {dac_name:self.hal.get_DAC_value(dac_name) 
                            for dac_name in DAC_DEFAULTS}
        # use the user's explicit overrides
        for dac, value in dacs.items():
            DAC_values[dac] = value

        # create tap points
        if ps.TPM is None:
            ps.TPM, _ = self.create_optimized_yx_taps(ps)

        # find safe fmax
        if ps.fmax is None:
            ps.fmax = self.optimize_fmax(ps, safety_margin=fmax_safety_margin)

        # if D == 1, cut diffusor down the middle
        if ps.D == 1 and ps.diffusor_cuts_yx is not None:
            ps.diffusor_cuts_yx = NetBuilder.get_diff_cuts_to_break_pool_in_half(ps.Y, ps.X)
        
        # get encs and offsets for our network, at a given bias
        def run_bias_exp(bias, main_ps):
            # copy ps, write in bias we want to test
            ps = main_ps.copy()
            ps.biases = bias

            # run experiment
            encs, offs, _, _ = self.get_encoders_and_offsets(
                    ps, dacs=DAC_values, num_sample_angles=3, solver='scipy_opt')
            return encs, offs

        # get twiddle offsets (shouldn't depend on exact network configuration)
        # get offsets over bias from cal_db
        if offset_source == 'calibration_db':
            all_tw_offsets = self.get_all_bias_twiddles(extrapolate=True)
            pool_tw_offsets = Calibrator.crop_calibration_to_pool(all_tw_offsets, ps)
            pool_tw_offsets = pool_tw_offsets.reshape((7, N))
        # or run a new experiment to get them
        elif offset_source == 'run_sweep':
            import pickle
            raw_offsets = np.zeros((7, N))
            for bias_idx, bias in enumerate([-3, -2, -1, 0, 1, 2, 3]):
                _, offsets = run_bias_exp(bias, ps)
                raw_offsets[bias_idx, :] = offsets

            pickle.dump(raw_offsets, open('test_raw_offsets.pck', 'wb'))

            pool_tw_offsets= raw_offsets.copy()
            orig_offsets_at_0 = raw_offsets[3, :]
            for bias_idx, bias in enumerate([-3, -2, -1, 0, 1, 2, 3]):
                 pool_tw_offsets[bias_idx, :] -= orig_offsets_at_0

            pickle.dump(pool_tw_offsets, open('test_pool_tw_offsets_pre_extrapolation.pck', 'wb'))

            pool_tw_offsets = Calibrator.extrapolate_bias_twiddles(pool_tw_offsets)

            pickle.dump(pool_tw_offsets, open('test_pool_tw_offsets.pck', 'wb'))

        # user directly specifies offsets
        elif isinstance(offset_source, np.ndarray) and offset_source.shape == (7, N):
            pool_tw_offsets = offset_source
        else:
            raise ValueError("unknown offset_source '" + offset_source + "'")

        # now get the offset values FOR THIS PARTICULAR NETWORK, at biases=3
        encs_at_b3, offsets_at_b3 = run_bias_exp(3, ps)

        opt_biases, opt_offsets, _, _, _ = \
            Calibrator.optimize_bias_twiddles(
                encs_at_b3, offsets_at_b3, pool_tw_offsets, policy=bias_twiddle_policy)

        # set ps with what we have learned
        ps.biases = opt_biases

        # validate optimization by running again
        encs_val, offsets_val = run_bias_exp(opt_biases, ps)

        dbg = {
            'pool_tw_offsets' : pool_tw_offsets,
            'before' : (encs_at_b3, offsets_at_b3),
            'expected' : (encs_at_b3, opt_offsets)}

        return ps, encs_val, offsets_val, dbg


