"""Provides the hardware abstraction layer"""
from pystorm.PyDriver import Driver
from pystorm.hal.neuromorph import map_network

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

        # what order do I initialize and start?
        self.driver.InitBD()
        self.driver.InitFIFO()
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
        self.driver.SetTagTrafficState(core_id, en=False)

    def disable_spike_recording(self):
        """Turns off spike recording from all neurons."""
        self.driver.SetSpikeTrafficState(core_id, en=False)

    def enable_output_recording(self):
        """Turns on recording from all outputs.

        These output values will go into a buffer that can be drained by calling
        get_outputs().
        """
        self.driver.SetTagTrafficState(core_id, en=True)

    def enable_spike_recording(self):
        """Turns on spike recording from all neurons.

        These spikes will go into a buffer that can be drained by calling
        get_spikes().
        """
        self.driver.SetSpikeTrafficState(core_id, en=True)

    def get_outputs(self):
        """Returns all pending output values gathered since this was last called.

        Data format: a numpy array of : [(output, dim, counts, time), ...]
        Timestamps are in microseconds
        """
        tagdata = self.driver.RecvTags(core_id)
        return tagdata

    def get_spikes(self):
        """Returns all the pending spikes gathered since this was last called.

        Data format: numpy array: [(timestamp, pool_id, neuron_index), ...]
        Timestamps are in microseconds
        """
        spkdata = self.driver.RecvSpikes(core_id)
        return spkdata

    # def send_spikes(self, target, spikes, times):
    #     """Send pregenerated spikes to the given target"""
    #     self.driver.SendSpikes(core_id, spikes, times)

    def send_inputs(self, inp, dim, counts, times):
        """Sends pregenerated tags to the given target
        
        input: Input object
        dim : which dim
        counts: list of number of spikes for each time
        times: list of times
        """
        self.driver.SendTags(core_id, tags, times)


    ##############################################################################
    #                           Mapping functions                                #
    ##############################################################################

    def set_weights(self, connection, weights):
        """Sets the weights for a connection (used to set decoders)."""
        self.Driver.SetMem(core_id, mem_id, data, start_addr)

    def map(self, network):
        """Maps a Network to low-level HAL objects and returns mapping info.

        Parameters
        ----------
        network: pystorm.hal.neuromorph.graph Network object
        """
        mapped_network, core = map_network(network)

