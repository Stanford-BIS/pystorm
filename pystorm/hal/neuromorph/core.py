import numpy as np
import rectpack # for NeuronAllocator
from pystorm.PyDriver import bddriver as bd
import sys
import logging

logger = logging.getLogger(__name__)

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
        self.NeuronArray_pools_y = self.NeuronArray_height // self.NeuronArray_pool_size_y
        self.NeuronArray_pools_x = self.NeuronArray_width // self.NeuronArray_pool_size_x
        self.NeuronArray_neurons_per_tap = ps_pars['NeuronArray_neurons_per_tap']
        self.NeuronArray_size = self.NeuronArray_height * self.NeuronArray_width

        self.PAT_size = self.NeuronArray_pools_x * self.NeuronArray_pools_y

        self.num_threshold_levels = ps_pars['num_threshold_levels']
        self.min_threshold_value = ps_pars['min_threshold_value']
        self.max_weight_value = ps_pars['max_weight_value']

        # set up allocable objects (Resource containers)

        MM_shape = (self.MM_width, self.MM_height)
        AM_shape = (self.AM_size,)
        TAT0_shape = (self.TAT_size,)
        TAT1_shape = (self.TAT_size,)
        PAT_shape = (self.NeuronArray_pools_y * self.NeuronArray_pools_x,)

        self.MM = MM(MM_shape, self.NeuronArray_pool_size)
        self.AM = AM(AM_shape)
        self.TAT0 = TAT(TAT0_shape)
        self.TAT1 = TAT(TAT1_shape)
        self.PAT = PAT(PAT_shape)
        self.neuron_array = NeuronArray(
            self.NeuronArray_height, self.NeuronArray_width,
            self.NeuronArray_pool_size_y, self.NeuronArray_pool_size_x)

        self.FPGASpikeFilters = FPGASpikeFilters()
        self.FPGASpikeGenerators = FPGASpikeGenerators()

    def Print(self):
        np.set_printoptions(threshold=np.nan)
        print("Printing Allocation maps")
        print("NeuronArray/PAT")
        self.neuron_array.alloc.Print()
        print("AM")
        self.AM.alloc.Print()
        print("TAT0")
        self.TAT0.alloc.Print()
        print("TAT1")
        self.TAT1.alloc.Print()
        print("MM")
        self.MM.alloc.Print()

    def __str__(self):
        return ("===== PAT =====\n"  + str(self.PAT)  +
                "===== TAT0 =====\n" + str(self.TAT0) +
                "===== TAT1 =====\n" + str(self.TAT1) +
                "===== MM =====\n"   + str(self.MM)   +
                "===== AM =====\n"   + str(self.AM))

    def write_mems_to_file(self, fname):
        f = open(fname, 'w')
        f.write(str(self))
        f.close()

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
        logger.debug("added", pid, "at", px, ",", py)
        self.packer.add_rect(px, py, rid=pid)

    def allocate(self, calling_pid):
        """Allocate neurons for a pool

        On first call, peforms the allocation for all pools and
        caches result for subsequent calls
        
        Returns (y, x, h, w), the start coordinates, width and height of the allocated pool

        Parameters
        ----------
        pid: id(pool)
        """
        if not self.pack_called:
            self.packer.pack()
            for rect in self.packer.rect_list():
                _, x, y, w, h, pid = rect
                logger.debug(x, y, w, h, pid)
                self.alloc_results[pid] = (y, x, h, w)
            self.pack_called = True

        return self.alloc_results[calling_pid]

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
            logger.critical("FAILED ALLOCATION. TRYING TO ALLOCATE DECODER WITH TOO MANY DIMS")
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
    def __init__(self, shape, default_val):
        self.shape = shape
        if len(self.shape) == 1:
            self.M = [default_val for i in range(self.shape[0])]
        else:
            self.M = [[default_val for i in range(self.shape[0])] for j in range(self.shape[1])]
        self.M = np.array(self.M, dtype=object)

    def assign_1d_block(self, mem, start):
        if isinstance(mem, np.ndarray):
            if len(self.shape) == 2 and isinstance(start, tuple):
                start = start[0] * self.shape[1] + start[1]
            idx_slice = slice(start, start + mem.shape[0])
            self.M.flat[idx_slice] = mem
        elif isinstance(mem, int):
            self.M.flat[start] = mem
        else:
            assert(False and "bad <mem> data type in assign_1d_block")

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
    def __init__(self, shape, default_val):
        super(StepMem, self).__init__(shape, default_val)

class PATMem(Memory):
    def __init__(self, shape, default_val):
        super(PATMem, self).__init__(shape, default_val)

