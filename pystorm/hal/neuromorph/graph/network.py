import numpy as np
from numbers import Number
from . import bucket
from . import pool
from . import input
from . import output
from . import connection

import pystorm.hal.neuromorph.hardware_resources as hwr
import pystorm.hal.neuromorph.core as core
n_core = core

#new dependancies for multi-core mapping
import metis
import networkx as nx

class IllegalMappingException(Exception): pass

class Network(object):
    def __init__(self, label):
        self.label = label
        self.buckets = []
        self.pools = []
        self.inputs = []
        self.outputs = []
        self.connections = []
        self.h_r_per_core = []
        self.num_cores = 1

        # core associated with this network
        self.cores = []

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

        for i in range(0, self.num_cores):
            if self.num_cores <= 1:
                hardware_resources = self.get_hardware_resources()
            else:
                hardware_resources = self.h_r_per_core[i]
            core = self.cores[i]

            for resource in hardware_resources:
                resource.pretranslate_early(core)
            if verbose:
                print("finished pretranslate_early")

        for i in range(0, self.num_cores):
            if self.num_cores <= 1:
                hardware_resources = self.get_hardware_resources()
            else:
                hardware_resources = self.h_r_per_core[i]
            core = self.cores[i]

            for resource in hardware_resources:
                resource.pretranslate(core)
            if verbose:
                print("finished pretranslate")

        for i in range(0, self.num_cores):
            if self.num_cores <= 1:
                hardware_resources = self.get_hardware_resources()
            else:
                hardware_resources = self.h_r_per_core[i]
            core = self.cores[i]

            for resource in hardware_resources:
                resource.allocate_early(core)
            if verbose:
                print("finished allocate_early")

        for i in range(0, self.num_cores):
            if self.num_cores <= 1:
                hardware_resources = self.get_hardware_resources()
            else:
                hardware_resources = self.h_r_per_core[i]
            core = self.cores[i]

            core.MM.alloc.switch_to_trans()  # switch allocation mode of MM
            for resource in hardware_resources:
                resource.allocate(core)
            if verbose:
                print("finished allocate")

        for i in range(0, self.num_cores):
            if self.num_cores <= 1:
                hardware_resources = self.get_hardware_resources()
            else:
                hardware_resources = self.h_r_per_core[i]
            core = self.cores[i]

            if premapped_neuron_array is not None:
                assert(isinstance(premapped_neuron_array, core.NeuronArray))
                core.NeuronArray = premapped_neuron_array
                if verbose:
                    print("  replaced core.neuron_array with premapped_neuron_array")

        for i in range(0, self.num_cores):
            if self.num_cores <= 1:
                hardware_resources = self.get_hardware_resources()
            else:
                hardware_resources = self.h_r_per_core[i]
            core = self.cores[i]

            for resource in hardware_resources:
                resource.posttranslate_early(core)
            if verbose:
                print("finished posttranslate_early")

        for i in range(0, self.num_cores):
            if self.num_cores <= 1:
                hardware_resources = self.get_hardware_resources()
            else:
                hardware_resources = self.h_r_per_core[i]
            core = self.cores[i]

            for resource in hardware_resources:
                resource.posttranslate(core)
            if verbose:
                print("finished posttranslate")

        for i in range(0, self.num_cores):
            if self.num_cores <= 1:
                hardware_resources = self.get_hardware_resources()
            else:
                hardware_resources = self.h_r_per_core[i]
            core = self.cores[i]

            for resource in hardware_resources:
                resource.assign(core)
            if verbose:
                print("finished assign")

        fname = "mapped_core.txt"
        np.set_printoptions(threshold=np.nan)
        print("mapping results written to", fname)
        f = open(fname, "w")

        for i in range(0, self.num_cores):
            core = self.cores[i]

            f.write("\n \n --------------- \n \n" + str(i) + "\n \n --------------- \n \n")
            f.write(str(core))

        f.close()

        # hardware_resources = self.get_hardware_resources()
        # for rec in hardware_resources:
        #     if rec.__class__.__name__ == "TATFanout":
        #         print(rec)
        #         for route in rec.global_routes:
        #             if route < 128: print(route)
        #             else: print(route-256)


    def create_output_translators(self):
        self.spike_filter_idx_to_output = {}
        for i in range(0, self.num_cores):
            self.spike_filter_idx_to_output[i] = {}

        for output in self.get_outputs():
            for dim_idx, filt_idx in enumerate(output.filter_idxs):
                self.spike_filter_idx_to_output[output._get_resource("Sink").core_id][filt_idx] = (output, dim_idx)

        self.spk_to_pool_nrn_idx = {}
        for i in range(0, self.num_cores):
            self.spk_to_pool_nrn_idx[i] = {}

        for pool in self.get_pools():
            xmin, ymin = pool.mapped_xy
            for y in range(pool.height):
                for x in range(pool.width):
                    pool_core = pool._get_resource("Neurons").core_id
                    spk_idx = (ymin + y) * self.cores[pool_core].NeuronArray_width + xmin + x 
                    pool_nrn_idx = y * pool.width + x
                    self.spk_to_pool_nrn_idx[pool_core][spk_idx] = (pool, pool_nrn_idx)

    def translate_spikes(self, core_ids, spk_ids, spk_times):
        pool_ids = []
        nrn_idxs = []
        filtered_spk_times = []
        for core_id, spk_id, spk_time in zip(core_ids, spk_ids, spk_times):
            if spk_id not in self.spk_to_pool_nrn_idx[core_id]:
                print("WARNING: translate_spikes: got out-of-bounds spike from neuron id (probably sticky bits)", spk_id)
            else:
                pool_id, nrn_idx = self.spk_to_pool_nrn_idx[core_id][spk_id]
                pool_ids.append(pool_id)
                nrn_idxs.append(nrn_idx)
                filtered_spk_times.append(spk_time)
        return pool_ids, nrn_idxs, filtered_spk_times

    def translate_tags(self, core_ids, filt_idxs, filt_states):
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

        outputs = [self.spike_filter_idx_to_output[core_id][filt_idx][0] for core_id, filt_idx in zip(core_ids, filt_idxs)]
        dims = [self.spike_filter_idx_to_output[core_id][filt_idx][1] for core_id, filt_idx in zip(core_ids, filt_idxs)]

        return outputs, dims, counts

    def map(self, core_parameters, keep_pool_mapping=False, verbose=False, num_cores=1):
        """Create Resources and map them to a Core

        Parameters
        ----------
        core_parameters: parameters of the Core to be created

        Returns
        -------
        core: array of ready-to-program core objects
        """

        # preserve old pool mapping, if necessary (how to keep this?)
        # if keep_pool_mapping:
        #     if self.cores is not None:
        #         premapped_neuron_array = self.cores.neuron_array
        #     else:
        #         print("  WARNING: keep_pool_mapping=True used before map_network() was called at least once. Ignoring.")
        #         premapped_neuron_array = None
        # else:
        #     premapped_neuron_array = None
        self.num_cores = num_cores
        premapped_neuron_array = None

        # create new core
        self.cores = [0]*num_cores
        for i in range(0, num_cores):
            self.cores[i] = core.Core(core_parameters)

        # create resources
        self.reinit_hardware_resources()
        self.create_hardware_resources()

        #parition network
        if num_cores > 1:
            self.partition_network(num_cores, verbose)
        else:
            for rec in self.get_hardware_resources():
                rec.core_id = 0

        # map resources to core
        self.map_resources_to_core(premapped_neuron_array, verbose)

        # create output translators
        self.create_output_translators()

        #test for correctness
        if num_cores > 1:
            self.sanity_check()

        return self.cores

    #sanity check for the multi-core partitioning
    def sanity_check(self):
        #check 1:
        #the only connections from core to core should be TATFanouts
        hardware_resources = self.get_hardware_resources()
        for rec in hardware_resources:
            if rec.__class__.__name__ != "TATFanout":
                for conn in rec.conns_out:
                    if conn.tgt.core_id != rec.core_id:
                        raise IllegalMappingException("Non-fanout HWResource " + str(rec) + " on core " + str(rec.core_id) +
                            " connects to off core HWResource " + str(conn.tgt) + " on core " + str(conn.tgt.core_id))
        return 0

    def partition_network(self, num_cores, verbose = False):

        #sort into stuff that has to go together
        graph_objects = self.get_hardware_resources()
        graph_tuples = []
        index_dict = {} #i know this is jank but whatever
        for i in range(0, len(graph_objects)):
            #-1 is unsorted, 0+ is sorted
            graph_tuples.append([graph_objects[i], -1]) 
            index_dict[graph_objects[i]] = i

        index = 0
        for i in range(0, len(graph_objects)): #recursively divide the graph into groups
            if (graph_tuples[i][1] == -1):
                graph_tuples = self.group_network(graph_tuples, [i], index, index_dict)
                index += 1

        # for i in range(0,index):
        #     for j in range(0, len(graph_tuples)):
        #         if(graph_tuples[j][1] == i):
        #             print(graph_tuples[j])
        #     print("\n")

        #get number of groups
        nodes = 0
        for i in range(0, len(graph_objects)):
            if graph_tuples[i][1] > nodes: nodes = graph_tuples[i][1]
        nodes += 1

        #define connections between the groups
        adjlist = self.get_connections_and_weights(graph_tuples, nodes, index_dict)
        #appoximate load-per-node using number of neurons (for load balancing)
        nodew = self.get_load_weights(graph_tuples, nodes, index_dict)
        #convert to metis
        metis_graph = metis.adjlist_to_metis(adjlist, nodew)
        neuron_constraints = [1/num_cores] * num_cores #assuming all cores have the same weights

        #call metis
        print("partitioning graph across " + str(num_cores) + " cores")
        part_tuple = metis.part_graph(graph=metis_graph, nparts=num_cores, tpwgts=neuron_constraints, recursive=False, objtype='vol')
        core_assignments = part_tuple[1];
        #temporarily force split
        # core_assignments = [0, 1]

        #split resources up based on core
        h_r_per_core = self.make_sub_resources(graph_tuples, core_assignments, num_cores)
        core_index_dict = {} #i know this is still bad practice, fight me
        for i in range(0, len(h_r_per_core)):
            for resource in h_r_per_core[i]: 
                core_index_dict[resource] = i;

        h_r_per_core = self.slice_and_glue(h_r_per_core, core_index_dict)

        for core in range(0, len(h_r_per_core)):
            for rec in h_r_per_core[core]:
                rec.core_id = core

        # self.print_network(core_index_dict)

        self.h_r_per_core = h_r_per_core

        # print(adjlist)
        # print(nodew)
        # print(part_tuple)
        if(verbose):
            print("Cores:")
            for i in range(0,len(h_r_per_core)):
                print("Core " + str(i))
                for j in range(0, len(h_r_per_core[i])):
                    print(h_r_per_core[i][j])
            print("\n")
            print("Core Assignments:")
            print(core_assignments)


    def print_network(self, core_index_dict):
        index_dict = {} #i know this is jank but whatever
        network = self.get_hardware_resources()
        for i in range(0, len(network)):
            index_dict[network[i]] = i
        start = 0
        for i in range(0,len(network)):
            if network[i].__class__.__name__ == "Source":
                start = i 
        color = " \x1b[6;30;4" + str(core_index_dict[network[start]]+1) + "m"
        self.print_index(start, index_dict, color + network[start].__class__.__name__+ " "+str(hex(id(network[start])))+ "\x1b[0m", core_index_dict)


    def print_index(self, i, index_dict, string, core_index_dict):
        network = self.get_hardware_resources()
        conns_out = network[i].conns_out
        if len(conns_out) == 0:
            print(string)
            return
        for conn in conns_out:
            if(network[index_dict[conn.tgt]] in core_index_dict):
                color = " \x1b[6;30;4" + str(core_index_dict[network[index_dict[conn.tgt]]]+1) + "m"
                self.print_index(index_dict[conn.tgt], index_dict, 
                    string + color + network[index_dict[conn.tgt]].__class__.__name__ + " "+str(hex(id(network[index_dict[conn.tgt]])))+ "\x1b[0m",
                    core_index_dict)
            else: 
                self.print_index(index_dict[conn.tgt], index_dict, 
                    string + " " + network[index_dict[conn.tgt]].__class__.__name__+ " "+str(hex(id(network[index_dict[conn.tgt]]))),
                    core_index_dict)

    def slice_and_glue(self, h_r_per_core, core_index_dict):
        #make a dictionary to look up where to add the new fanouts
        graph_objects = self.get_GraphObjects()
        graph_dict = {}
        for graph_object in graph_objects:
            for k, v in graph_object.resources.items():
                graph_dict[v] = graph_object
        #find and add the new fanouts
        new_h_r_per_core = h_r_per_core
        for i in range(0, len(h_r_per_core)):
            for resource in h_r_per_core[i]: 
                temp_fanout = None
                if resource.__class__.__name__ == "AMBuckets":
                    out_conns = resource.conns_out
                    for conn in out_conns:
                        if core_index_dict[conn.tgt] != i: #off-core connection
                            resource.disconnect(conn.tgt, conn)
                            if temp_fanout == None:
                                temp_fanout = hwr.TATFanout(resource.dimensions_out)
                                resource.connect(temp_fanout)
                            temp_fanout.connect(conn.tgt)
                            del conn
                    if temp_fanout != None:
                        graph_dict[resource]._append_resource("TATFanout", temp_fanout)
                        new_h_r_per_core[i].append(temp_fanout)
        return new_h_r_per_core

    def make_sub_resources(self, graph_tuples, core_assignments, num_cores):
        h_r_per_core = [[] for _ in range(num_cores)]
        for graph_tuple in graph_tuples:
            h_r_per_core[core_assignments[graph_tuple[1]]].append(graph_tuple[0])
        return h_r_per_core

    def get_load_weights(self, graph_tuples, nodes, index_dict):
        weights = [0]*nodes #weight[node] = weight
        for i in range(0, nodes):
            for j in range(0, len(graph_tuples)):
                #count neurons in pools
                curr_obj = graph_tuples[j][0]
                if graph_tuples[j][1] == i and curr_obj.__class__.__name__ == "Neurons":
                    #assuming neurons dominate internal load
                    weight = curr_obj.N #num neurons
                    weights[i] += weight
        return weights

    def get_connections_and_weights(self, graph_tuples, nodes, index_dict):
        #weight of src->dest = weights[src][dest] ***zero indexed now***
        weights = [[0 for _ in range(nodes)] for _ in range(nodes)]

        for i in range(0, nodes):
            for j in range(0, len(graph_tuples)):
                #we can only cut after buckets or fanouts
                curr_obj = graph_tuples[j][0]
                if graph_tuples[j][1] == i and curr_obj.__class__.__name__ == "AMBuckets":
                    #assuming traffic is linearly dependant on number of dimensions
                    out_weight = curr_obj.dimensions_out
                    out_conns = curr_obj.conns_out
                    for conn in out_conns:
                        k = index_dict[conn.tgt]
                        if graph_tuples[k][1] != i:
                            weights[i][graph_tuples[k][1]] += out_weight;
                elif graph_tuples[j][1] == i and curr_obj.__class__.__name__ == "TATFanout":
                    #assuming traffic is linearly dependant on number of dimensions
                    out_weight = curr_obj.dimensions_in
                    out_conns = curr_obj.conns_out
                    for conn in out_conns:
                        k = index_dict[conn.tgt]
                        if graph_tuples[k][1] != i:
                            weights[i][graph_tuples[k][1]] += out_weight;

        #convert to tuples
        weight_tuples = []
        for i in range(0, nodes):
            temp_list = []
            for j in range(0, nodes):
                if weights[i][j] != 0:
                    temp_list.append((j, weights[i][j]))
            weight_tuples.append(tuple(temp_list))
        return weight_tuples

    def group_network(self, graph_tuples, to_check, index, index_dict):
        to_check_next = []
        for i in to_check:
            graph_tuples[i][1] = index
            to_check_next = [] #this depends on the object type
            curr_obj = graph_tuples[i][0]

            #for a bucket, add any non-bucket that feeds it and any fanout it feeds
            if curr_obj.__class__.__name__ == "AMBuckets":
                in_conns = curr_obj.conns_in
                for conn in in_conns:
                    if conn.src.__class__.__name__ != "AMBuckets":
                        j = index_dict[conn.src]
                        if graph_tuples[j][1] != index:
                            to_check_next.append(index_dict[conn.src])
                out_conns = curr_obj.conns_out
                for conn in out_conns:
                    if conn.tgt.__class__.__name__ == "TATFanout":
                        j = index_dict[conn.tgt]
                        if graph_tuples[j][1] != index:
                            to_check_next.append(index_dict[conn.tgt])

            #for a TATFanout, pick up any buckets behind you and any sinks in front of you
            elif curr_obj.__class__.__name__ == "TATFanout":
                in_conns = curr_obj.conns_in
                for conn in in_conns:
                    if conn.src.__class__.__name__ == "AMBuckets":
                        j = index_dict[conn.src]
                        if graph_tuples[j][1] != index:
                            to_check_next.append(index_dict[conn.src])
                out_conns = curr_obj.conns_out
                for conn in out_conns:
                    if conn.tgt.__class__.__name__ == "Sink":
                        j = index_dict[conn.tgt]
                        if graph_tuples[j][1] != index:
                            to_check_next.append(index_dict[conn.tgt])

            #for anything else, add anything it feeds and anything feeding it but a bucket or a fanout
            elif curr_obj.__class__.__name__ != "ResourceConnection":
                out_conns = curr_obj.conns_out
                for conn in out_conns:
                    j = index_dict[conn.tgt]
                    if graph_tuples[j][1] != index:
                        to_check_next.append(index_dict[conn.tgt])
                in_conns = curr_obj.conns_in
                for conn in in_conns:
                    if conn.src.__class__.__name__ != "AMBuckets" and conn.src.__class__.__name__ != "TATFanout":
                        j = index_dict[conn.src]
                        if graph_tuples[j][1] != index:
                            to_check_next.append(index_dict[conn.src])

        if len(to_check_next) == 0:
            return graph_tuples
        return self.group_network(graph_tuples, to_check_next, index, index_dict)



