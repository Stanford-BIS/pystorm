import argparse
import numpy as np
from pystorm.hal import HAL
from pystorm.hal.net_builder import NetBuilder
from pystorm.hal.calibrator import Calibrator
from utils.file_io import load_txt_data, set_data_dir
import pickle

DATA_DIR = set_data_dir(__file__)

SOMA_OFFSET = 2
NUM_CHUNKS_YX = 2 # avoid overwhelming bandwidth by breaking run into chunks

Y = 64 // NUM_CHUNKS_YX
X = 64 // NUM_CHUNKS_YX
N = X * Y

SY = Y // 2
SX = X // 2

PLOT = False

def parse_args():
    parser = argparse.ArgumentParser(description='Characterize soma bias twiddle values. For twiddles that cannot be estimated directly, makes a best guess')
    parser.add_argument("--soma_offset", dest="soma_offset", type=int, default=SOMA_OFFSET, help="Set DAC_SOMA_OFFSET bias. Default {}".format(SOMA_OFFSET))
    parser.add_argument('--fit_only', dest="fit_only", action='store_true', help='use pickled data, just do postprocessing')
    parser.add_argument('--make_plots', dest="make_plots", action='store_true', help='generate plots when done')
    args = parser.parse_args()
    return args

def make_plots(offsets, valid_fout_diffs, fout_diffs):
    import matplotlib.pyplot as plt

    # histograms of fout_diffs for each twiddle level
    def make_histograms(fout_diffs, plt_fname):
        num_fout_diffs = len(fout_diffs)

        fig, axes = plt.subplots(num_fout_diffs, 1, figsize=(3, num_fout_diffs*3), sharex=True)
        for idx, offset in enumerate(fout_diffs):
            axes[idx].hist(fout_diffs[offset], bins=20)
            axes[idx].set_title('offset = ' + str(offset))

        plt.xlim(np.sort(fout_diffs[-3])[10], np.sort(fout_diffs[3])[-10])
        plt.suptitle('twiddle bit values')
        plt.savefig(plt_fname)

    make_histograms(valid_fout_diffs, 'data/soma_bias_twiddle_valid_only.png')
    make_histograms(fout_diffs, 'data/soma_bias_twiddle.png')

