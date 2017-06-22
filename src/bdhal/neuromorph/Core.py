from bitutils import *
import numpy as np
from FunnelHorn import *

class Core(object):
    def __init__(self, pars):
        # base parameters
        self.SELF_ROUTE = pars['SELF_ROUTE']
        self.MTAG = pars['MTAG']
        self.MGRT = pars['MGRT']
        self.MCT = pars['MCT']
        self.MTAP = pars['MTAP']
        self.MNRNY = pars['MNRNY']
        self.MNRNX = pars['MNRNX']
        self.MPOOL = pars['MPOOL']
        self.MAMA = pars['MAMA']
        self.MMMAY = pars['MMMAY']
        self.MMMAX = pars['MMMAX']
        self.MW = pars['MW']
        self.MVAL = pars['MVAL']
        self.MTHR = pars['MTHR']

        # constants
        self.MDELAY = 6
        self.MTOGGLE_PRE_FIFO = 2
        self.MTOGGLE_POST_FIFO = 2
        self.MTOGGLE_NRN_DUMP = 2
        self.MINIT_FIFO_HT = 1
        self.MDAC = 11
        self.MADC = 3
        self.MOVFLW = 1;

        # derived parameters
        self.MNRN = self.MNRNX + self.MNRNY
        self.NNRN = 2**self.MNRN
        self.NPOOL = 2**self.MPOOL
        self.MAMW = 1 + self.MVAL + self.MTHR
        self.MMMA = self.MMMAX + self.MMMAY
        self.MACC = self.MMMA + self.MAMA
        self.MFO = self.MTAG + self.MGRT
        self.MSSI = 2 + 2*self.MTAP
        self.MTATW = 3 + max(self.MACC, max(self.MFO, self.MSSI))
        self.MPATW = self.MAMA + self.MMMA - self.MPOOL
        self.MPATMA = self.MNRNY + self.MNRNX - self.MPOOL;
        self.MNRN_INJECT = self.MTAP + 1
        self.MINIT_FIFO_DCT = self.MTAG

        self.MPROG_PAT = self.MPATW + 1 + self.MPATMA
        self.MPROG_TAT = self.MTATW + 2
        self.MPROG_AM = max(2*self.MAMW, self.MAMA) + 2 
        self.MPROG_MM = max(self.MW, self.MMMA) + 2
        self.MPROG_AMMM = max(self.MPROG_AM, self.MPROG_MM)

        self.MDUMP_TAT       = self.MTATW;
        self.MDUMP_PAT       = self.MPATW;
        self.MDUMP_AM        = 2*self.MAMW;
        self.MDUMP_MM        = self.MW;
        self.MDUMP_PRE_FIFO  = self.MTAG+self.MCT;
        self.MDUMP_POST_FIFO = self.MTAG-1+self.MCT;

        self.MPROG_AER = self.MNRN + 6

        # resource containers
        MM_shape = (2**(self.MMMAY), 2**(self.MMMAX))
        AM_shape = (2**(self.MAMA),)
        TAT0_shape = (2**(self.MTAG-1),)
        TAT1_shape = (2**(self.MTAG-1),)
        PAT_shape = (2**(self.MNRNY + self.MNRNX - self.MPOOL),)

        self.MM = MM(MM_shape, self.MW, self.NPOOL)
        self.AM = AM(AM_shape, 2*self.MAMW)
        self.TAT0 = TAT(TAT0_shape, self.MTATW)
        self.TAT1 = TAT(TAT1_shape, self.MTATW)
        self.PAT = PAT(PAT_shape, self.MPATW)
        self.NeuronArray = NeuronArray(self.NNRN, self.NPOOL)

        # funnel and horn leaf widths
        # subtrees
        self.FH_leaf_widths = {}

        self.FH_leaf_widths["DH"] = [
            self.MDELAY,
            self.MPROG_TAT,
            self.MINIT_FIFO_HT,
            self.MTOGGLE_PRE_FIFO,
            self.MTOGGLE_POST_FIFO,
            self.MINIT_FIFO_DCT,
            self.MPROG_PAT,
            self.MPROG_AMMM+2]

        self.FH_leaf_widths["NH"] = [
            self.MTOGGLE_NRN_DUMP,
            self.MNRN_INJECT,
            self.MPROG_AER]

        self.FH_leaf_widths["BH"] = [
            self.MDAC,
            self.MADC]

        self.FH_leaf_widths["DF"] = [
            self.MDUMP_TAT,
            self.MDUMP_AM,
            self.MDUMP_MM,
            self.MDUMP_PAT,
            self.MOVFLW,
            self.MDUMP_PRE_FIFO,
            self.MDUMP_POST_FIFO]

        subtree_names = ["DF", "DH", "NH", "BH"]
        subtree_root_bitwidths = GetRootBitwidthsForTrees(self.FH_leaf_widths, subtree_names)

        self.MDF = subtree_root_bitwidths["DF"]
        self.MDH = subtree_root_bitwidths["DH"]
        self.MNH = subtree_root_bitwidths["NH"]
        self.MBH = subtree_root_bitwidths["BH"]

        # top-level trees

        self.FH_leaf_widths["TH"] = [
            self.MCT + self.MTAG,
            self.MDH,
            self.MBH,
            self.MNH]
        
        self.FH_leaf_widths["TF"] = [
            self.MCT + self.MTAG + self.MGRT,
            self.MCT + self.MAMW,
            self.MDF,
            self.MNRN]

        tree_names = ["TF", "TH"]
        tree_root_bitwidths = GetRootBitwidthsForTrees(self.FH_leaf_widths, tree_names)
        self.MTF = tree_root_bitwidths["TF"]
        self.MTH = tree_root_bitwidths["TH"]

        # compute funnel and horn routes
        self.TF, self.TH = CreateTFAndTH(self.FH_leaf_widths)

        # FIXME this doesn't belong in the core
        self.ExternalSinks = ExternalSinks()
        self.COMP_ROUTE = 1


    def Map(self, R, verbose=False):
        # map resources to this core
        for k,v in R.iteritems():
            v.PreTranslate(self)
        if verbose: print "finished PreTranslate"

        for k,v in R.iteritems():
            v.AllocateEarly(self)
        if verbose: print "finished AllocateEarly"
        
        self.MM.alloc.SwitchToTrans() # switch allocation mode of MM
        for k,v in R.iteritems():
            v.Allocate(self)
        if verbose: print "finished Allocate"

        for k,v in R.iteritems():
            v.PostTranslate(self)
        if verbose: print "finished PostTranslate"

        for k,v in R.iteritems():
            v.Assign(self)
        if verbose: print "finished Assign"

    def WriteProgStreams(self):
        PAT = self.PAT.mem.WriteProgStream()
        AM = self.AM.mem.WriteProgStream()
        MM = self.MM.mem.WriteProgStream()
        TAT0 = self.TAT0.mem.WriteProgStream()
        TAT1 = self.TAT1.mem.WriteProgStream()

        AM_type = np.zeros_like(AM).astype(int)
        AM_stop = np.zeros_like(AM).astype(int)
        AM = Pack([AM_type, AM, AM_stop], [1, self.MPROG_AMMM, 1])

        MM_type = np.ones_like(MM).astype(int)
        MM_stop = np.zeros_like(MM).astype(int)
        MM_stop[-1] = 1
        MM = Pack([MM_type, MM, MM_stop], [1, self.MPROG_AMMM, 1])
        #print "MM"
        #print "####################################"
        #print MM
        #print "AM"
        #print "####################################"
        #print AM

        AMMM = np.hstack((AM, MM))

        return {'PAT': PAT,
                'AMMM': AMMM,
                'TAT0': TAT0,
                'TAT1': TAT1}

    def WriteHornProgStreams(self):
        # the full chip has funnels and horns, need to pack the route bits

        raw_prog = self.WriteProgStreams()

        # funnel route bits are packed in MSBs
        # horn route bits are packed in LSBs
        # (programming streams go into the horn)

        leaf_names =  ["PROG_PAT", "PROG_AMMM", "PROG_TAT[0]", "PROG_TAT[1]"]
        short_names = ["PAT",      "AMMM",      "TAT0",        "TAT1"]

        # break raw_prog into chunks as necessary, prepend routes
        prog = {} 
        for leaf_name, short_name in zip(leaf_names, short_names):
            prog[short_name] = self.TH.EncodeStream([leaf_name], [raw_prog[short_name]])

        return prog


    def Print(self):
        print "NeuronArray"
        self.NeuronArray.alloc.Print()
        print "PAT"
        self.PAT.alloc.Print()
        print "AM"
        self.AM.alloc.Print()
        print "MM"
        self.MM.alloc.Print()
        print "TAT0"
        self.TAT0.alloc.Print()
        print "TAT1"
        self.TAT1.alloc.Print()

    def WriteMemsToFile(self, fname_pre):
        self.PAT.WriteToFile(fname_pre, self)
        self.TAT0.WriteToFile(fname_pre, self, 0)
        self.TAT1.WriteToFile(fname_pre, self, 1)
        self.MM.WriteToFile(fname_pre, self)
        self.AM.WriteToFile(fname_pre, self)


