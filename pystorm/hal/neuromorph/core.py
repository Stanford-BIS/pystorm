import numpy as np

# for BDWord
from PyDriver.pystorm.bddriver import AMWord, MMWord, PATWord, TATAccWord, TATTagWord, TATSpikeWord
from PyDriver.pystorm.bddriver import GetField

# for SetMem
from PyDriver.pystorm.bddriver.bdpars import BDMemId

class Core(object):
    """Represents a braindrop/brainstorm core

    Contains the neuron array, digital datapath, and memories.

    Parameters
    ----------
    ps_pars: _PyStorm CorePars instance
    """
    def __init__(self, ps_pars):
        self.MM_height = ps_pars['MM_height']
        self.MM_width = ps_pars['MM_width']

        self.AM_size = ps_pars['AM_size']

        self.TAT_size = ps_pars['TAT_size']

        self.NeuronArray_height = ps_pars['NeuronArray_height']
        self.NeuronArray_width  = ps_pars['NeuronArray_width']
        self.NeuronArray_pool_size = ps_pars[
            'NeuronArray_pool_size'] # number of neurons that share each PAT entry
        self.NeuronArray_neurons_per_tap = ps_pars['NeuronArray_neurons_per_tap']
        self.NeuronArray_size = self.NeuronArray_height * self.NeuronArray_width

        self.PAT_size = self.NeuronArray_size // self.NeuronArray_pool_size

        self.num_threshold_levels = ps_pars['num_threshold_levels']
        self.min_threshold_value = ps_pars['min_threshold_value']
        self.max_weight_value = ps_pars['max_weight_value']

        # set up allocable objects (Resource containers)

        MM_shape = (self.MM_width, self.MM_height)
        AM_shape = (self.AM_size,)
        TAT0_shape = (self.TAT_size,)
        TAT1_shape = (self.TAT_size,)
        PAT_shape = (self.PAT_size,)

        self.MM = MM(MM_shape, self.NeuronArray_pool_size)
        self.AM = AM(AM_shape)
        self.TAT0 = TAT(TAT0_shape)
        self.TAT1 = TAT(TAT1_shape)
        self.PAT = PAT(PAT_shape)
        self.NeuronArray = NeuronArray(self.NeuronArray_size, self.NeuronArray_pool_size)

        # FIXME this maybe doesn't belong in the core?
        self.ExternalSinks = ExternalSinks()

    def Print(self):
        print("Printing Allocation maps")
        print("NeuronArray")
        self.NeuronArray.alloc.Print()
        print("PAT")
        self.PAT.alloc.Print()
        print("AM")
        self.AM.alloc.Print()
        print("MM")
        self.MM.alloc.Print()
        print("TAT0")
        self.TAT0.alloc.Print()
        print("TAT1")
        self.TAT1.alloc.Print()

    def WriteMemsToFile(self, fname_pre):
        self.PAT.WriteToFile(fname_pre, self)
        self.TAT0.WriteToFile(fname_pre, self, 0)
        self.TAT1.WriteToFile(fname_pre, self, 1)
        self.MM.WriteToFile(fname_pre, self)
        self.AM.WriteToFile(fname_pre, self)

    def Implement(self, driver, core_id):
        """Calls a supplied driver to implement this Core to BD"""
        driver.SetMem(core_id, MemId.PAT,  self.PAT.m,  0);
        driver.SetMem(core_id, MemId.TAT0, self.TAT0.m, 0);
        driver.SetMem(core_id, MemId.TAT1, self.TAT1.m, 0);
        driver.SetMem(core_id, MemId.MM,   self.MM.m,   0);
        driver.SetMem(core_id, MemId.AM,   self.AM.m,   0);

class MemAllocator(object):
    def __init__(self, shape):
        self.shape = shape
        self.L = np.zeros(shape).astype(bool) # binary map of allocation

    def CheckBlockUnallocated(self, idx_slice):
        if isinstance(idx_slice, tuple):
            assert len(idx_slice) == 2
            assert isinstance(idx_slice[0], slice)
            assert isinstance(idx_slice[1], slice)
        else:
            assert isinstance(idx_slice, slice)

        # if the slice is out of range, fail now
        if isinstance(idx_slice, tuple):
            # 2D indexing on 2D array
            for i in range(2):
                if idx_slice[i].start >= self.shape[i]:
                    return False
                if idx_slice[i].stop > self.shape[i]:
                    return False
        else:
            # flat indexing on flat array
            if len(self.shape) == 1:
                if idx_slice.start >= self.shape[0]:
                    return False
                if idx_slice.stop > self.shape[0]:
                    return False
            # flat indexing on 2D array
            else:
                if idx_slice.start >= self.L.size:
                    return False
                if idx_slice.stop > self.L.size:
                    return False

        # otherwise, the slice is compeletely in-bounds, check all entries
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
        print(1*self.L)

