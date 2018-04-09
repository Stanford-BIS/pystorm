"""Do spikes output directly from soma match spikes routed through acc with weight 1?

Vary DAC current driving soma bias to vary spike rates
"""
import time
import numpy as np

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.PyDriver import bddriver as bd
from pystorm.hal.hal import parse_hal_binned_tags, parse_hal_spikes, bin_tags_spikes
HAL = HAL()

CORE = 0
BIAS_OFFSETS = [128, 256, 512, 1024]
BIAS_REF = 1024
NNEURONS = 64
RUN_TIME = 15.
REL_ERROR_TOLERANCE = 0.01  # tolerable relative error

def set_analog_config():
    """Sets the DACs and soma configurations"""
    HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_REF, BIAS_REF)
    for i in range(4096):
        HAL.driver.SetSomaGain(CORE, i, bd.bdpars.SomaGainId.ONE)
        HAL.driver.SetSomaOffsetSign(CORE, i, bd.bdpars.SomaOffsetSignId.POSITIVE)
        HAL.driver.SetSomaOffsetMultiplier(CORE, i, bd.bdpars.SomaOffsetMultiplierId.THREE)
    for x in range(64):
        for y in range(64):
            HAL.driver.SetSomaEnableStatus(CORE, x*64+y, bd.bdpars.SomaStatusId.DISABLED)
    HAL.driver.SetSomaEnableStatus(CORE, 0, bd.bdpars.SomaStatusId.ENABLED)
    HAL.flush()

def build_decoded_net():
    """Network that decodes with the identity matrix"""
    NNEURONS = 64
    Dout = 1
    decoders = np.eye(Dout, NNEURONS)
    tap_matrix = np.zeros((NNEURONS, Dout))

    net = graph.Network("net")
    net.pool = net.create_pool("p1", tap_matrix)
    b1 = net.create_bucket("b1", Dout)
    net.output = net.create_output("o1", Dout)
    net.create_connection("c_p1_to_b1", net.pool, b1, decoders)
    net.create_connection("c_b1_to_o1", b1, net.output, None)
    HAL.map(net)
    set_analog_config()
    return net

def build_raw_spike_net():
    """Network that only collects raw spikes"""
    NNEURONS = 64
    Dout = 1
    tap_matrix = np.zeros((NNEURONS, Dout))
    net = graph.Network("net")
    net.pool = net.create_pool("p1", tap_matrix)
    HAL.map(net)
    set_analog_config()
    return net

def toggle_hal_recording(spikes, tags):
    """Start and stop HAL traffic

    Parameters
    ----------
    spikes: bool
    tags: bool
    """
    # clear queues
    HAL.start_traffic(flush=False)
    if spikes:
        HAL.enable_spike_recording(flush=False)
    else:
        HAL.disable_spike_recording(flush=False)
    if tags:
        HAL.enable_output_recording(flush=True)
    else:
        HAL.disable_output_recording(flush=True)
    start_time = HAL.get_time()
    time.sleep(RUN_TIME)
    stop_time = HAL.get_time()
    HAL.stop_traffic(flush=False)
    HAL.disable_spike_recording(flush=False)
    HAL.disable_output_recording(flush=True)
    return start_time, stop_time

def test_raw_spikes_vs_decode_1():
    """run the test"""
    raw_spike_rates = np.zeros(len(BIAS_OFFSETS))
    net = build_raw_spike_net()
    for idx, bias_offset in enumerate(BIAS_OFFSETS):
        HAL.driver.SetDACCount(0, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, bias_offset)
        start, stop = toggle_hal_recording(True, False)
        spikes = parse_hal_spikes(HAL.get_spikes())
        raw_spike_rate = bin_tags_spikes(spikes, [start, stop])[net.pool][0, 0]
        raw_spike_rates[idx] = raw_spike_rate

    decode_spike_rates = np.zeros(len(BIAS_OFFSETS))
    net = build_decoded_net()
    for idx, bias_offset in enumerate(BIAS_OFFSETS):
        HAL.driver.SetDACCount(0, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, bias_offset)
        start, stop = toggle_hal_recording(False, True)
        hal_binned_tags = HAL.get_outputs()
        tags = parse_hal_binned_tags(hal_binned_tags)
        decode_spike_rate = bin_tags_spikes(tags, [start, stop])[net.output][0, 0]
        decode_spike_rates[idx] = decode_spike_rate

    relative_error = np.abs(raw_spike_rates - decode_spike_rates)/raw_spike_rates
    print("Bias offsets tested:")
    print("\t" + str(BIAS_OFFSETS))
    print("Raw spike rates:")
    print("\t" + str(raw_spike_rates))
    print("Decoded spike rates:")
    print("\t" + str(decode_spike_rates))
    print("Relative error:")
    print("\t" + str(relative_error))
    assert np.all(relative_error < REL_ERROR_TOLERANCE), (
        "Exceeded relative error tolerance in test_raw_spikes_vs_decode_1\n" +
        "\tRelative error tolerance is {:.2%}".format(REL_ERROR_TOLERANCE))

if __name__ == "__main__":
    test_raw_spikes_vs_decode_1()