class MemAllocator(object):
    def __init__(self, shape):
        self.shape = shape
        self.L = np.zeros(shape).astype(bool)

    def AllocateBlock(self, idx_slice):
        if isinstance(idx_slice, tuple):
            assert len(idx_slice) == 2
            assert isinstance(idx_slice[0], slice)
            assert isinstance(idx_slice[1], slice)
        else:
            assert isinstance(idx_slice, slice)
        
        if len(self.shape) == 2 and isinstance(idx_slice, slice): # using flat indexing on 2D
            if np.sum(1*self.L.flat[idx_slice]) == 0:
                self.L.flat[idx_slice] = True
                return True
            else:
                return False
        else: # as many slice indices as full dims
            if np.sum(1*self.L[idx_slice]) == 0:
                self.L[idx_slice] = True
                return True
            else:
                return False

    def Print(self):
        print 1*self.L

class StepMemAllocator(MemAllocator):
    # the TAT is simple to allocate, there are no constraints on positioning,
    # so we just fill it up from the bottom
    def __init__(self, shape):
        super(StepMemAllocator, self).__init__(shape)
        self.pos = 0

    def Allocate(self, alloc_size):
        try_slice = slice(self.pos, self.pos + alloc_size)
        assert self.AllocateBlock(try_slice) # allocate
        self.pos += alloc_size
        return try_slice.start

