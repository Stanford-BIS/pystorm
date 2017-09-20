import numpy as np
from . Core import *
from . MemWordEnums import *
from . MemWordPlaceholders import *

# make sure that a slice connection between object doesn't break any rules.
# objects must define DI() and DO()
def TestDimsCorrect(src_DO, tgt_DI, src_range, tgt_range):
    # range lengths must match
    print(src_range, tgt_range)
    assert(len(src_range) == len(tgt_range))

    # indices must be valid dimensions
    for i in src_range:
        assert(i < src_DO)
    for i in tgt_range:
        assert(i < tgt_DI)

    # no multiple connection
    assert(len(src_range) <= tgt_DI)
    assert(len(tgt_range) <= src_DO)

# ResourceConnection connects two resources, allows slicing
class ResourceConnection(object):
    def __init__(self, src, tgt, src_range=None, tgt_range=None):
        self.src = src
        self.tgt = tgt

        # type check
        assert(type(src) in tgt.connectable_types_in)

        # sliceability check
        if src_range != None:
            assert(src.sliceable_out)
        if tgt_range != None:
            assert(tgt.sliceable_in)

        # number of connections check
        if src.max_conns_out is not None:
            assert(len(src.conns_out) + 1 <= src.max_conns_out)
        if tgt.max_conns_in is not None:
            assert(len(tgt.conns_in) + 1 <= tgt.max_conns_in)

        # range == None is full range of src/tgt objects outputs/inputs
        self.src_range = src_range
        self.tgt_range = tgt_range
        if src_range == None:
            self.src_range = range(src.DO())
        if tgt_range == None:
            self.tgt_range = range(tgt.DI())

        # check dimensional correctness
        TestDimsCorrect(src.DO(), tgt.DI(), self.src_range, self.tgt_range)


# Resources represent chunks of un-allocated braindrop hardware
# "a graph of hardware resources needed to implement the network"
# most of the functionality of the objects is in the allocation process
# basically, Resource objects know how to map themselves to a Core object
class Resource(object):

    def __init__(
            self, 
            connectable_types_in, # list of Resource types that can make an incoming connection to this object
            connectable_types_out,  # list of Resource types that outgoing connections from this object can go to
            sliceable_in=True,  # if the input dimension range can be sliced
            sliceable_out=True, # if the output dimension range can be sliced
            max_conns_in=None, # maximum number of incoming connections
            max_conns_out=None): # maximum number of outgoing connections

        self.connectable_types_in = connectable_types_in
        self.connectable_types_out = connectable_types_out
        self.sliceable_in = sliceable_in
        self.sliceable_out = sliceable_out
        self.max_conns_in = max_conns_in
        self.max_conns_out = max_conns_out

        self.conns_in = []
        self.conns_out = []

    # get input dimensionality
    def DI(self):
        assert(False and "not implemented!")

    # get output dimensionality
    def DO(self):
        assert(False and "not implemented!")

    # XXX should InTags() be a method?

    # connect this Resource to another one, possibly using a slice of this object's 
    # DO() range or a slice of the tgt's DI() range
    def Connect(
            self,
            tgt, # target Resource object
            src_range=None, # slice indexing of this object's output range to make connections from
            tgt_range=None): # slice indexing of tgt object's output range to make connections to

        new_conn = ResourceConnection(self, tgt, src_range, tgt_range)
        self.conns_out += [new_conn]
        tgt.conns_in += [new_conn]

    # The mapping process is divided into phases

    # perform any bookeeping that's needed before Allocate
    # this could sometimes be done during construction, but we give it a separate phase
    # some preprocessing also needs Core parameters, which aren't available during construction
    def PreTranslate(self, core):
        pass

    # 
    def AllocateEarly(self, core):
        pass

    # perform the primary operation, allocating the Core to the Resources
    def Allocate(self, core):
        pass

    # perform any bookeeping that's needed after Allocation before Assign
    def PostTranslate(self, core):
        pass

    # assign the Resources' values to the Core at the allocated locations
    # should be light on logic, anything elaborate should go in PostTranslate
    def Assign(self, core):
        pass

