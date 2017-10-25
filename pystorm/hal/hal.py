"""Provides the hardware abstraction layer"""
import numpy as np
from pystorm.PyDriver import Driver
from pystorm.hal.neuromorph import map_network

# for SetMem
from pystorm.PyDriver.pystorm.bddriver.bdpars import BDMemId

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
        self.driver = Driver()

        # maps between neuromorph graph Input/Output to tag (indices?)
        self.ng_input_to_tags = None
        self.tags_to_ng_output = None

        # map between spike id and pool/neuron_idx
        self.spk_to_pool_nrn = None

        # map between neuromorph graph Connection and main memory
        self.ng_conn_to_memid = None

        # what order do I initialize and start?
        self.driver.InitBD()
        self.driver.InitFIFO(CORE_ID)
        self.start_hardware()

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
        self.driver.SetTagTrafficState(CORE_ID, en=False)

    def disable_spike_recording(self):
        """Turns off spike recording from all neurons."""
        self.driver.SetSpikeTrafficState(CORE_ID, en=False)

    def enable_output_recording(self):
        """Turns on recording from all outputs.

        These output values will go into a buffer that can be drained by calling
        get_outputs().
        """
        self.driver.SetTagTrafficState(CORE_ID, en=True)

    def enable_spike_recording(self):
        """Turns on spike recording from all neurons.

        These spikes will go into a buffer that can be drained by calling
        get_spikes().
        """
        self.driver.SetSpikeTrafficState(CORE_ID, en=True)

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

    def implement_core(self):
        """Calls a supplied driver to implement this Core to BD"""
        driver.SetMem(CORE_ID, MemId.PAT,  self.PAT.m,  0);
        driver.SetMem(CORE_ID, MemId.TAT0, self.TAT0.m, 0);
        driver.SetMem(CORE_ID, MemId.TAT1, self.TAT1.m, 0);
        driver.SetMem(CORE_ID, MemId.MM,   self.MM.m,   0);
        driver.SetMem(CORE_ID, MemId.AM,   self.AM.m,   0);



    ##############################################################################
    #                           Mapping functions                                #
    ##############################################################################

    def set_weights(self, connection, weights):
        """Sets the weights for a connection (used to set decoders)

        Parameters
        ----------
        connection: neuromorph graph Connection object
        weights: numpy array
            connection weights
        """
        mem_id = self.ng_conn_to_memid[connection]
        start_addr = None # TODO need another map?
        self.driver.SetMem(CORE_ID, mem_id, weights, start_addr)

    def map(self, network):
        """Maps a Network to low-level HAL objects and returns mapping info.

        Parameters
        ----------
        network: pystorm.hal.neuromorph.graph Network object
        """
        ng_obj_to_hw, hardware_resources, core = map_network(network)
        hw_to_ng_obj = {v: k for k, v in ng_obj_to_hw.items()}

        # implement core objects, calling driver
        core.Implement(self.driver, CORE_ID)

        # what do I set after getting this mapping?
        # tags to Input/Output mapping?
        self.ng_input_to_tags = None # TODO
        self.tags_to_ng_output = None # TODO
        self.spk_to_pool_nrn = None #TODO
        self.ng_conn_to_memid = None # TODO
