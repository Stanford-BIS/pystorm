import numpy as np
from bitutils import *
from Core import *

# make sure that a slice connection between object doesn't break any rules.
# objects must define DI() and DO()
def TestDimsCorrect(src_DO, tgt_DI, src_range, tgt_range):
    # range lengths must match
    assert(len(src_range) == len(self.tgt_range))

    # indices must be valid dimensions
    for i in src_range:
        assert(i < src_DO)
    for i in tgt_range:
        assert(i < tgt_DI)

    # no multiple connection
    assert(len(src_range) < tgt_DI)
    assert(len(tgt_range) < src_DO)

# ResourceConnection connects two resources, allows slicing
class ResourceConnection(object):
    def __init__(self, src, tgt, src_range=None, tgt_range=None):
        self.src = src
        self.tgt = tgt
        self.matrix = matrix

        # type check
        assert(type(tgt) in src.connectable_types_out)
        assert(type(src) in tgt.connectable_types_in)

        # sliceability check
        if src_range != None:
            assert(src.slicable_out)
        if tgt_range != None:
            assert(tgt.slicable_in)

        # range == None is full range of src/tgt objects outputs/inputs
        self.src_range = src_range
        self.tgt_range = tgt_range
        if src_range == None:
            self.src_range = range(src.DO()) # XXX possibly wasteful--can change to xrange?
        if tgt_range == None:
            self.tgt_range = range(tgt.DI())

        # check dimensional correctness, left and right side of matrix
        TestDimsCorrect(src.DO(), tgt.DI(), self.src_range, self.tgt_range)


# Resources represent chunks of un-allocated braindrop hardware
# "a graph of hardware resources needed to implement the network"
# most of the functionality of the objects is in the allocation process
# basically, Resource objects know how to map themselves to a Core object
class Resource(object):

    def __init__(self, connectable_types_in, connectable_types_out, slicable_in=True, slicable_out=True):
        self.connectable_types_in = connectable_types_in
        self.connectable_types_out = connectable_types_out
        self.slicable_in = slicable_in
        self.slicable_out = slicable_out

        self.conns_in = []
        self.conns_out = []

    # some common methods that work similarly for all Resources

    # get input dimensionality
    def DI(self):
        assert(False and "not implemented!")

    # get output dimensionality
    def DO(self):
        assert(False and "not implemented!")

    # connect this Resource to another one, possibly using a slice of this object's 
    # DO() range or a slice of the tgt's DI() range
    def Connect(self, tgt, src_range, tgt_range):
        new_conn = ResourceConnection(self, tgt, src_range, tgt_range)
        self.conns_out.append(new_conn)
        tgt.conns_in.append(new_conn)

    # The mapping process is divided into phases

    # perform any bookeeping that's needed before Allocate
    # this could be done during construction, but we give it a separate phase
    def PreTranslate(self, core):
        pass

    # XXX hack for something Matrix needed
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
class Neurons(Resource):
    def __init__(self, N):
        # XXX syntax probably wrong
        super(Resource).__init__(self, [TATTapPoint], [MMWeights, Sink], False, False);
        self.N = N

        # PreTranslate
        self.n_unit_pools = None

        # Allocate
        self.start_pool_idx = None
        self.start_nrn_idx_idx = None

    def DI(self):
        return self.N

    def DO(self):
        return self.N

    def PreTranslate(self, core):
        self.n_unit_pools = int(np.ceil(self.N / core.NPOOL))

    def Allocate(self, core):
        self.start_pool_idx = core.NeuronArray.Allocate(self.n_unit_pools)
        self.start_nrn_idx_idx = self.start * core.NPOOL

# entries in weight (main) memory
# this is a *little* special because of weight matrix sharing
# keeps a static class member dictionary to keep track of which MMWeights
# used the same memory entries
# XXX later can code this up to automate matrix caching
class MMWeights(Resource):

    # XXX this is probably the wrong syntax
    # keep track of MMWeights -> unique hash id for combined matrices
    MMWeights.forward_cache = {}
    # keep track of unique hash ids for combined matrices -> MMWeights
    MMWeights.reverse_cache = {}

    def __init__(self, W, is_dec, hash_id=None):
        # XXX syntax probably wrong
        super(Resource).__init__([Neurons, TATAcc], [AMBuckets], slicable_in=True, slicable_out=False)

        self.user_W = W
        self.is_dec = is_dec
        self.DI = user_W.shape[0]
        self.DO = user_W.shape[1]

        if hash_id == None: # user says this isn't shared
            # no hash specified, just use your own object id
            MMWeights.forward_cache[self] = self 
            MMWeights.reverse_cache[self].append(self)
        else: # this uses a shared weight matrix
            MMWeights.forward_cache[self] = hash_id
            MMWeights.reverse_cache[hash_id] = self

        # filled in PreTranslate
        self.W_programmed = None

        # filled in allocation
        self.start_addr = None