class MMAllocator(MemAllocator):
    # we have to place the decoders carefully, but we have a lot of freedom with transforms
    def __init__(self, shape, NPOOL):
        super(MMAllocator, self).__init__(shape)
        self.xpos = 0
        self.ypos = 0
        self.NPOOL = NPOOL

    def AllocatePoolDec(self, D):
        if self.xpos + D < self.shape[1]:
            try_slice = (slice(self.ypos, self.ypos + self.NPOOL), slice(self.xpos, self.xpos + D))
            assert self.AllocateBlock(try_slice) # allocate
            self.xpos += D
        elif self.xpos + D == self.shape[1]:
            try_slice = (slice(self.ypos, self.ypos + self.NPOOL), slice(self.xpos, self.xpos + D))
            assert self.AllocateBlock(try_slice) # allocate
            self.xpos = 0
            self.ypos += self.NPOOL
        else:
            try_slice = (slice(self.ypos + self.NPOOL, self.ypos + 2*self.NPOOL), slice(0, D))
            assert self.AllocateBlock(try_slice) # allocate
            self.xpos = D
            self.ypos += self.NPOOL
        return (try_slice[0].start, try_slice[1].start)

    def SwitchToTrans(self):
        # this is lazy, wastes a little space
        if self.xpos != 0:
            self.ypos += self.NPOOL
        self.xpos = 0

    def AllocateTransRow(self, D):
        # very similar to StepMemAllocator Allocate
        dcurr = 0
        flat_pos = self.ypos * self.shape[1] + self.xpos
        try_slice = slice(flat_pos, flat_pos + D)
        assert self.AllocateBlock(try_slice)
        self.ypos = (flat_pos + D) / self.shape[1]
        self.xpos = (flat_pos + D) % self.shape[1]
        start_y = try_slice.start / self.shape[1]
        start_x = try_slice.start % self.shape[1]
        return (start_y, start_x)

class Memory(object):
    def __init__(self, shape, MW):
        self.shape = shape
        self.MW = MW
        self.M = np.zeros(shape).astype(int)

    def Assign1DBlock(self, mem, start):
        if len(self.shape) == 2 and isinstance(start, tuple):
            start = start[0] * self.shape[1] + start[1]
        assert np.sum(mem >= 2**self.MW) == 0
        idx_slice = slice(start, start + mem.shape[0])
        self.M.flat[idx_slice] = mem

    def Assign2DBlock(self, mem, start):
        assert len(self.shape) == 2
        assert len(mem.shape) == 2
        assert len(start) == 2
        assert np.sum(mem >= 2**self.MW) == 0
        idx_slice = (slice(start[0], start[0] + mem.shape[0]), slice(start[1], start[1] + mem.shape[1]))
        self.M[idx_slice] = mem


