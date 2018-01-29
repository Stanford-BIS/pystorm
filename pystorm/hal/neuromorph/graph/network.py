import numpy as np
from numbers import Number
from . import bucket
from . import pool
from . import input
from . import output
from . import connection

class Network(object):
    def __init__(self, label):
        self.label = label
        self.buckets = []
        self.pools = []
        self.inputs = []
        self.outputs = []
        self.connections = []

    def get_label(self):
        return label

    def get_buckets(self):
        return self.buckets

    def get_pools(self):
        return self.pools

    def get_inputs(self):
        return self.inputs

    def get_outputs(self):
        return self.outputs

    def get_connections(self):
        return self.connections

    @staticmethod
    def _flat_to_rectangle(n_neurons):
        """find the squarest rectangle to fit n_neurons
        
        Returns the x and y dimensions of the rectangle
        """
        assert isinstance(n_neurons, (int, np.integer))
        y = int(np.floor(np.sqrt(n_neurons)))
        while n_neurons % y != 0:
            y -= 1
        x = n_neurons // y
        assert x*y == n_neurons
        return x, y

    def create_pool(self, label, encoders, xy=None):
        """Adds a Pool object to the network.
        
        Parameters
        ----------
        label: string
            name of pool
        encoders:
            encoder matrix (pre-diffuser), size neurons-by-dimensions.
            Elements must be in {-1, 0, 1}.
            Implicitly describes pool dimensionality and number of neurons.
        xy: tuple: (int, int)
            user-specified x, y shape. x * y must match encoder shape
        """
        n_neurons, dimensions = encoders.shape

        # if xy
        if xy is None:
            x, y = self._flat_to_rectangle(n_neurons)
        else:
            x, y = xy

        p = pool.Pool(label, encoders, x, y)
        self.pools.append(p)
        return p

    def create_input(self, label, dimensions):
        i = input.Input(label, dimensions)
        self.inputs.append(i)
        return i

    def create_connection(self, label, src, dest, weights):

        if weights is not None and not isinstance(dest, bucket.Bucket):
            print("connection weights are only used when the destination node is a Bucket")
            raise NotImplementedError
        if isinstance(weights, Number):
            weights = np.array([[weights]])
        c = connection.Connection(label, src, dest, weights)
        self.connections.append(c)
        return c

    def create_bucket(self, label, dimensions):
        b = bucket.Bucket(label, dimensions)
        self.buckets.append(b)
        return b

    def create_output(self, label, dimensions):
        o = output.Output(label, dimensions)
        self.outputs.append(o)
        return o