# entries in accumulator memory
# an array of buckets, defined by their threshold values and next address targets
# in the hardware, the last bucket has the stop bit
class AMBuckets(Resource):
    def __init__(self, D):
        # XXX syntax probably wrong
        super(Resource).__init__([Neurons, TATAcc], [TATAcc, TATFanout, TATTapPoint, Sink], slicable_in=False, slicable_out=True)
        self.dims = D

        # filled in PreTranslate
        self.thr = None

        # filled in allocation
        self.start_addr = None

        # filled in PostTranslate
        self.AM_entries = None

    # AMBucket can connect to TAT* or Sink
    # if start_idx == None, then connect all buckets to target indices (checks dimensions)

    def WeightToMem(self, W, core):
        # first, determine bucket thresholds
        #print W
        max_row_W = np.max(np.abs(W), axis=1) # biggest weight in each row (feeding each bucket)
        assert np.max(max_row_W) <= 127/64. # weights are basically in (-2, 2) (open set)

        thr_multiples = np.array([2**(t-1) for t in xrange(2**core.MTHR)]) # 1/(possible threshold values)
        #print thr_multiples
        #print max_row_W
        thr = np.searchsorted(thr_multiples, 1./max_row_W) - 1 # programmed thr memory entries
        #print thr
        thr_vals = thr_multiples[thr]
        #print thr_vals
        W_programmed = (W.T * thr_vals).T # we get to make the weights effectively larger, preserving dynamic range when they are very small
        W_programmed *= 2**(core.MW-1)
        #print W_programmed
        W_programmed = np.round(W_programmed) # better ways to do this
        W_programmed[W_programmed == 2**(core.MW-1)] = 2**(core.MW-1)-1 # XXX probably not the right thing to do math-wise, but close
        W_programmed = dec2onesc(W_programmed, core.MW)

        Wfoo = W_programmed / 2**(core.MW-1)
        #print Wfoo
        #print "########", Wfoo.dot(np.ones((Wfoo.shape[1],1))*10000)
        return W_programmed, thr

    def InDimToMMA(self, dim):
        slice_width = self.Wslices[0].shape[1]
        slice_idx = dim / slice_width
        slice_offset = dim % slice_width
        MM_start = self.MM_starts[slice_idx]
        MMAY = MM_start[0] + slice_offset
        MMAX = MM_start[1]
        return [MMAY, MMAX]

    def PreTranslate(self, core):
        # first, derive thresholds and weight matrix entries
        #print self.W
        self.W_programmed, self.thr = self.WeightToMem(self.W, core)

        # break weight matrix into per-pool (or per-dim, for transforms) slices
        if self.isDec:
            slice_width = core.NPOOL
        else:
            slice_width = 1
        
        n = 0
        while n * slice_width < self.W_programmed.shape[1]:
            lo = n * slice_width
            hi = min(self.W_programmed.shape[1], (n+1) * slice_width)
            Wslice = self.W_programmed[:, lo:hi]
            self.Wslices += [Wslice]
            n += 1
        #print self.Wslices

    def AllocateEarly(self, core):
        # allocate MM slices for each pool dec
        if self.isDec:
            for Wslice in self.Wslices:
                start = core.MM.AllocateDec(self.DO)
                self.MM_starts += [start]

            # allocate AM
            self.AM_start = core.AM.Allocate(self.DO)

    def Allocate(self, core):
        # allocate transforms after
        if not self.isDec:
            for Wslice in self.Wslices:
                start = core.MM.AllocateTrans(self.DO)
                self.MM_starts += [start]

            # allocate AM
            self.AM_start = core.AM.Allocate(self.DO)


    def PostTranslate(self, core):
        # we need to create the (stop, value, thresholds) and next address entries for the AM
        stop = np.zeros((self.DO,)).astype(int)
        stop[-1] = 1
        val = np.zeros((self.DO,)).astype(int)
        thr = self.thr

        assert len(self.tgts) == 1
        NAs = self.tgts[0].in_tags

        self.AM_entries = Pack([val, thr, stop, NAs], [core.MVAL, core.MTHR, 1, core.MAMW])

    def Assign(self, core):
        for Wslice, start in zip(self.Wslices, self.MM_starts):
            if self.isDec:
                core.MM.AssignDec(Wslice.T, start) # transpose, the memory is flipped from linalg orientation
            else:
                core.MM.AssignTrans(Wslice, start) # 1D slices in trans row case
        core.AM.Assign(self.AM_entries, self.AM_start)


