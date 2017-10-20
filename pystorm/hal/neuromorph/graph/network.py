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

    def create_pool(self, label, n_neurons, dimensions):
        p = pool.Pool(label, n_neurons, dimensions)
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

def create_network(label):
    return Network(label)
