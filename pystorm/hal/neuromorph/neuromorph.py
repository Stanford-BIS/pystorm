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
        """Create a new wrapper"""
        return dispatcher.dispatch(args[1].__class__)(*args, **kwargs)
    wrapper.register = dispatcher.register
    update_wrapper(wrapper, func)
    return wrapper

@singledispatch
def create_resources(pystorm_obj):
    """Creates the hardware resources for the _PyStorm object

    For example, for a _PyStorm Input, creates a hardware resource Source
    """
    raise TypeError("This type isn't supported: {}".format(type(pystorm_obj)))

@create_resources.register(ps.Input)
def _create_resources_ps_input(ps_input):
    """create_resources for _PyStorm.Input"""
    return Source(ps_input.GetNumDimensions())

@create_resources.register(ps.Output)
def _create_resources_ps_output(ps_output):
    """create_resources for _PyStorm.Output"""
    return Sink(ps_output.GetNumDimensions())

@create_resources.register(ps.Pool)
def _create_resources_ps_pool(ps_pool):
    """create_resources for _PyStorm.Pool"""
    return Neurons(ps_pool.GetNumNeurons())

@create_resources.register(ps.Bucket)
def _create_resources_ps_bucket(ps_bucket):
    """create_resources for _PyStorm.Bucket"""
    return AMBuckets(ps_bucket.GetNumDimensions())

def create_mm_weights(ps_weights):
    """Create a representation of the main memory weights"""
    weights = np.array(ps_weights.GetNumRows(),
                       ps_weights.GetNumColumns())
    for i in range(ps_weights.GetNumRows()):
        for j in range(ps_weights.GetNumColumns()):
            weights[i][j] = ps_weights.GetElement(i, j)
    ret_value = MMWeights(weights)
    return ret_value

class PyStormHWMapper(object):
    """Maps a _PyStorm object to hardware resources

    Instances of this class form the nodes of a directed graph
    based on the objects in a _PyStorm network.

    Hardware resources are allocated as each instance is created.

    Hardware resources for connections are not one-to-one with the _PyStorm connections,
    so we delay allocating resources for link in the graph until all links are added.
        Connections might share hardware (e.g. when weights are the same) or
        require extra hardware (e.g. when a _PyStorm object fans out to multiple objects).

    Parameters
    ----------
    ps_obj: A _PyStorm object (i.e. Input, Pool, Bucket, or Output)

    Attributes
    ----------
    ps_obj: The _PyStorm object
    hardware_resource: The hardware resource associated with the _PyStorm object
    dest_mappers: List of (weights, PystormObjectMappers) this instance connects to
    """
    def __init__(self, ps_obj):
        self.ps_obj = ps_obj
        self.hardware_resource = create_resources(ps_obj)
        self.dest_mappers = []

    def connect_src_to_dest_mapper(self, mapper, weight=None):
        """Set this source PyStormHWMapper to connect to a destination PyStormHWMapper"""
        self.dest_mappers.append((weight, mapper))

    def connect(self, hardware_resources):
        """Allocate hardware resources to implement this instance's connections

        Update the hardware_resources list parameter if necessary.

        The following describes the map between
            _PyStorm object connections and hardware_resources connections

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

        Note:
        The first five connection types are from one object to one object. 
        Connections 6 and 7 are from one object to multiple objects;
            this requires additional hardware_resources.

        Parameters
        ----------
        hardware_resources: A list of hardware_resources instances
        """
        n_tgt_nodes = len(self.dest_mappers)

        if n_tgt_nodes == 0:
            return

        elif n_tgt_nodes == 1:
            dest_node = self.dest_mappers[0]
            dest_node_ps_obj = dest_node.second
            dest_node_weights = dest_node.first

            # Conn 1: Input -> Pool
            if isinstance(self.ps_obj, ps.Input) and isinstance(dest_node_ps_obj, ps.Pool):
                num_neurons = self.ps_obj.GetNumNeurons()
                matrix_dims = 2
                adj_node_dims = dest_node_ps_obj.GetNumDimensions()

                source = self.hardware_resource
                tap = TATTapPoint(
                    np.random.randint(num_neurons, size=(matrix_dims, adj_node_dims)),
                    np.random.randint(1, size=(matrix_dims, adj_node_dims))*2 - 1,
                    num_neurons)
                neurons = dest_node_ps_obj.hardware_resource

                source.connect(tap)
                tap.connect(neurons)

                hardware_resources.append(tap)

            #   Conn 2: Input -> Bucket
            if isinstance(self.ps_obj, ps.Input) and isinstance(dest_node_ps_obj, ps.Bucket):
                source = self.hardware_resource
                tat_acc_resource = TATAccumulator(self.ps_obj.GetNumDimensions())
                weight_resource = create_mm_weights(dest_node_weights)
                am_bucket = dest_node_ps_obj.hardware_resource

                source.connect(tat_acc_resource)
                tat_acc_resource.connect(weight_resource)
                weight_resource.connect(am_bucket)

                hardware_resources.append(tat_acc_resource)
                hardware_resources.append(weight_resource)

            #   Conn 3: Pool -> Bucket
            if isinstance(self.ps_obj, ps.Pool) and isinstance(dest_node_ps_obj, ps.Bucket):
                neurons = self.hardware_resource
                weight_resource = create_mm_weights(dest_node_weights)
                am_bucket = dest_node_ps_obj.hardware_resource

                neurons.connect(weight_resource)
                weight_resource.connect(am_bucket)

                hardware_resources.append(weight_resource)

            #   Conn 4: Bucket -> Bucket
            if isinstance(self.ps_obj, ps.Bucket) and isinstance(dest_node_ps_obj, ps.Bucket):
                am_bucket_in = self.hardware_resource
                weight_resource = create_mm_weights(dest_node_weights)
                am_bucket_out = dest_node_ps_obj.hardware_resource

                am_bucket_in.connect(weight_resource)
                weight_resource.connect(am_bucket_out)

                hardware_resources.append(weight_resource)

            #   Conn 5: Bucket -> Output
            if isinstance(self.ps_obj, ps.Bucket) and isinstance(dest_node_ps_obj, ps.Output):
                am_bucket = self.hardware_resource
                sink = dest_node_ps_obj.hardware_resource

                am_bucket.connect(sink)

        elif n_tgt_nodes > 1:
            if isinstance(self.ps_obj, ps.Bucket):
                am_bucket_in = self.hardware_resource
                tat_fanout = TATFanout(self.ps_obj.GetNumDimensions())

                am_bucket_in.connect(tat_fanout)

                hardware_resources.append(tat_fanout)

                for dest_node in self.dest_mappers:
                    dest_node_ps_obj = dest_node.second
                    adj_node_weight = dest_node.first
            #   Conn 6: Bucket -> Bucket
            #                  -> Bucket
                if isinstance(dest_node_ps_obj, ps.Bucket):
                    weight_resource = create_mm_weights(adj_node_weight)
                    am_bucket_out = dest_node_ps_obj

                    hardware_resources.append(weight_resource)

                    tat_fanout.connect(weight_resource)
                    weight_resource.connect(am_bucket_out)

            #   Conn 7: Bucket -> Output
            #                  -> Output
                if isinstance(dest_node_ps_obj, ps.Output):
                    sink = dest_node_ps_obj.hardware_resource
                    tat_fanout.connect(sink)

