import Pystorm as ps
import Core
import CoreParsPlaceHolder
import Resources


class MapController(object):
    def __init__(self):
        pass

    def _CreateResourceObjects(self, pystorm_network):
        pass

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

        resources = _CreateResourceObjects(pystorm_network)

        core = _MapResourceObjectsToCore(core, resources)

        pystorm_mem = _CreatePystormMemObjects(core)

        mapped_network = (pystorm_network, pystorm_mem)

        return mapped_network
