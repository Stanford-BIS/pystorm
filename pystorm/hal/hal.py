"""Provides the hardware abstraction layer"""
from time import sleep
import numpy as np
import pystorm
from pystorm.PyDriver import bddriver as bd
from pystorm.hal.neuromorph.core import Core
from pystorm.hal.neuromorph.core_pars import CORE_PARAMETERS

import logging

logger = logging.getLogger(__name__)

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

class Singleton:
    """Decorator class ensuring that at most one instance of a decorated class exists"""
    def __init__(self,klass):
        self.klass = klass
        self.instance = None
    def __call__(self,*args,**kwds):
        if self.instance == None:
            self.instance = self.klass(*args,**kwds)
        return self.instance

@Singleton
class HAL:
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

        okfile = "/".join(
            pystorm.__file__.split('/')[:-2])+"/FPGA/quartus/output_files/OKCoreBD.rbf"
        self.driver.SetOKBitFile(okfile)
        self.start_hardware()

        # default time resolution
        self.downstream_ns = 10000
        self.upstream_ns   = 1000000

        self.last_mapped_network = None
        self.last_mapped_core = None

        self.init_hardware()

    def init_hardware(self):
        logger.info("HAL: clearing hardware state")

        # stop spikes before resetting
        #self.stop_all_inputs()

        self.driver.InitBD()

        # DAC settings (should be pretty close to driver defaults)

        # magnitude of the three synapse inputs (can be used to balance exc/inh)
        # there are scale factors on each of the outputs
        # excitatory/8 - DC/16 is the height of the excitatory synapse pulse
        # DC/16 - inhibitory/128 is the height of the inhibitory synapse pulse
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_EXC     , 512) # excitatory level, scaled 1/8
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_DC      , 544) # DC baseline level, scaled 1/16
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_INH     , 512) # inhibitory level, scaled 1/128

        # 1/DAC_SYN_LK ~ synaptic time constant, 10 is around .1 ms
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_LK      , 10)

        # synapse pulse extender rise time/fall time
        # 1/DAC_SYN_PD ~ synapse PE fall time 
        # 1/DAC_SYN_PU ~ synapse PE rise time
        # the synapse is "on" during the fall, and "off" during the rise
        # making the rise longer doesn't have much of a practical purpose
        # when saturated, fall time/rise time is the peak on/off duty cycle (proportionate to synaptic strength)
        # be careful setting these too small, you don't want to saturate the synapse
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_PD      , 40)
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_PU      , 1024)

        # the ratio of DAC_DIFF_G / DAC_DIFF_R controls the diffusor spread
        # lower ratio is more spread out
        # R ~ conductance of the "sideways" resistors, G ~ conductance of the "downwards" resistors
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_DIFF_G      , 1024)
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_DIFF_R      , 500)

        # 1/DAC_SOMA_REF ~ soma refractory period, 10 is around 1 ms
        self.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SOMA_REF    , 10)

        # DAC_SOMA_OFFSET scales the bias twiddle bits
        # Ben says that increasing this beyond 10 could cause badness
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
        assert comm_state >= 0, "Comm failed to init"

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
        """Start hardware's internal traffic flow"""
        self.driver.SetTagTrafficState(CORE_ID, True, flush=False)
        self.driver.SetSpikeTrafficState(CORE_ID, True, flush=flush)

    def stop_traffic(self, flush=True):
        """Stop  hardware's internal traffic flow"""
        self.driver.SetTagTrafficState(CORE_ID, False, flush=False)
        self.driver.SetSpikeTrafficState(CORE_ID, False, flush=flush)

    def enable_output_recording(self, flush=True):
        """Turns on recording from all outputs.

        These output values are binned and go into a buffer
        that can be drained by calling get_outputs().
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

        outputs, dims, counts = self.last_mapped_network.translate_tags(filt_idxs, filt_states)

        return np.array([times, outputs, dims, counts]).T

    def get_spikes(self):
        """Returns all the pending spikes gathered since this was last called.

        Data format: numpy array: [(timestamp, pool_id, neuron_index), ...]
        Timestamps are in microseconds
        """
        spk_ids, spk_times = self.driver.RecvXYSpikes(CORE_ID)

        pool_ids, nrn_idxs, filtered_spk_times = self.last_mapped_network.translate_spikes(spk_ids, spk_times)

        ret_data = np.array([filtered_spk_times, pool_ids, nrn_idxs]).T
        return ret_data
    
    def stop_all_inputs(self, time=0, flush=True):
        """Stop all tag stream generators"""

        if self.last_mapped_core is not None:
            num_gens = self.last_mapped_core.FPGASpikeGenerators.gens_used
            if num_gens > 0:
                for gen_idx in range(num_gens):
                    # it's ok to set tag out to 0, if you turn up the rate later, it'll program the right tag
                    self.driver.SetSpikeGeneratorRates(CORE_ID, [gen_idx], [0], [0], time, True)

    def set_input_rate(self, inp, dim, rate, time=0, flush=True):
        """Controls a single tag stream generator's rate (on the FPGA)

        On startup, all rates are 0.
        Every FPGA time unit, the FPGA loops through the spike generators
        and decides whether or not to emit a tag for each one.
        The SGs can be reprogrammed to change their individual rates and targets

        inp: Input object
        dim : int
            dimension within the Input object to target
        rate: int
            desired tag rate for the Input/dimension in Hz
        time: int (default=0)
            time to send inputs, in microseconds. 0 means immediately
        flush: bool (default true)
            whether to flush the inputs through the driver immediately.
            If you're making several calls, it may be advantageous to only flush
            the last one

        WARNING: If <flush> is True, calling this will block traffic until the max <time>
        provided has passed!
        If you're queing up rates, make sure you call this in the order of the times
        """

        self.set_input_rates([inp], [dim], [rate], time, flush)

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
            whether to flush the inputs through the driver immediately.
            If you're making several calls, it may be advantageous to only flush
            the last one

        WARNING: If <flush> is True, calling this will block traffic until the max <time>
        provided has passed!
        """
        assert len(inputs) == len(dims) == len(rates)

        gen_idxs = [
            inp.generator_idxs[dim] for inp, dim in zip(inputs, dims)]
        out_tags = [
            inp.generator_out_tags[dim] for inp, dim in zip(inputs, dims)]

        self.driver.SetSpikeGeneratorRates(CORE_ID, gen_idxs, out_tags, rates, time, flush)


    ##############################################################################
    #                           Mapping functions                                #
    ##############################################################################

    def remap_weights(self):
        """Reprogram weights that have been modified in the network objects
        Effectively calls map again, but keeping pool allocations
        """

        # this call is deprecated, has the following effect
        self.map(self.last_mapped_network, remap=True)

    def map(self, network, remap=False, verbose=False):
        """Maps a Network to low-level HAL objects and returns mapping info.

        Parameters
        ----------
        network: pystorm.hal.neuromorph.graph Network object
        remap: reuse as much of the previous mapping as possible (e.g. pools will retain their physical locations on the chip)
        """
        logger.info("HAL: doing logical mapping")

        # should eventually get CORE_PARAMETERS from the driver itself (BDPars)
        core = network.map(CORE_PARAMETERS, keep_pool_mapping=remap, verbose=verbose)

        self.last_mapped_network = network
        self.last_mapped_core = core

        # implement core objects, calling driver
        logger.info("HAL: programming mapping results to hardware")
        self.implement_core()

    def dump_core(self):
        logger.info("PAT")
        logger.info(self.driver.DumpMem(CORE_ID, bd.bdpars.BDMemId.PAT))
        logger.info("TAT0")
        logger.info(self.driver.DumpMem(CORE_ID, bd.bdpars.BDMemId.TAT0)[0:10])
        logger.info("TAT1")
        logger.info(self.driver.DumpMem(CORE_ID, bd.bdpars.BDMemId.TAT1)[0:10])
        logger.info("AM")
        logger.info(self.driver.DumpMem(CORE_ID, bd.bdpars.BDMemId.AM)[0:10])
        logger.info("MM")
        logger.info(self.driver.DumpMem(CORE_ID, bd.bdpars.BDMemId.MM)[0:10])

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

        # connect diffusor around pools

        for tile_id in range(core.NeuronArray_height_in_tiles * core.NeuronArray_width_in_tiles):
            self.driver.CloseDiffusorAllCuts(CORE_ID, tile_id)

        for pool, pool_allocation in core.neuron_array.pool_allocations.items():
            # convert minimum pool units into tile units
            # a pool consists of 4 (2x2 tiles)
            # XXX this constant of 2 shouldn't be hardcoded
            x_min = pool_allocation['px']*2
            y_min = pool_allocation['py']*2
            x_max = x_min + pool_allocation['pw']*2
            y_max = y_min + pool_allocation['ph']*2


            logger.debug("pool {}".format(str(pool)))
            logger.debug("    px_min {}".format(x_min))
            logger.debug("    px_max {}".format(x_max))
            logger.debug("    py_min {}".format(y_min))
            logger.debug("    py_max {}".format(y_max))

            # cut top edge
            for x_idx in range(x_min, x_max):
                self.driver.OpenDiffusorCutXY(CORE_ID, x_idx, y_max-1, DIFFUSOR_NORTH_LEFT)
                self.driver.OpenDiffusorCutXY(CORE_ID, x_idx, y_max-1, DIFFUSOR_NORTH_RIGHT)
            # cut left edge
            for y_idx in range(y_min, y_max):
                self.driver.OpenDiffusorCutXY(CORE_ID, x_min, y_idx, DIFFUSOR_WEST_TOP)
                self.driver.OpenDiffusorCutXY(CORE_ID, x_min, y_idx, DIFFUSOR_WEST_BOTTOM)
            # cut bottom edge if not at edge of neuron array
            if y_max < core.NeuronArray_height_in_tiles-1:
                for x_idx in range(x_min, x_max):
                    self.driver.OpenDiffusorCutXY(CORE_ID, x_idx, y_max, DIFFUSOR_NORTH_LEFT)
                    self.driver.OpenDiffusorCutXY(CORE_ID, x_idx, y_max, DIFFUSOR_NORTH_RIGHT)
            # cut right edge if not at edge of neuron array
            if x_max < core.NeuronArray_width_in_tiles-1:
                for y_idx in range(y_min, y_max):
                    self.driver.OpenDiffusorCutXY(CORE_ID, x_max, y_idx, DIFFUSOR_WEST_TOP)
                    self.driver.OpenDiffusorCutXY(CORE_ID, x_max, y_idx, DIFFUSOR_WEST_BOTTOM)

        # enable somas inside pool
        # remember, x_min/x_max are tile units, 16 neurons per tile
        assert(core.NeuronArray_width == core.neuron_array.nrns_used.shape[1])
        assert(core.NeuronArray_height == core.neuron_array.nrns_used.shape[0])
        for x in range(core.NeuronArray_width):
            for y in range(core.NeuronArray_height):
                if core.neuron_array.nrns_used[y, x] == 1:
                    logger.debug("enabling soma %d, %d (x, y)", x, y)
                    self.driver.EnableSomaXY(CORE_ID, x, y)

        # enable used synapses
        for tx, ty in core.neuron_array.syns_used:
            logger.debug("enabling synapse", tx, ty)
            self.driver.EnableSynapseXY(CORE_ID, tx, ty)

        # set gain and bias twiddle bits
        assert(core.NeuronArray_width == core.neuron_array.gain_divisors.shape[1])
        assert(core.NeuronArray_height == core.neuron_array.gain_divisors.shape[0])
        assert(core.NeuronArray_width == core.neuron_array.biases.shape[1])
        assert(core.NeuronArray_height == core.neuron_array.biases.shape[0])

        gain_ids = [
                bd.bdpars.SomaGainId.ONE,
                bd.bdpars.SomaGainId.ONE_HALF,
                bd.bdpars.SomaGainId.ONE_THIRD,
                bd.bdpars.SomaGainId.ONE_FOURTH]
        bias_ids = [
                bd.bdpars.SomaOffsetMultiplierId.ZERO,
                bd.bdpars.SomaOffsetMultiplierId.ONE,
                bd.bdpars.SomaOffsetMultiplierId.TWO,
                bd.bdpars.SomaOffsetMultiplierId.THREE]
        bias_signs = [
                bd.bdpars.SomaOffsetSignId.NEGATIVE,
                bd.bdpars.SomaOffsetSignId.POSITIVE]

        for x in range(core.NeuronArray_width):
            for y in range(core.NeuronArray_height):
                gain_div_val = core.neuron_array.gain_divisors[y, x]
                gain_id = gain_ids[gain_div_val - 1]
                self.driver.SetSomaGainXY(CORE_ID, x, y, gain_id)

                bias_val = core.neuron_array.biases[y, x]
                bias_sign_id = bias_signs[int(bias_val > 0)]
                bias_id = bias_ids[abs(bias_val)]
                self.driver.SetSomaOffsetSignXY(CORE_ID, x, y, bias_sign_id)
                self.driver.SetSomaOffsetMultiplierXY(CORE_ID, x, y, bias_id)

        # set spike filter decay constant
        # the following sets the filters to "count mode"
        # exponential decay is also possible
        self.driver.SetSpikeFilterDecayConst(CORE_ID, 0)
        self.driver.SetSpikeFilterIncrementConst(CORE_ID, 1)

        # remove any evidence of old network in driver queues
        logger.info("HAL: clearing queued-up outputs")
        self.driver.ClearOutputs()

        # voodoo sleep, (wait for everything to go in)
        sleep(2)

def parse_hal_binned_tags(hal_binned_tags):
    """Parses the tag information from the output of HAL

    Parameters
    ----------
    hal_binned_tags: output of HAL.get_outputs() (list of tuples)

    Returns a nested dictionary:
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

    Returns a nested dictionary:
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
        if isinstance(obj, pystorm.hal.neuromorph.graph.Pool):
            max_idx = obj.n_neurons
        elif isinstance(obj, pystorm.hal.neuromorph.graph.Output):
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