class MM(object):
    """Represents a Core's main memory"""
    def __init__(self, shape, neurons_per_pool):
        self.mem = StepMem(shape, 0)
        self.alloc = MMAllocator(shape, neurons_per_pool)

    def allocate_dec(self, D):
        return self.alloc.allocate_pool_dec(D)

    def allocate_trans(self, D):
        return self.alloc.allocate_trans_row(D)

    def assign_dec(self, data, start):
        self.mem.assign_2d_block(data, start)

    def assign_trans(self, data, start):
        self.mem.assign_1d_block(data, start)

    def __str__(self):
        s = "MM : entries are one's complement weights\n"
        return(s +str(self.mem.M))

class AM(object):
    def __init__(self, shape):
        default_val = bd.PackWord([[bd.AMWord.STOP, 1]]) # the stop is the only important part
        self.mem = StepMem(shape, default_val)
        self.alloc = StepMemAllocator(shape)

    def allocate(self, size):
        return self.alloc.allocate(size)

    def assign(self, data, start):
        self.mem.assign_1d_block(data, start)

    def __str__(self):
        s = "AM: [ val | thr | stop | na ]\n"
        for idx in range(self.mem.shape[0]):
            m    = self.mem.M[idx]
            val  = bd.GetField(m, bd.AMWord.ACCUMULATOR_VALUE)
            thr  = bd.GetField(m, bd.AMWord.THRESHOLD)
            stop = bd.GetField(m, bd.AMWord.STOP)
            na   = bd.GetField(m, bd.AMWord.NEXT_ADDRESS)
            s += "[ " + str(val) + " | " + str(thr) + " | " + str(stop) + " | " + str(na) + " ]\n"
        return s

class TAT(object):
    def __init__(self, shape):
        default_val = bd.PackWord([[bd.TATTagWord.STOP, 1], [bd.TATTagWord.TAG, 2047], [bd.TATTagWord.GLOBAL_ROUTE, 255]]) 
        self.mem = StepMem(shape, default_val)
        self.alloc = StepMemAllocator(shape)

    def allocate(self, size):
        return self.alloc.allocate(size)

    def assign(self, data, start):
        logger.debug("assign called: {} {}".format(str(type(data)), data))
        self.mem.assign_1d_block(data, start)

    def __str__(self):
        s  = "TAT : acc : [ stop | type | ama | mma ]\n"
        s += "      nrn : [ stop | type | tap | sign | tap | sign | X ]\n"
        s += "      fo  : [ stop | type | tag | gtag | X ]\n"
        for idx in range(self.mem.shape[0]):
            m = self.mem.M[idx]

            if bd.GetField(m, bd.TATTagWord.FIXED_2) == 2:
                ty   = 2
                stop = bd.GetField(m, bd.TATTagWord.STOP)
                tag  = bd.GetField(m, bd.TATTagWord.TAG)
                grt  = bd.GetField(m, bd.TATTagWord.GLOBAL_ROUTE)
                X    = bd.GetField(m, bd.TATTagWord.UNUSED)
                s += "[ " + str(stop) + " | " + str(ty) + " | " + str(tag) + " | " + str(grt) + " | " + str(X) + " ]\n"

            elif bd.GetField(m, bd.TATSpikeWord.FIXED_1) == 1:
                ty   = 1
                stop = bd.GetField(m, bd.TATSpikeWord.STOP)
                tap0 = bd.GetField(m, bd.TATSpikeWord.SYNAPSE_ADDRESS_0)
                s0   = bd.GetField(m, bd.TATSpikeWord.SYNAPSE_SIGN_0)
                tap1 = bd.GetField(m, bd.TATSpikeWord.SYNAPSE_ADDRESS_1)
                s1   = bd.GetField(m, bd.TATSpikeWord.SYNAPSE_SIGN_1)
                X    = bd.GetField(m, bd.TATSpikeWord.UNUSED)
                s += "[ " + str(stop) + " | " + str(ty) + " | " + str(tap0) + " | " + str(s0) + " | " + str(tap1) + " | " + str(s1) + " | " + str(X) + " ]\n"

            else:
                ty   = 0
                stop = bd.GetField(m, bd.TATAccWord.STOP)
                ama  = bd.GetField(m, bd.TATAccWord.AM_ADDRESS)
                mma  = bd.GetField(m, bd.TATAccWord.MM_ADDRESS)
                s += "[ " + str(stop) + " | " + str(ty) + " | " + str(ama) + " | " + str(mma) + " ]\n"

        return s

