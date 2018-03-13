from graph_object import GraphObject, ConnectionTypeError, FanoutError
import pystorm.hal.neuromorph.hardware_resources as hwr

import numpy as np

class Pool(GraphObject):
    """Represents a pool of neurons
    
    Parameters
    ----------
    label: string
        name of pool
    encoders:
        encoder matrix (pre-diffuser), size neurons-by-dimensions.
        Elements must be in {-1, 0, 1}.
        Implicitly describes pool dimensionality and number of neurons.
    x: int
        neuron pool is physically a rectangle; x dimension of neuron pool
    y: int
        neuron pool is physically a rectangle; y dimension of neuron pool
    """
    def __init__(self, label, encoders, x, y, gain_divisors=1, biases=0):
        super(GraphObject, self).__init__(label)
        self.label = label
        self.encoders = encoders

        if isinstance(encoders, tuple):
            self.n_neurons, tap_list = encoders
            self.dimensions = len(tap_list)
        else:
            self.n_neurons, self.dimensions = encoders.shape

        self.x = x
        self.y = y
        self.gain_divisors = gain_divisors
        self.biases = biases

        # if user supplied int for entire population, expand into array
        if isinstance(self.gain_divisors, int):
            self.gain_divisors = np.ones((self.n_neurons,), dtype='int') * self.gain_divisors
        if isinstance(self.biases, int):
            self.biases = np.ones((self.n_neurons,), dtype='int') * self.biases

        # check that it's an array now
        for obj in [self.gain_divisors, self.biases]:
            assert(isinstance(obj, np.ndarray) and "gain and bias parameters must be ints or numpy arrays of ints") 
            assert(obj.dtype == np.dtype('int64') or obj.dtype == np.dtype('int32') and "gain and bias parameters must be arrays of ints")
            assert(len(obj) == self.n_neurons and "gain and bias parameters must be arrays of length N")

        # allowed gains are 1x, 1/2x, 1/3x, 1/4x
        assert(np.all(self.gain_divisors >= 1) and np.all(self.gain_divisors <= 4))
        # allowed biases are -3, 2, 1, 0, 1, 2, 3
        assert(np.all(self.biases >= -3) and np.all(self.biases <= 3))

        assert(len(self.biases) == self.n_neurons)
        assert(self.n_neurons == x * y)

    def __repr__(self):
        return "Pool " + self.label

    def get_num_dimensions(self):
        return self.dimensions

    def get_num_neurons(self):
        return self.n_neurons

    def create_intrinsic_resources(self):
        # unlike other GraphObjects, pool has two intrinsic resources (that are always connected)
        self._append_resource("TATTapPoint", hwr.TATTapPoint(self.encoders))
        self._append_resource("Neurons", hwr.Neurons(self.y, self.x, self.gain_divisors, self.biases))

        self._get_resource("TATTapPoint").connect(self._get_resource("Neurons"))

    def create_connection_resources(self):
        """
        Conn 3:
              neuromorph graph: Pool ─> Bucket
            hardware_resources: Neurons ─> MMWeights ─> AMBuckets
        """
        conn, tgt = self.get_single_conn_out()

        #   Conn 3: Pool -> Bucket
        if isinstance(tgt, Bucket):
    
            neurons = self._get_resource("Neurons")
            weights = hwr.MMWeights(conn.weights) # create weights
            bucket = tgt._get_resource("AMBuckets")

            # make connections
            neurons.connect(weights)
            weights.connect(bucket)

            # append to resources
            self._append_resource(("MMWeights", tgt), weights)

        else:
            raise ConnectionTypeError(self, tgt)

    def get_mapped_xy(self):
        neurons = self._get_resource("Neurons")
        return neurons.x_loc, neurons.y_loc

    def get_width(self):
        return self.x

    def get_height(self):
        return self.y