class TATAccumulator(Resource):
    def __init__(self, tgts):
        # XXX syntax probably wrong
        super(Resource).__init__([AMBuckets, Source, TATFanout], [MMWeights], slicable_in=True, slicable_out=True)

        self.tgts = tgts
        self.D = tgts[0].DI
        assert sum([t.DI == self.D for t in tgts]) == len(tgts)
        assert sum([isinstance(t, Matrix) for t in tgts]) == len(tgts)
        self.res_type = 0

        # PreTranslate
        self.size = None
        self.start_offsets = None

        # Allocate
        self.start = None
        self.in_tags = None

        # PostTranslate
        self.contents = None

    def PreTranslate(self, core):
        self.size = self.D * len(self.tgts)
        self.start_offsets = np.array(range(self.D)) * len(self.tgts) # D acc sets, each with len(tgts) fanout

    def Allocate(self, core):
        self.start = core.TAT0.Allocate(self.size)
        self.in_tags = self.start + self.start_offsets

    def PostTranslate(self, core):
        MMAs = []
        AMAs = []
        stops = []
        for d in xrange(self.D):
            for t in xrange(len(self.tgts)):
                stop = 1*(t == len(self.tgts)-1)
                MMAY, MMAX = self.tgts[t].InDimToMMA(d)
                AMA = self.tgts[t].AM_start
                MMAs += [Pack([MMAX, MMAY], [core.MMMAX, core.MMMAY])]
                AMAs += [AMA]
                stops += [stop]

        MMAs = np.array(MMAs)
        AMAs = np.array(AMAs)
        stops = np.array(stops)
        self.contents =  Pack([stops, self.res_type, AMAs, MMAs], [1, 2, core.MAMA, core.MMMA])
        assert self.contents.shape[0] == self.size

    def Assign(self, core):
        core.TAT0.Assign(self.contents, self.start)


class TATTapPoint(Resource):
    def __init__(self, taps, signs, pool):
        # XXX syntax probably wrong
        super(Resource).__init__([AMBuckets, Source, TATFanout], [Neurons], slicable_in=True, slicable_out=False)

        self.start = None
        self.D = taps.shape[1]
        self.K = taps.shape[0]
        assert self.K % 2 == 0 and "need even number of tap points"
        assert taps.shape == signs.shape
        self.taps = taps
        self.signs = signs
        self.res_type = 1
        self.tgts = [pool]

        def DO(self):
            # XXX it's complicated
            # N-ish

        # PreTranslate
        self.size = None

        # Allocate
        self.start = None
        self.in_tags = None

        # PostTranslate
        self.contents = None

    def TapMapFn(self, tap):
        # allow user to specify tap points at neuron granularity
        # but actually there are fewer synapses than neurons
        return tap / self.nrn_per_syn

    def PreTranslate(self, core):
        self.nrn_per_syn = 2**(core.MTATTapPoint - core.MTAP)
        self.size = self.D * self.K / 2
        self.start_offsets = np.array(range(self.D)) * self.K / 2 # D acc sets, each with len(tgts) fanout

    def Allocate(self, core):
        self.start = core.TAT1.Allocate(self.size)
        self.in_tags = self.start + self.start_offsets + 2**core.MTAG / 2 # in TAT1

    def PostTranslate(self, core):
        contents = []
        self.mapped_taps = self.TapMapFn(self.tgts[0].start_nrn_idx + self.taps)
        #print self.tgts[0].N
        #print self.nrn_per_syn
        #print self.mapped_taps
        #print self.tgts[0].start_nrn_idx
        for d in xrange(self.D):
            for k in xrange(0, self.K, 2):
                stop = 1*(k == self.K - 2)
                tap0 = self.mapped_taps[k, d]
                s0 = self.signs[k, d]
                tap1 = self.mapped_taps[k+1, d]
                s1 = self.signs[k+1, d]
                #print tap0, tap1
                data = [stop, self.res_type, tap0, s0, tap1, s1]
                widths = [1, 2, core.MTAP, 1, core.MTAP, 1]
                contents += [Pack(data, widths)]
        self.contents = np.array(contents)
        assert self.contents.shape[0] == self.size

    def Assign(self, core):
        core.TAT1.Assign(self.contents, self.start)