# a chunk of the neuron array needed to implement a logical pool of neurons
# also includes the direct-mapped PAT memory needed to get to the accumulator
class Neurons(Resource):
    def __init__(self, N):
        super().__init__([TATTapPoint], [MMWeights, Sink], 
                sliceable_in=False, sliceable_out=False, max_conns_out=1);
        self.N = N

        # PreTranslate
        self.n_unit_pools = None

        # Allocate
        self.start_pool_idx = None
        self.start_nrn_idx = None

        # PostTranslate
        self.PAT_contents = None

    def DI(self):
        return self.N

    def DO(self):
        return self.N

    # neuron array allocation prep
    def PreTranslate(self, core):
        self.n_unit_pools = int(np.ceil(self.N // core.NeuronArray_pool_size))

        # assert no more than one connection (only one decoder allowed)
        assert(len(self.conns_out) == 1)

    # neuron array allocation
    def Allocate(self, core):
        self.start_pool_idx = core.NeuronArray.Allocate(self.n_unit_pools)
        self.start_nrn_idx = self.start_pool_idx * core.NeuronArray_pool_size

    # PAT assignment setup
    def PostTranslate(self, core):
        if len(self.conns_out) == 1:
            weights = self.conns_out[0].tgt
            buckets = self.conns_out[0].tgt.conns_out[0].tgt
            self.PAT_contents = []
            for p in range(self.n_unit_pools):
                in_idx = p * core.NeuronArray_pool_size
                MMAY, MMAX = weights.InDimToMMA(in_idx)
                AMA = buckets.start_addr

                self.PAT_contents += [BDWord({
                        PATField.AM_ADDRESS: AMA,
                        PATField.MM_ADDRESS_LO: MMAX,
                        PATField.MM_ADDRESS_HI: MMAY})]
            self.PAT_contents = np.array(self.PAT_contents, dtype=object)

    # PAT assignment
    def Assign(self, core):
        if self.PAT_contents is not None:
            pool_slice = slice(self.start_pool_idx, self.start_pool_idx + self.n_unit_pools)
            core.PAT.Assign(self.PAT_contents, pool_slice)

# entries in weight (main) memory
# this is a *little* special because of weight matrix sharing
# keeps a static class member dictionary to keep track of which MMWeights
# used the same memory entries
# XXX later can code this up to automate matrix caching
class MMWeights(Resource):

    # XXX this is probably the wrong syntax
    # keep track of MMWeights -> unique id for combined matrices
    forward_cache = {}
    # keep track of unique ids for combined matrices -> list(MMWeights)
    reverse_cache = {}

    def __init__(self, W):
        super().__init__([Neurons, TATAccumulator], [AMBuckets], 
                sliceable_in=True, sliceable_out=False, max_conns_in=1, max_conns_out=1)

        self.user_W = W

        # do caching based on object id of W, object id of self
        MMWeights.forward_cache[id(self)] = id(W)
        if self in MMWeights.reverse_cache:
            MMWeights.reverse_cache[id(W)] += [id(self)]
        else:
            MMWeights.reverse_cache[id(W)] = [id(self)]

        # PreTranslate (XXX by AMBuckets's PreTranslate!)
        self.is_dec = None
        self.programmed_W = None
        self.W_slices = []

        # Allocate (by AMBuckets)
        self.slice_start_addrs = []

        # PostTranslate
        self.W_slices_contents = []

    def InDimToMMA(self, dim):
        slice_width = self.W_slices[0].shape[1]
        slice_idx = dim // slice_width
        slice_offset = dim % slice_width
        MM_start_addr = self.slice_start_addrs[slice_idx]
        MMAY = MM_start_addr[0] + slice_offset
        MMAX = MM_start_addr[1]
        return [MMAY, MMAX]

    def DI(self):
        return self.user_W.shape[1]

    def DO(self):
        return self.user_W.shape[0]

    def AllocateEarly(self, core):
        # allocate decoders (big slices) first
        if self.is_dec:
            for W_slice in self.W_slices:
                start_addr = core.MM.AllocateDec(self.DO())
                self.slice_start_addrs += [start_addr]

    def Allocate(self, core):
        # allocate transforms (small slices) after
        if not self.is_dec:
            for W_slice in self.W_slices:
                start_addr = core.MM.AllocateTrans(self.DO())
                self.slice_start_addrs += [start_addr]

    # convert to BDWord, even though it's just a single int
    def PostTranslate(self, core):
        for W_slice in self.W_slices:
            contents = [[BDWord({MMField.WEIGHT: W_slice[i,j]}) for j in range(W_slice.shape[1])] for i in range(W_slice.shape[0])]

            self.W_slices_contents += [np.array(contents, dtype=object)]


    def Assign(self, core):
        for W_slice, start_addr in zip(self.W_slices_contents, self.slice_start_addrs):
            if self.is_dec:
                core.MM.AssignDec(W_slice.T, start_addr) # transpose, the memory is flipped from linalg orientation
            else:
                core.MM.AssignTrans(W_slice, start_addr) # 1D slices in trans row case


# entries in accumulator memory
# an array of buckets, defined by their threshold values and next address targets
# in the hardware, the last bucket has the stop bit
class AMBuckets(Resource):

    # Given the user's desired weight matrix and the core parameters,
    # determine the best implementable weights and threshold values
    # is a namespace fn, not a member fn
    def WeightToMem(user_W, core):

        # convert decimal number to one's complement representation used in the hardware
        # XXX should be in driver?
        def dec2onesc(x, max_weight):

            def invert_bits(x, all_ones):
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

        # first, determine bucket thresholds
        user_max_abs_W_rows = np.max(np.abs(user_W), axis=1) # biggest weight in each row (feeding each bucket)

        # XXX With the threshold at the minimum value (64),
        # it's technically possible to have weights that behave sort of like they're > 1 or < -1.
        # To have well-defined behavior, with threshold = 64, the weights must be in [-64, 64]
        # this is equivalent to restricting the user weights to be in [-1, 1]
        assert np.max(user_max_abs_W_rows) <= 1

        # compute hardware threshold vals (64, 128, 256, ...)
        thr_vals = np.array(
                [core.min_threshold_value * 2**t for t in range(core.num_threshold_levels)]) 

        # compute max possible weight, in user-space, for each threshold value
        # the first value (for threshold = 64) is special, as noted above
        user_max_weights_for_thr_vals = np.array(
                [1.] + [core.max_weight_value / thr_val for thr_val in thr_vals[1:]])

        # find max_row_Ws in the user_max_weights we just computed
        # user_max_weights is descending, so we provide the sorter argument
        #sorter_idxs = np.array(range(len(thr_vals))[::-1])
        #thr_idxs = np.searchsorted(user_max_weights_for_thr_vals, user_max_abs_W_rows, sorter=sorter_idxs) - 1 
        thr_idxs = np.searchsorted(-user_max_weights_for_thr_vals, -user_max_abs_W_rows) - 1 

        # this is the threshold value we should use (and scale the user weights by)
        row_thr_vals = thr_vals[thr_idxs]

        W = (user_W.T * row_thr_vals).T
        W = np.round(W) # XXX other ways to do this, possibly better to round probabilistically

        W = dec2onesc(W, core.max_weight_value)

        return W, thr_idxs
    
    # extends WeightToMem to work for multiple matrices sharing the same bucket
    # works by stacking the matrices, calling the original function, then unstacking
    # is a namespace fn, not a member fn
    def WeightsToMem(W_list, core):
        W = np.hstack(W_list)

        programmed_W, thr = AMBuckets.WeightToMem(W, core)

        input_dims = [w.shape[1] for w in W_list]
        split_indices = np.cumsum(input_dims)

        if len(split_indices > 1):
            programmed_W_list = np.split(programmed_W, split_indices[:-1], axis=1)
        else:
            programmed_W_list = [programmed_W]

        return programmed_W_list, thr

    def __init__(self, D):
        super().__init__([MMWeights], [TATAccumulator, TATFanout, TATTapPoint, Sink], 
                sliceable_in=False, sliceable_out=True, max_conns_out=1)
        self.D = D

        # filled in PreTranslate
        self.thr = None

        # filled in allocation
        self.start_addr = None

        # filled in PostTranslate
        self.AM_entries = None

    # XXX these methods modify any attached MMWeights objects!

    def DI(self):
        return self.D

    def DO(self):
        return self.D

    def PreTranslate(self, core):
        # first, derive thresholds and weight matrix entries

        # collect all input matrices
        user_W_list = [conn.src.user_W for conn in self.conns_in]

        programmed_W_list, self.thr = AMBuckets.WeightsToMem(user_W_list, core)
        
        # iterate through all input MMWeights conns
        for conn, programmed_W in zip(self.conns_in, programmed_W_list):

            # assign programmed_W
            MM_weight = conn.src
            MM_weight.programmed_W = programmed_W

            # break weight matrix into per-pool (or per-dim, for transforms) slices

            if len(MM_weight.conns_in) > 0:
                MM_weight.is_dec = type(MM_weight.conns_in[0].src) == Neurons
            else:
                MM_weight.is_dec = False

            if MM_weight.is_dec: # can chop up decoders into NeuronArray_pool_size-column chunks
                slice_width = core.NeuronArray_pool_size
            else: # can chop up transforms by single columns
                slice_width = 1

            n = 0
            while n * slice_width < programmed_W.shape[1]:
                lo = n * slice_width
                hi = min(programmed_W.shape[1], (n+1) * slice_width)
                W_slice = programmed_W[:, lo:hi]
                MM_weight.W_slices += [W_slice]
                n += 1

    def Allocate(self, core):
        self.start_addr = core.AM.Allocate(self.DO())

    def PostTranslate(self, core):
        # we need to create the (stop, value, thresholds) and next address entries for the AM
        stop = np.zeros((self.DO(),)).astype(int)
        stop[-1] = 1
        val = np.zeros((self.DO(),)).astype(int)
        thr = self.thr

        if len(self.conns_out) > 0:
            NAs = self.conns_out[0].tgt.in_tags
        else:
            NAs = np.zeros_like(thr)

        self.AM_entries = []
        for s, v, t, n in zip(stop, val, thr, NAs):
            self.AM_entries += [BDWord({
                AMField.ACCUMULATOR_VALUE: v,
                AMField.THRESHOLD: t,
                AMField.STOP: s,
                AMField.NEXT_ADDRESS: n})]
        self.AM_entries = np.array(self.AM_entries, dtype=object)

    def Assign(self, core):
        core.AM.Assign(self.AM_entries, self.start_addr)


# XXX should upgrade for sliceable_out=True
class TATAccumulator(Resource):

    def __init__(self, D):
        super().__init__([AMBuckets, Source, TATFanout], [MMWeights], 
                sliceable_in=True, sliceable_out=False)

        self.D = D

        # PreTranslate
        self.size = None
        self.start_offsets = None

        # Allocate
        self.start_addr = None
        self.in_tags = None

        # PostTranslate
        self.contents = None

    def DI(self):
        return self.D

    def DO(self):
        return self.D

    def PreTranslate(self, core):
        self.size = self.D * len(self.conns_out)
        self.start_offsets = np.array(range(self.D)) * len(self.conns_out) # D acc sets, each with len(conns_out) fanout

    def Allocate(self, core):
        self.start_addr = core.TAT0.Allocate(self.size)
        self.in_tags = self.start_addr + self.start_offsets

    def PostTranslate(self, core):
        self.contents = []
        for d in range(self.D):
            for t in range(len(self.conns_out)):
                weights = self.conns_out[t].tgt
                buckets = self.conns_out[t].tgt.conns_out[0].tgt

                AMA = buckets.start_addr
                MMAY, MMAX = weights.InDimToMMA(d)
                stop = 1 * (t == len(self.conns_out) - 1)

                self.contents += [BDWord({
                    TATAccField.STOP: stop,
                    TATAccField.AM_ADDRESS: AMA,
                    TATAccField.MM_ADDRESS_LO: MMAX,
                    TATAccField.MM_ADDRESS_HI: MMAY})]
        self.contents = np.array(self.contents, dtype=object)

    def Assign(self, core):
        core.TAT0.Assign(self.contents, self.start_addr)


# XXX not supporting fanout to multiple pools (and therefore no output slicing). Using TATFanout for that.
# also apparently not supporting non-square taps (different K)
class TATTapPoint(Resource):
    def __init__(self, taps, signs, N):
        super().__init__([AMBuckets, Source, TATFanout], [Neurons], 
                sliceable_in=True, sliceable_out=False, max_conns_out=1)

        self.N = N
        self.D = taps.shape[0]
        self.K = taps.shape[1]
        self.taps = taps
        self.signs = signs

        assert self.K % 2 == 0 and "need even number of tap points"
        assert self.taps.all() < self.N

        # PreTranslate
        self.size = None

        # Allocate
        self.start_addr = None
        self.in_tags = None

        # PostTranslate
        self.contents = None

    def DO(self):
        return self.N

    def DI(self):
        return self.D

    def TapMapFn(self, tap):
        # allow user to specify tap points at neuron granularity
        # but actually there are fewer synapses than neurons
        # XXX FIXME need to talk about this
        return tap // self.nrn_per_syn

    def PreTranslate(self, core):
        self.nrn_per_syn = core.NeuronArray_neurons_per_tap
        self.size = self.D * self.K // 2
        self.start_offsets = np.array(range(self.D)) * self.K // 2 # D acc sets, each with len(conns_out) fanout

    def Allocate(self, core):
        self.start_addr = core.TAT1.Allocate(self.size)
        self.in_tags = self.start_addr + self.start_offsets + core.TAT_size // 2 # in TAT1

    def PostTranslate(self, core):
        self.mapped_taps = self.TapMapFn(self.conns_out[0].tgt.start_nrn_idx + self.taps)

        self.contents = []
        for d in range(self.D):
            for k in range(0, self.K, 2):
                stop = 1*(k == self.K - 2)
                tap0 = self.mapped_taps[k, d]
                s0 = self.signs[k, d]
                tap1 = self.mapped_taps[k+1, d]
                s1 = self.signs[k+1, d]

                self.contents += [BDWord({
                    TATSpikeField.STOP: stop,
                    TATSpikeField.SYNAPSE_ADDRESS_0: tap0,
                    TATSpikeField.SYNAPSE_SIGN_0: s0,
                    TATSpikeField.SYNAPSE_ADDRESS_1: tap1,
                    TATSpikeField.SYNAPSE_SIGN_1: s1})]
        self.contents = np.array(self.contents, dtype=object)

    def Assign(self, core):
        core.TAT1.Assign(self.contents, self.start_addr)


# XXX should implement output slicing
class TATFanout(Resource):
    def __init__(self, D):
        super().__init__([AMBuckets, Source, TATFanout], [TATAccumulator, TATTapPoint, TATFanout, Sink], 
                sliceable_in=True, sliceable_out=False)

        self.D = D

        # PreTranslate
        self.size = None

        # Allocate
        self.start_addr = None
        self.in_tags = None

        # PostTranslate
        self.contents = None

    def DI(self):
        return self.D

    def DO(self):
        return self.D

    def PreTranslate(self, core):
        self.size = self.D * len(self.conns_out)
        self.start_offsets = np.array(range(self.D)) * len(self.conns_out) # D acc sets, each with len(conns_out) fanout

    def Allocate(self, core):
        self.start_addr = core.TAT1.Allocate(self.size)
        self.in_tags = self.start_addr + self.start_offsets + core.TAT_size // 2 # in TAT1

    def PostTranslate(self, core):
        self.contents = []
        for d in range(self.D):
            for t in range(len(self.conns_out)):
                stop = 1 * (t == len(self.conns_out) - 1)
                tgt = self.conns_out[t].tgt
                tag = tgt.in_tags[d]
                global_route = 0

                self.contents += [BDWord({
                    TATTagField.STOP: stop,
                    TATTagField.TAG: tag,
                    TATTagField.GLOBAL_ROUTE: global_route})]
        self.contents = np.array(self.contents, dtype=object)

    def Assign(self, core):
        core.TAT1.Assign(self.contents, self.start_addr)


class Sink(Resource):
    def __init__(self, D):
        super().__init__([Neurons, TATFanout, AMBuckets], [], 
                sliceable_in=True, sliceable_out=False, max_conns_out=0)

        self.num = None
        self.D = D

        # Allocate
        self.in_tags = None

    def DI(self):
        return self.D

    def Allocate(self, core):
        self.in_tags = core.ExternalSinks.Allocate(self.D)


class Source(Resource):
    def __init__(self, D):
        super().__init__([], [TATTapPoint, TATAccumulator, TATFanout], 
                sliceable_in=False, sliceable_out=True, max_conns_in=0)
        self.D = D

        # PostTranslate
        self.out_tags = None

    def DO(self):
        return self.D

    def PostTranslate(self, core):
        self.out_tags = [conn.tgt.in_tags for conn in self.conns_out]

