"""Provides the mapping functionality of the HAL"""
from functools import singledispatch, update_wrapper
import numpy as np
import pystorm._PyStorm as ps
from .core import Core
from .hardware_resources import (
    AMBuckets, MMWeights, Neurons, Sink, Source, TATAccumulator, TATTapPoint, TATFanout)

def instance_method_singledispatch(func):
    """Provides a singledispatch decorator for instance method

    singledispatch allows for modifying a function's behavior
    based on the first argument's type
    by registering one or more alterantive function implementations to be called (dispatched)
    based on the first argument type in place of the original function.

    Since instance functions always have self as the first argument,
    we modify the singledispatch to check the second argument type
    """
    dispatcher = singledispatch(func)
    def wrapper(*args, **kwargs):
        return dispatcher.dispatch(args[1].__class__)(*args, **kwargs)
    wrapper.register = dispatcher.register
    update_wrapper(wrapper, func)
    return wrapper

class PyStormObjectMapper(object):
    """Maps _PyStorm objects to hardware resources

    Requests allocations of hardware resources to implement
    a _PyStorm object and its connections on hardware

    Parameters
    ----------
    ps_obj: A _PyStorm object (i.e. Input, Pool, Bucket, or Output)

    Attributes
    ----------
    ps_obj: The _PyStorm object
    hardware_resource: The hardware resource associated with the _PyStorm object
    """
    @staticmethod
    @singledispatch
    def create_resources(pystorm_obj):
        """Creates the hardware resources for the _PyStorm object

        For example, for a _PyStorm Input, creates a hardware resource Source
        """
        raise TypeError("This type isn't supported: {}".format(type(pystorm_obj)))

    @staticmethod
    @create_resources.register(ps.Input)
    def _create_resources_ps_input(ps_input):
        """create_resources for _PyStorm.Input"""
        return Source(ps_input.GetNumDimensions())

    @staticmethod
    @create_resources.register(ps.Output)
    def _create_resources_ps_output(ps_output):
        """create_resources for _PyStorm.Output"""
        return Sink(ps_output.GetNumDimensions())

    @staticmethod
    @create_resources.register(ps.Pool)
    def _create_resources_ps_pool(ps_pool):
        """create_resources for _PyStorm.Pool"""
        return Neurons(ps_pool.GetNumNeurons())

    @staticmethod
    @create_resources.register(ps.Bucket)
    def _create_resources_ps_bucket(ps_bucket):
        """create_resources for _PyStorm.Bucket"""
        return AMBuckets(ps_bucket.GetNumDimensions())

    @staticmethod
    def create_mm_weights(ps_weights):
        """Create a representation of the main memory weights"""
        weights = np.array(ps_weights.GetNumRows(),
                           ps_weights.GetNumColumns())
        for i in range(ps_weights.GetNumRows()):
            for j in range(ps_weights.GetNumColumns()):
                weights[i][j] = ps_weights.GetElement(i, j)
        ret_value = MMWeights(weights)
        return ret_value

    def __init__(self, ps_obj):
        self.ps_obj = ps_obj
        self.hardware_resource = self.create_resources(ps_obj)
        self.dest_obj_mapper = []

    def connect_mapper(self, mapper, weight=None):
        """Set this source PyStormObjectMapper to connect to a destination PyStormObjectMapper"""
        self.dest_obj_mapper.append((weight, mapper))

    def connect(self, hardware_resources):
        """Connect the PyStormObjectMapper's _PyStorm object to it's adjacent nodes

        Update the resources list parameter, if necessary.

        The following describes the map between
            _PyStorm object connections and
            hardware_resources object connections

        Conn 1:
                      _Pystorm: Input ─> Pool
            hardware_resources: Source ─> TATTapPoint ─> Neurons
        Conn 2:
                      _Pystorm: Input ─> Bucket
            hardware_resources: Source ─> TATAcuumulator ─> MMWeights ─> AMBuckets
        Conn 3:
                      _PyStorm: Pool ─> Bucket
            hardware_resources: Neurons ─> MMWeights ─> AMBuckets
        Conn 4:
                      _PyStorm: Bucket ─> Bucket
            hardware_resources: AMBuckets ─> MMWeights ─> AMBuckets
        Conn 5:
                      _PyStorm: Bucket ─> Output
            hardware_resources: AMBuckets ─> Sink
         Conn 6:
                      _PyStorm: Bucket ─┬─> Bucket
                                        └─> Bucket
            hardware_resources: AMBuckets ─> TATFanout ─┬─> MMWeights ─> AMBuckets
                                                        └─> MMWeights ─> AMBuckets
         Conn 7:
                      _PyStorm: Bucket ─┬─> Output
                                        └─> Output
            hardware_resources: AMBuckets ─> TATFanout  ─┬─> Sink
                                                         └─> Sink

        Note that the first five connections are from one object to one
        object, however, connections 6 and 7 are from one object to multiple
        objects which require a different set of resources.

        Parameters
        ----------
        hardware_resources: A list of hardware_resources instances
        """
        number_of_adj_nodes = len(self.dest_obj_mapper)

        if number_of_adj_nodes == 0:
            return

        elif number_of_adj_nodes == 1:
            adj_node = self.dest_obj_mapper[0]
            adj_node_net_obj = adj_node.second
            adj_node_weights = adj_node.first

            # Conn 1: Input -> Pool
            if isinstance(self.ps_obj, ps.Input) and isinstance(adj_node_net_obj, ps.Pool):
                num_neurons = self.ps_obj.GetNumNeurons()
                matrix_dims = 2
                adj_node_dims = adj_node_net_obj.GetNumDimensions()

                source = self.resource
                tap = TATTapPoint(
                    np.random.randint(num_neurons, size=(matrix_dims, adj_node_dims)),
                    np.random.randint(1, size=(matrix_dims, adj_node_dims))*2 - 1,
                    num_neurons)
                neurons = adj_node_net_obj.resource

                source.connect(tap)
                tap.connect(neurons)

                resources.append(tap)

            #   Conn 2: Input -> Bucket
            if isinstance(self.ps_obj, ps.Input) and isinstance(adj_node_net_obj, ps.Bucket):
                source = self.resource
                tat_acc_resource = TATAccumulator(self.ps_obj.GetNumDimensions())
                weight_resource = self.create_mm_weights(adj_node_weights)
                am_bucket = adj_node_net_obj.resource

                source.connect(tat_acc_resource)
                tat_acc_resource.connect(weight_resource)
                weight_resource.connect(am_bucket)

                resources.append(tat_acc_resource)
                resources.append(weight_resource)

            #   Conn 3: Pool -> Bucket
            if isinstance(self.ps_obj, ps.Pool) and isinstance(adj_node_net_obj, ps.Bucket):
                neurons = self.resource
                weight_resource = self.create_mm_weights(adj_node_weights)
                am_bucket = adj_node_net_obj.resource

                neurons.connect(weight_resource)
                weight_resource.connect(am_bucket)

                resources.append(weight_resource)

            #   Conn 4: Bucket -> Bucket
            if isinstance(self.ps_obj, ps.Bucket) and isinstance(adj_node_net_obj, ps.Bucket):
                am_bucket_in = self.resource
                weight_resource = self.create_mm_weights(adj_node_weights)
                am_bucket_out = adj_node_net_obj.resource

                am_bucket_in.connect(weight_resource)
                weight_resource.connect(am_bucket_out)

                resources.append(weight_resource)

            #   Conn 5: Bucket -> Output
            if isinstance(self.ps_obj, ps.Bucket) and isinstance(adj_node_net_obj, ps.Output):
                am_bucket = self.resource
                sink = adj_node_net_obj.resource

                am_bucket.connect(sink)

        elif number_of_adj_nodes > 1:
            if isinstance(self.ps_obj, ps.Bucket):
                am_bucket_in = self.resource
                tat_fanout = TATFanout(self.ps_obj.GetNumDimensions())

                am_bucket_in.connect(tat_fanout)

                resources.append(tat_fanout)

                for adj_node in self.dest_obj_mapper:
                    adj_node_net_obj = adj_node.second
                    adj_node_weight = adj_node.first
            #   Conn 6: Bucket -> Bucket
            #                  -> Bucket
                if isinstance(adj_node_net_obj, ps.Bucket):
                    weight_resource = self.create_mm_weights(adj_node_weight)
                    am_bucket_out = adj_node_net_obj

                    resources.append(weight_resource)

                    tat_fanout.connect(weight_resource)
                    weight_resource.connect(am_bucket_out)

            #   Conn 7: Bucket -> Output
            #                  -> Output
                if isinstance(adj_node_net_obj, ps.Output):
                    sink = adj_node_net_obj.resource
                    tat_fanout.connect(sink)