class TATFanout(Resource):
    def __init__(self, tgts):
        # XXX syntax probably wrong
        super(Resource).__init__([AMBuckets, Source, TATFanout], [TATAccumulator, TATTapPoint, TATFanout, Sink], slicable_in=True, slicable_out=False)

        self.start = None
        self.tgts = tgts
        self.D = tgts[0].D
        assert sum([t.D == self.D for t in tgts]) == len(tgts)
        self.res_type = 2

        # PreTranslate
        self.size = None

        # Allocate
        self.start = None
        self.in_tags = None

        # PostTranslate
        self.contents = None

    def PreTranslate(self, core):
        self.size = self.D * len(self.tgts)
        self.start_offsets = np.array(range(self.D)) * len(self.tgts) # D acc sets, each with len(tgts) fanout

    def Allocate(self, core):
        self.start = core.TAT1.Allocate(self.size)
        self.in_tags = self.start + self.start_offsets + 2**core.MTAG / 2 # in TAT1

    def PostTranslate(self, core):
        contents = []
        for d in xrange(self.D):
            for t in xrange(len(self.tgts)):
                stop = 1*(t == len(self.tgts) - 1)
                tgt = self.tgts[t]
                data = [stop, self.res_type, tgt.in_tags[d]]
                widths = [1, 2, core.MTAG + core.MGRT]
                contents += [Pack(data, widths)]
        self.contents = np.array(contents)

    def Assign(self, core):
        core.TAT1.Assign(self.contents, self.start)


class PAT(Resource):
    def __init__(self, nrn_arr, dec):
        self.nrn_arr = nrn_arr
        self.dec = dec
        self.tgts = None # unused

        # PostTranslate
        self.contents = None

        # PAT is direct-mapped so our allocation is determined
        # by allocation of Neuronss

    def PostTranslate(self, core):
        contents = []
        for p in xrange(self.nrn_arr.n_unit_pools):
            in_idx = p * core.NPOOL
            MMAY, MMAX = self.dec.InDimToMMA(in_idx)
            AMA = self.dec.AM_start
            MMAY_lo, MMAY_hi = Unpack(MMAY, [core.MPOOL])
            data = [AMA, MMAX, MMAY_hi]
            widths = [core.MAMA, core.MMMAX, core.MMMAY - core.MPOOL]
            contents += [Pack(data, widths)]
        self.contents = np.array(contents)
        assert len(self.contents) == self.nrn_arr.n_unit_pools

    def Assign(self, core):
        pool_slice = slice(self.nrn_arr.start_pool_idx, self.nrn_arr.start_pool_idx + self.nrn_arr.n_unit_pools)
        core.PAT.Assign(self.contents, pool_slice)


class Sink(Resource):
    def __init__(self, D):
        # XXX syntax probably wrong
        super(Resource).__init__([Neurons, TATFanout, AMBuckets], [], slicable_in=True, slicable_out=False)

        self.num = None
        self.D = D
        self.tgts = None # unused

        # Allocate
        self.in_tags = None

    def Allocate(self, core):
        ltags = core.ExternalSinks.Allocate(self.D)
        self.in_tags = Pack([ltags, core.COMP_ROUTE], [core.MTAG, core.MGRT])


class Source(Resource):
    def __init__(self, D, tgts):
        # XXX syntax probably wrong
        super(Resource).__init__([], [TATTapPoint, TATAccumulator, TATFanout], slicable_in=False, slicable_out=True)
        self.D = D
        self.tgts = tgts

        # PostTranslate
        self.out_tags = None

    def PostTranslate(self, core):
        self.out_tags = [t.in_tags for t in self.tgts]

