from .graph_object import GraphObject
import pystorm.hal.neuromorph.hardware_resources as hwr

import numpy as np

class Pool(GraphObject):
    """Represents a pool of neurons
    
    Parameters
    ----------
    label: string
        name of pool
    tap_spec:
        Two options:
        1. encoder matrix (pre-diffuser), size neurons-by-dimensions.
           Elements must be in {-1, 0, 1}.
           Implicitly describes pool dimensionality and number of neurons.
        2. sparse tap list (N, [[tap dim 0 list], ... , [tap dim D-1 list]]) the equivalent tap list
           [tap dim d list] has elements (neuron idx, tap sign) where tap sign is in {-1, 1}
    x: int
        neuron pool is physically a rectangle; x dimension of neuron pool
    y: int
        neuron pool is physically a rectangle; y dimension of neuron pool
    allow_weird_taps : bool (default False)
        suppress check for whether tap points are used multiple times
    """
    def __init__(self, label, tap_spec, x, y, 
            gain_divisors=1, biases=0, user_xy_loc=(None, None),
            allow_weird_taps=False, diffusor_cuts_yx=None):
        super(Pool, self).__init__(label)
        self.label = label

        self.x = x
        self.y = y
        self.user_xy_loc = user_xy_loc
        self.gain_divisors = gain_divisors
        self.biases = biases
        self.diffusor_cuts_yx = diffusor_cuts_yx

        # create tap list from tap matrix, if necessary
        if isinstance(tap_spec, tuple):
            self.n_neurons, tap_list = tap_spec
            self.dimensions = len(tap_list)
            self.tap_list = tap_list
            self._tap_matrix = None
        else:
            self.n_neurons, self.dimensions = tap_spec.shape
            self.tap_list = Pool.tap_matrix_to_list(tap_spec)
            self._tap_matrix = tap_spec

        if not allow_weird_taps:
            self.check_tap_list_for_collision()

        # if user supplied int for entire population, expand into array
        if np.issubdtype(type(self.gain_divisors), np.integer):
            self.gain_divisors = np.ones((self.n_neurons,), dtype='int') * self.gain_divisors
        if np.issubdtype(type(self.biases), np.integer):
            self.biases = np.ones((self.n_neurons,), dtype='int') * self.biases

        # check that it's an array now
        for obj in [self.gain_divisors, self.biases]:
            assert isinstance(obj, np.ndarray), (
                "gain and bias parameters must be ints or numpy arrays of ints")
            assert (obj.dtype == np.dtype('int64') or obj.dtype == np.dtype('int32')), (
                "gain and bias parameters must be arrays of ints")
            assert len(obj) == self.n_neurons, (
                "gain and bias parameters must be arrays of length N")

        # allowed gains are 1x, 1/2x, 1/3x, 1/4x
        assert(np.all(self.gain_divisors >= 1) and np.all(self.gain_divisors <= 4))
        # allowed biases are -3, 2, 1, 0, 1, 2, 3
        assert(np.all(self.biases >= -3) and np.all(self.biases <= 3))

        assert(len(self.biases) == self.n_neurons)
        assert(self.n_neurons == x * y)

    # make sure that the tap point assignment isn't likely to cause problems
    def check_tap_list_for_collision(self):
        syn_use_counts = self.syn_use_counts_matrix

        if (syn_use_counts > 1).any():
            errstr = "Bad tap point assignment:\n"
            errstr += "  Synapse use counts (by any dim). Should be <= 1:\n"
            errstr += str(syn_use_counts) + '\n'
            errstr += 'Collision detected at (syn_y, syn_x) = ' + str(key) + \
                ', (nrn_y, nrn_x) = ' + str((nrn_y, nrn_x)) + \
                ' (and maybe other places, see above matrix). ' + \
                'Supply the "allow_redudant_taps" argument to suppress this error ' + \
                '(if you know what you\'re doing).'
            raise ValueError(errstr)

    def __repr__(self):
        return "Pool " + self.label + ". dims in = " + str(self.dimensions) + ". neurons = " + str(self.n_neurons)

    @staticmethod
    def tap_list_to_matrix(L, N):
        """Converts tap matrix to tap list
        Inputs:
        ======
        L [[tap dim 0 list], ... , [tap dim D-1 list]] : the list-of-lists specifying the tap points
            where tap dim n list is [(tap idx, sign), (tap_idx, sign), ...]
        N (int) : the number of neurons

        Returns:
        =======
        (DxN array, elements in {-1, 0, 1}) : the encoder-like tap matrix
        """
        D = len(L)
        M = np.zeros((N, D), dtype=int)

        for d_idx, dim_taps in enumerate(L):
            for tap_idx, sign in dim_taps:
                M[tap_idx, d_idx] = sign
        return M

    @staticmethod
    def tap_matrix_to_list(M): 
        """Converts tap matrix to tap list
        Inputs:
        ======
        M (DxN array, elements in {-1, 0, 1}) : the encoder-like tap matrix

        Returns:
        =======
        [[tap dim 0 list], ... , [tap dim D-1 list]] : the list-of-lists specifying the same tap points
            where tap dim n list is [(tap idx, sign), (tap_idx, sign), ...]
        """
        nrns = M.shape[0]
        dims = M.shape[1]

        taps_and_signs = [[] for d in range(dims)]
        for d in range(dims):
            for n in range(nrns):
                entry = M[n, d]
                if entry not in [-1, 0, 1]:
                    raise ValueError("tap_matrix entries must be in -1, 0, 1")
                if entry != 0:
                    t = n
                    s = int(entry)
                    taps_and_signs[d].append((t, s))

        return taps_and_signs

    @property
    def tap_matrix(self):
        # generates tap_matrix on-demand if requested
        if self._tap_matrix is None:
            self._tap_matrix = Pool.tap_list_to_matrix(self.tap_list, self.n_neurons)
        return self._tap_matrix

    @staticmethod
    def syn_use_counts_from_TPM(tap_matrix, height, width):
        used_by_dims = np.sum(np.abs(tap_matrix), axis=1) # used by any dim
        # shaped H//2, 2 , W//2, 2, D
        syn_blocks_used = used_by_dims.reshape((height//2, 2, width//2, 2)) 
        syn_use_counts = np.sum(syn_blocks_used, axis=(1, 3))
        return syn_use_counts

    @property
    def syn_use_counts_matrix(self):
        return Pool.syn_use_counts_from_TPM(self.tap_matrix, self.height, self.width)

    @property
    def encoders(self):
        raise NotImplementedError("requested deprecated Pool.encoders. Pool.tap_list now has the same information")

    def get_num_dimensions(self):
        return self.dimensions

    def get_num_neurons(self):
        return self.n_neurons

    def create_intrinsic_resources(self):
        # unlike other GraphObjects, pool has two intrinsic resources (that are always connected)
        self._append_resource("TATTapPoint", hwr.TATTapPoint(self.n_neurons, self.tap_list))
        self._append_resource("Neurons", hwr.Neurons(self.y, self.x, self.gain_divisors, self.biases, 
                              xy_loc=self.user_xy_loc, diffusor_cuts_yx=self.diffusor_cuts_yx))

        self._get_resource("TATTapPoint").connect(self._get_resource("Neurons"))

    def create_connection_resources(self):
        if (len(self.out_conns) > 0):
            conn, tgt = self._get_single_conn_out()
            tgt._connect_from(self, "Neurons", conn)

    def _connect_from(self, src, src_resource_key, conn):
        self._check_conn_from_type(src, ["Input", "Bucket"])
        src_resource = src._get_resource(src_resource_key)
        src_resource.connect(self._get_resource("TATTapPoint"))

    @property
    def mapped_xy(self):
        neurons = self._get_resource("Neurons")
        return neurons.x_loc, neurons.y_loc

    @property
    def mapped_yx(self):
        return self.mapped_xy[::-1]

    @property
    def width(self):
        return self.x

    @property
    def height(self):
        return self.y

    @property
    def is_mapped(self):
        re
