"""Provides the hardware abstraction layer"""
from time import sleep
import numpy as np
import pystorm
from pystorm.hal.neuromorph import map_network, remap_resources, graph
from pystorm.PyDriver import bddriver as bd

DIFFUSOR_NORTH_LEFT = bd.bdpars.DiffusorCutLocationId.NORTH_LEFT
DIFFUSOR_NORTH_RIGHT = bd.bdpars.DiffusorCutLocationId.NORTH_RIGHT
DIFFUSOR_WEST_TOP = bd.bdpars.DiffusorCutLocationId.WEST_TOP
DIFFUSOR_WEST_BOTTOM = bd.bdpars.DiffusorCutLocationId.WEST_BOTTOM

# notes that affect nengo BE:
# - send_inputs->set_inputs/get_outputs are different (SpikeFilter/Generator now used)
# - arguments for remap_core/implement_core have changed (use the self.last_mapped objects now)
# - graph.Pool now takes an argument with its encoders.
#   Its input dimensionality is now self.dimensions.
#   therefore making a connection to a pool will have no weight: the only kind of graph
#   connection that should have weights is a connection going into a Bucket.
#   not sure when I changed this/if Terry has incorporated changes. I had forgotten my
#   original intent.

CORE_ID = 0 # hardcoded for now