class StepMemAllocator(MemAllocator):
    """the TAT is simple to allocate, there are no constraints on positioning,
    so we just fill it up from the bottom
    """
    def __init__(self, shape):
        super(StepMemAllocator, self).__init__(shape)
        self.pos = 0

    def Allocate(self, alloc_size):
        try_slice = slice(self.pos, self.pos + alloc_size)

        assert self.CheckBlockUnallocated(try_slice) # allocate

        self.pos += alloc_size
        return try_slice.start

class MMAllocator(MemAllocator):
    """we have to place the decoders carefully, but we have a lot of freedom with transforms"""
    def __init__(self, shape, NPOOL):
        super(MMAllocator, self).__init__(shape)
        self.xpos = 0
        self.ypos = 0
        self.NPOOL = NPOOL

    def AllocatePoolDec(self, D):

        if D > self.shape[1]:
            print("FAILED ALLOCATION. TRYING TO ALLOCATE DECODER WITH TOO MANY DIMS")
            assert False

        # can fit in the current mem row, just increment xpos when done
        elif self.xpos + D < self.shape[1]:
            try_slice = (slice(self.ypos, self.ypos + self.NPOOL), slice(self.xpos, self.xpos + D))
            assert self.CheckBlockUnallocated(try_slice) # allocate
            self.xpos += D

        # can *barely* fit in the current mem row, move to the next pool row when done
        elif self.xpos + D == self.shape[1]:
            try_slice = (slice(self.ypos, self.ypos + self.NPOOL), slice(self.xpos, self.xpos + D))
            assert self.CheckBlockUnallocated(try_slice) # allocate
            self.xpos = 0
            self.ypos += self.NPOOL

        # can't fit in the current row, skip to the next one
        else:
            try_slice = (slice(self.ypos + self.NPOOL, self.ypos + 2*self.NPOOL), slice(0, D))
            assert self.CheckBlockUnallocated(try_slice) # allocate
            self.xpos = D
            self.ypos += self.NPOOL
        return (try_slice[0].start, try_slice[1].start)

    def SwitchToTrans(self):
        """this is lazy, wastes a little space"""
        if self.xpos != 0:
            self.ypos += self.NPOOL
        self.xpos = 0

    def AllocateTransRow(self, D):
        """very similar to StepMemAllocator Allocate"""
        dcurr = 0
        flat_pos = self.ypos * self.shape[1] + self.xpos
        try_slice = slice(flat_pos, flat_pos + D)
        assert self.CheckBlockUnallocated(try_slice)
        self.ypos = (flat_pos + D) // self.shape[1]
        self.xpos = (flat_pos + D) % self.shape[1]
        start_y = try_slice.start // self.shape[1]
        start_x = try_slice.start % self.shape[1]
        return (start_y, start_x)

class Memory(object):
    def __init__(self, shape, ):
        self.shape = shape
        if len(self.shape) == 1:
            self.M = [0 for i in range(self.shape[0])]
        else:
            self.M = [[0 for i in range(self.shape[0])] for j in range(self.shape[1])]
        self.M = np.array(self.M, dtype=object)

    def Assign1DBlock(self, mem, start):
        if len(self.shape) == 2 and isinstance(start, tuple):
            start = start[0] * self.shape[1] + start[1]
        idx_slice = slice(start, start + mem.shape[0])
        self.M.flat[idx_slice] = mem

    def Assign2DBlock(self, mem, start):
        assert len(self.shape) == 2
        assert len(mem.shape) == 2
        assert len(start) == 2
        idx_slice = (slice(start[0], start[0] + mem.shape[0]), slice(start[1], start[1] + mem.shape[1]))
        self.M[idx_slice] = mem

class StepMem(Memory):
    """Read-Write Memory"""
    def __init__(self, shape):
        super(StepMem, self).__init__(shape)

class PATMem(Memory):
    def __init__(self, shape):
        super(PATMem, self).__init__(shape)

