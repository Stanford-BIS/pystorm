"""This module defines the hardware resources of braindrop/brainstorm"""
from abc import ABC, abstractmethod, abstractproperty
import numpy as np
from pystorm.PyDriver import bddriver

import logging
logger = logging.getLogger(__name__)

HOME_ROUTE = 255

class ResourceConnection(object):
    """ResourceConnection connects two resources, allows slicing"""
    def __init__(self, src, tgt, src_range=None, tgt_range=None):
        self.src = src
        self.tgt = tgt

        # type check
        assert isinstance(src, tuple(tgt.connectable_types_in))

        # sliceability check
        if src_range is not None:
            assert src.sliceable_out
        if tgt_range is not None:
            assert tgt.sliceable_in

        # number of connections check
        if src.max_conns_out is not None:
            assert len(src.conns_out) + 1 <= src.max_conns_out
        if tgt.max_conns_in is not None:
            assert len(tgt.conns_in) + 1 <= tgt.max_conns_in

        # range == None is full range of src/tgt objects outputs/inputs
        self.src_range = src_range
        self.tgt_range = tgt_range
        if src_range is None:
            self.src_range = range(src.dimensions_out)
        if tgt_range is None:
            self.tgt_range = range(tgt.dimensions_in)

        # check dimensional correctness
        self._check_dims(src.dimensions_out, tgt.dimensions_in)

    def _check_dims(self, src_dimensions_out, tgt_dimensions_in):
        """Check correctness of a slice connection between objects"""
        # range lengths must match
        assert len(self.src_range) == len(self.tgt_range)

        # indices must be valid dimensions
        for idx in self.src_range:
            assert idx < src_dimensions_out
        for idx in self.tgt_range:
            assert idx < tgt_dimensions_in

        # no multiple connection
        assert len(self.src_range) <= tgt_dimensions_in
        assert len(self.tgt_range) <= src_dimensions_out

class Resource(ABC):
    """Resources represent chunks of allocateable braindrop hardware

    A graph of hardware resources is necessary to implement the network
    Most of the functionality of the objects is in the allocation process
    Basically, Resource objects know how to map themselves to a Core object

    Inputs
    ------
    connectable_types_in: list of Resource types
        Resource types that can make an incoming connection to this object
    connectable_types_out:  list of Resource types
        Resource types that outgoing connections from this object can go to
    sliceable_in:  if the input dimension range can be sliced
    sliceable_out: if the output dimension range can be sliced
    max_conns_in: maximum number of incoming connections
    max_conns_out): maximum number of outgoing connections
    """
    def __init__(self,
                 connectable_types_in, connectable_types_out,
                 sliceable_in=True, sliceable_out=True,
                 max_conns_in=None, max_conns_out=None):

        self.connectable_types_in = connectable_types_in
        self.connectable_types_out = connectable_types_out
        self.sliceable_in = sliceable_in
        self.sliceable_out = sliceable_out
        self.max_conns_in = max_conns_in
        self.max_conns_out = max_conns_out

        self.conns_in = []
        self.conns_out = []

    @abstractproperty
    def dimensions_in(self):
        """Get input dimensionality"""
        raise NotImplementedError

    @abstractproperty
    def dimensions_out(self):
        """Get output dimensionality"""
        raise NotImplementedError

    # XXX should InTags() be a method?

    def connect(self, tgt, src_range=None, tgt_range=None):
        """Connect this Resource to another one

        Inputs
        ------
        tgt: target Resource object
        src_range: slice index of this object's output range to make connections from
        tgt_range: slice index of tgt object's output range to make connections to
        """
        new_conn = ResourceConnection(self, tgt, src_range, tgt_range)
        self.conns_out += [new_conn]
        tgt.conns_in += [new_conn]

    # The mapping process is divided into phases

    def pretranslate_early(self, core):
        """Perform any bookeeping that's needed before allocation
        This might include things like determining how to break up matrices or pools
        into the chunks that go into the allocator.
        """
        pass

    def pretranslate(self, core):
        """Perform any bookeeping that's needed before allocation
        This might include things like determining how to break up matrices or pools
        into the chunks that go into the allocator.
        """
        pass

    def allocate_early(self, core):
        """Some objects must be allocated before others, the first objects are allocated here"""
        # for now, only MMWeights has allocate early, to allocate decoders before transforms
        pass

    @abstractmethod
    def allocate(self, core):
        """Allocate the core to the resources. These should be simple calls.
        Elaborate logic is meant to go in pretranslate.
        """
        pass

    def posttranslate_early(self, core):
        """Perform bookeeping after allocating, but before other bookkeeping"""
        # for now, only AMBuckets has posttranslate_early, to compute max_row_weights,
        # which is needed by MMWeights' posttranslate
        pass

    def posttranslate(self, core):
        """Perform bookeeping after allocating the core to the resources
        This might include things like determining accumulator threshold values/
        weight matrix scalings. It also includes packing BDWords before assign()
        """
        pass

    def assign(self, core):
        """assign the Resources' values to the Core at the allocated locations
        should be light on logic, anything elaborate should go in posttranslate
        """
        pass