class StepMem(Memory):
    # r_w_memory
    # has 2-bit type (00 "0" -> addr, 01 "1" -> write, 10 "2" -> read)
    def __init__(self, shape, MW):
        super(StepMem, self).__init__(shape, MW)

    def WriteProgStream(self, addr_slice=None):
        if addr_slice is None:
            mem = self.M.flatten()
            start_addr = 0
        else:
            mem = self.M.flatten()[addr_slice] # has to be a flat slice
            start_addr = addr_slice.start # should be fine with 2D, but be careful

        prog = np.zeros((mem.shape[0]+1,)).astype(int)
        prog[0] = 4*start_addr # XXX I didn't use pack because I didn't want to compute addr bits
        prog[1:] = Pack([1, mem], [2, self.MW]) # type = 1 means write

        return prog

    def WriteDumpStream(self, addr_slice=None):
        if addr_slice is None:
            start_addr = 0
        else:
            start_addr = addr_slice.start # should be fine with 2D, but be careful

        prog = np.zeros((mem.shape[0]+1,))
        prog[0] = 4*start_addr # XXX I didn't use pack because I didn't want to compute addr bits
        prog[1:] = 2 # type = 2 means read

        return prog

class RMWStepMem(Memory):
    # r_rmw_memory (accumulator)
    # have to do our own increments!
    # has 2-bit type (00 "0" -> addr, 01 "1" -> write, 10 "2" -> inc(!!!))
    def __init__(self, shape, MW):
        super(RMWStepMem, self).__init__(shape, MW)

    def WriteProgStream(self, addr_slice=None):
        if addr_slice is None:
            mem = self.M.flatten()
            start_addr = 0
        else:
            mem = self.M.flatten()[addr_slice] # has to be a flat slice
            start_addr = addr_slice.start # should be fine with 2D, but be careful

        prog = np.zeros((2*mem.shape[0]+1,)).astype(int)
        prog[0] = 4*start_addr # XXX I didn't use pack because I didn't want to compute addr bits
        prog[1::2] = Pack([1, mem], [2, self.MW]) # type = 1 means write
        prog[2::2] = 2

        return prog

    def WriteDumpStream(self, addr_slice=None):
      return WriteProgStream(self, addr_slice) # for RMW, read current/program is a combined operation

class PATMem(Memory):
    def __init__(self, shape, MW):
        super(PATMem, self).__init__(shape, MW)

    def WriteProgStream(self, addr_slice=None):
        assert addr_slice == None
        assert np.sum(self.M >= 2**self.MW) == 0 # MW = MPATW
        addr_bits = int(sum(np.ceil(np.log2(self.shape))))
        addr = np.arange(self.M.size)
        wr = np.ones_like(addr).astype(int)
        return Pack([addr, wr, self.M], [addr_bits, 1, self.MW])

    def WriteDumpStream(self, addr_slice=None):
        assert addr_slice == None
        assert np.sum(self.M >= 2**self.MW) == 0 # MW = MPATW
        addr_bits = int(sum(np.ceil(np.log2(self.shape))))
        addr = np.arange(self.M.size)
        rd = np.zeros_like(addr).astype(int)
        return Pack([addr, rd, self.M], [addr_bits, 1, self.MW]) # only difference is r/w bit

class MM(object):
    def __init__(self, shape, MW, NPOOL):
        self.mem = StepMem(shape, MW)
        self.alloc = MMAllocator(shape, NPOOL) 

    def AllocateDec(self, D):
        return self.alloc.AllocatePoolDec(D)

    def AllocateTrans(self, D):
        return self.alloc.AllocateTransRow(D)
    
    def AssignDec(self, data, start):
        self.mem.Assign2DBlock(data, start)

    def AssignTrans(self, data, start):
        self.mem.Assign1DBlock(data, start)

    def WriteToFile(self, fname_pre, core):
        f = open(fname_pre + "MM.txt", 'wb')
        for y in xrange(self.mem.shape[0]):
            for x in xrange(self.mem.shape[1]):
                numstr = str(self.mem.M[y,x])
                spaces = ' ' * max(1, 4 - len(numstr))
                if x == 0:
                    f.write('[' + spaces)
                else:
                    f.write(spaces)
                f.write(numstr)
                if x == self.mem.shape[1] - 1:
                    f.write(' ]\n')
                else:
                    f.write(' |')
        f.close()