def create_network_resources(pystorm_network):
    """Create the required resources for a  _PyStorm network

    Parameters
    ----------
    pystorm_network: A _Pystorm.Network object
        The objects in the _PyStorm network may not form a graph.
        If a _PyStorm object is connected to another _PyStorm
        network object, the corresponding Resource objects should
        be connected as well.

    Returns
    -------
    resources: A list of hardware_resources.Resource objects
    """

    resources = []
    # A map from _PyStorm objects to PyStormObjectMappers
    # The map forms a graph of _PyStorm.Network/Resource objects
    network_obj_map = dict()

    # Populate the PyStormObjectMappers in the graph
    for inp in pystorm_network.GetInputs():
        network_obj_map[inp] = PyStormObjectMapper(inp)
        resources.append(network_obj_map[inp].get_resource())

    for out in pystorm_network.GetOutputs():
        network_obj_map[out] = PyStormObjectMapper(out)
        resources.append(network_obj_map[out].get_resource())

    for pool in pystorm_network.GetPools():
        network_obj_map[pool] = PyStormObjectMapper(pool)
        resources.append(network_obj_map[pool].get_resource())

    for bucket in pystorm_network.GetBuckets():
        network_obj_map[bucket] = PyStormObjectMapper(bucket)
        resources.append(network_obj_map[bucket].get_resource())

    # Connect the PyStormObjectMappers to their adjacent PyStormObjectMappers
    connections = pystorm_network.GetConnections()
    for conn in connections:
        source = conn.GetSource()
        dest = conn.GetDest()
        weights = conn.GetWeights()  # weights is either of type Weights or None
        network_obj_map[source].add_adjacent_node(network_obj_map[dest], weights)

    # Connect the adjacent nodes creating Resources relevant to each connection
    for _, net_obj_node in network_obj_map.items():
        net_obj_node.connect(resources)

    return resources

def create_pystorm_mem_objects(core):
    """
    instead of WriteMemsToFile, create a set of structures
    supplied by _PyStorm that represent what you are mapping
    the original network to.
    """
    pass

def map_resources_to_core(resources, core, verbose=False):
    """Annotate a Core object with hardware_resources.Resource objects

    Parameters
    ----------
    resources: A list of hardware_resources.Resource objects
    core: A Core object
    verbose: A bool

    Returns
    -------
    core: The modified Core object
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

def map_network(pystorm_network, verbose=False):
    """Create a MappedNetwork object given a pystorm._PyStorm.Network object

    Parameters
    ----------
    pystorm_network: An object of type pystorm._PyStorm.Network


    Returns
    -------
    pystorm_network: a mapped pystorm._PyStorm.Network
    pystorm_mem: a list of structures representing what will be written to memory.
    """
    pars = ps.GetCorePars()

    core = Core(pars)

    resources = create_network_resources(pystorm_network)

    core = map_resources_to_core(resources, core, verbose)

    pystorm_mem = create_pystorm_mem_objects(core)

    mapped_network = (pystorm_network, pystorm_mem)

    return mapped_network
