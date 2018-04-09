import numpy as np
from numbers import Number
from . import bucket
from . import pool
from . import input
from . import output
from . import connection

import pystorm.hal.neuromorph.core as core

class Network(object):
    def __init__(self, label):
        self.label = label
        self.buckets = []
        self.pools = []
        self.inputs = []
        self.outputs = []
        self.connections = []

        # core associated with this network
        self.core = None

        # mapping tables for interpreting outputs from hardware
        self.spike_filter_idx_to_output = {}
        self.spk_to_pool_nrn_idx = {}

    def __gt__(self, network2):
        return self.label > network2.label

    def __repr__(self):
        return "Network " + self.label

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

    def get_GraphObjects(self):
        return self.inputs + self.pools + self.buckets + self.outputs


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

    def create_pool(self, label, encoders, gain_divisors=1, biases=0, xy=None):
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
        if isinstance(encoders, tuple):
            n_neurons, tap_list = encoders
            dimensions = len(tap_list)
        else:
            n_neurons, dimensions = encoders.shape

        # if xy
        if xy is None:
            x, y = self._flat_to_rectangle(n_neurons)
        else:
            x, y = xy

        p = pool.Pool(label, encoders, x, y, gain_divisors, biases)
        self.pools.append(p)
        return p

    def create_input(self, label, dimensions):
        i = input.Input(label, dimensions)
        self.inputs.append(i)
        return i

    def create_connection(self, label, src, dest, weights):
        """Add a connection to the network"""
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

    # mapping functions

    def get_hardware_resources(self):
        all_resources = []
        for obj in self.get_GraphObjects():
            for k, v in obj.resources.items():
                all_resources.append(v)
        return all_resources

    def reinit_hardware_resources(self):
        for obj in self.get_GraphObjects():
            obj.reinit_resources()

    def create_hardware_resources(self):
        """Create the required hardware resources for this network
        """
        graph_objs = self.get_GraphObjects()

        # create instrinsic resources
        for obj in graph_objs:
            obj.create_intrinsic_resources()

        # create connection resources
        for obj in graph_objs:
            obj.create_connection_resources()

    def map_resources_to_core(self, premapped_neuron_array=None, verbose=False):
        """Annotate a Core object with hardware_resources.Resource objects

        Parameters
        ----------
        core: the Core object to map the network onto
        premapped_neuron_array: a NeuronArray object, supplying the result
          of a prior mapping will preserve Pool mapped locations
        verbose: A bool
        """

        hardware_resources = self.get_hardware_resources()
        core = self.core

        for resource in hardware_resources:
            resource.pretranslate_early(core)
        if verbose:
            print("finished pretranslate_early")

        for resource in hardware_resources:
            resource.pretranslate(core)
        if verbose:
            print("finished pretranslate")

        for resource in hardware_resources:
            resource.allocate_early(core)
        if verbose:
            print("finished allocate_early")

        core.MM.alloc.switch_to_trans()  # switch allocation mode of MM
        for resource in hardware_resources:
            resource.allocate(core)
        if verbose:
            print("finished allocate")

        if premapped_neuron_array is not None:
            assert(isinstance(premapped_neuron_array, core.NeuronArray))
            core.NeuronArray = premapped_neuron_array
            if verbose:
                print("  replaced core.neuron_array with premapped_neuron_array")

        for resource in hardware_resources:
            resource.posttranslate_early(core)
        if verbose:
            print("finished posttranslate_early")

        for resource in hardware_resources:
            resource.posttranslate(core)
        if verbose:
            print("finished posttranslate")

        for resource in hardware_resources:
            resource.assign(core)
        if verbose:
            print("finished assign")

        fname = "mapped_core.txt"
        np.set_printoptions(threshold=np.nan)
        print("mapping results written to", fname)
        f = open(fname, "w")
        f.write(str(core))
        f.close()

    def create_output_translators(self):
        self.spike_filter_idx_to_output = {}
        for output in self.get_outputs():
            for dim_idx, filt_idx in enumerate(output.filter_idxs):
                self.spike_filter_idx_to_output[filt_idx] = (output, dim_idx)

        self.spk_to_pool_nrn_idx = {}
        for pool in self.get_pools():
            xmin, ymin = pool.mapped_xy
            for y in range(pool.height):
                for x in range(pool.width):
                    spk_idx = (ymin + y) * self.core.NeuronArray_width + xmin + x 
                    pool_nrn_idx = y * pool.width + x
                    self.spk_to_pool_nrn_idx[spk_idx] = (pool, pool_nrn_idx)

    def translate_spikes(self, spk_ids, spk_times):
        pool_ids = []
        nrn_idxs = []
        filtered_spk_times = []
        for spk_id, spk_time in zip(spk_ids, spk_times):
            if spk_id not in self.spk_to_pool_nrn_idx:
                print("WARNING: translate_spikes: got out-of-bounds spike from neuron id (probably sticky bits)", spk_id)
            else:
                pool_id, nrn_idx = self.spk_to_pool_nrn_idx[spk_id]
                pool_ids.append(pool_id)
                nrn_idxs.append(nrn_idx)
                filtered_spk_times.append(spk_time)
        return pool_ids, nrn_idxs, filtered_spk_times

    def translate_tags(self, filt_idxs, filt_states):
        # for now, working in count mode
        # the filter state is a count, and it is signed
        counts = []
        for f in filt_states:
            if f > 2**26 - 1:
                to_append = f - 2**27
            else:
                to_append = f

            if abs(to_append) < 1000:
                counts.append(to_append)
            else:
                print("WARNING: discarding absurdly large tag filter value (probably sticky bits, or abuse of tag filter)")
                counts.append(0)

        outputs = [self.spike_filter_idx_to_output[filt_idx][0] for filt_idx in filt_idxs]
        dims = [self.spike_filter_idx_to_output[filt_idx][1] for filt_idx in filt_idxs]

        return outputs, dims, counts

    def map(self, core_parameters, keep_pool_mapping=False, verbose=False):
        """Create Resources and map them to a Core

        Parameters
        ----------
        core_parameters: parameters of the Core to be created

        Returns
        -------
        core: ready-to-program core object
        """

        # preserve old pool mapping, if necessary
        if keep_pool_mapping:
            if self.core is not None:
                premapped_neuron_array = self.core.neuron_array
            else:
                print("  WARNING: keep_pool_mapping=True used before map_network() was called at least once. Ignoring.")
                premapped_neuron_array = None
        else:
            premapped_neuron_array = None

        # create new core
        self.core = core.Core(core_parameters)

        # create resources
        self.reinit_hardware_resources()
        self.create_hardware_resources()

        # map resources to core
        self.map_resources_to_core(premapped_neuron_array, verbose)

        # create output translators
        self.create_output_translators()

        return self.core