class AM(object):
    def __init__(self, shape, MAMW):
        self.mem = RMWStepMem(shape, MAMW)
        self.alloc = StepMemAllocator(shape)

    def Allocate(self, size):
        return self.alloc.Allocate(size)
    
    def Assign(self, data, start):
        self.mem.Assign1DBlock(data, start)

    def WriteToFile(self, fname_pre, core):
        f = open(fname_pre + "AM.txt", 'wb')
        f.write("AM: [ val | thr | stop | na ]\n")
        for idx in xrange(self.mem.shape[0]):
            m = self.mem.M[idx]
            val, thr, stop, na = Unpack(m, [core.MVAL, core.MTHR, 1])
            f.write("[ " + str(val) + " | " + str(thr) + " | " + str(stop) + " | " + str(na) + " ]\n")
        f.close()

class TAT(object):
    def __init__(self, shape, MTATW):
        self.mem = StepMem(shape, MTATW)
        self.alloc = StepMemAllocator(shape)

    def Allocate(self, size):
        return self.alloc.Allocate(size)
    
    def Assign(self, data, start):
        self.mem.Assign1DBlock(data, start)

    def WriteToFile(self, fname_pre, core, tat_idx):
        f = open(fname_pre + "TAT" + str(tat_idx) + ".txt", 'wb')
        f.write("TAT" + str(tat_idx) + ": acc : [ stop | type | ama | mmax | mmay | X ]\n")
        f.write("      nrn : [ stop | type | tap | sign | tap | sign | X ]\n")
        f.write("      fo  : [ stop | type | tag | gtag | X ]\n")
        for idx in xrange(self.mem.shape[0]):
            m = self.mem.M[idx]
            s, ty, pay = Unpack(m, [1, 2])
            if ty == 0:
                ama, mmax, mmay, X = Unpack(pay, [core.MAMA, core.MMMAX, core.MMMAY])
                f.write("[ " + str(s) + " | " + str(ty) + " | " + str(ama) + " | " + str(mmax) + " | " + str(mmay) + " | " + str(X) + " ]\n")
            elif ty == 1:
                tap0, s0, tap1, s1, X = Unpack(pay, [core.MTAP, 1, core.MTAP, 1])
                f.write("[ " + str(s) + " | " + str(ty) + " | " + str(tap0) + " | " + str(s0) + " | " + str(tap1) + " | " + str(s1) + " | " + str(X) + " ]\n")
            elif ty == 2:
                tag, grt, X = Unpack(pay, [core.MTAG, core.MGRT])
                f.write("[ " + str(s) + " | " + str(ty) + " | " + str(tag) + " | " + str(grt) + " | " + str(X) + " ]\n")
        f.close()

class PAT(object):
    def __init__(self, shape, MPATW):
        self.mem = PATMem(shape, MPATW)
        self.alloc = MemAllocator(shape) # kind of unecessary, but a nice assert

    def Assign(self, data, pool_slice):
        assert self.alloc.AllocateBlock(pool_slice)
        self.mem.Assign1DBlock(data, pool_slice.start)

    def WriteToFile(self, fname_pre, core):
        f = open(fname_pre + "PAT.txt", 'wb')
        f.write("PAT : [ ama | mmax | mmay_base ]\n")
        for idx in xrange(self.mem.shape[0]):
            m = self.mem.M[idx]
            ama, mmax, mmay_base = Unpack(m, [core.MAMA, core.MMMAX])
            f.write("[ " + str(ama) + " | " + str(mmax) + " | " + str(mmay_base) + " ]\n")
        f.close()


class NeuronArray(object):
    def __init__(self, N, NPOOL):
        self.NPOOL = NPOOL
        self.n_pools = N / NPOOL
        shape = (self.n_pools,)
        self.alloc = StepMemAllocator(shape) # FIXME need smarter allocator to make square-ish shapes eventually

    def Allocate(self, n_pools):
        return self.alloc.Allocate(n_pools)

class ExternalSinks(object):
    curr_idx = 0

    def __init__(self):
        pass

    def Allocate(self, D):
        base_idx = ExternalSinks.curr_idx
        ExternalSinks.curr_idx += D
        return np.array(range(base_idx, base_idx + D))
    