class HAL(object):
    """Hardware Abstraction Layer

    Abstracts away the details of the underlying hardware, and presents a
    unified API for higher level clients.

    Attributes
    ----------
    driver: Instance of pystorm.PyDriver Driver
    """
    def __init__(self, use_soft_driver=False):
        if use_soft_driver:
            self.driver = bd.BDModelDriver()
        else:
            self.driver = bd.Driver()

        # neuromorph graph Input -> tags
        self.ng_input_to_tags = {}
        # neuromorph graph Input -> spike generator idx
        self.ng_input_to_SG_idxs_and_tags = {}
        # spike filter idx -> Output/dim
        self.spike_filter_idx_to_output = {}
        # spike id -> pool/neuron_idx
        self.spk_to_pool_nrn_idx = {}
        okfile = "/".join(
            pystorm.__file__.split('/')[:-2])+"/FPGA/quartus/output_files/OKCoreBD.rbf"
        self.driver.SetOKBitFile(okfile)
        self.start_hardware()

        # default time resolution
        self.downstream_ns = 10000
        self.upstream_ns   = 1000000

        self.last_mapped_resources = None
        self.last_mapped_core = None

        self.init_hardware()

    def init_hardware(self):
        print("HAL: clearing hardware state")

        self.driver.InitBD()

        # DAC settings (should be pretty close to driver defaults)
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_EXC     , 512)
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_DC      , 544)
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_INH     , 512)
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_LK      , 10)
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_PD      , 40)
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_PU      , 1023)
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_DIFF_G      , 1023)
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_DIFF_R      , 500)
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , 2)

        self.driver.SetTimeUnitLen(self.downstream_ns) # 10 us downstream resolution
        self.driver.SetTimePerUpHB(self.upstream_ns) # 1 ms upstream resolution/tag binning


    def set_time_resolution(self, downstream_ns=10000, upstream_ns=1000000):
        """Controls Driver/FPGA time resolutions

        Parameters
        ==========
        downstream_ns: Controls the fineness of when the FPGA can inject inputs to BD.
                       Also controls the time resolution of the FPGA tag stream generators
                       (set_input_rates() periods will be a multiple of this).
        upstream_ns: Controls the period of upstream heartbeats from the FPGA.
                     Every upstream_ns, the FPGA reports the current time.
                     The Driver uses the most recent HB to timestamp upstream traffic.
                     Also controls the period with which the FPGA emits filtered outputs.
                     get_outputs() will have a new entry every upstream_ns.
        """
        self.driver.SetTimeUnitLen(downstream_ns) # 10 us downstream resolution
        self.driver.SetTimePerUpHB(upstream_ns) # 1 ms upstream resolution/tag binning
        self.downstream_ns = downstream_ns
        self.upstream_ns = upstream_ns

    def __del__(self):
        self.stop_hardware()

    def get_time(self):
        return self.driver.GetFPGATime()

    def reset_time(self):
        self.driver.ResetFPGATime()

    def start_hardware(self):
        """Starts the driver"""
        comm_state = self.driver.Start()
        if comm_state < 0:
            print("Comm failed to init fully, exiting")
            exit(0)

    def stop_hardware(self):
        """Stops the driver"""
        self.driver.Stop()

    ##############################################################################
    #                           Data flow functions                              #
    ##############################################################################

    def flush(self):
        """Commits any queued up traffic

        Inputs to HAL that have a time parameter (set_input_rate(s)()) are not committed
        to the hardware until flushed. After being flushed, inputs are committed
        to BD in the order of their times. This will block any subsequently flushed
        inputs until the maximum previously flushed time has elapsed.

        This call (along with the flush parameters of other calls) gives the HAL
        user some freedom in ordering their calls.
        """
        self.driver.Flush()

    def start_traffic(self, flush=True):
        self.driver.SetTagTrafficState(CORE_ID, True, flush=False)
        self.driver.SetSpikeTrafficState(CORE_ID, True, flush=flush)

    def stop_traffic(self, flush=True):
        self.driver.SetTagTrafficState(CORE_ID, False, flush=False)
        self.driver.SetSpikeTrafficState(CORE_ID, False, flush=flush)

    def enable_output_recording(self, flush=True):
        """Turns on recording from all outputs.

        These output values will go into a buffer that can be drained by calling
        get_outputs().
        """
        N_SF = self.last_mapped_core.FPGASpikeFilters.filters_used
        self.driver.SetNumSpikeFilters(CORE_ID, N_SF, flush=flush)

    def enable_spike_recording(self, flush=True):
        """Turns on spike recording from all neurons.

        These spikes will go into a buffer that can be drained by calling
        get_spikes().
        """
        self.driver.SetSpikeDumpState(CORE_ID, en=True, flush=flush)

    def disable_output_recording(self, flush=True):
        """Turns off recording from all outputs."""
        # by setting the number of spike filters to 0, the FPGA SF array
        # no longer reports any values
        self.driver.SetNumSpikeFilters(CORE_ID, 0, flush=flush)

    def disable_spike_recording(self, flush=True):
        """Turns off spike recording from all neurons."""
        self.driver.SetSpikeDumpState(CORE_ID, en=False, flush=flush)

    def get_overflow_counts(self):
        """prints the total number of FIFO overflows"""
        o0, o1 = self.driver.GetFIFOOverflowCounts(CORE_ID)
        return o0 + o1

    def get_outputs(self, timeout=1000):
        """Returns all binned output tags gathered since this was last called.

        Data format: a numpy array of : [(time, output, dim, counts), ...]
        Timestamps are in nanoseconds
        Counts are the number of tags received since the last report:
            Every FPGA time unit, the Spike Filter array loops through N_SF
            filters, reports the tallied tag counts since the last report,
            and resets each count to 0

        Whether or not you get return values is enabled/disabled by
        enable/disable_output_recording()
        """
        filt_idxs, filt_states, times = self.driver.RecvSpikeFilterStates(CORE_ID, timeout)

        # for now, working in count mode
        # the filter state is a count, and it is signed
        counts = []
        for f in filt_states:
            if f > 2**26 - 1:
                to_append = f - 2**27
            else:
                to_append = f

            if abs(to_append) < 100:
                counts.append(to_append)
            else:
                print("discarding absurdly large spike filter value, probably a glitch")
                counts.append(0)

        outputs = [self.spike_filter_idx_to_output[filt_idx][0] for filt_idx in filt_idxs]
        dims = [self.spike_filter_idx_to_output[filt_idx][1] for filt_idx in filt_idxs]

        return np.array([times, outputs, dims, counts]).T

    def get_spikes(self):
        """Returns all the pending spikes gathered since this was last called.

        Data format: numpy array: [(timestamp, pool_id, neuron_index), ...]
        Timestamps are in microseconds
        """
        spk_ids, spk_times = self.driver.RecvXYSpikes(CORE_ID)
        timestamps = []
        pool_ids = []
        nrn_idxs = []
        for spk_id, spk_time in zip(spk_ids, spk_times):
            if spk_id not in self.spk_to_pool_nrn_idx:
                print("got out-of-bounds spike from neuron id", spk_id)
            else:
                pool_id, nrn_idx = self.spk_to_pool_nrn_idx[spk_id]
                pool_ids.append(pool_id)
                nrn_idxs.append(nrn_idx)
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

        WARNING: If <flush> is True, calling this will block traffic until the max <time>
        provided has passed!
        If you're queing up rates, make sure you call this in the order of the times
        """
        if flush is False:
            assert(False and "there's currently a Driver bug with set_input_rate flush=False")

        # every FPGA time unit, the FPGA loops through the spike generators and decides whether or
        # not to emit a tag for each one. The SGs can be reprogrammed to change their individual rates
        # and to target different tags
        gen_idx = self.ng_input_to_SG_idxs_and_tags[inp][0][dim]
        out_tag = self.ng_input_to_SG_idxs_and_tags[inp][1][dim]

        self.driver.SetSpikeGeneratorRates(CORE_ID, [gen_idx], [out_tag], [rate], time, True)

        # XXX there is a bug (probably with how inputs are sorted by time)
        # calling SetSpikeGeneratorRates with more than one element per list
        # or not calling flush after each invocation will cause undefined behavior

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

        WARNING: If <flush> is True, calling this will effectively block traffic until the max <time>
        provided has passed!
        """
        assert len(inputs) == len(dims) == len(rates)

        #gen_idxs = [self.ng_input_to_SG_idxs_and_tags[inp][0][dim] for inp, dim in zip(inputs, dims)]
        #out_tags = [self.ng_input_to_SG_idxs_and_tags[inp][1][dim] for inp, dim in zip(inputs, dims)]
        #self.driver.SetSpikeGeneratorRates(CORE_ID, gen_idxs, out_tags, rates, time, flush)

        # XXX see comment on set_input_rate(). For now this doesn't work as intended
        # this should work (with some negligible additional latency)
        for inp, dim, rate in zip(inputs, dims, rates):
            self.set_input_rate(inp, dim, rate, time, flush)


    ##############################################################################
    #                           Mapping functions                                #
    ##############################################################################

    def remap_weights(self):
        """Call a subset of map()'s functionality to reprogram weights
        that have been modified in the network objects
        """

        # generate a new core based on the new allocation, but assigning the new weights
        core = remap_resources(self.last_mapped_resources)
        self.last_mapped_core = core

        self.implement_core()


    def map(self, network):
        """Maps a Network to low-level HAL objects and returns mapping info.

        Parameters
        ----------
        network: pystorm.hal.neuromorph.graph Network object
        """
        print("HAL: doing logical mapping")
        ng_obj_to_ghw_mapper, hardware_resources, core = map_network(network, verbose=True)

        self.last_mapped_resources = hardware_resources
        self.last_mapped_core = core

        # implement core objects, calling driver
        print("HAL: programming mapping results to hardware")
        self.implement_core()

        # clear dictionaries in case we're calling map more than once
        # neuromorph graph Input -> tags
        self.ng_input_to_tags = {}
        # neuromorph graph Input -> spike generator idx
        self.ng_input_to_SG_idxs_and_tags = {}
        # spike filter idx -> Output/dim
        self.spike_filter_idx_to_output = {}
        # spike id -> pool/neuron_idx
        self.spk_to_pool_nrn_idx = {}

        # neuromorph graph Input -> tags
        for ng_inp in network.get_inputs():
            hwr_source = ng_obj_to_ghw_mapper[ng_inp].get_resource()
            #print(ng_inp, "->", (hwr_source.generator_idxs, hwr_source.out_tags))
            self.ng_input_to_SG_idxs_and_tags[ng_inp] = (hwr_source.generator_idxs, hwr_source.out_tags)
        # spike filter idx -> Output/dim
        for ng_out in network.get_outputs():
            hwr_sink = ng_obj_to_ghw_mapper[ng_out].get_resource()
            for dim_idx, filt_idx in enumerate(hwr_sink.filter_idxs):
                self.spike_filter_idx_to_output[filt_idx] = (ng_out, dim_idx)
        for ng_pool in network.get_pools():
            hwr_neurons = ng_obj_to_ghw_mapper[ng_pool].get_resource()[1] # two resources for pool, [TATTapPoint and Neurons]
            xmin = hwr_neurons.px_loc * core.NeuronArray_pool_size_x
            ymin = hwr_neurons.py_loc * core.NeuronArray_pool_size_y
            for x in range(hwr_neurons.x):
                for y in range(hwr_neurons.y):
                    spk_idx = xmin + x + (ymin + y)*core.NeuronArray_width
                    pool_nrn_idx = x + y*hwr_neurons.x
                    self.spk_to_pool_nrn_idx[spk_idx] = (ng_pool, pool_nrn_idx)

        #print('spike mapper')
        #for k in self.spk_to_pool_nrn_idx:
        #    print(k, ":", self.spk_to_pool_nrn_idx[k][1])


    def dump_core(self):
        print("PAT")
        print(self.driver.DumpMem(CORE_ID, bd.bdpars.BDMemId.PAT))
        print("TAT0")
        print(self.driver.DumpMem(CORE_ID, bd.bdpars.BDMemId.TAT0)[0:10])
        print("TAT1")
        print(self.driver.DumpMem(CORE_ID, bd.bdpars.BDMemId.TAT1)[0:10])
        print("AM")
        print(self.driver.DumpMem(CORE_ID, bd.bdpars.BDMemId.AM)[0:10])
        print("MM")
        print(self.driver.DumpMem(CORE_ID, bd.bdpars.BDMemId.MM)[0:10])

    def implement_core(self):
        """Implements a core that resulted from map_network. This is called by map and remap_weights"""

        # start with a clean slate
        self.init_hardware()

        core = self.last_mapped_core

        # datapath memory programming

        self.driver.SetMem(
            CORE_ID, bd.bdpars.BDMemId.PAT, np.array(core.PAT.mem.M).flatten().tolist(), 0)
        self.driver.SetMem(
            CORE_ID, bd.bdpars.BDMemId.TAT0, np.array(core.TAT0.mem.M).flatten().tolist(), 0)
        self.driver.SetMem(
            CORE_ID, bd.bdpars.BDMemId.TAT1, np.array(core.TAT1.mem.M).flatten().tolist(), 0)
        self.driver.SetMem(
            CORE_ID, bd.bdpars.BDMemId.AM, np.array(core.AM.mem.M).flatten().tolist(), 0)
        self.driver.SetMem(
            CORE_ID, bd.bdpars.BDMemId.MM, np.array(core.MM.mem.M).flatten().tolist(), 0)

        # open diffusor around pools

        for tile_id in range(core.NeuronArray_height_in_tiles * core.NeuronArray_width_in_tiles):
            self.driver.OpenDiffusorAllCuts(CORE_ID, tile_id)

        for pool_allocation in core.neuron_array.pool_allocations:
            # convert minimum pool units into tile units
            x_min = pool_allocation['px']*2
            x_max = pool_allocation['px']*2+pool_allocation['pw']*2-1
            y_min = pool_allocation['py']*2
            y_max = pool_allocation['py']*2+pool_allocation['ph']*2-1

            #print("x_min", x_min)
            #print("x_max", x_max)
            #print("y_min", y_min)
            #print("y_max", y_max)

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

            # enable somas inside pool
            # remember, x_min/x_max are tile units, 16 neurons per tile
            for nrn_y_idx in range(4*y_min, 4*(y_max+1)):
                for nrn_x_idx in range(4*x_min, 4*(x_max+1)):
                    #print("enabling soma", nrn_y_idx, nrn_x_idx)
                    self.driver.EnableSomaXY(CORE_ID, nrn_x_idx, nrn_y_idx)

        # enable used synapses
        for tx, ty in core.neuron_array.syns_used:
            #print("enabling synapse", tx, ty)
            self.driver.EnableSynapseXY(CORE_ID, tx, ty)

        # set spike filter decay constant
        # the following sets the filters to "count mode"
        # exponential decay is also possible
        self.driver.SetSpikeFilterDecayConst(CORE_ID, 0)
        self.driver.SetSpikeFilterIncrementConst(CORE_ID, 1)

        # remove any evidence of old network in driver queues
        print("HAL: clearing queued-up outputs")
        self.driver.ClearOutputs()

        # voodoo sleep, (wait for everything to go in)
        sleep(2)