class Neurons(Resource):
    """A chunk of the physical neuron array that implements a logical neuron pool

    Also includes the direct-mapped PAT memory needed to get to the accumulator
    """

    pool_yx_to_aer = None

    def __init__(self, y, x, gain_divisors, biases, xy_loc=(None, None)):
        super().__init__(
            [TATTapPoint], [MMWeights, Sink],
            sliceable_in=False, sliceable_out=False,
            max_conns_out=1)
        self.y = y
        self.x = x
        self.gain_divisors = gain_divisors
        self.biases = biases
        self.N = y * x

        # pretranslate_early
        # y and x sizes in units of minimum pool x and y dimensions
        self.py = None
        self.px = None

        # allocate
        # y and x locations in units of minimum pool x and y dimensions
        self.x_loc, self.y_loc = xy_loc # user can specify (for all pools only)
        self.py_loc = None
        self.px_loc = None

        # posttranslate
        self.PAT_contents = None

    @property
    def dimensions_in(self):
        return self.N

    @property
    def dimensions_out(self):
        return self.N

    @property
    def N_slices(self):
        return self.py * self.px

    def pretranslate_early(self, core):
        """neuron array allocation prep"""

        # round size y/x up to the number of pools needed
        self.py = int(np.ceil(self.y / core.NeuronArray_pool_size_y))
        self.px = int(np.ceil(self.x / core.NeuronArray_pool_size_x))

        # let the allocator know about it
        core.neuron_array.add_pool(self)

        # fill in yx_to_AER if not done already
        if Neurons.pool_yx_to_aer is None:
            pools_y = core.NeuronArray_pools_y
            pools_x = core.NeuronArray_pools_x
            Neurons.pool_yx_to_aer = np.zeros((pools_y, pools_x), dtype=int)
            for aer_sub_addr in range(pools_y * pools_x):
                yx = bddriver.Driver.BDPars.GetSomaXYAddr(aer_sub_addr)
                y = yx // core.NeuronArray_width
                x = yx  % core.NeuronArray_width
                Neurons.pool_yx_to_aer[y, x] = aer_sub_addr

    def allocate(self, core):
        """neuron array allocation"""
        self.py_loc, self.px_loc = core.neuron_array.allocate(self)
        logger.debug("pool alloced to {}, {}".format(self.py_loc, self.px_loc))
        self.y_loc = self.py_loc * core.NeuronArray_pool_size_y
        self.x_loc = self.px_loc * core.NeuronArray_pool_size_x

    def posttranslate(self, core):
        """PAT assignment setup"""
        # assert no more than one connection (only one decoder allowed)
        if len(self.conns_out) == 1:
            weights = self.conns_out[0].tgt
            buckets = self.conns_out[0].tgt.conns_out[0].tgt

            # addressed y-x, they can't necessarily be written to the memory contiguously
            self.PAT_contents = np.empty((self.py, self.px), dtype=object)

            for py_idx in range(self.py):
                for px_idx in range(self.px):
                    # first nrn idx in block
                    nrn_idx = (py_idx * self.px + px_idx) * core.NeuronArray_pool_size
                    MMAY, MMAX = weights.in_dim_to_mma(nrn_idx)
                    # MMAY will be in {0, 64, 128, 192}, we map to {0, 1, 2, 3}
                    MMAY_prog = MMAY // core.NeuronArray_pool_size

                    AMA = buckets.start_addr

                    self.PAT_contents[py_idx, px_idx] = bddriver.PackWord([
                        (bddriver.PATWord.AM_ADDRESS, AMA),
                        (bddriver.PATWord.MM_ADDRESS_LO, MMAX),
                        (bddriver.PATWord.MM_ADDRESS_HI, MMAY_prog)])
        elif len(self.conns_out) > 1:
            logger.critical("pool had", len(self.conns_out), "output connections")
            assert(False and "Neurons can have only one output connection")

    def assign(self, core):
        """PAT assignment"""
        logger.debug("pool at {}, {}".format(self.px_loc, self.py_loc))
        if len(self.conns_out) == 1:
            for py_idx in range(self.py):
                for px_idx in range(self.px):
                    aer_pool_addr_bits = Neurons.pool_yx_to_aer[
                        py_idx + self.py_loc, px_idx + self.px_loc]
                    to_assign = self.PAT_contents[py_idx, px_idx]
                    logger.debug(
                        "assigning for sub addr {}, {} at {}".format(
                            px_idx, py_idx, aer_pool_addr_bits))
                    core.PAT.assign(to_assign, aer_pool_addr_bits)

        core.neuron_array.assign(self)

