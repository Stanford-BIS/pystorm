"""This module contains utilities for processing data obtained from HAL"""
import numpy as np

def lpf(signal, tau, dt):
    """Low pass filters a 1D timeseries"""
    ret = np.zeros(signal.shape)
    decay = np.expm1(-dt/tau)+1
    increment = -np.expm1(-dt/tau)/dt
    ret += increment*signal
    for idx in range(1, len(signal)):
        ret[idx] += ret[idx-1]*decay
    return ret

def bin_to_spk_times(bins, bin_times):
    """Convert binned spikes to spike times"""
    ubin_vals = np.unique(bins)[1:]
    spk_times = []
    for uval in ubin_vals:
        spk_times += list(bin_times[np.nonzero(bins==uval)[0]])
    spk_times = np.sort(np.array(spk_times))
    return spk_times
