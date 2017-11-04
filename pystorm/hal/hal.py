"""Provides the hardware abstraction layer"""
import numpy as np
from pystorm.hal.neuromorph import map_network, remap_resources
from pystorm.PyDriver import bddriver

# notes that affect nengo BE:
# set_weights -> remap_weights (set_weights should just modify network directly, call remap_weights() when done

# notes that affect driver
# need to expose some functionality to do (y,x) <-> AER encoding

CORE_ID = 0 # hardcoded for now

class HAL(object):
    """Hardware Abstraction Layer

    Abstracts away the details of the underlying hardware, and presents a
    unified API for higher level clients.

    Attributes
    ----------
    driver: Instance of pystorm.PyDriver Driver
    """
    def __init__(self):
        self.driver = bddriver.Driver()

        # maps between neuromorph graph Input/Output to tag (indices?)
        self.ng_input_to_tags = None
        self.tags_to_ng_output = None

        # map between spike id and pool/neuron_idx
        self.spk_to_pool_nrn = None

        self.start_hardware()

        self.driver.InitBD()

    def start_hardware(self):
        """Starts the driver"""
        self.driver.Start()

    def stop_hardware(self):
        """Stops the driver"""
        self.driver.Stop()

    ##############################################################################
    #                           Data flow functions                              #
    ##############################################################################

    def disable_output_recording(self):
        """Turns off recording from all outputs."""
        # XXX make some FPGA calls
        pass 

    def disable_spike_recording(self):
        """Turns off spike recording from all neurons."""
        self.driver.SetSpikeDumpState(CORE_ID, en=False)

    def enable_output_recording(self):
        """Turns on recording from all outputs.

        These output values will go into a buffer that can be drained by calling
        get_outputs().
        """
        # XXX make some FPGA calls
        pass 

    def enable_spike_recording(self):
        """Turns on spike recording from all neurons.

        These spikes will go into a buffer that can be drained by calling
        get_spikes().
        """
        self.driver.SetSpikeDumpState(CORE_ID, en=True)

    def get_outputs(self):
        """Returns all pending output values gathered since this was last called.

        Data format: a numpy array of : [(output, dim, counts, time), ...]
        Timestamps are in microseconds
        """
        tags, times = self.driver.RecvTags(CORE_ID)
        outputs = []
        dims = []
        counts = []
        for tag in tags:
            out, dim, count = self.tags_to_ng_output[tag]
            outputs.append(out)
            dims.append(dim)
            counts.append(count)

        ret_data = np.array([outputs, dims, counts, times]).T
        return ret_data

    def get_spikes(self):
        """Returns all the pending spikes gathered since this was last called.

        Data format: numpy array: [(timestamp, pool_id, neuron_index), ...]
        Timestamps are in microseconds
        """
        spk_data = self.driver.RecvSpikes(CORE_ID)
        timestamps = []
        pool_ids = []
        nrn_idxs = []
        for spk_id, spk_time in spk_data:
            pool_id, nrn_idx = self.spk_to_pool_nrn[spk_id]
            pool_ids.append(pool_id)
            nrn_idx.append(nrn_idx)
            timestamps.append(spk_time)
        ret_data = np.array([timestamps, pool_ids, nrn_idxs]).T
        return ret_data

    def send_inputs(self, inputs, dims, counts, times):
        """Sends pregenerated tags to the given target

        inputss: list of Input object
        dims : list of ints
            dimensions within eacn Input object to send to
        counts: list of ints
            number of spikes for each time
        times: list of ints
            times to send each entry in counts
        """
        assert len(inputs) == len(dims) == len(counts) == len(times)

        tags = []
        for inp, dim, count in zip(inputs, dims, counts):
            tags.append(self.ng_input_to_tags[inp]) # TODO: what is out_tags of a Source? what about count?

        self.driver.SendTags(CORE_ID, tags, times)


    ##############################################################################
    #                           Mapping functions                                #
    ##############################################################################

    def remap_weights(self, network, hardware_resources):
        """Call a subset of map()'s functionality to reprogram weights
        that have been modified in the network objects

        Parameters
        ----------
        network: pystorm.hal.neuromorph.graph Network object
        hardware_resources: output of previous map() call
        """

        # network is unused: hardware_resources references it

        # generate a new core based on the new allocation, but assigning the new weights
        core = remap_resources(hardware_resource)
        implement_core(core)

    def map(self, network):
        """Maps a Network to low-level HAL objects and returns mapping info.

        Parameters
        ----------
        network: pystorm.hal.neuromorph.graph Network object
        """
        ng_obj_to_hw, hardware_resources, core = map_network(network)
        hw_to_ng_obj = {v: k for k, v in ng_obj_to_hw.items()}

        # implement core objects, calling driver
        implement_core(core)

        self.ng_input_to_tags = {}
        self.tags_to_ng_output = {}
        self.spk_to_pool_nrn = {}
        for resource in hardware_resources:
            ng_obj = hw_to_ng_obj[resource]

            # input -> tags mapping
            if isinstance(resource, Source):
                self.ng_input_to_tags[ng_obj] = resource.out_tags
        
            # tags -> (output, dim) mapping
            elif isinstance(resource, Sink):
                for dim_idx, tag in enumerate(resource.out_tags):
                    self.tags_to_ng_output[tag] = (ng_obj, dim_idx)

            # neuron (x,y) -> (pool, x, y)
            elif isinstance(resource, Neurons):
                pass


    def implement_core(self, core):
        """Implements a supplied core to BD"""
        driver.SetMem(CORE_ID, bddriver.bdpars.MemId.PAT,  core.PAT.m,  0);
        driver.SetMem(CORE_ID, bddriver.bdpars.MemId.TAT0, core.TAT0.m, 0);
        driver.SetMem(CORE_ID, bddriver.bdpars.MemId.TAT1, core.TAT1.m, 0);
        driver.SetMem(CORE_ID, bddriver.bdpars.MemId.MM,   core.MM.m,   0);
        driver.SetMem(CORE_ID, bddriver.bdpars.MemId.AM,   core.AM.m,   0);
