from Pystorm import *
from . import Core
from . Resources import *
from . netobjnode import NetObjNode


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

        # A map from Pystorm.Network objects to NetObjNodes
        # The map forms a graph of Pystorm.Network/Resource objects
        network_obj_map = dict()

        # Populate the NetObjNodes in the graph
        for inp in pystorm_network.get_inputs():
            network_obj_map[inp] = NetObjNode(inp)
            resources.append(network_obj_map[inp].get_resource())

        for out in pystorm_network.get_outputs():
            network_obj_map[out] = NetObjNode(out)
            resources.append(network_obj_map[out].get_resource())

        for pool in pystorm_network.get_pools():
            network_obj_map[pool] = NetObjNode(pool)
            resources.append(network_obj_map[pool].get_resource())

        for bucket in pystorm_network.get_buckets():
            network_obj_map[bucket] = NetObjNode(bucket)
            resources.append(network_obj_map[bucket].get_resource())

        # Connect the NetObjNodes to their adjacent NetObjNodes
        connections = pystorm_network.get_connections()

        for conn in connections:
            source = conn.get_source()
            dest = conn.get_dest()
            weights = conn.get_weights()  # weights is either of type Weights or None

            network_obj_map[source].add_adjacent_node(network_obj_map[dest], weights)

        # Connect the adjacent nodes creating Resources relevant to each connection
        for net_obj, net_obj_node in network_obj_map.items():
            net_obj_node.connect(resources)

        return network_obj_map, resources

    def map_resources_to_core(self, resources, core, verbose=False):
        """Map each Resource to the Core object

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
        """Create a map between the Pystorm network and a set of Resources.

        :param pystorm_network: A Pystorm.Network object
        :param verbose: A bool telling MapController to print status during mapping

        :return pystorm_network: The Pystorm.Network object passed in
        :return pystorm_net_obj_map: A dict from each Pystorm object to a NetObjNode.
                                     The NetObjNode's form a grap representing the
                                     PystormNetwork and their associated Resource
                                     objects.
        :return resources: A list of Resource objects created for the Pystorm.Network object
                           passed in.
        :return core: The Core object Resource instances are mapped to.

        """
        pars = ps.get_core_pars()

        core = Core(pars)

        (pystorm_net_obj_map, resources) = self.create_resources(pystorm_network)

        core = self.map_resources_to_core(resources, core, verbose)

        return pystorm_network, pystorm_net_obj_map, resources, core