def create_network_resources(pystorm_network):
    """Create the required hardware resources for a  _PyStorm network

    Parameters
    ----------
    pystorm_network: A _Pystorm.Network object
        The objects in the _PyStorm network may not form a graph.
        If a _PyStorm object is connected to another _PyStorm
        network object, the corresponding Resource objects should
        be connected as well.

    Returns
    -------
    hardware_resources: A list of hardware_resources.Resource objects
    """

    hardware_resources = []
    # A map from _PyStorm objects to PyStormHWMappers
    # The map forms a graph of _PyStorm.Network/Resource objects
    ps_hw_map = dict()

    # Populate the PyStormHWMappers in the graph
    for inp in pystorm_network.GetInputs():
        ps_hw_map[inp] = PyStormHWMapper(inp)
        hardware_resources.append(ps_hw_map[inp].get_resource())

    for out in pystorm_network.GetOutputs():
        ps_hw_map[out] = PyStormHWMapper(out)
        hardware_resources.append(ps_hw_map[out].get_resource())

    for pool in pystorm_network.GetPools():
        ps_hw_map[pool] = PyStormHWMapper(pool)
        hardware_resources.append(ps_hw_map[pool].get_resource())

    for bucket in pystorm_network.GetBuckets():
        ps_hw_map[bucket] = PyStormHWMapper(bucket)
        hardware_resources.append(ps_hw_map[bucket].get_resource())

    # Connect source PyStormHWMappers to their destination PyStormHWMappers
    connections = pystorm_network.GetConnections()
    for conn in connections:
        source = conn.GetSource()
        dest = conn.GetDest()
        weights = conn.GetWeights()  # weights is either of type Weights or None
        ps_hw_map[source].connect_src_to_dest_mapper(ps_hw_map[dest], weights)

    # Connect the adjacent nodes creating Resources relevant to each connection
    for _, ps_hw_mapper in ps_hw_map.items():
        ps_hw_mapper.connect(hardware_resources)

    return hardware_resources

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
    """
    pars = ps.GetCorePars()

    core = Core(pars)

    hardware_resources = create_network_resources(pystorm_network)

    core = map_resources_to_core(hardware_resources, core, verbose)

    mapped_network = pystorm_network

    return mapped_network
