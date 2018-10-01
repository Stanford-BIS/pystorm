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
        self.hal = HAL
        self.net = net

    def add_net(self, net):
        self.net = net

    def create_single_pool_net_from_spec(self, ps, decoders=None):
        return self.create_single_pool_net(
            ps.Y, ps.X, 
            tap_matrix=ps.TPM, 
            decoders=decoders, 
            biases=ps.biases,
            gain_divs=ps.gain_divisors,
            loc_yx=ps.loc_yx,
            diffusor_cuts_yx=ps.diffusor_cuts_yx)

    def create_single_pool_net(self, Y, X, tap_matrix=None, decoders=None, 
            biases=0, gain_divs=1, loc_yx=(0, 0), diffusor_cuts_yx=None):
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
        net.pool = net.create_pool("p1", tap_spec, 
                biases=biases, gain_divisors=gain_divs, 
                xy=(X, Y), user_xy_loc=(loc_yx[1], loc_yx[0]),
                diffusor_cuts_yx=diffusor_cuts_yx)

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
    def to_synspace(nrny, nrnx):
        """converts y, x nrn coordinate to synapse coordinate"""
        return nrny // 2, nrnx // 2

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
        can be converted to (Y*X, D) what Pool takes as tap_spec 
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

    @staticmethod
    def get_diff_cuts_to_break_pool_in_half(height, width):
        x = width // 2
        cut_yxs = []
        for y in range(0, height, 4):
            cut_yxs.append((y, x + 1, 'left'))
        return cut_yxs

    def break_pool_in_half(self, pool):
        """Opens the diffusor down the middle of a pool. 

        Good for 1D pools with default tap points (improves yield).
        
        Parameters:
        ==========
        pool (Pool object) the pool (in the currently mapped network) to cut
        """ 

        if self.net is None:
            raise RuntimeError("no Network attached to NetBuilder")
        if self.hal.last_mapped_network != self.net:
            raise RuntimeError("Trying to run un-mapped network. Run map first.")
        if pool not in self.net.get_pools():
            raise ValueError("supplied pool was not in the current network")
            
        loc_y, loc_x = pool.mapped_yx
        cut_yxs = NetBuilder.get_diff_cuts_to_break_pool_in_half(pool.height, pool.width)
        for y, x, direction in cut_yxs:
            self.hal.set_diffusor(y + loc_y, x + loc_x, direction, 'broken')

    def open_all_diff_cuts(self):
        """Opens all the diffusor cuts (no current passes)

        works on an already-mapped network. Remapping will erase this state.
        """

        # this isn't strictly necessary (the fn doesn't operate on self.net)
        # but it does enforce that the network is already mapped
        if self.net is None:
            raise RuntimeError("no Network attached to NetBuilder")
        if self.hal.last_mapped_network != self.net:
            raise RuntimeError("Trying to run un-mapped network. Run map first.")

        CORE_ID = 0        
        # connect diffusor around pools
        for tile_id in range(256):
            self.hal.driver.OpenDiffusorAllCuts(CORE_ID, tile_id)

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

