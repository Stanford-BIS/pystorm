"""Provides the hardware abstraction layer"""
import numpy as np
from pystorm.hal.neuromorph import map_network, remap_resources
from pystorm.PyDriver import bddriver

DIFFUSOR_NORTH_LEFT = bddriver.bdpars.DiffusorCutLocationId.NORTH_LEFT
DIFFUSOR_NORTH_RIGHT = bddriver.bdpars.DiffusorCutLocationId.NORTH_RIGHT
DIFFUSOR_WEST_TOP = bddriver.bdpars.DiffusorCutLocationId.WEST_TOP
DIFFUSOR_WEST_BOTTOM = bddriver.bdpars.DiffusorCutLocationId.WEST_BOTTOM

# notes that affect nengo BE:
# send_inputs->set_inputs/get_outputs are different (SpikeFilter/Generator now used)
# arguments for remap_core/implement_core have changed (use the self.last_mapped objects now)

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
        # neuromorph graph Input -> spike generator idx
        self.ng_input_to_SG_idxs_and_tags = {}
        # spike filter idx -> Output/dim
        self.spike_filter_idx_to_output = {}
        # spike id -> pool/neuron_idx
        self.spk_to_pool_nrn_idx = {}

        self.start_hardware()

        self.driver.InitBD()

        self.last_mapped_resources = None
        self.last_mapped_core = None

    def __del__(self):
        self.stop_hardware()

    def start_hardware(self):
        """Starts the driver"""
        comm_state = self.driver.Start()
        if comm_state != 0:
            print("Comm failed to init fully, exiting")
            exit(0)

    def stop_hardware(self):
        """Stops the driver"""
        self.driver.Stop()

    ##############################################################################
    #                           Data flow functions                              #
    ##############################################################################

    def disable_output_recording(self):
        """Turns off recording from all outputs."""
        self.driver.SetNumSpikeFilters(CORE_ID, 0)

    def disable_spike_recording(self):
        """Turns off spike recording from all neurons."""
        self.driver.SetSpikeDumpState(CORE_ID, en=False)

    def enable_output_recording(self):
        """Turns on recording from all outputs.

        These output values will go into a buffer that can be drained by calling
        get_outputs().
        """
        N_SF = self.last_mapped_core.FPGASpikeFilters.filters_used
        self.driver.SetNumSpikeFilters(CORE_ID, N_SF)

    def enable_spike_recording(self):
        """Turns on spike recording from all neurons.

        These spikes will go into a buffer that can be drained by calling
        get_spikes().
        """
        self.driver.SetSpikeDumpState(CORE_ID, en=True)

    def get_outputs(self, timeout=1000):
        """Returns all pending output values gathered since this was last called.

        Data format: a numpy array of : [(output, dim, counts, time), ...]
        Timestamps are in microseconds
        """
        words, times = self.driver.RecvSpikeFilterStates(CORE_ID, timeout)
        
        filt_idxs = [bddriver.GetField(w, bddriver.SFWORD.FILTIDX) for w in words]
        counts = [bddriver.GetField(w, bddriver.SFWORD.STATE) for w in words]

        outputs = [self.spike_filter_idx_to_output[filt_idx][0] for filt_idx in filt_idxs]
        dims = [self.spike_filter_idx_to_output[filt_idx][1] for filt_idx in filt_idxs]

        return np.array([outputs, dims, counts, times]).T

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

    def set_input_rate(self, inp, dim, rate, time=0, flush=True):
        """Controls a single tag stream generator's rate (on the FPGA)

        on startup, all rates are 0

        inp: Input object
        dim : ints
            dimensions within each Input object to send to
        rate: ints
            desired tag rate for each Input/dimension in Hz
        time: int (default=0)
            time to send inputs, in microseconds. 0 means immediately
        flush: bool (default true)
            whether or not to flush the inputs through the driver immediately.
            If you're making several calls, it may be advantageous to only flush
            the last one
        """
        gen_idx = self.ng_input_to_SG_idxs_and_tags[inp][0][dim]
        out_tag = self.ng_input_to_SG_idxs_and_tags[inp][1][dim]
        self.driver.SetSpikeGeneratorRates(CORE_ID, [gen_idx], [out_tag], [rate], time, flush)

    def set_input_rates(self, inputs, dims, rates, time=0, flush=True):
        """Controls tag stream generators rates (on the FPGA)

        on startup, all rates are 0

        inputs: list of Input object
        dims : list of ints
            dimensions within each Input object to send to
        rates: list of ints
            desired tag rate for each Input/dimension in Hz
        time: int (default=0)
            time to send inputs, in microseconds. 0 means immediately
        flush: bool (default true)
            whether or not to flush the inputs through the driver immediately.
            If you're making several calls, it may be advantageous to only flush
            the last one
        """
        assert len(inputs) == len(dims) == len(rates)

        gen_idxs = [self.ng_input_to_SG_idxs_and_tags[inp][0][dim] for inp, dim in zip(inputs, dims)]
        out_tags = [self.ng_input_to_SG_idxs_and_tags[inp][1][dim] for inp, dim in zip(inputs, dims)]
        self.driver.SetSpikeGeneratorRates(CORE_ID, gen_idxs, out_tags, rates, time, flush)


    ##############################################################################
    #                           Mapping functions                                #
    ##############################################################################

    def remap_weights(self, network):
        """Call a subset of map()'s functionality to reprogram weights
        that have been modified in the network objects

        Parameters
        ----------
        network: pystorm.hal.neuromorph.graph Network object
        hardware_resources: output of previous map() call
        """

        # network is unused: hardware_resources references it

        # generate a new core based on the new allocation, but assigning the new weights
        core = remap_resources(core.last_mapped_resources)
        self.implement_core(core)

        self.last_mapped_core = core

    def map(self, network):
        """Maps a Network to low-level HAL objects and returns mapping info.

        Parameters
        ----------
        network: pystorm.hal.neuromorph.graph Network object
        """
        ng_obj_to_ghw_mapper, hardware_resources, core = map_network(network, verbose=True)

        # implement core objects, calling driver
        self.implement_core(core)

        # neuromorph graph Input -> tags
        for ng_inp in network.get_inputs():
            hwr_source = ng_obj_to_ghw_mapper[ng_inp].get_resource()
            self.ng_input_to_SG_idxs_and_tags[ng_inp] = (hwr_source.generator_idxs, hwr_source.out_tags)
        # spike filter idx -> Output/dim
        for ng_out in network.get_outputs():
            hwr_sink = ng_obj_to_ghw_mapper[ng_out].get_resource()
            for dim_idx, filt_idx in enumerate(hwr_sink.filter_idxs):
                self.spike_filter_idx_to_output[filt_idx] = (ng_out, dim_idx)
        for ng_pool in network.get_pools():
            hwr_neurons = ng_obj_to_ghw_mapper[ng_pool].get_resource()
            xmin = hwr_neurons.px_loc * core.NeuronArray_pool_size_x
            ymin = hwr_neurons.py_loc * core.NeuronArray_pool_size_y
            for x in range(hwr_neurons.x):
                for y in range(hwr_neurons.y):
                    spk_idx = xmin + x + (ymin + y)*core.NeuronArray_width
                    pool_nrn_idx = x + y*hwr_neurons.x
                    self.spk_to_pool_nrn_idx[spk_idx] = (ng_pool, pool_nrn_idx)

        self.last_mapped_resources = hardware_resources
        self.last_mapped_core = core

    def implement_core(self):
        """Implements a supplied core to BD"""

        core = self.last_mapped_core

        # datapath memory programming 

        self.driver.SetMem(
            CORE_ID, bddriver.bdpars.BDMemId.PAT, np.array(core.PAT.mem.M).flatten().tolist(), 0)
        self.driver.SetMem(
            CORE_ID, bddriver.bdpars.BDMemId.TAT0, np.array(core.TAT0.mem.M).flatten().tolist(), 0)
        self.driver.SetMem(
            CORE_ID, bddriver.bdpars.BDMemId.TAT1, np.array(core.TAT1.mem.M).flatten().tolist(), 0)
        self.driver.SetMem(
            CORE_ID, bddriver.bdpars.BDMemId.MM, np.array(core.MM.mem.M).flatten().tolist(), 0)
        self.driver.SetMem(
            CORE_ID, bddriver.bdpars.BDMemId.AM, np.array(core.AM.mem.M).flatten().tolist(), 0)

        # neuron tile config SRAM: open diffusor cuts

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

        # set spike filter decay constant 
        # the following sets the filters to "count mode"
        # exponential decay is also possible
        self.driver.SetSpikeFilterDecayConst(CORE_ID, 0)
        self.driver.SetSpikeFilterIncrementConst(CORE_ID, 1)
