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
    """
    def __init__(self, label, tap_spec, x, y, gain_divisors=1, biases=0, user_xy_loc=(None, None)):
        super(Pool, self).__init__(label)
        self.label = label

        # create tap list from tap matrix, if necessary
        if isinstance(tap_spec, tuple):
            self.n_neurons, tap_list = tap_spec
            self.dimensions = len(tap_list)
            self.tap_list = tap_list
            self.tap_matrix = None
        else:
            self.n_neurons, self.dimensions = tap_spec.shape
            self.tap_list = Pool.tap_matrix_to_list(tap_spec)
            self.tap_matrix = tap_spec

        self.x = x
        self.y = y
        self.user_xy_loc = user_xy_loc
        self.gain_divisors = gain_divisors
        self.biases = biases

        # if user supplied int for entire population, expand into array
        if isinstance(self.gain_divisors, int):
            self.gain_divisors = np.ones((self.n_neurons,), dtype='int') * self.gain_divisors
        if isinstance(self.biases, int):
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

    def __repr__(self):
        return "Pool " + self.label + ". dims in = " + str(self.dimensions) + ". neurons = " + str(self.n_neurons)

    @staticmethod
    def tap_matrix_to_list(M): 
        """Converts tap matrix to tap list
        Inputs:
        ======
        M, the tap matrix

        Returns:
        =======
        [[tap dim 0 list], ... , [tap dim D-1 list]] the equivalent tap list
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

    def get_num_dimensions(self):
        return self.dimensions

    def get_num_neurons(self):
        return self.n_neurons

    def create_intrinsic_resources(self):
        # unlike other GraphObjects, pool has two intrinsic resources (that are always connected)
        self._append_resource("TATTapPoint", hwr.TATTapPoint(self.n_neurons, self.tap_list))
        self._append_resource("Neurons", hwr.Neurons(self.y, self.x, self.gain_divisors, self.biases, self.user_xy_loc))

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
    def width(self):
        return self.x

    @property
    def height(self):
        return self.y


