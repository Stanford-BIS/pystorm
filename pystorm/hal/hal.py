"""Provides the hardware abstraction layer"""
import numpy as np
from pystorm.hal.neuromorph import map_network, remap_resources
from pystorm.PyDriver import bddriver

DIFFUSOR_NORTH_LEFT = bddriver.bdpars.DiffusorCutLocationId.NORTH_LEFT
DIFFUSOR_NORTH_RIGHT = bddriver.bdpars.DiffusorCutLocationId.NORTH_RIGHT
DIFFUSOR_WEST_TOP = bddriver.bdpars.DiffusorCutLocationId.WEST_TOP
DIFFUSOR_WEST_BOTTOM = bddriver.bdpars.DiffusorCutLocationId.WEST_BOTTOM

# notes that affect nengo BE:
# set_weights -> remap_weights (set_weights should just modify network directly, call remap_weights() when done

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
        self.driver = bddriver.BDModelDriver()

        # neuromorph graph Input -> tags
        self.ng_input_to_tags = {}
        # tags -> neuromorph graph Output, dimension index
        self.tags_to_ng_output_dim_idx = {}
        # spike id -> pool/neuron_idx
        self.spk_to_pool_nrn_idx = {}

        self.start_hardware()

        self.driver.InitBD()

    def __del__(self):
        self.stop_hardware()

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
            out, dim, count = self.tags_to_ng_output_dim_idx[tag]
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
        spk_ids, spk_times = self.driver.RecvSpikes(CORE_ID)
        timestamps = []
        pool_ids = []
        nrn_idxs = []
        for spk_id, spk_time in zip(spk_ids, spk_times):
            pool_id, nrn_idx = self.spk_to_pool_nrn_idx[spk_id]
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
        ng_obj_to_ghw_mapper, hardware_resources, core = map_network(network)
        ghw_mapper_to_ng_obj = {v: k for k, v in ng_obj_to_ghw_mapper.items()}

        # implement core objects, calling driver
        self.implement_core(core)

        # neuromorph graph Input -> tags
        for ng_inp in network.get_inputs():
            hwr_source = ng_obj_to_ghw_mapper[ng_inp].get_resource()
            self.ng_input_to_tags[ng_inp] = hwr_source
        # neuromorph graph Output -> tags
        for ng_out in network.get_outputs():
            hwr_sink = ng_obj_to_ghw_mapper[ng_out].get_resource()
            for dim_idx, tag in enumerate(hwr_sink.in_tags):
                self.tags_to_ng_output_dim_idx[tag] = (ng_out, dim_idx)
        # spike id -> pool, neuron index
        for ng_pool in network.get_pools():
            hwr_neurons = ng_obj_to_ghw_mapper[ng_pool].get_resource()
            xmin = hwr_neurons.px_loc * core.NeuronArray_pool_size_x
            ymin = hwr_neurons.py_loc * core.NeuronArray_pool_size_y
            for x in range(hwr_neurons.x):
                for y in range(hwr_neurons.y):
                    spk_idx = xmin + x + (ymin + y)*core.NeuronArray_width 
                    pool_nrn_idx = x + y*hwr_neurons.x
                    self.spk_to_pool_nrn_idx[spk_idx] = (ng_pool, pool_nrn_idx)

    def implement_core(self, core):
        """Implements a supplied core to BD"""
        self.driver.SetMem(
            CORE_ID, bddriver.bdpars.BDMemId.PAT, np.array(core.PAT.mem.M).flatten().tolist(),  0)
        self.driver.SetMem(
            CORE_ID, bddriver.bdpars.BDMemId.TAT0, np.array(core.TAT0.mem.M).flatten().tolist(), 0)
        self.driver.SetMem(
            CORE_ID, bddriver.bdpars.BDMemId.TAT1, np.array(core.TAT1.mem.M).flatten().tolist(), 0)
        self.driver.SetMem(
            CORE_ID, bddriver.bdpars.BDMemId.MM,   np.array(core.MM.mem.M).flatten().tolist(),   0)
        self.driver.SetMem(
            CORE_ID, bddriver.bdpars.BDMemId.AM,   np.array(core.AM.mem.M).flatten().tolist(),   0)

        # open all diffusor cuts
        for tile_id in range(core.NeuronArray_height_in_tiles * core.NeuronArray_width_in_tiles):
            self.driver.OpenDiffusorAllCuts(CORE_ID, tile_id)

        for pool_allocation in core.neuron_array.pool_allocations:
            # convert minimum pool units into tile units
            x_min = pool_allocation['px']*2
            x_max = pool_allocation['px']*2+pool_allocation['pw']*2-1
            y_min = pool_allocation['py']*2
            y_max = pool_allocation['py']*2+pool_allocation['ph']*2-1
            # cut top edge
            for x_idx in range(x_min, x_max+1):
                tile_id = x_idx + y_min*core.NeuronArray_width_in_tiles
                self.driver.CloseDiffusorCut(CORE_ID, tile_id, DIFFUSOR_NORTH_LEFT)
                self.driver.CloseDiffusorCut(CORE_ID, tile_id, DIFFUSOR_NORTH_RIGHT)
            # cut left edge
            for y_idx in range(y_min, y_max+1):
                tile_id = x_min + y_idx*core.NeuronArray_width_in_tiles
                self.driver.CloseDiffusorCut(CORE_ID, tile_id, DIFFUSOR_WEST_TOP)
                self.driver.CloseDiffusorCut(CORE_ID, tile_id, DIFFUSOR_WEST_BOTTOM)
            # cut bottom edge if not at edge of neuron array
            if y_max < core.NeuronArray_height_in_tiles-1:
                for x_idx in range(x_min, x_max+1):
                    tile_id = x_idx + y_max+1*core.NeuronArray_width_in_tiles
                    self.driver.CloseDiffusorCut(CORE_ID, tile_id, DIFFUSOR_NORTH_LEFT)
                    self.driver.CloseDiffusorCut(CORE_ID, tile_id, DIFFUSOR_NORTH_RIGHT)
            # cut right edge if not at edge of neuron array
            if x_max < core.NeuronArray_width_in_tiles-1:
                for y_idx in range(y_min, y_max+1):
                    tile_id = x_max+1 + y_idx*core.NeuronArray_width_in_tiles
                    self.driver.CloseDiffusorCut(CORE_ID, tile_id, DIFFUSOR_WEST_TOP)
                    self.driver.CloseDiffusorCut(CORE_ID, tile_id, DIFFUSOR_WEST_BOTTOM)