class MMWeights(Resource):
    """Represents weight entries in Main Memory

    We support weight matrix sharing by keeping a static class member dictionary to track
    which MMWeights used the same memory entries.

    XXX later can code this up to automate matrix caching
    """

    # static member variables
    # keep track of MMWeights -> unique id for combined matrices
    forward_cache = {}
    # keep track of unique ids for combined matrices -> list(MMWeights)
    reverse_cache = {}

    yx_to_aer = None # maps y,x address in pool (64 neurons) to aer address (used in HW)

    @staticmethod
    def weight_to_mem(user_W, max_abs_row_weights, core):
        """Given the user's desired weight matrix, the max weights
        implemented in the decoder/transform row (which may be shared
        with other user_Ws), and the core parameters,
        determine the best implementable weights and threshold values
        """

        def dec2onesc(x, max_weight):
            """convert decimal to one's complement representation used in hardware
            XXX should be in driver?
            """

            assert np.sum(x > max_weight) == 0
            assert np.sum(x < -max_weight) == 0
            x = np.array(x)
            xonesc = x.copy().astype(int)
            neg = x < 0

            # invert bits
            all_ones = max_weight * 2 + 1
            xonesc[neg] += all_ones

            return xonesc

        # this is the threshold value we should use (and scale the user weights by, in this case)
        _, thr_vals = AMBuckets.thr_idxs_vals(max_abs_row_weights, core)

        W = (user_W.T * thr_vals).T
        W = np.round(W) # XXX other ways to do this, possibly better to round probabilistically

        logger.debug("thrs: {} W: {}".format(thr_vals, W))
        W = dec2onesc(W, core.max_weight_value)

        return W

    def __init__(self, W):
        super().__init__(
            [Neurons, TATAccumulator], [AMBuckets],
            sliceable_in=True, sliceable_out=False,
            max_conns_in=1, max_conns_out=1)

        self.user_W = W

        # do caching based on object id of W, object id of self
        MMWeights.forward_cache[id(self)] = id(W)
        if self in MMWeights.reverse_cache:
            MMWeights.reverse_cache[id(W)] += [id(self)]
        else:
            MMWeights.reverse_cache[id(W)] = [id(self)]

        # pretranslate
        self.slice_width = None
        self.N_slices = None
        self.is_dec = None

        # allocate (by AMBuckets)
        self.slice_start_addrs = []

        # posttranslate
        self.programmed_W = None
        self.W_slices = None
        self.W_slices_BDWord = None

    def in_dim_to_mma(self, dim):
        """Calculate the max x and y coordinates in main memory associated with dim"""
        slice_idx = dim // self.slice_width
        slice_offset = dim % self.slice_width
        MM_start_addr = self.slice_start_addrs[slice_idx]
        MMAY = MM_start_addr[0] + slice_offset
        MMAX = MM_start_addr[1]
        return [MMAY, MMAX]

    @property
    def dimensions_in(self):
        return self.user_W.shape[1]

    @property
    def dimensions_out(self):
        return self.user_W.shape[0]

    def pretranslate(self, core):
        """Determine matrix slicing.

        Decoders are broken into 64 entry tall slices, Transforms into 1 entry tall slices
        """

        # first, decide if this is a decoder or a transform
        if len(self.conns_in) > 0:
            self.is_dec = isinstance(self.conns_in[0].src, Neurons)
        else:
            self.is_dec = False
            logger.warning("in pretranslate, odd situation: MMWeights with no input connection")

        # generate slice indexing
        if self.is_dec: # can chop up decoders into NeuronArray_pool_size-column chunks
            self.slice_width = core.NeuronArray_pool_size
            self.N_slices = self.conns_in[0].src.N_slices
        else: # can chop up transforms by single columns
            self.slice_width = 1
            self.N_slices = self.dimensions_in

    def allocate_early(self, core):
        """allocate decoders (big slices) first"""
        if self.is_dec:
            for slice_idx in range(self.N_slices):
                # actually doesn't matter what the slice is, always allocate 64xDO
                start_addr = core.MM.allocate_dec(self.dimensions_out)
                self.slice_start_addrs += [start_addr]

    def allocate(self, core):
        """allocate transforms (small slices) after"""
        if not self.is_dec:
            for slice_idx in range(self.N_slices):
                # actually doesn't matter what the slice is, always allocate 1xDO
                start_addr = core.MM.allocate_trans(self.dimensions_out)
                self.slice_start_addrs += [start_addr]

    def posttranslate(self, core):
        self.W_slices = []
        self.W_slices_BDWord = []

        """calculate implemented weights"""
        # look at AMBuckets.max_user_W, compute weights to program
        self.programmed_W = MMWeights.weight_to_mem(
            self.user_W, self.conns_out[0].tgt.max_abs_row_weights, core)

        # slice up W according to slice indexing

        # this is kind of ugly for decoders (and this code could definitely be optimized)
        if self.is_dec:

            # fill in yx_to_AER if not done already
            if MMWeights.yx_to_aer is None:
                MMWeights.yx_to_aer = np.zeros(
                    (core.NeuronArray_pool_size_y, core.NeuronArray_pool_size_x), dtype=int)
                for aer_sub_addr in range(core.NeuronArray_pool_size):
                    yx = bddriver.Driver.BDPars.GetSomaXYAddr(aer_sub_addr)
                    y = yx // core.NeuronArray_width
                    x = yx  % core.NeuronArray_width
                    MMWeights.yx_to_aer[y, x] = aer_sub_addr
                logger.debug("aer-xy conversion table:")
                logger.debug(MMWeights.yx_to_aer)

            # init with zeros, there are potentially unused entries for the "edge" neurons
            self.W_slices = [np.zeros((self.dimensions_out, self.slice_width)).astype(int)
                             for i in range(self.N_slices)]

            # iterate over neurons in y,x address order (matrix indexing order)
            yx_nrn_idx = 0

            for y in range(self.conns_in[0].src.y):
                for x in range(self.conns_in[0].src.x):
                    # slices are stored in y,x pool address order
                    py = y // core.NeuronArray_pool_size_y
                    px = x // core.NeuronArray_pool_size_x
                    slice_idx = py * self.conns_in[0].src.px + px

                    # use AER address
                    suby = y % core.NeuronArray_pool_size_y
                    subx = x % core.NeuronArray_pool_size_x
                    aer_sub_idx = MMWeights.yx_to_aer[suby, subx]

                    self.W_slices[slice_idx][:, aer_sub_idx] = self.programmed_W[:, yx_nrn_idx]
                    yx_nrn_idx += 1

                    logger.debug("yx_nrn_idx %d", yx_nrn_idx)
                    logger.debug("suby %d", suby)
                    logger.debug("subx %d", subx)
                    logger.debug("aer_sub_idx %d", aer_sub_idx)


        else:
            slice_shape = (self.programmed_W.shape[0], 1)
            self.W_slices = [self.programmed_W[:, i].reshape(*slice_shape)
                             for i in range(self.N_slices)]

        # Pack into BDWord (which doesn't actually do anything)
        for W_slice in self.W_slices:
            contents = [
                [bddriver.PackWord([(bddriver.MMWord.WEIGHT, W_slice[i, j])])
                 for j in range(W_slice.shape[1])]
                for i in range(W_slice.shape[0])]
            self.W_slices_BDWord += [np.array(contents, dtype=object)]

    def assign(self, core):
        for W_slice, start_addr in zip(
                self.W_slices_BDWord, self.slice_start_addrs):
            if self.is_dec:
                # transpose, the memory is flipped from linalg orientation
                core.MM.assign_dec(W_slice.T, start_addr)
            else:
                # 1D slices in trans row case
                core.MM.assign_trans(W_slice, start_addr)