def estimate_bias_twiddles(args):
    """Estimates the change in firing rate caused by each bias level

    The pool is the entire chip, the tap points are all 1s, and the diffusor is broad. 
    Synapses are driven close to fmax, so as many neurons bifurcate as possible.

    The bias recorded is the difference between the output firing rate for 
    the current twiddle value, and the output firing rate at twiddle=0. 
    For neurons that didn't fire at all for a given bias level, NaNs are recorded

    READ THIS: you probably want to access this calibration through 
    Calibrator.get_all_bias_twiddles or Calibrator.get_pool_bias_twiddles(),
    this will package the data in a convenient form, and give you the option to fill 
    in the NaN entries with reasonable guesses.

    The common use case for this calibration is to determine how to set twiddles to maximize
    "neuron yield", the set of neurons where |g / b| > 1:

    ~~~~~~~~~~~~~~~~

    f'(x) = x * e_unit * g + b 
    f(x) = max(0, f'(x))
    
    for x * e_unit = 1 (biggest input) want neuron to be on: f'(x) > 0

    1 * g + b > 0
    1 > -b/g
    1 < -g/b
    -1 > g/b

    for x * e_unit = -1 (smallest input) want neuron to be off: f'(x) < 0

    -1 * g + b < 0
    1 < g/b

    The intersection of these, |g / b| > 1 is the set of 'good' neurons (i.e. neurons
    that bifurcate somewhere inside the range)

    ~~~~~~~~~~~~~~~~

    Another way to see this by solving for the neuron intercept:
    
    f(x) = 0 = f'(x) = x * e_unit * g + b (intercept = x condition)
    g/b = -1/(x * e_unit)

    if x = (norm * e_unit) where norm in [-1, 1] (intercept happens in the range) then:
    max(g/b) = -1/-1 = 1
    min(g/b) = -1/1 = -1
    => |g / b| < 1

    ~~~~~~~~~~~~~~~~

    The bias value recorded by this calibration is in the same units as that returned
    by Calibrator.get_encoders_and_offsets()'s offsets (b in the above equations). 
    Given the encoders and offsets taken at bias twiddle = 0, b, as well as the output
    of this calibration, b(tw), it is straightforward to optimize optimize the following:

    |g / (b + b(tw))| < 1

    if this is met for multiple values of tw, then additional criteria can be used
    (say, optimizing the distributions of intercepts)

    """

    # bias levels allowed
    biases = np.array([3, 2, 1, 0, -1, -2, -3], dtype=int)

    # collect data, or load pickle
    pck_fname = DATA_DIR + 'soma_bias_twiddle_raw_data.pck'
    if args.fit_only:
        all_offsets = pickle.load(open(pck_fname, 'rb'))
        hal = None
    else: 
        all_offsets = {}

        for bias in biases:

            print("Y,X", Y, X)
            bias_offsets = np.zeros((64, 64))
            all_offsets[bias] = bias_offsets

            for chunk_y, chunk_x in [(y, x) for y in range(NUM_CHUNKS_YX) 
                                            for x in range(NUM_CHUNKS_YX)]:
                print(chunk_y, chunk_x)
                Y_loc = chunk_y * Y
                X_loc = chunk_x * X
                SY_loc = Y_loc // 2
                SX_loc = X_loc // 2


                print("===============")
                print("RUNNING BIAS", bias)
                print("===============")
                hal = HAL()

                # set up NetBuilder, we're going to make a basic pool
                net_builder = NetBuilder(hal)

                # we need some other basic calibration data
                cal = Calibrator(hal)
                bad_syn, _ = cal.get_bad_syns()
                # use all-1 taps, except bad synapses
                taps = np.zeros((Y, X))
                taps[::2, ::2] = ~bad_syn[SY_loc:SY_loc + SY, 
                                          SX_loc:SX_loc + SX]
                tap_matrix = taps.reshape((N, 1))
                NetBuilder.make_taps_even(tap_matrix)


                net = net_builder.create_single_pool_net(Y, X, tap_matrix, biases=bias, loc_yx=(Y_loc, X_loc))
                pool = net.get_pools()[0]
                print(net.get_inputs())
                inp = net.get_inputs()[0]

                hal.map(net)

                # broad diffusor
                hal.set_DAC_value('DAC_DIFF_G', 64)
                hal.set_DAC_value('DAC_DIFF_R', 1024)

                # don't want saturation, min t_ref
                hal.set_DAC_value('DAC_SOMA_REF', 1024)

                # some crazy guys might saturate, but you won't overwhelm the USB
                #hal.set_DAC_value('DAC_SOMA_REF', 10)

                # stress the safety margin: we want as many neurons to fire as possible
                safe_fmaxes = net_builder.determine_safe_fmaxes(safety_margin=.95)
                FMAX = safe_fmaxes[pool]

                est_encs, est_offsets, insufficient_samples, debug = \
                    net_builder.determine_encoders_and_offsets(pool, inp, FMAX, num_sample_angles=1)
                bias_offsets[Y_loc:Y_loc + Y, 
                              X_loc:X_loc + X] = est_offsets.reshape((Y, X))

        # reshape data
        for bias in all_offsets:
            all_offsets[bias] = all_offsets[bias].flatten()

        # save results
        pickle.dump(all_offsets, open(pck_fname, 'wb'))

    # parse data
    all_fout_diffs = {}
    all_valid_fout_diffs = {}

    for bias in biases[biases != 0]:
        offset_diff = all_offsets[bias] - all_offsets[0]

        all_valid_fout_diffs[bias] = offset_diff[~np.isnan(offset_diff)]

        # compute mean of valid entries, use it for any invalid entries
        mean_diff = np.mean(offset_diff[~np.isnan(offset_diff)])

        # NOTE: this is now done in a smarter way by Calibrator.extrapolate_bias_twiddles()
        # offset_diff[np.isnan(offset_diff)] = mean_diff

        all_fout_diffs[bias] = offset_diff

    # make plots
    if args.make_plots:
        make_plots(all_offsets, all_valid_fout_diffs, all_fout_diffs)

    # add to cal_db (one calibration type per twiddle level)
    for bias in biases[biases != 0]:
        # construct string labeling this calibration in cal_db
        # each nonzero bias level has its own calibration
        if bias > 0:
            sign = 'p'
        else:
            sign = 'n'
        soma_cal_type = 'bias_twiddle_' + sign + str(abs(bias)) + '_dac_' + str(args.soma_offset)

        if hal is None:
            hal = HAL()
        hal.add_calibration('soma', soma_cal_type, all_fout_diffs[bias])

if __name__ == "__main__":
    estimate_bias_twiddles(parse_args())