def parse_hal_binned_tags(hal_binned_tags):
    """Parses the tag information from the output of HAL

    Parameters
    ----------
    hal_binned_tags: output of HAL.get_outputs() (list of tuples)

    Returns a dictionary:
        [output_id][dimension] = list of (times, count) tuples
    """
    parsed_tags = {}
    for time, output_id, dim, count in hal_binned_tags:
        if output_id not in parsed_tags:
            parsed_tags[output_id] = {}
        if dim not in parsed_tags[output_id]:
            parsed_tags[output_id][dim] = []
        parsed_tags[output_id][dim].append((time, count))
    return parsed_tags

def parse_hal_spikes(hal_spikes):
    """Parses the tag information from the output of HAL

    Parameters
    ----------
    hal_spikes: output of HAL.get_spikes() (list of tuples)

    Returns a dictionary:
        [pool][neuron] = list of (times, 1) tuples
        The 1 is for consistency with the return of parse_hal_tags
    """
    parsed_spikes = {}
    for time, pool, neuron in hal_spikes:
        if pool not in parsed_spikes:
            parsed_spikes[pool] = {}
        if neuron not in parsed_spikes[pool]:
            parsed_spikes[pool][neuron] = []
        parsed_spikes[pool][neuron].append((time, 1))
    return parsed_spikes

