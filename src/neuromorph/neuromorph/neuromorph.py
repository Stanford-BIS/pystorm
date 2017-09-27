from .mapcontroller import MapController


def map(pystorm_network):
    """Create a MappedNetwork object given a Pystorm.Network object

    :param pystorm_network: An object of type Pystorm.Network

    :return (Pystorm.Network, mapinfo): A tuple binding the
    Pystorm.Network to a list of structures representing what
    will be written to memory.
    """
    return MapController().map(pystorm_network)
