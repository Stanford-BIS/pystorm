"""Provides the mapping functionality of the HAL"""
from functools import singledispatch, update_wrapper
import numpy as np
import pystorm._PyStorm as _PyStorm
from pystorm.hal.neuromorph.graph import (Bucket, Input, Output, Pool)
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
    """Creates the hardware resources for a neuromorph graph object"""
    raise TypeError("This type isn't supported: {}".format(type(pystorm_obj)))

@create_resources.register(Input)
def _create_resources_ps_input(ps_input):
    """create_resources for a neuromorph graph Input"""
    return Source(ps_input.get_num_dimensions())

@create_resources.register(Output)
def _create_resources_ps_output(ps_output):
    """create_resources for neuromorph graph Output"""
    return Sink(ps_output.get_num_dimensions())

@create_resources.register(Pool)
def _create_resources_ps_pool(ps_pool):
    """create_resources for neuromorph graph Pool"""
    return Neurons(ps_pool.get_num_neurons())

@create_resources.register(Bucket)
def _create_resources_ps_bucket(ps_bucket):
    """create_resources for neuromorph graph Bucket"""
    return AMBuckets(ps_bucket.get_num_dimensions())

def create_mm_weights(ps_weights):
    """Create a representation of the main memory weights"""
    ret_value = MMWeights(ps_weights)
    return ret_value

