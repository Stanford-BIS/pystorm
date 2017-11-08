import numpy as np
import rectpack # for NeuronAllocator
from pystorm.PyDriver import bddriver

class Core(object):
    """Represents a braindrop/brainstorm core

    Contains the neuron array, digital datapath, and memories.

    Parameters
    ----------
    ps_pars: dictionary of core parameters
        core_pars CORE_PARAMETERS
    """
    def __init__(self, ps_pars):
        self.MM_height = ps_pars['MM_height']
        self.MM_width = ps_pars['MM_width']

        self.AM_size = ps_pars['AM_size']

        self.TAT_size = ps_pars['TAT_size']

        self.NeuronArray_height = ps_pars['NeuronArray_height']
        self.NeuronArray_width  = ps_pars['NeuronArray_width']
        self.NeuronArray_height_in_tiles = ps_pars['NeuronArray_height_in_tiles']
        self.NeuronArray_width_in_tiles = ps_pars['NeuronArray_width_in_tiles']
        # number of neurons that share each PAT entry
        self.NeuronArray_pool_size_y = ps_pars['NeuronArray_pool_size_y']
        self.NeuronArray_pool_size_x = ps_pars['NeuronArray_pool_size_x']
        self.NeuronArray_pool_size = self.NeuronArray_pool_size_y * self.NeuronArray_pool_size_x
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
        PAT_shape = (self.NeuronArray_height // self.NeuronArray_pool_size_y, 
                     self.NeuronArray_width // self.NeuronArray_pool_size_x)

        self.MM = MM(MM_shape, self.NeuronArray_pool_size)
        self.AM = AM(AM_shape)
        self.TAT0 = TAT(TAT0_shape)
        self.TAT1 = TAT(TAT1_shape)
        self.PAT = PAT(PAT_shape)
        self.neuron_array = NeuronArray(
            self.NeuronArray_height, self.NeuronArray_width,
            self.NeuronArray_pool_size_y, self.NeuronArray_pool_size_x)

        # FIXME this maybe doesn't belong in the core?
        self.ExternalSinks = ExternalSinks()

    def Print(self):
        print("Printing Allocation maps")
        print("NeuronArray/PAT")
        self.neuron_array.alloc.Print()
        print("AM")
        self.AM.alloc.Print()
        print("MM")
        self.MM.alloc.Print()
        print("TAT0")
        self.TAT0.alloc.Print()
        print("TAT1")
        self.TAT1.alloc.Print()

    def write_mems_to_file(self, fname_pre):
        self.PAT.write_to_file(fname_pre, self)
        self.TAT0.write_to_file(fname_pre, self, 0)
        self.TAT1.write_to_file(fname_pre, self, 1)
        self.MM.write_to_file(fname_pre, self)
        self.AM.write_to_file(fname_pre, self)

class NeuronAllocator(object):
    """Allocates neuron resources
    
    Based on rectpack python module
    Works at minimum pool-sized granularity
    
    Parameters
    ----------
    size_py: int
        number of pools available in y dimension
    size_px: int
        number of pools available in x dimension
    """
    def __init__(self, size_py, size_px):
        self.packer = rectpack.newPacker(
                mode=rectpack.PackingMode.Offline,
                rotation=False)
        self.packer.add_bin(size_py, size_px)
        self.pack_called = False
        self.alloc_results = {} # filled in after pack is called

    def add_pool(self, py, px, pid):
        """Prepare a pool for allocation
        
        Parameters
        ----------
        py: int
            pool y size in units of minimum pool y size
        px: int
            pool x size in units of minimum pool y size
        pid: int
            id(pool)
        """
        self.packer.add_rect(py, px, rid=pid)

    def allocate(self, pid):
        """Allocate neurons for a pool

        On first call, peforms the allocation for all pools and
        caches result for subsequent calls
        
        Returns (y, x, w, h), the start coordinates, width and height of the allocated pool

        Parameters
        ----------
        pid: id(pool)
        """
        if not self.pack_called:
            self.packer.pack()
            for rect in self.packer.rect_list():
                _, y, x, w, h, pid = rect
                self.alloc_results[pid] = (y, x, w, h)
            self.pack_called = True

        return self.alloc_results[pid]

    def Print(self):
        print(self.alloc_results)

class MemAllocator(object):
    def __init__(self, shape):
        self.shape = shape
        self.L = np.zeros(shape).astype(bool) # binary map of allocation

    def check_block_unallocated(self, idx_slice):
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

    def allocate(self, alloc_size):
        try_slice = slice(self.pos, self.pos + alloc_size)

        assert self.check_block_unallocated(try_slice) # allocate

        self.pos += alloc_size
        return try_slice.start

class MMAllocator(MemAllocator):
    """we have to place the decoders carefully, but we have a lot of freedom with transforms"""
    def __init__(self, shape, neurons_per_pool):
        super(MMAllocator, self).__init__(shape)
        self.xpos = 0
        self.ypos = 0
        self.neurons_per_pool = neurons_per_pool

    def allocate_pool_dec(self, D):

        if D > self.shape[1]:
            print("FAILED ALLOCATION. TRYING TO ALLOCATE DECODER WITH TOO MANY DIMS")
            assert False

        # can fit in the current mem row, just increment xpos when done
        elif self.xpos + D < self.shape[1]:
            try_slice = (
                slice(self.ypos, self.ypos + self.neurons_per_pool),
                slice(self.xpos, self.xpos + D))
            assert self.check_block_unallocated(try_slice) # allocate
            self.xpos += D

        # can *barely* fit in the current mem row, move to the next pool row when done
        elif self.xpos + D == self.shape[1]:
            try_slice = (
                slice(self.ypos, self.ypos + self.neurons_per_pool),
                slice(self.xpos, self.xpos + D))
            assert self.check_block_unallocated(try_slice) # allocate
            self.xpos = 0
            self.ypos += self.neurons_per_pool

        # can't fit in the current row, skip to the next one
        else:
            try_slice = (
                slice(self.ypos + self.neurons_per_pool, self.ypos + 2*self.neurons_per_pool),
                slice(0, D))
            assert self.check_block_unallocated(try_slice) # allocate
            self.xpos = D
            self.ypos += self.neurons_per_pool
        return (try_slice[0].start, try_slice[1].start)

    def switch_to_trans(self):
        """this is lazy, wastes a little space"""
        if self.xpos != 0:
            self.ypos += self.neurons_per_pool
        self.xpos = 0

    def allocate_trans_row(self, D):
        """very similar to StepMemAllocator Allocate"""
        dcurr = 0
        flat_pos = self.ypos * self.shape[1] + self.xpos
        try_slice = slice(flat_pos, flat_pos + D)
        assert self.check_block_unallocated(try_slice)
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

    def assign_1d_block(self, mem, start):
        if len(self.shape) == 2 and isinstance(start, tuple):
            start = start[0] * self.shape[1] + start[1]
        idx_slice = slice(start, start + mem.shape[0])
        self.M.flat[idx_slice] = mem

    def assign_2d_block(self, mem, start):
        assert len(self.shape) == 2
        assert len(mem.shape) == 2
        assert len(start) == 2
        idx_slice = (
            slice(start[0], start[0] + mem.shape[0]),
            slice(start[1], start[1] + mem.shape[1]))
        self.M[idx_slice] = mem

class StepMem(Memory):
    """Read-Write Memory"""
    def __init__(self, shape):
        super(StepMem, self).__init__(shape)

class PATMem(Memory):
    def __init__(self, shape):
        super(PATMem, self).__init__(shape)

class MM(object):
    """Represents a Core's main memory"""
    def __init__(self, shape, neurons_per_pool):
        self.mem = StepMem(shape)
        self.alloc = MMAllocator(shape, neurons_per_pool)

    def allocate_dec(self, D):
        return self.alloc.allocate_pool_dec(D)

    def allocate_trans(self, D):
        return self.alloc.allocate_trans_row(D)

    def assign_dec(self, data, start):
        self.mem.assign_2d_block(data, start)

    def assign_trans(self, data, start):
        self.mem.assign_1d_block(data, start)

    def write_to_file(self, fname_pre, core):
        f = open(fname_pre + "MM.txt", 'w')
        for y in range(self.mem.shape[0]):
            for x in range(self.mem.shape[1]):
                numstr = str(bddriver.GetField(self.mem.M[y,x], MMWord.WEIGHT))
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

    def allocate(self, size):
        return self.alloc.allocate(size)

    def assign(self, data, start):
        self.mem.assign_1d_block(data, start)

    def write_to_file(self, fname_pre, core):
        f = open(fname_pre + "AM.txt", 'w')
        f.write("AM: [ val | thr | stop | na ]\n")
        for idx in range(self.mem.shape[0]):
            m    = self.mem.M[idx]
            val  = bddriver.GetField(m, bddriver.AMWord.ACCUMULATOR_VALUE)
            thr  = bddriver.GetField(m, bddriver.AMWord.THRESHOLD)
            stop = bddriver.GetField(m, bddriver.AMWord.STOP)
            na   = bddriver.GetField(m, bddriver.AMWord.NEXT_ADDRESS)
            f.write("[ " + str(val) + " | " + str(thr) + " | " + str(stop) + " | " + str(na) + " ]\n")
        f.close()

class TAT(object):
    def __init__(self, shape):
        self.mem = StepMem(shape)
        self.alloc = StepMemAllocator(shape)

    def allocate(self, size):
        return self.alloc.allocate(size)

    def assign(self, data, start):
        self.mem.assign_1d_block(data, start)

    def write_to_file(self, fname_pre, core, tat_idx):
        f = open(fname_pre + "TAT" + str(tat_idx) + ".txt", 'w')
        f.write("TAT" + str(tat_idx) + ": acc : [ stop | type | ama | mmax | mmay ]\n")
        f.write("      nrn : [ stop | type | tap | sign | tap | sign | X ]\n")
        f.write("      fo  : [ stop | type | tag | gtag | X ]\n")
        for idx in range(self.mem.shape[0]):
            m = self.mem.M[idx]

            if bddriver.GetField(m, bddriver.TATTagWord.FIXED_2) == 2:
                ty  = 2
                s   = bddriver.GetField(m, bddriver.TATTagWord.STOP)
                tag = bddriver.GetField(m, bddriver.TATTagWord.TAG)
                grt = bddriver.GetField(m, bddriver.TATTagWord.GLOBAL_ROUTE)
                X   = bddriver.GetField(m, bddriver.TATTagWord.UNUSED)
                f.write("[ " + str(s) + " | " + str(ty) + " | " + str(tag) + " | " + str(grt) + " | " + str(X) + " ]\n")

            elif bddriver.GetField(m, TATSpikeWord.FIXED_1) == 1:
                ty   = 1
                s    = bddriver.GetField(m, bddriver.TATSpikeWord.STOP)
                tap0 = bddriver.GetField(m, bddriver.TATSpikeWord.SYNAPSE_ADDRESS_0)
                s0   = bddriver.GetField(m, bddriver.TATSpikeWord.SYNAPSE_SIGN_0)
                tap1 = bddriver.GetField(m, bddriver.TATSpikeWord.SYNAPSE_ADDRESS_1)
                s1   = bddriver.GetField(m, bddriver.TATSpikeWord.SYNAPSE_SIGN_1)
                X    = bddriver.GetField(m, bddriver.TATSpikeWord.UNUSED)
                f.write("[ " + str(s) + " | " + str(ty) + " | " + str(tap0) + " | " + str(s0) + " | " + str(tap1) + " | " + str(s1) + " | " + str(X) + " ]\n")

            else:
                ty  = 0
                s   = bddriver.GetField(m, bddriver.TATAccWord.STOP)
                ama = bddriver.GetField(m, bddriver.TATAccWord.AM_ADDRESS)
                mma = bddriver.GetField(m, bddriver.TATAccWord.MM_ADDRESS)
                mmay = mma / core.MM_width
                mmax = mma % core.MM_width
                f.write("[ " + str(s) + " | " + str(ty) + " | " + str(ama) + " | " + str(mmax) + " | " + str(mmay) + " ]\n")

        f.close()

class PAT(object):
    def __init__(self, shape):
        self.mem = PATMem(shape)

    def assign(self, data, start):
        self.mem.assign_2d_block(data, start)

    def write_to_file(self, fname_pre, core):
        f = open(fname_pre + "PAT.txt", 'w')
        f.write("PAT : [ ama | mmax | mmay_base ]\n")
        for idx in range(self.mem.shape[0]):
            for jdx in range(self.mem.shape[1]):
                m         = self.mem.M[idx, jdx]
                ama       = bddriver.GetField(m, bddriver.PATWord.AM_ADDRESS)
                mmax      = bddriver.GetField(m, bddriver.PATWord.MM_ADDRESS_LO)
                mmay_base = bddriver.GetField(m, bddriver.PATWord.MM_ADDRESS_HI)
                f.write("[ " + str(ama) + " | " + str(mmax) + " | " + str(mmay_base) + " ]\n")
        f.close()

class NeuronArray(object):
    """Represents a Core's neuron array
    
    Parameters
    ----------
    y: int
        array y dimension ; y*x = n_neurons
    x: int
        array x dimension; y*x = n_neurons
    pool_size_y: int
        minimum pool y size in number of neurons
    pool_size_x: int
        minimum pool x size in number of neurons
    """
    def __init__(self, y, x, pool_size_y, pool_size_x):
        self.y = y
        self.x = x
        self.pool_size_y = pool_size_y
        self.pool_size_x = pool_size_x
        assert(x % pool_size_x == 0)
        assert(y % pool_size_y == 0)
        self.pools_y = self.y // self.pool_size_y 
        self.pools_x = self.x // self.pool_size_x

        self.N = y * x # total neurons
        self.neurons_per_pool = self.pool_size_x * self.pool_size_y

        shape = (self.y, self.x)
        self.alloc = NeuronAllocator(self.pools_y, self.pools_x)
        self.pool_allocations = [] # store pool allocation results

    def add_pool(self, pool):
        self.alloc.add_pool(pool.py, pool.px, id(pool))

    def allocate(self, pool):
        """Allocate neuron array area for a pool
        
        Returns the start coordinates of the pool in units of minimum pool size
        """
        # coordinates and dimensions in units of minimum pool size
        py, px, pw, ph = self.alloc.allocate(id(pool))
        self.pool_allocations.append(dict(
            pool=pool, py=py, px=px, pw=pw, ph=ph))
        return (py, px)

class ExternalSinks(object):
    curr_idx = 0

    def __init__(self):
        pass

    def allocate(self, D):
        base_idx = ExternalSinks.curr_idx
        ExternalSinks.curr_idx += D
        return np.array(range(base_idx, base_idx + D))
