import Pystorm as ps
import Core
import CoreParsPlaceHolder
import Resources
import functools
import numpy as np

class MapController(object):
    def __init__(self):
        self._CreateResourcesForObject = functools.singledispatch(self._createResources)
        self._createResources.register(Pystorm.Input, 
            self._CreateResourcesForInput)
        self._createResources.register(Pystorm.Input, 
            self._CreateResourcesForInput)
        self._createResources.register(Pystorm.Input, 
            self._CreateResourcesForInput)
        self._createResources.register(Pystorm.Input, 
            self._CreateResourcesForInput)
        self._createResources.register(Pystorm.Input, 
            self._CreateResourcesForInput)

    def _CreateResourcesForObject(self, obj):
        raise TypeError("This type isn't supported: {}".format(type(obj)))

    def _CreateResourcesForInput(self, input):
        return Source(input.GetNumDimensions())

    def _CreateResourcesForOutput(self, output):
        return Sink(output.GetNumDimensions())

    def _CreateResourcesForPool(self, pool):
        return Neurons(pool.GetNumNeurons())

    def _CreateResourcesForBucket(self, bucket):
        return AMBucket(bucket.GetNumDimensions())

    def _CreateResources(self, pystorm_network):
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
                resource = _CreateResourcesForObject(source)
                netobj_resource_dict[source] = resource

            if source not in netobj_conn_out_count[source]:
                 netobj_conn_out_count[source] = 0

            netobj_conn_out_count[source] = netobj_conn_out_count[source] + 1

            if dest not in netobj_resource_dict:
                resource = _CreateResourceForObject(dest)
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
                    if bucket_fanout[source] is None:
                        fanout = TATFanout(source.GetNumDimensions())
                        bucket_fanout[source] = fanout
                    else
                        fanout = bucket_fanout[source]
                    source_resource.connect(fanout)
                    source_resource = fanout

            # this connection may have weights
            if conn.GetWeights() is not None:
                weights = conn.GetWeights()
                # create the weight matrix correctly
                if type(source) == Pystorm.Pool:
                    weight_resource = MMWeights(
                        np.zeros(dest.GetNumDimensions, source.GetNumNeurons()))
                else:
                    weight_resource = MMWeights(
                        np.zeros(dest.GetNumDimensions,
                            source.GetNumDimensions()))
                source_resource.connect(weight_resource)
                source_resource = weight_resource
                
            source_resource.connect(dest_resource)

    def _CreatePystormMemObjects(self, core):
        pass
        # instead of WriteMemsToFile, create a set of structures
        # supplied by Pystorm that represent what you are mapping
        # the original network to.

    def _MapResourceObjectsToCore(self, core, resources):
        for node in resources:
            node.PreTranslate(core)
        if verbose: print("finished PreTranslate")

        for node in resources:
            node.AllocateEarly(core)
        if verbose: print("finished AllocateEarly")
        
        self.MM.alloc.SwitchToTrans() # switch allocation mode of MM
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
        pars = ps.GetCorePars();

        core = Core(pars)

        resources = _CreateResources(pystorm_network)

        core = _MapResourceObjectsToCore(core, resources)

        pystorm_mem = _CreatePystormMemObjects(core)

        mapped_network = (pystorm_network, pystorm_mem)

        return mapped_network