class GraphHWMapper(object):
    """Maps a neuromorph graph object to hardware resources

    Instances of this class form the nodes of a directed graph
    based on the objects in a neuromorph graph network.

    Hardware resources are allocated as each instance is created.

    Hardware resources for connections are not one-to-one with the neuromorph graph connections,
    so we delay allocating resources for link in the graph until all links are added.
        Connections might share hardware (e.g. when weights are the same) or
        require extra hardware (e.g. when a neuromorph graph object fans out to multiple objects).

    Parameters
    ----------
    ps_obj: A neuromorph graph object (i.e. Input, Pool, Bucket, or Output)

    Attributes
    ----------
    ps_obj: The neuromorph graph object
    hardware_resource: The hardware resource associated with the neuromorph graph object
    dest_mappers: List of (weights, PystormObjectMappers) this instance connects to
    """
    def __init__(self, ps_obj):
        self.ps_obj = ps_obj
        self.hardware_resource = create_resources(ps_obj)
        self.dest_mappers = []

    def get_resource(self):
        return self.hardware_resource

    def connect_src_to_dest_mapper(self, mapper, weight=None):
        """Connect this GraphHWMapper to another GraphHWMapper
        
        Parameters
        ----------
        mapper: Another GraphHWMapper instance
            The self instance is the source, and mapper is the destination
        """
        self.dest_mappers.append((weight, mapper))

    def connect(self, hardware_resources):
        """Allocate hardware resources to implement this instance's connections

        Update the hardware_resources list parameter if necessary.

        The following describes the map between
            neuromorph graph object connections and hardware_resources connections

        Conn 1:
              neuromorph graph: Input ─> Pool
            hardware_resources: Source ─> TATTapPoint ─> Neurons
        Conn 2:
              neuromorph graph: Input ─> Bucket
            hardware_resources: Source ─> TATAcuumulator ─> MMWeights ─> AMBuckets
        Conn 3:
              neuromorph graph: Pool ─> Bucket
            hardware_resources: Neurons ─> MMWeights ─> AMBuckets
        Conn 4:
              neuromorph graph: Bucket ─> Bucket
            hardware_resources: AMBuckets ─> MMWeights ─> AMBuckets
        Conn 5:
              neuromorph graph: Bucket ─> Output
            hardware_resources: AMBuckets ─> Sink
         Conn 6:
              neuromorph graph: Bucket ─┬─> Bucket / Output
                                        └─> Bucket / Output
                                        ⋮
                                        └─> Bucket / Output
            hardware_resources: AMBuckets ─> TATFanout ─┬─> (MMWeights ─> AMBuckets) / Sink
                                                        └─> (MMWeights ─> AMBuckets) / Sink
                                                        ⋮
                                                        └─> (MMWeights ─> AMBuckets) / Sink
        Note:
        The first five connection types are from one object to one object.
        Connections 6 is from one object to multiple objects;
            this requires additional hardware_resources.

        Parameters
        ----------
        hardware_resources: A list of hardware_resources instances
        """
        n_tgt_nodes = len(self.dest_mappers)

        if n_tgt_nodes == 1:
            dest_node = self.dest_mappers[0]
            dest_node_ps_obj = dest_node[1].ps_obj
            dest_node_weights = dest_node[0]

            # Conn 1: Input -> Pool
            if isinstance(self.ps_obj, Input) and isinstance(dest_node_ps_obj, Pool):
                num_neurons = dest_node_ps_obj.get_num_neurons()
                matrix_dims = 2
                adj_node_dims = self.ps_obj.get_num_dimensions()

                source = self.hardware_resource
                tap = TATTapPoint(
                    # TODO: This should not be random! 
                    # Put in the H-Tree algorithm here!
                    np.random.randint(num_neurons, size=(adj_node_dims, matrix_dims)),
                    np.random.randint(2, size=(adj_node_dims, matrix_dims))*2 - 1,
                    num_neurons)
                neurons = dest_node[1].hardware_resource

                source.connect(tap)
                tap.connect(neurons)

                hardware_resources.append(tap)

            #   Conn 2: Input -> Bucket
            elif isinstance(self.ps_obj, Input) and isinstance(dest_node_ps_obj, Bucket):
                source = self.hardware_resource
                tat_acc_resource = TATAccumulator(self.ps_obj.get_num_dimensions())
                weight_resource = create_mm_weights(dest_node_weights)
                am_bucket = dest_node_ps_obj.hardware_resource

                source.connect(tat_acc_resource)
                tat_acc_resource.connect(weight_resource)
                weight_resource.connect(am_bucket)

                hardware_resources.append(tat_acc_resource)
                hardware_resources.append(weight_resource)

            #   Conn 3: Pool -> Bucket
            elif isinstance(self.ps_obj, Pool) and isinstance(dest_node_ps_obj, Bucket):
                neurons = self.hardware_resource
                weight_resource = create_mm_weights(dest_node_weights)
                am_bucket = dest_node[1].hardware_resource

                neurons.connect(weight_resource)
                weight_resource.connect(am_bucket)

                hardware_resources.append(weight_resource)

            #   Conn 4: Bucket -> Bucket
            elif isinstance(self.ps_obj, Bucket) and isinstance(dest_node_ps_obj, Bucket):
                am_bucket_in = self.hardware_resource
                weight_resource = create_mm_weights(dest_node_weights)
                am_bucket_out = dest_node[1].hardware_resource

                am_bucket_in.connect(weight_resource)
                weight_resource.connect(am_bucket_out)

                hardware_resources.append(weight_resource)

            #   Conn 5: Bucket -> Output
            elif isinstance(self.ps_obj, Bucket) and isinstance(dest_node_ps_obj, Output):
                am_bucket = self.hardware_resource
                sink = dest_node[1].hardware_resource

                am_bucket.connect(sink)
            else:
                raise TypeError("connections between %s and %s are not currently supported"%(
                    str(self.ps_obj), str(dest_node_ps_obj)))

        #   Conn 6: Bucket ─┬─> Bucket / Output
        #                   └─> Bucket / Output
        #                   ⋮
        #                   └─> Bucket / Output
        elif n_tgt_nodes > 1:
            assert isinstance(self.ps_obj, Bucket)
            am_bucket_in = self.hardware_resource
            tat_fanout = TATFanout(self.ps_obj.GetNumDimensions())

            am_bucket_in.connect(tat_fanout)

            hardware_resources.append(tat_fanout)

            for dest_node in self.dest_mappers:
                dest_node_ps_obj = dest_node.second
                adj_node_weight = dest_node.first
                if isinstance(dest_node_ps_obj, Bucket):
                    weight_resource = create_mm_weights(adj_node_weight)
                    am_bucket_out = dest_node_ps_obj

                    hardware_resources.append(weight_resource)

                    tat_fanout.connect(weight_resource)
                    weight_resource.connect(am_bucket_out)
                elif isinstance(dest_node_ps_obj, Output):
                    sink = dest_node_ps_obj.hardware_resource
                    tat_fanout.connect(sink)
                else:
                    raise TypeError(
                        "fanout connections between %s and %s are not currently supported"%(
                            str(self.ps_obj), str(dest_node_ps_obj)))

