"""Utility functions for experiments in calibration modules"""
from time import sleep
import numpy as np

def clear_overflows(hal, sample_time, core_id=0):
    """Repeatedly clear overflow counts until no more overflow events detected

    Parameters
    ----------
    hal: HAL instance
    sample_time: float
        time between samples of overflow counts
    """
    overflow_0, overflow_1 = hal.driver.GetFIFOOverflowCounts(core_id)
    while overflow_0 or overflow_1:
        sleep(sample_time)
        overflow_0, overflow_1 = hal.driver.GetFIFOOverflowCounts(core_id)

def clear_tags(hal, tag_sample_time, core_id=0):
    """Repeatedly clear tags until no more tags are detected

    Parameters
    ----------
    hal: HAL instance
    tag_sample_time: float
        time between samples of tag counts
    """
    binned_tags, binned_times = hal.driver.RecvTags(core_id)
    while binned_tags or binned_times:
        sleep(tag_sample_time)
        binned_tags, binned_times = hal.driver.RecvTags(core_id)

def clear_spikes(hal, sample_time):
    """Repeatedly clear spike counts until no more spikes detected

    Parameters
    ----------
    hal: HAL instance
    sample_time: float
        time between samples get_spikes
    """
    spikes = hal.get_spikes()
    while spikes.shape[0] > 0:
        sleep(sample_time)
        spikes = hal.get_spikes()

def compute_spike_gen_rates(min_rate, max_rate, spike_gen_time_unit_ns):
    """Compute the FPGA's spike generator rates between the min and max rates

    Parameters
    ----------
    min_rate: number
        lower bound of rates to compute
    min_rate: number
        upper bound of rates to compute
    spike_gen_time_unit_ns: int
        number of nanoseconds for each spike generator's time unit
        Equivalent to downstream_ns in HAL

    Returns an array of rates rounded to the nearest int that may be used
    in calls to HAL's set_input_rate(s) functions
    """
    assert max_rate > min_rate, "max_rate must be greater than min_rate"
    max_period = 1./min_rate
    min_period = 1./max_rate
    spike_gen_time_unit = spike_gen_time_unit_ns*1E-9
    max_spike_gen_time_units = max_period/spike_gen_time_unit
    min_spike_gen_time_units = min_period/spike_gen_time_unit
    max_spike_gen_time_units = int(np.round(max_spike_gen_time_units))
    min_spike_gen_time_units = max(int(np.round(min_spike_gen_time_units)), 1)
    periods_spike_gen_time_units = np.arange(
        max_spike_gen_time_units, min_spike_gen_time_units-1, -1)
    rates = 1./(periods_spike_gen_time_units*spike_gen_time_unit_ns*1E-9)
    rates = np.round(rates).astype(int)
    return rates