class MM(object):
    def __init__(self, shape, NPOOL):
        self.mem = StepMem(shape)
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
        f = open(fname_pre + "MM.txt", 'w')
        for y in range(self.mem.shape[0]):
            for x in range(self.mem.shape[1]):
                numstr = str(GetField(self.mem.M[y,x], MMWord.WEIGHT))
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
    def __init__(self, shape):
        self.mem = StepMem(shape)
        self.alloc = StepMemAllocator(shape)

    def Allocate(self, size):
        return self.alloc.Allocate(size)

    def Assign(self, data, start):
        self.mem.Assign1DBlock(data, start)

    def WriteToFile(self, fname_pre, core):
        f = open(fname_pre + "AM.txt", 'w')
        f.write("AM: [ val | thr | stop | na ]\n")
        for idx in range(self.mem.shape[0]):
            m    = self.mem.M[idx]
            val  = GetField(m, AMWord.ACCUMULATOR_VALUE)
            thr  = GetField(m, AMWord.THRESHOLD)
            stop = GetField(m, AMWord.STOP)
            na   = GetField(m, AMWord.NEXT_ADDRESS)
            f.write("[ " + str(val) + " | " + str(thr) + " | " + str(stop) + " | " + str(na) + " ]\n")
        f.close()

class TAT(object):
    def __init__(self, shape):
        self.mem = StepMem(shape)
        self.alloc = StepMemAllocator(shape)

    def Allocate(self, size):
        return self.alloc.Allocate(size)

    def Assign(self, data, start):
        self.mem.Assign1DBlock(data, start)

    def WriteToFile(self, fname_pre, core, tat_idx):
        f = open(fname_pre + "TAT" + str(tat_idx) + ".txt", 'w')
        f.write("TAT" + str(tat_idx) + ": acc : [ stop | type | ama | mmax | mmay ]\n")
        f.write("      nrn : [ stop | type | tap | sign | tap | sign | X ]\n")
        f.write("      fo  : [ stop | type | tag | gtag | X ]\n")
        for idx in range(self.mem.shape[0]):
            m = self.mem.M[idx]

            if GetField(m, TATTagWord.FIXED_2) == 2:
                ty  = 2
                s   = GetField(m, TATTagWord.STOP)
                tag = GetField(m, TATTagWord.TAG)
                grt = GetField(m, TATTagWord.GLOBAL_ROUTE)
                X   = GetField(m, TATTagWord.UNUSED)
                f.write("[ " + str(s) + " | " + str(ty) + " | " + str(tag) + " | " + str(grt) + " | " + str(X) + " ]\n")

            elif GetField(m, TATSpikeWord.FIXED_1) == 1:
                ty   = 1
                s    = GetField(m, TATSpikeWord.STOP)
                tap0 = GetField(m, TATSpikeWord.SYNAPSE_ADDRESS_0)
                s0   = GetField(m, TATSpikeWord.SYNAPSE_SIGN_0)
                tap1 = GetField(m, TATSpikeWord.SYNAPSE_ADDRESS_1)
                s1   = GetField(m, TATSpikeWord.SYNAPSE_SIGN_1)
                X    = GetField(m, TATSpikeWord.UNUSED)
                f.write("[ " + str(s) + " | " + str(ty) + " | " + str(tap0) + " | " + str(s0) + " | " + str(tap1) + " | " + str(s1) + " | " + str(X) + " ]\n")

            else:
                ty  = 0
                s   = GetField(m, TATAccWord.STOP)
                ama = GetField(m, TATAccWord.AM_ADDRESS)
                mma = GetField(m, TATAccWord.MM_ADDRESS)
                mmay = mma / core.MM_width
                mmax = mma % core.MM_width
                f.write("[ " + str(s) + " | " + str(ty) + " | " + str(ama) + " | " + str(mmax) + " | " + str(mmay) + " ]\n")

        f.close()

class PAT(object):
    def __init__(self, shape):
        self.mem = PATMem(shape)
        self.alloc = MemAllocator(shape) # kind of unecessary, but a nice assert

    def Assign(self, data, pool_slice):
        assert self.alloc.CheckBlockUnallocated(pool_slice)
        self.mem.Assign1DBlock(data, pool_slice.start)

    def WriteToFile(self, fname_pre, core):
        f = open(fname_pre + "PAT.txt", 'w')
        f.write("PAT : [ ama | mmax | mmay_base ]\n")
        for idx in range(self.mem.shape[0]):
            m         = self.mem.M[idx]
            ama       = GetField(m, PATWord.AM_ADDRESS)
            mmax      = GetField(m, PATWord.MM_ADDRESS_LO)
            mmay_base = GetField(m, PATWord.MM_ADDRESS_HI)
            f.write("[ " + str(ama) + " | " + str(mmax) + " | " + str(mmay_base) + " ]\n")
        f.close()

class NeuronArray(object):
    def __init__(self, x, y, pool_x, pool_y):
        self.y = y
        self.x = x
        self.pool_y = pool_y
        self.pool_x = pool_x
        assert(x % pool_x == 0)
        assert(y % pool_y == 0)
        self.n_pools = N // NPOOL
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
