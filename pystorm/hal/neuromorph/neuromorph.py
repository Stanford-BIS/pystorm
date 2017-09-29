from pystorm._PyStorm import *
from .core import Core
from .resources import *
from functools import singledispatch, update_wrapper
import numpy as np

def map(pystorm_network):
    """Create a MappedNetwork object given a Pystorm.Network object

    :param pystorm_network: An object of type Pystorm.Network

    :return (Pystorm.Network, mapinfo): A tuple binding the
    Pystorm.Network to a list of structures representing what
    will be written to memory.
    """
    return MapController().map(pystorm_network)

def methdispatch(func):
    dispatcher = singledispatch(func)
    def wrapper(*args, **kw):
        return dispatcher.dispatch(args[1].__class__)(*args,**kw)
    wrapper.register = dispatcher.register
    update_wrapper(wrapper, func)
    return wrapper

class NetObjNode(object):
    def __init__(self, net_obj):
        """Construct the object updating the resource_map if necessary

        :param net_obj: A Pystorm.Network object (e.g. Input, Pool, Bucket, Output)

        """
        self.net_obj = net_obj
        self.resource = self.create_resources_for_(net_obj)
        self.adjacent_net_obj = []

    @methdispatch
    def create_resources_for_(self, obj):
        raise TypeError("This type isn't supported: {}".format(type(obj)))

    @create_resources_for_.register(Input)
    def _(self, inp):
        return Source(inp.GetNumDimensions())

    @create_resources_for_.register(Output)
    def _(output):
        return Sink(output.GetNumDimensions())

    @create_resources_for_.register(Pool)
    def _(pool):
        return Neurons(pool.GetNumNeurons())

    @create_resources_for_.register(Bucket)
    def _(bucket):
        return AMBuckets(bucket.GetNumDimensions())

    def add_adjacent_node(self, node, weight=None):
        self.adjacent_net_obj.append((weight, node))

    def get_net_obj(self):
        return self.net_obj

    def get_resource(self):
        return self.resource

    def create_mm_weights(self, pystorm_weights):
        weights = np.array(pystorm_weights.GetNumRows(),
                           pystorm_weights.GetNumColumns())

        for i in range(pystorm_weights.GetNumRows()):
            for j in range(pystorm_weights.GetNumColumns()):
                weights[i][j] = pystorm_weights.GetElement(i, j)

        ret_value = MMWeights(weights)

        return ret_value

    def connect(self, resources):
        """Connect the NetObjNode to it's adjacent nodes

            Update the resources list parameter, if necessary.

        :param resources: A list of Resource instances
        :return:

        What type of Pystorm.Network object connections can we expect and
        what Resource objects (and connections) must we create given the
        Pystorm.Network objects?

        The following information describes the Pystorm.Network objects
        followed by their associated Resources objects.
        Note that the first five connections are from one object to one
        object, however, connections 6 and 7 are from one object to multiple
        objects which require a different set of resources.

          Conn 1: Input -> Pool

                      Source -> TATTapPoint -> Neurons

          Conn 2: Input -> Bucket

              Source -> TATAcuumulator -> MMWeights -> AMBuckets

          Conn 3: Pool -> Bucket

             Neurons -> MMWeights -> AMBuckets

          Conn 4: Bucket -> Bucket

              AMBuckets -> MMWeights -> AMBuckets

          Conn 5: Bucket -> Output

             AMBuckets -> Sink

         Conn 6: Bucket -> Bucket
                        -> Bucket

             AMBuckets -> TATFanout -> MMWeights -> AMBuckets
                                    -> MMWeights -> AMBuckets

         Conn 7: Bucket -> Output
                        -> Output

             AMBuckets -> TATFanout -> Sink
                                    -> Sink

        """
        number_of_adj_nodes = len(self.adjacent_net_obj)

        if number_of_adj_nodes == 0:

            return

        elif number_of_adj_nodes == 1:

            adj_node = self.adjacent_net_obj[0]
            adj_node_net_obj = adj_node.second
            adj_node_weights = adj_node.first

            # Conn 1: Input -> Pool
            if type(self.net_obj) == Input and type(adj_node_net_obj) == Pool:
                num_neurons = self.net_obj.GetNumNeurons()
                matrix_dims = 2
                adj_node_dims = adj_node_net_obj.GetNumDimensions()

                source = self.resource
                tap = TATTapPoint(np.random.randint(num_neurons,
                                  size=(matrix_dims, adj_node_dims)),
                                  np.random.randint(1, size=(matrix_dims,
                                                    adj_node_dims)) * 2 - 1,
                                  num_neurons)
                neurons = adj_node_net_obj.resource

                source.Connect(tap)
                tap.Connect(neurons)

                resources.append(tap)

            #   Conn 2: Input -> Bucket
            if type(self.net_obj) == Input and type(adj_node_net_obj) == Bucket:
                source = self.resource
                tat_acc_resource = TATAccumulator(self.net_obj.GetNumDimensions())
                weight_resource = self.create_mm_weights(adj_node_weights)
                am_bucket = adj_node_net_obj.resource

                source.Connect(tat_acc_resource)
                tat_acc_resource.Connect(weight_resource)
                weight_resource.Connect(am_bucket)

                resources.append(tat_acc_resource)
                resources.append(weight_resource)

            #   Conn 3: Pool -> Bucket
            if type(self.net_obj) == Pool and type(adj_node_net_obj) == Bucket:
                neurons = self.resource
                weight_resource = self.create_mm_weights(adj_node_weights)
                am_bucket = adj_node_net_obj.resource

                neurons.Connect(weight_resource)
                weight_resource.Connect(am_bucket)

                resources.append(weight_resource)

            #   Conn 4: Bucket -> Bucket
            if type(self.net_obj) == Bucket and type(adj_node_net_obj) == Bucket:
                am_bucket_in = self.resource
                weight_resource = self.create_mm_weights(adj_node_weights)
                am_bucket_out = adj_node_net_obj.resource

                am_bucket_in.Connect(weight_resource)
                weight_resource.Connect(am_bucket_out)

                resources.append(weight_resource)

            #   Conn 5: Bucket -> Output
            if type(self.net_obj) == Bucket and type(adj_node_net_obj) == Output:
                am_bucket = self.resource
                sink = adj_node_net_obj.resource

                am_bucket.Connect(sink)

        elif number_of_adj_nodes > 1:

                if type(self.net_obj) == Bucket:
                    am_bucket_in = self.resource
                    tat_fanout = TATFanout(self.net_obj.GetNumDimensions())

                    am_bucket_in.Connect(tat_fanout)

                    resources.append(tat_fanout)

                    for adj_node in self.adjacent_net_obj:
                        adj_node_net_obj = adj_node.second
                        adj_node_weight = adj_node.first
            #   Conn 6: Bucket -> Bucket
            #                  -> Bucket
                        if type(adj_node_net_obj) == Bucket:
                            weight_resource = self.create_mm_weights(adj_node_weight)
                            am_bucket_out = adj_node_net_obj

                            resources.append(weight_resource)

                            tat_fanout.Connect(weight_resource)
                            weight_resource.Connect(am_bucket_out)

            #   Conn 7: Bucket -> Output
            #                  -> Output
                        if type(adj_node_net_obj) == Output:
                            sink = adj_node_net_obj.resource

                            tat_fanout.Connect(sink)

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

        resources = []
        # A map to Pystorm.Network objects to NetObjNodes
        # The map forms a graph of Pystorm.Network/Resource objects
        network_obj_map = dict()

        # Populate the NetObjNodes in the graph
        for inp in pystorm_network.GetInputs():
            network_obj_map[inp] = NetObjNode(inp)
            resources.append(network_obj_map[inp].get_resource())

        for out in pystorm_network.GetOutputs():
            network_obj_map[out] = NetObjNode(out)
            resources.append(network_obj_map[out].get_resource())

        for pool in pystorm_network.GetPools():
            network_obj_map[pool] = NetObjNode(pool)
            resources.append(network_obj_map[pool].get_resource())

        for bucket in pystorm_network.GetBuckets():
            network_obj_map[bucket] = NetObjNode(bucket)
            resources.append(network_obj_map[bucket].get_resource())

        # Connect the NetObjNodes to their adjacent NetObjNodes
        connections = pystorm_network.GetConnections()

        for conn in connections:
            source = conn.GetSource()
            dest = conn.GetDest()
            weights = conn.GetWeights()  # weights is either of type Weights or None

            network_obj_map[source].add_adjacent_node(network_obj_map[dest], weights)

        # Connect the adjacent nodes creating Resources relevant to each connection
        for net_obj, net_obj_node in network_obj_map.items():
            net_obj_node.connect(resources)

        return resources

    def create_pystorm_mem_objects(self, core):
        pass
        # instead of WriteMemsToFile, create a set of structures
        # supplied by Pystorm that represent what you are mapping
        # the original network to.

    def map_resources_to_core(self, resources, core, verbose=False):
        """

        :param resources: A list of Resource objects
        :param core: A Core object
        :param verbose: A bool
        :return core: The modified Core object
        """
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

        core = self.map_resources_to_core(resources, core, verbose)

        pystorm_mem = self.create_pystorm_mem_objects(core)

        mapped_network = (pystorm_network, pystorm_mem)

        return mapped_network
