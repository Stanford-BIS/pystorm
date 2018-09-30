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

def bins_to_rates(counts, orig_bin_times, desired_bin_edges, init_discard_frac=0, discard_idxs=None):
    """Re-bins the input counts, divides by the duration of each bin
    to obtain rates. 

    !!!For this to work well, either the output bins should divide the input bins
    evenly, or the input bins should be much finer than the output ones!!!

    Caution: This is not a high-performance call. There are faster,
    but less convenient ways to achieve this when performance is an issue.

    Parameters:
    ===========
    counts (SxN array) : S samples from N channels
    orig_bin_times (len S (or S+1) array) : 
        if len S : start time for each bin, final bin is assumed to
            have same duration as S-1th bin
        if len S+1 : original bin boundaries
    desired_bin_edges (len SD+1 array) : desired bin boundaries
    init_discard_frac (float in [0, 1], default 0) : 
        how much of the beginning of each bin to discard 

    Returns:
    ========
    SDxN array with rates

    """
    S, N = counts.shape
    SD = len(desired_bin_edges) - 1
    
    rates = np.zeros((SD, N))
    for t_idx, time in enumerate(desired_bin_edges[:-1]):
        # XXX this is part of why the call is slow: should be able to infer indices
        # in most cases, orig_bin_times will be constant-increment

        rebin_start_time = time
        rebin_end_time = desired_bin_edges[t_idx + 1]

        # discard initial points if requested
        tbin = (rebin_end_time - rebin_start_time) * (1 - init_discard_frac)
        rebin_start_time = rebin_end_time - tbin
        
        start_bin_idx = np.searchsorted(orig_bin_times, rebin_start_time)
        end_bin_idx = np.searchsorted(orig_bin_times, rebin_end_time)

        summed_counts = np.sum(counts[start_bin_idx:end_bin_idx], axis=0)
        rates[t_idx] = summed_counts / (tbin / 1e9)

    # discard some bins if requested
    if discard_idxs is not None:
        discard_mask = np.zeros((rates.shape[0],), dtype=bool)
        discard_mask[discard_idxs] = True
        rates = rates[~discard_mask]        

    return rates

