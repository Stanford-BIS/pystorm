from Pystorm import *
from . Core import *
from . Resources import *
from functools import singledispatch
import numpy as np


@singledispatch
def create_resources_for_(obj):
    raise TypeError("This type isn't supported: {}".format(type(obj)))


@create_resources_for_(Input)
def _(inp):
    return Source(inp.GetNumDimensions())


@create_resources_for_(Output)
def _(output):
    return Sink(output.GetNumDimensions())


@create_resources_for_(Pool)
def _(pool):
    return Neurons(pool.GetNumNeurons())


@create_resources_for_(Bucket)
def _(bucket):
    return AMBuckets(bucket.GetNumDimensions())


class NetObjNode(object):
    def __init__(self, net_obj, resource_map):
        """Construct the object updating the resource_map if necessary

        :param net_obj:
        :param resource_map:
        """
        self.net_obj = net_obj
        self.resource = create_resources_for_(net_obj)
        self.adjacent_net_obj = []

        resource_map[type(self.resource)].append(self.resource)

    def add_adjacent_node(self, node, weight=None):
        self.adjacent_net_obj.append((weight, node))

    def connect(self, resource_map):
        """Connect the NetObjNode to it's adjacent nodes

            Update the resource map parameter, if necessary.

        :param resource_map: A map of Resource types to Resource instances
        :return:
        """
        number_of_adj_nodes = len(self.adjacent_net_obj)

        if number_of_adj_nodes == 0:
            return

        for adj_node in self.adjacent_net_obj:
            adj_node_net_obj = adj_node.second
            adj_node_weight = adj_node.first

            # What type of Pystorm.Network object connections can we expect and
            # what Resource objects (and connections) must we create given the
            # Pystorm.Network objects?
            #
            # The following information describes the Pystorm.Network objects
            # followed by their associated Resources objects.
            #
            #   Conn 1: Input -> Pool
            #
            #               Source -> TATTapPoint -> Neurons
            #
            #   Conn 2: Input -> Bucket
            #
            #       Source -> TATAcuumulator -> MMWeights -> AMBuckets
            #
            #   Conn 3: Pool -> Bucket
            #
            #      Neurons -> MMWeights -> AMBuckets
            #
            #   Conn 4: Bucket -> Bucket
            #
            #       AMBuckets -> MMWeights -> AMBuckets
            #
            #   Conn 5: Bucket -> Bucket
            #                  -> Bucket
            #
            #       AMBuckets -> TATFanout -> MMWeights -> AMBuckets
            #                              -> MMWeights -> AMBuckets
            #
            #   Conn 6: Bucket -> Output
            #
            #       AMBuckets -> Sink
            #
            #   Conn 7: Bucket -> Output
            #                  -> Output
            #
            #       AMBuckets -> TATFanout -> Sink
            #                              -> Sink

            # Conn 1: Input -> Pool
            if type(self.net_obj) == Input and type(adj_node_net_obj) == Pool:
                num_neurons = self.net_obj.GetNumNeurons()
                matrix_dims = 2
                adj_node_dims = adj_node_net_obj.GetNumDimensions()

                tap = TATTapPoint(np.random.randint(num_neurons, size=(matrix_dims, adj_node_dims)),
                                  np.random.randint(1, size=(matrix_dims, adj_node_dims)) * 2 - 1,
                                  num_neurons)

                self.resource.Connect(tap)
                tap.Connect(adj_node_net_obj.resource)

                resource_map[TATTapPoint].append(tap)

            #   Conn 2: Input -> Bucket
            if type(self.net_obj) == Pool and type(adj_node_net_obj) == Bucket:
                pass

            #   Conn 3: Pool -> Bucket
            if type(self.net_obj) == Pool and type(adj_node_net_obj) == Bucket:
                weight_resource = MMWeights(np.zeros(self.net_obj.GetNumNeurons(),
                                            adj_node_net_obj.GetNumDimensions()))

                self.resource.Connect(weight_resource)
                weight_resource.Connect(adj_node_net_obj.resource)

                resource_map[MMWeights].append(weight_resource)

            #   Conn 4: Bucket -> Bucket
            if type(self.net_obj) == Pool and type(adj_node_net_obj) == Bucket:
                pass

            #   Conn 5: Bucket -> Bucket
            #                  -> Bucket
            if type(self.net_obj) == Pool and type(adj_node_net_obj) == Bucket:
                pass

            #   Conn 6: Bucket -> Output
            if type(self.net_obj) == Pool and type(adj_node_net_obj) == Bucket:
                pass

            #   Conn 7: Bucket -> Output
            #                  -> Output
            if type(self.net_obj) == Pool and type(adj_node_net_obj) == Bucket:
                pass


class MapController(object):
    def __init__(self):
        pass

    def create_resources(self, pystorm_network):
        """Given a Pystorm network, return a list of required Resource objects.

        :param pystorm_network: A Pystorm.Network object

        :return resources: A list of Resource objects

        .. note::
            The objects in the Pystorm network may not form a graph

            If a Pystorm network object is connected to another Pystorm
            network object, the corresponding Resource objects should
            be connected as well.

        """

        resource_map = dict()
        resources = []
        # Key: Pystorm.Network object, Value: NetObj
        network_obj_map = dict()

        # Populate the nodes in the network graph taking only
        # the Inputs, Outputs, Pools and Buckets
        for inp in pystorm_network.GetInputs():
            network_obj_map[inp] = NetObjNode(inp, resource_map)

        for out in pystorm_network.GetOutputs():
            network_obj_map[out] = NetObjNode(out, resource_map)

        for pool in pystorm_network.GetPools():
            network_obj_map[pool] = NetObjNode(pool, resource_map)

        for bucket in pystorm_network.GetBuckets():
            network_obj_map[bucket] = NetObjNode(bucket, resource_map)

        connections = pystorm_network.GetConnections()

        # Populate the adjacent nodes in the graph
        for conn in connections:
            source = conn.GetSource()
            dest = conn.GetDest()
            weights = conn.GetWeights()  # weights is either of type Weights or None

            network_obj_map[source].add_adjacent_node(network_obj_map[dest], weights)

        # Connect the nodes creating the resources relevant to the connections in the graph
        for node in network_obj_map:
            node.connect(resource_map)

        for resource in resource_map:
            resources.append(resource)

        return resources

    def create_pystorm_mem_objects(self, core):
        pass
        # instead of WriteMemsToFile, create a set of structures
        # supplied by Pystorm that represent what you are mapping
        # the original network to.

    def map_resource_objects_to_core(self, core, resources, verbose=False):
        for node in resources:
            node.PreTranslate(core)
        if verbose:
            print("finished PreTranslate")

        for node in resources:
            node.AllocateEarly(core)
        if verbose:
            print("finished AllocateEarly")
        
        core.MM.alloc.SwitchToTrans()  # switch allocation mode of MM
        for node in resources:
            node.Allocate(core)
        if verbose:
            print("finished Allocate")

        for node in resources:
            node.PostTranslate(core)
        if verbose:
            print("finished PostTranslate")

        for node in resources:
            node.Assign(core)
        if verbose:
            print("finished Assign")

        return core

    def map(self, pystorm_network, verbose=False):
        pars = ps.GetCorePars()

        core = Core(pars)

        resources = self.create_resources(pystorm_network)

        core = self.map_resource_objects_to_core(core, resources, verbose)

        pystorm_mem = self.create_pystorm_mem_objects(core)

        mapped_network = (pystorm_network, pystorm_mem)

        return mapped_network