class PAT(object):
    def __init__(self, shape):
        default_val = 0
        self.mem = PATMem(shape, 2**20-1)

    def assign(self, data, start):
        self.mem.assign_1d_block(data, start)

    def __str__(self):
        s = "PAT : [ ama | mmax | mmay_base ]\n"
        for idx in range(self.mem.shape[0]):
            m         = self.mem.M[idx]
            ama       = bd.GetField(m, bd.PATWord.AM_ADDRESS)
            mmax      = bd.GetField(m, bd.PATWord.MM_ADDRESS_LO)
            mmay_base = bd.GetField(m, bd.PATWord.MM_ADDRESS_HI)
            s += "[ " + str(ama) + " | " + str(mmax) + " | " + str(mmay_base) + " ]\n"
        return s

class NeuronArray(object):
    """Represents a Core's neuron array
    keeps track of which neurons and synapses are used
    
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
    # XXX this implementation puts all the "work" of allocation/assignment
    # in this object, instead of in hardware_resources (in Neurons), unlike most other core objs
    # it does this by taking the object as the sole argument to each fn.
    # I think I might like this more, it should keep any core parameters contained here
    # most of the current "bleed" of driver stuff into hardware resources could be put in assignment, I think
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
        self.pool_allocations = {} # store pool allocation results

        self.syns_used = [] # list of synapses that are targeted

        # filled in assign
        self.gain_divisors = np.ones((self.y, self.x), dtype='int')
        self.biases = np.zeros((self.y, self.x), dtype='int')
        self.diffusor_cuts = dict(left = np.zeros((self.y, self.x), dtype='bool'),
                                  up = np.zeros((self.y, self.x), dtype='bool'),
                                  right = np.zeros((self.y, self.x), dtype='bool'),
                                  down = np.zeros((self.y, self.x), dtype='bool'))
        self.nrns_used = np.zeros((self.y, self.x), dtype='int')

        # whether the user is supplying xy locations for all pools 
        # or whether the rectpacker is used
        self.user_specified_alloc = False

    def add_pool(self, pool):
        self.alloc.add_pool(pool.py, pool.px, id(pool))

    def allocate(self, pool):
        """Allocate neuron array area for a pool
        
        Returns the start coordinates of the pool in units of minimum pool size
        """
        # coordinates and dimensions in units of minimum pool size
        if pool.x_loc is None and pool.y_loc is None:
            assert(not self.user_specified_alloc and "either specify xy for all pools or none")
            py, px, pw, ph = self.alloc.allocate(id(pool))
        else:
            self.user_specified_alloc = True
            py = pool.y_loc // self.pool_size_y
            px = pool.x_loc // self.pool_size_x
            pw = pool.x // self.pool_size_x
            ph = pool.y // self.pool_size_y

        #print("in NeuronArray allocate for", id(pool))
        #print(py, px, ph, pw)
        self.pool_allocations[pool] = dict(py=py, px=px, pw=pw, ph=ph)
        return (py, px)
    
    def assign(self, pool):
        if pool not in self.pool_allocations:
            logger.critical(
                "did not find supplied pool in core.neuron_array.pool_allocations.\n"+
                "  You are probably using neuromorph.map_resources_to_core with a premapped_neuron_aray argument\n"+
                "  If this is the case, the network you are mapping is probably different than\n"+
                "  the one that was used to map the core that premapped_neuron_array came from\n")
            assert(False)

        alloc = self.pool_allocations[pool]
        y_loc = alloc['py'] * self.pool_size_y
        x_loc = alloc['px'] * self.pool_size_x

        for y in range(pool.y):
            for x in range(pool.x):
                nrn_idx = y * pool.x + x
                abs_idx = (y_loc + y, x_loc + x)
                self.gain_divisors[abs_idx] = pool.gain_divisors[nrn_idx]
                self.biases[abs_idx]        = pool.biases[nrn_idx]

                assert(self.nrns_used[abs_idx] == 0)
                self.nrns_used[abs_idx] = 1

        if pool.diffusor_cuts_yx is not None:
            for y, x, direction in pool.diffusor_cuts_yx:
                self.diffusor_cuts[direction][y + y_loc, x + x_loc] = True

class FPGASpikeFilters(object):

    def __init__(self):
        self.filters_used = 0

    def allocate(self, D):
        base_idx = self.filters_used
        self.filters_used += D
        return np.array(range(base_idx, base_idx + D))

class FPGASpikeGenerators(object):

    def __init__(self):
        self.gens_used = 0

    def allocate(self, D):
        base_idx = self.gens_used
        self.gens_used += D
        return np.array(range(base_idx, base_idx + D))