def bin_tags_spikes(tagspikes, bin_time_boundaries, time_scale=1e-9):
    """Bin tags or spikes into rates

    Parameters
    ----------
    tagspikes: dict returned by parse_hal_tags or parse_hal_spikes
    bin_time_boundaries: list-like of floats
        boundaries of the time bins
    time_scale: scaling factor to convert bin times into seconds
    """
    n_bins = len(bin_time_boundaries) - 1
    bin_sizes = np.diff(bin_time_boundaries)*time_scale
    # initialize A matrices
    activity_matrices = {}
    for obj in tagspikes:
        if isinstance(obj, graph.Pool):
            max_idx = obj.n_neurons
        elif isinstance(obj, graph.Output):
            max_idx = obj.dimensions
        activity_matrices[obj] = np.zeros((max_idx, n_bins)).astype(int)

    # bin spikes, filling in A
    for obj, obj_data in tagspikes.items():
        for obj_idx, obj_idx_data in obj_data.items():
            times_and_counts = np.array(obj_idx_data)
            times = times_and_counts[:, 0]
            counts = times_and_counts[:, 1]
            binned_counts = np.histogram(
                times, bin_time_boundaries, weights=counts)[0]
            activity_matrices[obj][obj_idx, :] = binned_counts / bin_sizes
    return activity_matrices