class AMBuckets(Resource):
    """Represents entries in accumulator memory

    The accumulator memory contains an array of buckets defined by
    their threshold values and next address targets in the hardware.
    The last bucket has the stop bit
    """

    @staticmethod
    def thr_idxs_vals(max_abs_row_weights, core):
        """for a set of max_abs_row_weights, get the thr idx (programmed values) and
        effective weight values that the AMBuckets should use
        """

        # With the threshold at the minimum value (64),
        # it's technically possible to have weights that behave sort of like
        # they're > 1 or < -1.
        # To have well-defined behavior, with threshold = 64, the weights
        # must be in [-64, 64]
        # this is equivalent to restricting the user weights to be in [-1, 1]
        assert np.max(max_abs_row_weights) <= 1

        # compute all possible hardware threshold vals (64, 128, 256, ...)
        all_thr_vals = np.array(
            [core.min_threshold_value * 2**t for t in range(core.num_threshold_levels)])

        # compute max possible weight, in user-space, for each threshold value
        # the first value (for threshold = 64) is special, as noted above
        user_max_weights_for_thr_vals = np.array(
            [1.] + [core.max_weight_value / thr_val for thr_val in all_thr_vals[1:]])

        # find max_row_Ws in the user_max_weights we just computed
        # user_max_weights is descending, so we provide the sorter argument
        thr_idxs = np.searchsorted(
            -user_max_weights_for_thr_vals, -max_abs_row_weights, side="right") - 1
        assert np.all(thr_idxs >= 0)

        thr_vals = all_thr_vals[thr_idxs]

        return thr_idxs, thr_vals

    def __init__(self, D):
        super().__init__([MMWeights], [TATAccumulator, TATFanout, TATTapPoint],
                         sliceable_in=False, sliceable_out=True, max_conns_out=1)
        self.D = D

        # filled in pretranslate
        self.thr = None

        # filled in allocation
        self.start_addr = None

        # filled in posttranslate_early
        self.max_abs_row_weights = None

        # filled in posttranslate
        self.AM_entries = None

    @property
    def dimensions_in(self):
        return self.D

    @property
    def dimensions_out(self):
        return self.D

    def allocate(self, core):
        self.start_addr = core.AM.allocate(self.dimensions_out)

    def posttranslate_early(self, core):
        """Determines the largest weight in each dimension feeding these buckets is.
        MMWeights posttranslate depends on this, so it uses posttranslate_early.
        """
        W_list = [conn.src.user_W for conn in self.conns_in]
        W_stack = np.hstack(W_list)

        self.max_abs_row_weights = np.max(np.abs(W_stack), axis=1)

    def posttranslate(self, core):

        # we need to create the (stop, value, thresholds) and next address entries for the AM
        stop = np.zeros((self.dimensions_out,)).astype(int)
        stop[-1] = 1
        val = np.zeros((self.dimensions_out,)).astype(int)
        thr_idx, self.thr = AMBuckets.thr_idxs_vals(self.max_abs_row_weights, core)

        if len(self.conns_out) > 0:
            if isinstance(self.conns_out[0].tgt, Sink):
                # if we target a sink, we're targetting a SF on the FPGA
                # tag is the filter idx, give it a global route of HOME_ROUTE
                NAs = self.conns_out[0].tgt.filter_idxs + (
                    2**bddriver.FieldWidth(bddriver.AccOutputTag.TAG) * HOME_ROUTE)
            else:
                # otherwise, we're going to the TAT
                NAs = self.conns_out[0].tgt.in_tags
        else:
            NAs = np.zeros_like(thr_idx)

        self.AM_entries = []
        for s, v, t, n in zip(stop, val, thr_idx, NAs):
            self.AM_entries += [bddriver.PackWord([
                (bddriver.AMWord.ACCUMULATOR_VALUE, v),
                (bddriver.AMWord.THRESHOLD, t),
                (bddriver.AMWord.STOP, s),
                (bddriver.AMWord.NEXT_ADDRESS, n)])]
        self.AM_entries = np.array(self.AM_entries, dtype=object)

    def assign(self, core):
        core.AM.assign(self.AM_entries, self.start_addr)

