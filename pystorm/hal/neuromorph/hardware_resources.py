"""This module defines the hardware resources of braindrop/brainstorm"""
from abc import ABC, abstractmethod, abstractproperty
import numpy as np
from pystorm.PyDriver import bddriver

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

    def pretranslate(self, core):
        """Perform any bookeeping that's needed before allocation
        This might include things like determining how to break up matrices or pools
        into the chunks that go into the allocator.
        """
        pass

    def allocate_early(self, core):
        """Some objects need to be allocated before others, the first set of objects is allocated here"""
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
    def __init__(self, y, x):
        super().__init__(
            [TATTapPoint], [MMWeights, Sink],
            sliceable_in=False, sliceable_out=False,
            max_conns_out=1)
        self.y = y
        self.x = x
        self.N = y * x

        # pretranslate
        # y and x sizes in units of minimum pool x and y dimensions
        self.py = None
        self.px = None

        # allocate
        # y and x locations in units of minimum pool x and y dimensions
        self.py_loc = None
        self.px_loc = None
        self.start_nrn_idx = None

        # posttranslate
        self.PAT_contents = None

    @property
    def dimensions_in(self):
        return self.N

    @property
    def dimensions_out(self):
        return self.N

    def pretranslate(self, core):
        """neuron array allocation prep"""

        # round size y/x up to the number of pools needed
        self.py = int(np.ceil(self.y / core.NeuronArray_pool_size_y))
        self.px = int(np.ceil(self.x / core.NeuronArray_pool_size_x))

        # let the allocator know about it
        core.NeuronArray.AddPool(self)

    def allocate(self, core):
        """neuron array allocation"""
        self.py_loc, self.px_loc = core.NeuronArray.Allocate(self)
        self.start_nrn_idx = (self.py_loc * core.NeuronArray.pools_x + self.px_loc) * core.NeuronArray_pool_size

    def posttranslate(self, core):
        """PAT assignment setup"""
        assert(len(self.conns_out) == 1) # assert no more than one connection (only one decoder allowed)

        weights = self.conns_out[0].tgt
        buckets = self.conns_out[0].tgt.conns_out[0].tgt

        self.PAT_contents = np.empty((self.py, self.px), dtype=object)

        for py_idx in range(self.py):
            for px_idx in range(self.px):

                nrn_idx = (py_idx * self.px + px_idx) * core.NeuronArray_pool_size # first nrn idx in block
                MMAY, MMAX = weights.InDimToMMA(nrn_idx)

                AMA = buckets.start_addr

                self.PAT_contents[py_idx, px_idx] = bddriver.PackWord([
                    (bddriver.PATWord.AM_ADDRESS, AMA),
                    (bddriver.PATWord.MM_ADDRESS_LO, MMAX),
                    (bddriver.PATWord.MM_ADDRESS_HI, MMAY)])

    def assign(self, core):
        """PAT assignment"""
        core.PAT.Assign(self.PAT_contents, (self.py_loc, self.px_loc))

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
            xonesc[neg] = invert_bits(-xonesc[neg], max_weight) # max_weight is all ones

            return xonesc

        # this is the threshold value we should use (and scale the user weights by, in this case)
        _, thr_vals = AMBuckets.thr_idxs_vals(max_abs_row_weights, core)

        W = (user_W.T * thr_vals).T
        W = np.round(W) # XXX other ways to do this, possibly better to round probabilistically

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
        self.W_slice_idxs = []
        self.is_dec = None

        # allocate (by AMBuckets)
        self.slice_start_addrs = []

        # posttranslate
        self.programmed_W = None
        self.W_slices = []
        self.W_slices_BDWord = []


    def InDimToMMA(self, dim):
        """Calculate the max x and y coordinates in main memory associated with dim"""
        slice_width = self.dimensions_in
        slice_idx = dim // slice_width
        slice_offset = dim % slice_width
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
        """Determine matrix slicing. Decoders are broken into 64 entry tall slices, Transforms into 1 entry tall slices"""

        # first, decide if this is a decoder or a transform
        if len(self.conns_in) > 0:
            self.is_dec = isinstance(self.conns_in[0].src, Neurons)
        else:
            self.is_dec = False
            print("Warning: in pretranslate, odd situation: MMWeights with no input connection")

        # generate slice indexing
        if self.is_dec: # can chop up decoders into NeuronArray_pool_size-column chunks
            slice_width = core.NeuronArray_pool_size
        else: # can chop up transforms by single columns
            slice_width = 1
        
        self.W_slice_idxs = [(i, min(self.user_W.shape[1], i+slice_width)) for i in range(0, self.user_W.shape[1], slice_width)]

    def allocate_early(self, core):
        """allocate decoders (big slices) first"""
        if self.is_dec:
            for W_slice_idx in self.W_slice_idxs: 
                # actually doesn't matter what the slice is, always allocate 64xDO
                start_addr = core.MM.AllocateDec(self.dimensions_out)
                self.slice_start_addrs += [start_addr]

    def allocate(self, core):
        """allocate transforms (small slices) after"""
        if not self.is_dec:
            for W_slice_idx in self.W_slice_idxs:
                # actually doesn't matter what the slice is, always allocate 1xDO
                start_addr = core.MM.AllocateTrans(self.dimensions_out)
                self.slice_start_addrs += [start_addr]

    def posttranslate(self, core):
        """calculate implemented weights"""
        # look at AMBuckets.max_user_W, compute weights to program
        self.programmed_W = MMWeights.weight_to_mem(self.user_W, self.conns_out[0].tgt.max_abs_row_weights, core)

        # slice up W according to slice indexing
        for W_slice_idx in self.W_slice_idxs:
            self.W_slices += [self.programmed_W[:, W_slice_idx[0]:W_slice_idx[1]]]

        # Pack into BDWord (which doesn't actually do anything)
        for W_slice in self.W_slices:
            contents = [
                [bddriver.PackWord([(bddriver.MMWord.WEIGHT, W_slice[i, j])]) for j in range(W_slice.shape[1])] 
            for i in range(W_slice.shape[0])]

            self.W_slices_BDWord += [np.array(contents, dtype=object)]

    def assign(self, core):
        for W_slice, start_addr in zip(
                self.W_slices_BDWord, self.slice_start_addrs):
            if self.is_dec:
                # transpose, the memory is flipped from linalg orientation
                core.MM.AssignDec(W_slice.T, start_addr)
            else:
                # 1D slices in trans row case
                core.MM.AssignTrans(W_slice, start_addr)

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
        thr_idxs = np.searchsorted(-user_max_weights_for_thr_vals, -max_abs_row_weights) - 1

        thr_vals = all_thr_vals[thr_idxs]

        return thr_idxs, thr_vals

    def __init__(self, D):
        super().__init__([MMWeights], [TATAccumulator, TATFanout, TATTapPoint, Sink],
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
        self.start_addr = core.AM.Allocate(self.dimensions_out)

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
        core.AM.Assign(self.AM_entries, self.start_addr)

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
        self.start_offsets = np.array(
            range(self.D)) * len(self.conns_out) # D acc sets, each with len(conns_out) fanout

    def allocate(self, core):
        self.start_addr = core.TAT0.Allocate(self.size)
        self.in_tags = self.start_addr + self.start_offsets

    def posttranslate(self, core):
        self.contents = []
        for d in range(self.D):
            for t in range(len(self.conns_out)):
                weights = self.conns_out[t].tgt
                buckets = self.conns_out[t].tgt.conns_out[0].tgt

                AMA = buckets.start_addr
                MMAY, MMAX = weights.InDimToMMA(d)
                MMA = MMAY * core.MM_width + MMAX
                stop = 1 * (t == len(self.conns_out) - 1)

                self.contents += [bddriver.PackWord([
                    (bddriver.TATAccWord.STOP, stop),
                    (bddriver.TATAccWord.AM_ADDRESS, AMA),
                    (bddriver.TATAccWord.MM_ADDRESS, MMA)])]
        self.contents = np.array(self.contents, dtype=object)

    def assign(self, core):
        core.TAT0.Assign(self.contents, self.start_addr)

class TATTapPoint(Resource):
    """XXX not supporting fanout to multiple pools (and therefore no output slicing).
    Using TATFanout for that, also apparently not supporting non-square taps (different K)
    """
    def __init__(self, taps, signs, N):
        super().__init__([AMBuckets, Source, TATFanout], [Neurons],
                         sliceable_in=True, sliceable_out=False, max_conns_out=1)

        self.N = N
        self.D = taps.shape[1]
        self.K = taps.shape[0]
        self.taps = taps
        self.signs = signs

        assert self.K % 2 == 0 and "need even number of tap points"
        assert self.taps.all() < self.N

        # pretranslate
        self.nrn_per_syn = None
        self.size = None
        self.start_offsets = None

        # allocate
        self.start_addr = None
        self.in_tags = None

        # posttranslate
        self.mapped_taps = None
        self.contents = None

    @property
    def dimensions_out(self):
        return self.N

    @property
    def dimensions_in(self):
        return self.D

    def tap_map(self, tap):
        """allow user to specify tap points at neuron granularity
        but actually there are fewer synapses than neurons
        XXX FIXME need to talk about this
        """
        return tap // self.nrn_per_syn

    def pretranslate(self, core):
        self.nrn_per_syn = core.NeuronArray_neurons_per_tap
        self.size = self.D * self.K // 2
        self.start_offsets = np.array(
            range(self.D)) * self.K // 2 # D acc sets, each with len(conns_out) fanout

    def allocate(self, core):
        self.start_addr = core.TAT1.Allocate(self.size)
        self.in_tags = self.start_addr + self.start_offsets + core.TAT_size // 2 # in TAT1

    def posttranslate(self, core):

        def bin_sign(sign):
            return (sign + 1) // 2

        self.mapped_taps = self.tap_map(self.conns_out[0].tgt.start_nrn_idx + self.taps)

        self.contents = []
        for d in range(self.D):
            for k in range(0, self.K, 2):
                stop = 1*(k == self.K - 2)
                tap0 = self.mapped_taps[k, d]
                s0 = bin_sign(self.signs[k, d])
                tap1 = self.mapped_taps[k+1, d]
                s1 = bin_sign(self.signs[k+1, d])

                self.contents += [bddriver.PackWord([
                    (bddriver.TATSpikeWord.STOP, stop),
                    (bddriver.TATSpikeWord.SYNAPSE_ADDRESS_0, tap0),
                    (bddriver.TATSpikeWord.SYNAPSE_SIGN_0, s0),
                    (bddriver.TATSpikeWord.SYNAPSE_ADDRESS_1, tap1),
                    (bddriver.TATSpikeWord.SYNAPSE_SIGN_1, s1)])]
        self.contents = np.array(self.contents, dtype=object)

    def assign(self, core):
        core.TAT1.Assign(self.contents, self.start_addr)

class TATFanout(Resource):
    """Represents a Tag Action Table entry for fanning out tags
    
    XXX should implement output slicing
    """
    def __init__(self, D):
        super().__init__([AMBuckets, Source, TATFanout],
                         [TATAccumulator, TATTapPoint, TATFanout, Sink],
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
        self.start_offsets = np.array(
            range(self.D)) * len(self.conns_out) # D acc sets, each with len(conns_out) fanout

    def allocate(self, core):
        self.start_addr = core.TAT1.Allocate(self.size)
        self.in_tags = self.start_addr + self.start_offsets + core.TAT_size // 2 # in TAT1

    def posttranslate(self, core):
        self.contents = []
        for d in range(self.D):
            for t in range(len(self.conns_out)):
                stop = 1 * (t == len(self.conns_out) - 1)
                tgt = self.conns_out[t].tgt
                tag = tgt.in_tags[d]
                global_route = 0

                self.contents += [bddriver.PackWord([
                    (bddriver.TATTagWord.STOP, stop),
                    (bddriver.TATTagWord.TAG, tag),
                    (bddriver.TATTagWord.GLOBAL_ROUTE, global_route)])]
        self.contents = np.array(self.contents, dtype=object)

    def assign(self, core):
        core.TAT1.Assign(self.contents, self.start_addr)

class Sink(Resource):
    """Represents a sink"""
    def __init__(self, D):
        super().__init__([Neurons, TATFanout, AMBuckets], [],
                         sliceable_in=True, sliceable_out=False, max_conns_out=0)

        self.num = None
        self.D = D

        # allocate
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
        self.in_tags = core.ExternalSinks.Allocate(self.D)
        # XXX this needs to propagate to the FPGA somehow

class Source(Resource):
    """Represents a source"""
    def __init__(self, D):
        super().__init__([], [TATTapPoint, TATAccumulator, TATFanout],
                         sliceable_in=False, sliceable_out=True, max_conns_in=0)
        self.D = D

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

    def posttranslate(self, core):
        self.out_tags = [conn.tgt.in_tags for conn in self.conns_out]

    def allocate(self, core):
        # XXX eventually, allocate something on FPGA
        pass
