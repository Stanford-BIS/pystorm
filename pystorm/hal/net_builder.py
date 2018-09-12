import numpy as np
import pandas as pd
from pystorm.hal.run_control import RunControl
from pystorm.hal import data_utils

class NetBuilder(object):

    def __init__(self, HAL, net=None):
        """Initialize NetBuilder:

        Inputs:
        =======
        HAL (HAL object) : 
        net (hal.neuromorph.graph object, default None) : 
            User may provide a custom network they constructed.
            If no network is supplied, typically one will be added with a 
            call like NetBuilder.create_single_pool_net()
        """
        self.HAL = HAL
        self.net = net

    def add_net(self, net):
        self.net = net

    def create_single_pool_net(self, Y, X, tap_matrix=None, decoders=None, biases=0, gain_divs=1, loc_yx=(0, 0)):
        """Creates a Network with a single Pool
        
        Inputs:
        =======
        Y (int) : number of rows in the pool
        X (int) : number of columns in the pool
        tap_matrix ((N, dim) array or None (default)) :
            array of tap point/dimension assignments to each neuron
                if provided, Network will have an Input connected to its Pool
            if None, Network will not have an Input
        decoders ((dim, N) array or None (default)) :
            array of each neuron's decoding weight in each dimension
                if provided, Network will have an Ouput connected to its Pool
            if None, Network will not have an Output
        biases ((N,) int array or int) :
            bias bits for each neuron
        gain_divs ((N,) int array or int) :
            gain divisor bits for each neuron

        Returns:
        ========
        Network object
        """
        N = Y * X

        if tap_matrix is None:
            Din = 0
            tap_spec = np.zeros((N, 1)) # have to put something in, (N, [[]]) might work
        else:
            if isinstance(tap_matrix, list):
                Din = len(tap_matrix)
                tap_spec = (N, tap_matrix)
            else:
                Din = tap_matrix.shape[1]
                tap_spec = tap_matrix
        assert tap_spec.shape[0] == N, (
            "tap matrix has {} entries but Y*X={}".format(tap_spec.shape[0], Y*X))

        if decoders is None:
            Dout = 0
        else:
            Dout = decoders.shape[0]

        from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
        net = graph.Network("net")

        # decoders are initially zero
        # we remap them later (without touching the rest of the network) using HAL.remap_weights()
        net.pool = net.create_pool("p1", tap_spec, biases=biases, gain_divisors=gain_divs, xy=(X, Y), user_xy_loc=(loc_yx[1], loc_yx[0]))

        if Dout > 0:
            b1 = net.create_bucket("b1", Dout)
            net.output = net.create_output("o1", Dout)
            net.decoder_conn = net.create_connection("c_p1_to_b1", net.pool, b1, decoders)
            net.create_connection("c_b1_to_o1", b1, net.output, None)
        if Din > 0:
            net.input = net.create_input("i1", Din)
            net.create_connection("c_i1_to_p1", net.input, net.pool, None)

        self.net = net
        return net

    @staticmethod
    def create_default_yx_taps(SY, SX, D, bad_syn=None):
        """create 'good' (i.e. maximally adjacently orthogonal) arrays of synapses

        Inputs:
        ======
        SY, SX (int, int) : dimensions of grid to create synapses in
        D (int) : dimensionality of representation
        bad_syn (pandas dataframe indexed y,x or (y, x) np array) :
            synapses to avoid (e.g. because of high bias or long T_PE)

        Returns:
        =======
        (SY, SX, D)-array of tap points
        can be converted to (Y, X, D) what Pool takes as tap_spec 
            with syn_taps_to_nrn_taps()
        """
        
        if isinstance(bad_syn, np.ndarray) and bad_syn.shape != (SY, SX):
            raise ValueError("bad_syn should be 2D array-like and shape (SY, SX)")

        if bad_syn is None:
            bad_syn = np.array([[False] * SY] * SX, dtype=bool)

        def get_bad_syn(y, x):
            if isinstance(bad_syn, pd.DataFrame):
                return bad_syn.loc[y, x]
            else:
                return bad_syn[y, x]
            
        def find_closest_not_bad(y, x):
            # XXX unused
            # search in expanding manhattan radii
            # doing this dumb-ly, O(N**2) instead of O(N)
            # really want to encode an outward spiral
            R = 1
            while True:
                if R == max(SX, SY):
                    assert(False)

                ylo = max(y - R, 0)
                yhi = min(y + R, SY - 1)
                xlo = max(x - R, 0)
                xhi = min(x + R, SX - 1)

                # now pick the first good one
                for y in range(ylo, yhi):
                    for x in range(xlo, xhi):

                        if not get_bad_syn(y, x):
                            return y, x

                R += 1

        def eliminate_projections(base_vect, neighbors):
            """eliminate <neighbors> projections on base_vect"""
            if len(neighbors) == 1:
                proj = np.dot(neighbors[0], np.dot(neighbors[0], base_vect))
                base_vect -= proj
                assert(np.abs(np.dot(neighbors[0], base_vect)) < 1e-10)
            elif len(neighbors) > 1:
                to_elim = np.vstack(neighbors)
                U, S, VT = np.linalg.svd(to_elim)
                VpT = VT[:len(neighbors), :]
                proj = np.dot(VpT.T, np.dot(VpT, base_vect))
                base_vect -= proj
                assert(np.sum(np.abs(np.dot(to_elim, base_vect))) < 1e-10)
            return base_vect 

        def get_cartesian_vector_set(D):
            vects = np.zeros((2*D, D))
            for d in range(D):
                vects[2*d, d] = 1
                vects[2*d+1, d] = -1
            return vects

        def get_random_unit_vector(D):
            gaussian = np.random.randn(D)
            return gaussian / np.linalg.norm(gaussian)


        # for D == 1, use on/off halves
        if D == 1:
            tap_matrix = np.zeros((SY, SX))
            for y in range(SY):
                for x in range(SX):
                    if not get_bad_syn(y, x):
                        if x < SX // 2:
                            tap_matrix[y, x] = 1
                        else:
                            tap_matrix[y, x] = -1

        else:
            # can expose these later, I suppose
            use_mean = True
            cartesian = True
            cartesian_vects = get_cartesian_vector_set(D)

            # pick a random standard basis direction for each tap
            # try to keep adjacent vectors orthogonal

            # raster-scan, considering already-set vectors
            # neighborhood under consideration grows with dimensions
            tap_matrix = np.zeros((SY, SX, D), dtype=int)
            for y in range(SY):
                for x in range(SX):
                    if not get_bad_syn(y,x):
                        neighbors = []
                        if D >= 2:
                            if x > 0:
                                if ~get_bad_syn(y, x - 1):
                                    neighbors.append('l')
                                elif D == 2 and y > 0: # helps 2D with few taps
                                    neighbors.append('u')
                            elif D == 2 and y > 0: # helps 2D with few taps
                                neighbors.append('u')
                        if D >= 3:
                            if y > 0:
                                neighbors.append('u')
                        if D >= 4:
                            if x > 0 and y > 0:
                                neighbors.append('ul')
                        if D >= 5:
                            if x < grid_pts_X - 1 and y > 0:
                                neighbors.append('ur')

                        elim_vects = []
                        for n in neighbors:
                            if n == 'l':
                                elim_vects.append(tap_matrix[y, x - 1])
                            if n == 'u':
                                elim_vects.append(tap_matrix[y - 1, x])
                            if n == 'ul':
                                elim_vects.append(tap_matrix[y - 1, x - 1])
                            if n == 'ur':
                                elim_vects.append(tap_matrix[y - 1, x + 1])

                        base_vect_norm = 0
                        fails = 0

                        # debugging info
                        base_vect_tries = []
                        base_vect_elims = []
                        base_vect_tries_cart = []

                        while True:
                            # now assign the base_vect to eliminate projections from neighbors
                            # keep trying if we pick the base_vect badly

                            base_vect = get_random_unit_vector(D)
                            base_vect_tries.append(base_vect)

                            # if convert completely random vector into its nearest
                            # standard_basis vector
                            if cartesian:
                                similarities = np.dot(cartesian_vects, base_vect)
                                base_vect = cartesian_vects[np.argmax(similarities)].copy()
                                base_vect_tries_cart.append(base_vect)

                            # eliminate projections
                            # the base_vect we chose may be in the span of the neighbors
                            # if so, try again, up to some limit
                            try:
                                base_vect = eliminate_projections(base_vect, elim_vects)
                                base_vect_elims.append(base_vect)
                                base_vect_norm = np.linalg.norm(base_vect)

                                # if taking the neighbor's projections out of the
                                # random vector leaves you with anything, break out
                                if base_vect_norm > 1e-10:
                                    candidate_vect = base_vect / base_vect_norm

                                    # for any vector that "works", so does its opposite
                                    # use the one that moves the mean encoder closer to zero
                                    # XXX can also take into account if not orthogonal to 
                                    # some neighbors, esp for D == 2
                                    if use_mean:
                                        curr_sum = np.sum(tap_matrix, axis=(0,1))
                                        pos_norm = np.linalg.norm(curr_sum + candidate_vect)
                                        neg_norm = np.linalg.norm(curr_sum - candidate_vect)
                                        if neg_norm < pos_norm:
                                            candidate_vect *= -1

                                    break # leave while with candidate_vect

                            # shouldn't happen, but try again if it does 
                            except AssertionError: 
                                base_vect_norm = 0

                            # print debug info if something goes really wrong
                            fails += 1
                            if fails > 100:
                                print("failed at y,x: ", y, ",", x)
                                print("tap matrix neighborhood")
                                print(tap_matrix[y-1:y+1, x-1:x+1, :])
                                print("last ten tries:")
                                print("random vector candidates")
                                print(np.array(base_vect_tries[-10:]))
                                print("closest cartesian vector")
                                print(np.array(base_vect_tries_cart[-10:]))
                                print("after eliminating neighbor's projections")
                                print(np.array(base_vect_elims[-10:]))

                                raise RuntimeError("failed to get orthogonal vector 100 times" + 
                                    "something is probably wrong with neighborhood logic")

                        tap_matrix[y, x, :] = candidate_vect

        tap_matrix = tap_matrix.reshape((SY, SX, D))
        for i in range(D):
            items = np.nonzero(tap_matrix[:, i])[0]
            if len(items) % 2 == 1:
                tap_matrix[items[-1], i] = 0
        return tap_matrix

    def slice_pool_in_half(self, pool):
        """Opens the diffusor down the middle of a pool. Good for 1D pools with default diffusors
        
        Parameters:
        ==========
        pool (Pool object) the pool (in the currently mapped network) to cut
        """ 

        raise NotImplementedError("haven't tested this yet")

        if self.net is None:
            raise RuntimeError("no Network attached to NetBuilder")
        if self.HAL.last_mapped_network != self.net:
            raise RuntimeError("Trying to run un-mapped network. Run map first.")
        if pool not in self.net.get_pools():
            raise ValueError("supplied pool was not in the current network")
        
        ## these are all tile coordinates
        #x_off = LX // 2 // 2
        #y_off = LY // 2 // 2
        #num_tiles_vert = SY // 2
        #x_idx = SX // 2 // 2 #east of midline tile idx

        ##print(x_idx + x_off, y_idx + y_off)
        #hal.driver.OpenDiffusorCutXY(0, x_idx + x_off, y_idx + y_off, DIFFUSOR_WEST_TOP)
        #hal.driver.OpenDiffusorCutXY(0, x_idx + x_off, y_idx + y_off, DIFFUSOR_WEST_BOTTOM)

    def open_all_diff_cuts(self):
        """Opens all the diffusor cuts (no current passes)

        works on an already-mapped network. Remapping will erase this state.
        """

        # this isn't strictly necessary (the fn doesn't operate on self.net)
        # but it does enforce that the network is already mapped
        if self.net is None:
            raise RuntimeError("no Network attached to NetBuilder")
        if self.HAL.last_mapped_network != self.net:
            raise RuntimeError("Trying to run un-mapped network. Run map first.")

        CORE_ID = 0        
        # connect diffusor around pools
        for tile_id in range(256):
            self.HAL.driver.OpenDiffusorAllCuts(CORE_ID, tile_id)

    @staticmethod
    def syn_taps_to_nrn_taps(tap_matrix, spacing=1):
        SY, SX, D = tap_matrix.shape
        Y = SY * 2 * spacing
        X = SX * 2 * spacing
        nrn_tap_matrix = np.zeros((Y, X, D))
        for d in range(D):
            nrn_tap_matrix[::2*spacing, ::2*spacing, d] = tap_matrix[:, :, d]
        return nrn_tap_matrix.reshape((Y * X, D))

    @staticmethod
    def make_taps_even(taps):
        """taking a tap list or tap matrix, make the number of taps per dim even
        modifies taps, removing taps to meet the  eveness condition
        """
        if isinstance(taps, list):
            for tap_dim in taps:
                if len(tap_dim) % 2 == 1:
                    tap_dim = tap_dim[:-1]
        else:
            dims = taps.shape[1]
            for d in range(dims):
                tap_dim = taps[:, d]
                if int(np.sum(np.abs(tap_dim))) % 2 == 1:
                    nonzero_idxs = np.arange(len(tap_dim))[tap_dim != 0]
                    rand_nonzero_idx = nonzero_idxs[np.random.randint(np.sum(tap_dim != 0))]
                    taps[rand_nonzero_idx, d] = 0

    @staticmethod
    def get_approx_encoders(tap_list_or_matrix, pooly, poolx, lam):
        """from a tap_list or tap_matrix, infer approximate encoders
        See determine_encoders_and_offsets for more detail.
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
        See determine_encoders_and_offsets for more detail.
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

        See determine_encoders_and_offsets for more detail.
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

    def determine_encoders_and_offsets(self, pool, inp, fmax, num_sample_angles=3, solver='scipy_opt'):
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
        pool : Pool to collect data from
        inp : Input that drives that pool

        NOTE : one pool at a time, for now

        Returns:
        =======
        same returns as  estimate_encs_from_tuning_curves

        est_encoders: NxD array of estimated encoders * gains
        est_offsets: N-array of estimated offsets

        encoders * gains are in Hz, converting input value to output firing rate
        offsets are in Hz of output firing rate

        mean_residuals: N-array of fit errors
        insufficient_points: N-array of bools indicating that there weren't
            enough points to perform the fit accurately

        """

        SAMPLE_FUDGE = 2 # sample (D + 1) * SAMPLE_FUDGE pts per middle point
        SAMPLE_ANGLES = [np.pi / 2**i for i in range(2, num_sample_angles+2)]
        HOLD_TIME = 1 # seconds
        BASELINE_TIME = 1 # seconds
        LPF_DISCARD_TIME = HOLD_TIME / 2 # seconds

        if self.net is None:
            raise RuntimeError("no Network attached to NetBuilder")

        # estimate "middle points" for each neuron
        # first just compute approximate encoders based on exponential decays
        # then threshold and compute ceil to get something like [-1, 1, 0, 0, 1, 1]
        if pool.tap_matrix is not None:
            approx_enc_arg = pool.tap_matrix
        else:
            approx_enc_arg = pool.tap_list

        lam = 2 # constant for now, should guess from DAC value
        approx_enc = NetBuilder.get_approx_encoders(approx_enc_arg, pool.y, pool.x, lam)

        # set up run controller to help us do sweeps
        run = RunControl(self.HAL, self.net)

        ## get baseline firing rates
        #print("getting baseline firing rates of population")
        #tnow = self.HAL.get_time()
        #times = np.array([tnow + .1e9])
        #zero_rates = np.zeros((1, pool.dimensions))
        #input_vals = {inp: (times, zero_rates)}
        #_, spikes_and_bin_times = run.run_input_sweep(input_vals, get_raw_spikes=True, get_outputs=False, 
        #                                          start_time=times[0], end_time=times[-1] + BASELINE_TIME)
        #spikes, spike_bin_times = spikes_and_bin_times
        #baselines = np.mean(spikes[pool], axis=0)

        done = False
        unsolved_encs = approx_enc
        all_sample_pts = None
        all_spikes = None

        for sample_angle in SAMPLE_ANGLES:
            # generate sample points around unsolved_encs
            num_samples_per = (pool.dimensions + 1) * SAMPLE_FUDGE
            sample_pts, unique_encs = NetBuilder.get_sample_points_around_encs(unsolved_encs, sample_angle, num_samples_per)

            # the input sweep of those sample_pts
            print("running sample sweep at sample_angle =", sample_angle, "rad")
            print("  taking", sample_pts.shape[0], "sample points for", unique_encs.shape[0], "unique encs")
            print("  will run for", HOLD_TIME * sample_pts.shape[0] / 60, "min.")
            tnow = self.HAL.get_time()
            times = np.arange(sample_pts.shape[0]) * HOLD_TIME * 1e9 + tnow + .1e9
            times_w_end = np.arange(sample_pts.shape[0] + 1) * HOLD_TIME * 1e9 + tnow + .1e9
            sample_freqs = sample_pts * fmax
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
                NetBuilder.estimate_encs_from_tuning_curves(all_sample_pts, all_spikes, solver=solver)

            # whittle away at set of neurons we still need more data for
            unsolved_encs = approx_enc[insufficient_samples]
            print(np.sum(insufficient_samples), "neurons still need more points")

        # neurons without an estimated offset after trying tightest SAMPLE_ANGLE are assumed
        # to have offset < sqrt(D), we have given up on them

        debug = {'mean_residuals': mean_residuals,
                 'all_sample_pts': all_sample_pts,
                 'all_spikes': all_spikes}
        
        return est_encs, est_offsets, insufficient_samples, debug
                
    def validate_est_encs(self, est_encs, est_offsets, pool, inp, sample_pts, fmax):
        """Validate the output of determine_est_encs

        Samples neuron firing rates at supplied sample_pts, compares to 
        est_encs * sample_pts + est_offsets, to directly assess predictive
        quality of est_encs and est_offsets.

        Inputs:
        =======
        est_encs (NxD array) : encoder estimates
        est_offsets (N array) : offset estimates
        pool (Pool graph object) : pool that these apply to
        inp (Input graph object) : input that these apply to
        sample_pts (SxD array) : points to sample in the input space
        fmax (float) : fmax to use

        Returns:
        =======
        rmse_err, meas_rates, est_rates
        rmse_err (float) : RMSE firing rate error
        meas_rates (SxN array) : firing rates of each neuron at each sample_pt
        est_rates (SxN array) : what the est_encoders/offsets predicted
        """ 

        HOLD_TIME = 1 # seconds
        LPF_DISCARD_TIME = HOLD_TIME / 2 # seconds

        # set up run controller to help us do sweeps
        run = RunControl(self.HAL, self.net)

        tnow = self.HAL.get_time()
        times = np.arange(sample_pts.shape[0]) * HOLD_TIME * 1e9 + tnow + .1e9
        times_w_end = np.arange(sample_pts.shape[0] + 1) * HOLD_TIME * 1e9 + tnow + .1e9
        start_time = times[0]
        end_time = times[-1] + HOLD_TIME * 1e9
        sample_freqs = sample_pts * fmax

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
        
    def determine_bad_syns(self, pulse_attrition=.05):
        """Looks at synapse-related calibration data, discards synapses that have 
        high bias offset contributions and very slow synapses. 

        Uses the current value of DAC_SYN_PD when retrieving the synaptic delay calibration.

        Parameters:
        ==========
        pulse_attrition (float, defalut .05) : fraction of synapses to discard based
            on having long pulse extender values

        Returns:
        =======
        ((width, height) array of bools for which synapses are bad, debug_info)
            debug info is dict with keys 
                {'pulse widths',
                 'pulse widths w/o high_bias syns'}
        """

        # look up high bias synapses
        high_bias = self.HAL.get_calibration(
            'synapse', 'high_bias_magnitude', return_as_numpy=True)

        # look up pulse widths
        curr_DAC_SYN_PD = self.HAL.get_DAC_value('DAC_SYN_PD')
        pulse_widths = self.HAL.get_calibration(
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

    def determine_safe_fmaxes(self, safety_margin=.85, pool_locations=None, DAC_SYN_PD=None):
        """for all pools in the network, determine the best possible fmax * safety_margin
        given its tap point assignment

        The network that you're calling this for needs to be mapped, or you 
        need to tell it where you're going to put each pool.

        Parameters:
        ===========
        safety_margin (float, default .85) : fmax margin to allow for decode mixing
        pool (Pool object) : pool to evaluate this for (depends on tap points used)
        pool_point_locations : (dict {pool id : (y,x) location}, default None)
            manual specification of pool locations, avoids requirement that network is mapped
        DAC_SYN_PD (int, default None) : which value of DAC_SYN_PD to use to retrieve the fmax
            calibration data. Supplying none causes the function to use the currently-set value

        Returns:
        =======
        dict with:
        {pool id : safe fmax for pool}
        for all pools in the NetBuilder's current Network
        """

        if self.net is None:
            raise RuntimeError("no Network attached to NetBuilder")

        # fill in pool_locations from mapping info, if possible
        if pool_locations is None:

            if self.HAL.last_mapped_network != self.net:
                raise RuntimeError("Trying to run un-mapped network. Run map first.")

            pool_locations = {}
            for pool in self.net.get_pools():
                x, y = pool.mapped_xy
                pool_locations[pool] = (y, x)

        pool_wh = {}
        for pool in self.net.get_pools():
            pool_wh[pool] = (pool.width, pool.height)
        
        # pull pulse width calibration data
        if DAC_SYN_PD is None:
            DAC_SYN_PD = self.HAL.get_DAC_value('DAC_SYN_PD')
        pulse_widths = self.HAL.get_calibration('synapse', 'pulse_width_dac_' + str(DAC_SYN_PD), return_as_numpy=True)

        # for each pool, figure out which tap points it uses
        # align that set within the global pulse_widths data
        # figure out what the slowest allowed fmax is of all of them
        safe_fmaxes = {}
        for pool in pool_locations:
            pool_w, pool_h = pool_wh[pool]
            pool_y, pool_x = pool_locations[pool]
            pool_pulse_widths = pulse_widths[pool_y//2 : (pool_y + pool_h)//2, 
                                             pool_x//2 : (pool_x + pool_w)//2]

            syns_used = pool.syn_use_counts_matrix >= 1
            used_pw = pool_pulse_widths[syns_used]
            max_pw = np.max(used_pw)

            safe_fmaxes[pool] = 1 / max_pw * safety_margin

        return safe_fmaxes

    def get_all_bias_twiddles(self):
        """collect the different soma bias twiddle calibrations into one data structure
        use the data associated with the current DAC_SOMA_OFFSET setting

        Returns:
        =======
        6x64x64 array of offsets
        """

        # look up current DAC setting
        curr_DAC_SOMA_OFFSET = self.HAL.get_DAC_value('DAC_SOMA_OFFSET')

        all_offsets = np.zeros((7, 64, 64))
        for i, p, b in zip([0, 1, 2, 4, 5, 6], ['n']*3 + ['p']*3, [3, 2, 1, 1, 2, 3]):
            all_offsets[i] = self.HAL.get_calibration(
                'soma', 'bias_twiddle_' + p + str(b) + '_dac_' + str(curr_DAC_SOMA_OFFSET), 
                return_as_numpy=True)

        return all_offsets

    @staticmethod
    def extrapolate_bias_twiddles(bias_twiddles):
        pass
    
    def get_pool_bias_twiddles(self, pool):

        BIAS_LEVELS = 7
    
        if self.net is None:
            raise RuntimeError("no Network attached to NetBuilder")
        if self.HAL.last_mapped_network != self.net:
            raise RuntimeError("Trying to run un-mapped network. Run map first.")
        if pool not in self.net.get_pools():
            raise ValueError("supplied pool was not in the current network")

        all_offsets = self.get_all_bias_twiddles()

        x_loc, y_loc = pool.mapped_xy
        pool_tw_offsets = self.get_all_bias_twiddles()[:, y_loc : y_loc + pool.height,
                                                          x_loc : x_loc + pool.width]
        pool_tw_offsets = pool_tw_offsets.reshape((BIAS_LEVELS, pool.n_neurons))

        return pool_tw_offsets

    def optimize_bias_twiddles(self, pool, encoders, offsets_at_b3, policy='greedy_flat'):

        if self.net is None:
            raise RuntimeError("no Network attached to NetBuilder")
        if self.HAL.last_mapped_network != self.net:
            raise RuntimeError("Trying to run un-mapped network. Run map first.")
        if pool not in self.net.get_pools():
            raise ValueError("supplied pool was not in the current network")

        pool_tw_offsets = self.get_pool_bias_twiddles(pool)

        return NetBuilder.pick_good_twiddles(encoders, offsets_at_b3, pool_tw_offsets, policy)

    @staticmethod
    def pick_good_twiddles(encoders, offsets_at_b3, pool_tw_offsets, policy='greedy_flat'):

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

    @staticmethod
    def get_gains(encoders):
        return np.linalg.norm(encoders, axis=1)

    @staticmethod
    def get_good_mask(encoders, offsets):
        gains = NetBuilder.get_gains(encoders)
        intercepts = -offsets / gains
        good_mask = (intercepts < 1) & (intercepts > -1)
        return good_mask

    @staticmethod
    def plot_neuron_yield_cone(encoders, offsets, good, old_encs_offsets_and_vals=None, ax=None, xylim=None, title=None, figsize=(15, 15)):

        import matplotlib.pyplot as plt

        gains = NetBuilder.get_gains(encoders)

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
            old_gains = NetBuilder.get_gains(old_encs)
            cmap = plt.get_cmap('RdBu')(np.linspace(0, 1, 7))

            for n_idx, g0, g1, o0, o1, v in zip(range(len(old_gains)), old_gains, gains, old_offsets, offsets, vals):
                if np.isnan(o1):
                    ax.scatter(g0, o0, marker='x', c=colors[n_idx])
                else:
                    vidx = v + 3
                    ax.plot([g0, g1], [o0, o1], c=cmap[vidx])