class TATAccumulator(Resource):
    """Represents entries in the Tag Action Table for the Accumulator

    XXX should upgrade for sliceable_out=True
    """

    def __init__(self, D):
        super().__init__([AMBuckets, Source, TATFanout], [MMWeights],
                         sliceable_in=True, sliceable_out=False)

        self.D = D

        # pretranslate
        self.size = None
        self.start_offsets = None

        # allocate
        self.start_addr = None
        self.in_tags = None

        # posttranslate
        self.contents = None

    @property
    def dimensions_in(self):
        return self.D

    @property
    def dimensions_out(self):
        return self.D

    def pretranslate(self, core):
        self.size = self.D * len(self.conns_out)
        # D acc sets, each with len(conns_out) fanout
        self.start_offsets = np.array(range(self.D)) * len(self.conns_out)

    def allocate(self, core):
        self.start_addr = core.TAT0.allocate(self.size)
        self.in_tags = self.start_addr + self.start_offsets

    def posttranslate(self, core):
        self.contents = []
        for d in range(self.D):
            for t in range(len(self.conns_out)):
                weights = self.conns_out[t].tgt
                buckets = self.conns_out[t].tgt.conns_out[0].tgt

                AMA = buckets.start_addr
                MMAY, MMAX = weights.in_dim_to_mma(d)
                MMA = MMAY * core.MM_width + MMAX
                stop = 1 * (t == len(self.conns_out) - 1)

                self.contents += [bddriver.PackWord([
                    (bddriver.TATAccWord.STOP, stop),
                    (bddriver.TATAccWord.AM_ADDRESS, AMA),
                    (bddriver.TATAccWord.MM_ADDRESS, MMA)])]
        self.contents = np.array(self.contents, dtype=object)

    def assign(self, core):
        core.TAT0.assign(self.contents, self.start_addr)