def create_network_resources(network):
    """Create the required hardware resources for a  neuromorph graph network

    Parameters
    ----------
    network: A neuromorph graph Network object
        The objects in the neuromorph graph Network may not form a graph.
        If a neuromorph graph object is connected to another neuromorph graph
        object, the corresponding Resource objects should be connected as well.

    Returns
    -------
    hardware_resources: A list of hardware_resources.Resource objects
    """

    hardware_resources = []
    # A map from neuromorph graph objects to GraphHWMappers
    # The map forms a graph of neuromorph graph.Network/Resource objects
    ng_obj_hw_map = dict()

    # Populate the GraphHWMappers in the graph
    for inp in network.get_inputs():
        ng_obj_hw_map[inp] = GraphHWMapper(inp)
        hardware_resources.append(ng_obj_hw_map[inp].get_resource())

    for out in network.get_outputs():
        ng_obj_hw_map[out] = GraphHWMapper(out)
        hardware_resources.append(ng_obj_hw_map[out].get_resource())

    for pool in network.get_pools():
        ng_obj_hw_map[pool] = GraphHWMapper(pool)
        hardware_resources.append(ng_obj_hw_map[pool].get_resource())

    for bucket in network.get_buckets():
        ng_obj_hw_map[bucket] = GraphHWMapper(bucket)
        hardware_resources.append(ng_obj_hw_map[bucket].get_resource())

    # Connect source GraphHWMappers to their destination GraphHWMappers
    connections = network.get_connections()
    for conn in connections:
        source = conn.get_source()
        dest = conn.get_dest()
        weights = conn.get_weights()  # weights is either of type Weights or None
        ng_obj_hw_map[source].connect_src_to_dest_mapper(ng_obj_hw_map[dest], weights)

    # Connect the adjacent nodes creating Resources relevant to each connection
    for _, ng_obj_hw_mapper in ng_obj_hw_map.items():
        ng_obj_hw_mapper.connect(hardware_resources)

    return ng_obj_hw_map, hardware_resources

def map_resources_to_core(hardware_resources, core, verbose=False):
    """Annotate a Core object with hardware_resources.Resource objects

    Parameters
    ----------
    hardware_resources: A list of hardware_resources.Resource objects
    core: A Core object
    verbose: A bool

    Returns
    -------
    core: The modified Core object
    """
    for resource in hardware_resources:
        resource.pretranslate(core)
    if verbose:
        print("finished pretranslate")

    for resource in hardware_resources:
        resource.allocate_early(core)
    if verbose:
        print("finished allocate_early")

    core.MM.alloc.SwitchToTrans()  # switch allocation mode of MM
    for resource in hardware_resources:
        resource.allocate(core)
    if verbose:
        print("finished allocate")

    for resource in hardware_resources:
        resource.posttranslate(core)
    if verbose:
        print("finished posttranslate")

    for resource in hardware_resources:
        resource.assign(core)
    if verbose:
        print("finished assign")

    return core

def map_network(network, verbose=False):
    """Create a MappedNetwork object given a neuromorphg graph Network object

    Parameters
    ----------
    network: An object of type neuromorph graph Network

    Returns
    -------
    ng_obj_to_hw: dictionary
        keys: neuromorph graph objects in the input network
        values: hardware resources used to implement neuromorph graph object
    hardware_resources: list of total hardware resources allocated for the input network
    core: a representation of the hardware core
    """
    pars = _PyStorm.get_core_pars()

    core = Core(pars)

    ng_obj_to_hw, hardware_resources = create_network_resources(network)

    core = map_resources_to_core(hardware_resources, core, verbose)

    return ng_obj_to_hw, hardware_resources, core
