from MapController import *

class Neuromorph:
    ''' 
        Public interface to Neuromorph package
    '''
    def __init__(self):
        _mapController = MapController()

    def Map(self, pystorm_network):
        '''
            Create a MappedNetwork object given a Pystorm.Network object
    
            Params:
                pystorm_network - An object of type Pystorm.Network

            Returns:
                An object of type (TBD) which binds the Network to 
                a set of Braindrop core resources (type also TBD).
        '''
        return self._mapController.Map(pystorm_network)
