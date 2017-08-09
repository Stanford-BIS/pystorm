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


class MapController(object):
    def __init__(self):
        pass

    def create_resources(self, pystorm_network):
        resources = []
        connections = pystorm_network.GetConnections()

        netobj_resource_dict = dict()
        netobj_conn_out_count = dict()
        bucket_fanouts = dict()

        # create the resources for the network objects
        for conn in connections:
            source = conn.GetSource()
            dest = conn.GetDest()
            if source not in netobj_resource_dict:
                resource = create_resources_for_(source)
                netobj_resource_dict[source] = resource

            if source not in netobj_conn_out_count[source]:
                netobj_conn_out_count[source] = 0

            netobj_conn_out_count[source] = netobj_conn_out_count[source] + 1

            if dest not in netobj_resource_dict:
                resource = create_resources_for_(dest)
                netobj_resource_dict[dest] = resource
            
        # Create the resources for the connections and connect
        for conn in connections:
            source = conn.GetSource()
            dest = conn.GetDest()

            source_resource = netobj_resource_dict[source]
            dest_resource = netobj_resource_dict[dest] 

            # if the source is an Input, the dest will be a Pool or a Bucket
            # if a Pool, Create a tap point and move the source resource
            # if a Bucket, Create a tat accumulator and move the source resource
            if type(source_resource) == Source:
                if type(dest_resource) == Neurons:
                    M = dest.GetNumNeurons()
                    K = 2
                    D = source.GetNumDimensions()

                    tap = TATTapPoint(
                        np.random.randint(M, size=(K, D)), 
                        np.random.randint(1, size=(K, D))*2 - 1, M)

                    source_resource.connect(tap)
                    source_resource = tap

            # if the source is a Bucket, this connection may be part of a fanout
            if type(source_resource) == AMBuckets:
                if netobj_conn_out_count[source] > 1:
                    if bucket_fanouts[source] is None:
                        fanout = TATFanout(source.GetNumDimensions())
                        bucket_fanouts[source] = fanout
                    else:
                        fanout = bucket_fanouts[source]
                    source_resource.connect(fanout)
                    source_resource = fanout

            # this connection may have weights
            if conn.GetWeights() is not None:
                weights = conn.GetWeights()
                # create the weight matrix correctly
                if type(source) == Pool:
                    weight_resource = MMWeights(
                        np.zeros(dest.GetNumDimensions, source.GetNumNeurons()))
                else:
                    weight_resource = MMWeights(
                        np.zeros(dest.GetNumDimensions,
                            source.GetNumDimensions()))
                source_resource.connect(weight_resource)
                source_resource = weight_resource
                
            source_resource.connect(dest_resource)

            return resources

    def create_pystorm_mem_objects(self, core):
        pass
        # instead of WriteMemsToFile, create a set of structures
        # supplied by Pystorm that represent what you are mapping
        # the original network to.

    def map_resource_objects_to_core(self, core, resources, verbose=False):
        for node in resources:
            node.PreTranslate(core)
        if verbose: print("finished PreTranslate")

        for node in resources:
            node.AllocateEarly(core)
        if verbose: print("finished AllocateEarly")
        
        core.MM.alloc.SwitchToTrans() # switch allocation mode of MM
        for node in resources:
            node.Allocate(core)
        if verbose: print("finished Allocate")

        for node in resources:
            node.PostTranslate(core)
        if verbose: print("finished PostTranslate")

        for node in resources:
            node.Assign(core)
        if verbose: print("finished Assign")

        return core

    def Map(self, pystorm_network, verbose=False):
        pars = ps.GetCorePars()

        core = Core(pars)

        resources = self.create_resources(pystorm_network)

        core = self.map_resource_objects_to_core(core, resources)

        pystorm_mem = self.create_pystorm_mem_objects(core)

        mapped_network = (pystorm_network, pystorm_mem)

        return mapped_network
