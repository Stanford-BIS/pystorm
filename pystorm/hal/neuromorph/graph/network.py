import numpy as np
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

    def create_pool(self, label, n_neurons, dimensions):
        x, y = self._flat_to_rectangle(n_neurons)
        p = pool.Pool(label, n_neurons, dimensions, x, y)
        self.pools.append(p)
        return p

    def create_input(self, label, dimensions):
        i = input.Input(label, dimensions)
        self.inputs.append(i)
        return i

    def create_connection(self, label, src, dest, weights):
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