class TATTapPoint(Resource):
    def __init__(self, encoders_or_taps):
        super().__init__([AMBuckets, Source, TATFanout], [Neurons],
                         sliceable_in=True, sliceable_out=False, max_conns_out=1)

        # tap_and_signs is a list of lists of tuples. There is one list of tuples per dimension.
        if isinstance(encoders_or_taps, tuple):
            self.N, self.taps_and_signs = encoders_or_taps
            self.D = len(self.taps_and_signs)
        else:
            self.N, self.D = encoders_or_taps.shape
            self.taps_and_signs = self.encoders_to_taps(encoders_or_taps)

        self.Ks = [len(el) for el in self.taps_and_signs] # num taps for each dim

        for K in self.Ks:
            assert K % 2 == 0 and "need even number of tap points"
        for dim_taps in self.taps_and_signs:
            for tap, sign in dim_taps:
                assert tap < self.N and tap >= 0 and "tap point indexes neuron outside of pool size"
                assert sign in [-1, 1] and "sign must be -1 or 1"

        # pretranslate
        self.nrn_per_syn = None

        # allocate
        self.start_addr = None
        self.in_tags = None

        # posttranslate
        self.mapped_taps = None
        self.contents = None

    def encoders_to_taps(self, M):
        """encoder entries should be in {-1, 0, 1}
        converts this representation to the sparse representation used by TATTapPoint
        """
        nrns = M.shape[0]
        dims = M.shape[1]

        taps_and_signs = [[] for d in range(dims)]
        for d in range(dims):
            for n in range(nrns):
                entry = M[n, d]
                assert entry in [-1, 0, 1], (
                    "weights to a pool must be in -1, 0, 1 (encoders implemented as tap points!)")
                if entry != 0:
                    t = n
                    s = int(entry)
                    taps_and_signs[d].append((t, s))

        logger.debug("matrix:")
        logger.debug(M)
        logger.debug("taps and signs:")
        logger.debug(taps_and_signs)
        return taps_and_signs

    @property
    def dimensions_out(self):
        return self.N

    @property
    def dimensions_in(self):
        return self.D

    @property
    def size(self):
        return sum(self.Ks) // 2 # two taps per entry

    @property
    def start_offsets(self):
        zero_prepend = [0] + [K // 2 for K in self.Ks]
        return np.cumsum(zero_prepend[:-1])

    def tap_map(self, tap_in_nrn_space, tgt_pool_width):
        """allow user to specify tap points at neuron granularity
        but actually there are fewer synapses than neurons
        """
        tap_y = (tap_in_nrn_space // tgt_pool_width) // self.neurons_per_syn_y
        tap_x = (tap_in_nrn_space %  tgt_pool_width) // self.neurons_per_syn_x
        return tap_x, tap_y

    def pretranslate(self, core):
        self.array_width = core.NeuronArray_width
        self.neurons_per_syn_x = int(np.sqrt(core.NeuronArray_neurons_per_tap))
        self.neurons_per_syn_y = self.neurons_per_syn_x

    def allocate(self, core):
        self.start_addr = core.TAT1.allocate(self.size)
        self.in_tags = self.start_addr + self.start_offsets + core.TAT_size # in TAT1

    def posttranslate(self, core):

        def bin_sign(sign):
            return (int(sign) + 1) // 2

        tgt_pool_width = self.conns_out[0].tgt.x
        syn_start_y = self.conns_out[0].tgt.y_loc // self.neurons_per_syn_y
        syn_start_x = self.conns_out[0].tgt.x_loc // self.neurons_per_syn_x

        self.mapped_taps_unshifted = [[(*self.tap_map(t, tgt_pool_width), s) for t, s in dim_taps]
                                      for dim_taps in self.taps_and_signs]
        self.mapped_taps = [[(x + syn_start_x, y + syn_start_y, s) for x, y, s in dim_taps]
                            for dim_taps in self.mapped_taps_unshifted]

        self.tap_ys = []
        self.tap_xs = []
        self.signs = []
        self.stops = []
        for dim_taps in self.mapped_taps:
            for t_idx, (syn_x, syn_y, s) in enumerate(dim_taps):
                self.tap_ys.append(syn_y)
                self.tap_xs.append(syn_x)
                self.signs.append(bin_sign(s))
                logger.debug("HWR: x %d  y %d", syn_x, syn_y)
                if t_idx % 2 == 1:
                    stop = 1*(t_idx == self.Ks[-1] - 1)
                    self.stops.append(stop)
        self.contents = bddriver.Driver.PackTATSpikeWords(
            self.tap_xs, self.tap_ys, self.signs, self.stops)

        assert len(self.contents) == self.size
        self.contents = np.array(self.contents, dtype=object)

    def assign(self, core):
        core.TAT1.assign(self.contents, self.start_addr)

        for tx, ty in zip(self.tap_xs, self.tap_ys):
            core.neuron_array.syns_used.append((tx, ty))

class TATFanout(Resource):
    """Represents a Tag Action Table entry for fanning out tags

    XXX should implement output slicing
    """
    def __init__(self, D):
        super().__init__([AMBuckets, Source, TATFanout],
                         [TATAccumulator, TATTapPoint, TATFanout, Sink],
                         sliceable_in=True, sliceable_out=False)
        self.D = D
        self.clobber_pre_len = 2
        self.clobber_post_len = 1

        # pretranslate
        self.size = None
        self.start_offsets = None

        # allocate
        self.start_addr = None
        self.in_tags = None

        # posttranslate
        self.contents = None


    @property
    def dimensions_in(self):
        return self.D

    @property
    def dimensions_out(self):
        return self.D

    # note, there's a hack in here to work around the "repeated/clobbered" outputs problem
    # we need to surround all TATFanouts with two dummy fanouts before the actual fanouts,
    # and one dummy fanout after. These will clobber/be clobbered instead of the actual
    # messages

    @property
    def has_Sink_output(self):
        for conn in self.conns_out:
            if isinstance(conn.tgt, Sink):
                return True
        return False

    @property
    def fanout_entries(self):
        if self.has_Sink_output:
            entries = len(self.conns_out) + self.clobber_pre_len + self.clobber_post_len
        else:
            entries = len(self.conns_out)
        return entries

    # highest tag value is the clobber tag
    # can't be used for anything else
    def clobber_tag(self, core):
        return 2 * core.TAT_size - 1

    def pretranslate(self, core):
        self.size = self.D * self.fanout_entries
        # D sets, each with self.fanout_entries entries
        self.start_offsets = np.array(range(self.D)) * self.fanout_entries

    def allocate(self, core):
        self.start_addr = core.TAT1.allocate(self.size)
        self.in_tags = self.start_addr + self.start_offsets + core.TAT_size
        #logger.debug("IN TAGS", self.in_tags)

    def posttranslate(self, core):

        clobber_base = bddriver.PackWord([
            (bddriver.TATTagWord.STOP, 0),
            (bddriver.TATTagWord.TAG, self.clobber_tag(core)),
            (bddriver.TATTagWord.GLOBAL_ROUTE, HOME_ROUTE)])
        clobber_stop = bddriver.PackWord([
            (bddriver.TATTagWord.STOP, 1),
            (bddriver.TATTagWord.TAG, self.clobber_tag(core)),
            (bddriver.TATTagWord.GLOBAL_ROUTE, HOME_ROUTE)])

        clobber_pre = self.clobber_pre_len * [clobber_base]
        if self.clobber_post_len > 1:
            clobber_post = (self.clobber_post_len - 1) * [clobber_base] + [clobber_stop]
        else:
            clobber_post = [clobber_stop]

        self.contents = []

        for d in range(self.D):
            # prepend clobber entry
            if self.has_Sink_output:
                self.contents += clobber_pre

            for t in range(len(self.conns_out)):
                if self.has_Sink_output:
                    stop = 0 # stop provided by clobber
                else:
                    stop = 1 * (t == len(self.conns_out) - 1)

                tgt = self.conns_out[t].tgt
                tag = tgt.in_tags[d]

                if isinstance(self.conns_out[t].tgt, Sink):
                    global_route = HOME_ROUTE
                else:
                    global_route = 0

                self.contents += [bddriver.PackWord([
                    (bddriver.TATTagWord.STOP, stop),
                    (bddriver.TATTagWord.TAG, tag),
                    (bddriver.TATTagWord.GLOBAL_ROUTE, global_route)])]

            # postpend clobber entry
            if self.has_Sink_output:
                self.contents += clobber_post

        self.contents = np.array(self.contents, dtype=object)
        assert len(self.contents) == self.size

    def assign(self, core):
        core.TAT1.assign(self.contents, self.start_addr)

class Sink(Resource):
    """Represents an FPGA SpikeFilter (tag filter)"""
    def __init__(self, D):
        super().__init__([Neurons, TATFanout, AMBuckets], [],
                         sliceable_in=True, sliceable_out=False, max_conns_out=0)

        self.num = None
        self.D = D

        # allocate
        self.filter_idxs = None
        self.in_tags = None

    @property
    def dimensions_in(self):
        return self.D

    @property
    def dimensions_out(self):
        raise NameError(
            "A Sink Resource does not have a defined dimensions_out; " +
            "only dimensions_in is defined")

    def allocate(self, core):
        self.filter_idxs = core.FPGASpikeFilters.allocate(self.D)
        # other Resources expect this name, but filter_idxs is more accurate
        self.in_tags = self.filter_idxs

class Source(Resource):
    """Represents an FPGA SpikeGenerator (tag generator)"""
    def __init__(self, D):
        super().__init__([], [TATTapPoint, TATAccumulator, TATFanout],
                         sliceable_in=False, sliceable_out=True, max_conns_in=0, max_conns_out=1)
        self.D = D

        # allocate
        self.generator_idxs = None

        # posttranslate
        self.out_tags = None

    @property
    def dimensions_in(self):
        raise NameError(
            "A Source Resource does not have a defined dimensions_in; " +
            "only dimensions_out is defined")

    @property
    def dimensions_out(self):
        return self.D

    def allocate(self, core):
        self.generator_idxs = core.FPGASpikeGenerators.allocate(self.D)

    def posttranslate(self, core):
        self.out_tags = self.conns_out[0].tgt.in_tags
