import numpy as np
from pystorm.PyDriver import bddriver as bd
import time

def standard_startup(time_unit_ns=10000, upstream_hb_ns=100000):
    CORE = 0

    D = bd.Driver()

    comm_state = D.Start()
    if (comm_state < 0):
        print("* Driver failed to start!")
        exit(-1)

    print("* Resetting BD")
    D.ResetBD()

    # set time unit (SF/SG update interval) to .1 ms
    print("* Setting FPGA time units")
    D.SetTimeUnitLen(time_unit_ns)
    D.SetTimePerUpHB(upstream_hb_ns)

    print("* Init the FIFO")
    D.InitFIFO(CORE)

    print("* Enable tag traffic")
    D.SetTagTrafficState(CORE, True, True)

    return D

def standard_DAC_settings(D, CORE):

    print("* Init'ing DACs")
    time.sleep(2)
    D.InitDAC(CORE)

    # magical DAC settings (DC is the most important, with the default, inhibition doesn't work)
    D.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_EXC     , 512)
    D.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_DC      , 920)
    D.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_INH     , 512)
    D.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_LK      , 10)
    D.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_PD      , 10)
    D.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_PU      , 1023)
    D.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_DIFF_G      , 1023)
    D.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_DIFF_R      , 250)
    D.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , 1)

def compare_TAT_words(progged, dumped):
    if len(dumped) >= 1 and sum([i == j for i,j in zip(dumped, progged)]) == len(progged):
        print("* TAT matches!")
        return 0
    else:
        print("* TAT didn't match")
        if len(dumped) == 0:
            print("we read nothing!")
        for entry, progged_entry in zip(dumped[:len(progged)], progged):
            print("we programmed:")
            print("{0:b}".format(progged_entry))
            print("we read:")
            print("{0:b}".format(entry))
            print("stop", bd.GetField(entry, bd.TATTagWord.STOP))
            print("groute", bd.GetField(entry, bd.TATTagWord.GLOBAL_ROUTE))
            print("tag", bd.GetField(entry, bd.TATTagWord.TAG))
        return -1

def s_to_ns(s):
    return (1e9 * s).astype(int)

def ns_to_s(ns):
    return ns / 1e9

# stolen from hal/neuromporph/hardware_resources.py
def thr_idxs_vals(max_abs_row_weights):
    """for a set of max_abs_row_weights, get the thr idx (programmed values) and 
    effective weight values that the AMBuckets should use
    """
    min_threshold_value = 64
    num_threshold_levels = 8
    max_weight_value = 127

    # With the threshold at the minimum value (64),
    # it's technically possible to have weights that behave sort of like
    # they're > 1 or < -1.
    # To have well-defined behavior, with threshold = 64, the weights
    # must be in [-64, 64]
    # this is equivalent to restricting the user weights to be in [-1, 1]
    assert np.max(max_abs_row_weights) <= 1

    # compute all possible hardware threshold vals (64, 128, 256, ...)
    all_thr_vals = np.array(
        [min_threshold_value * 2**t for t in range(num_threshold_levels)])

    # compute max possible weight, in user-space, for each threshold value
    # the first value (for threshold = 64) is special, as noted above
    user_max_weights_for_thr_vals = np.array(
        [1.] + [max_weight_value / thr_val for thr_val in all_thr_vals[1:]])

    # find max_row_Ws in the user_max_weights we just computed
    # user_max_weights is descending, so we provide the sorter argument
    thr_idxs = np.searchsorted(-user_max_weights_for_thr_vals, -max_abs_row_weights) - 1

    thr_vals = all_thr_vals[thr_idxs]

    return thr_idxs, thr_vals

def weight_to_mem(user_W):
    """Given the user's desired weight matrix, the max weights
    implemented in the decoder/transform row (which may be shared
    with other user_Ws), and the core parameters,
    determine the best implementable weights and threshold values
    """

    def dec2onesc(x, max_weight):
        """convert decimal to one's complement representation used in hardware
        XXX should be in driver?
        """

        def invert_bits(x, all_ones):
            """Invert the bits in x"""
            ones = np.ones_like(x).astype(int) * all_ones
            return np.bitwise_xor(ones, x)

        assert np.sum(x > max_weight) == 0
        assert np.sum(x < -max_weight) == 0
        x = np.array(x)
        xonesc = x.copy().astype(int)
        neg = x < 0

        # invert bits
        print(xonesc)
        xonesc[neg] = invert_bits(-xonesc[neg], 255) # max_weight is all ones
        print(xonesc)

        return xonesc

    max_abs_row_weights = np.max(abs(user_W), axis=1)
    print("max absolute row weights")
    print(max_abs_row_weights)

    # this is the threshold value we should use (and scale the user weights by, in this case)
    thr_idxs, thr_vals = thr_idxs_vals(max_abs_row_weights)

    W = (user_W.T * thr_vals).T
    W = np.round(W) # XXX other ways to do this, possibly better to round probabilistically
    W_eff = (W / thr_vals).T

    W = dec2onesc(W, 127)

    return W, thr_idxs, thr_vals, W_eff
