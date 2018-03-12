"""Check the accuracy of the outputs for any sticky bits

Set up the following traffic flow:
set memory with known pattern (all zeros, then all ones) -> read memory for same patterns

If the output from the written memory doesn't have the same values as the input,
, then we can assume there is a problem with a sticky bit in the output.
"""
import time
import numpy as np
from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.PyDriver import bddriver as bd
HAL = HAL()
np.set_printoptions(precision=1)

CORE_ID = 0 # Set Core to 0, b/c Braindrop only has 1 core
DIM = 1 # 1 dimensional
WEIGHT = 1 # weight of connection from input to output
SLEEP_TIME = 0.1 # time to sample
MEM_DELAY = 15   # Value of Memory Delay between 0 and 15
MEM_TYPE = bd.bdpars.BDMemId.TAT1 # Allow for quick testing of any memory
MEM_WIDTH = 29  # The number bits in the data of the memory
NUM_MEM_ENTRIES = 1024    # number of memory entries in the full memory
N_CYCLES = 1

def set_memory_delays(delay):
    """Set memory delays"""
    HAL.driver.SetMemoryDelay(CORE_ID, bd.bdpars.BDMemId.PAT, delay, delay)
    HAL.driver.SetMemoryDelay(CORE_ID, bd.bdpars.BDMemId.AM, delay, delay)
    HAL.driver.SetMemoryDelay(CORE_ID, bd.bdpars.BDMemId.MM, delay, delay)
    HAL.driver.SetMemoryDelay(CORE_ID, bd.bdpars.BDMemId.TAT0, delay, delay)
    HAL.driver.SetMemoryDelay(CORE_ID, bd.bdpars.BDMemId.TAT1, delay, delay)
    HAL.driver.SetMemoryDelay(CORE_ID, bd.bdpars.BDMemId.FIFO_DCT, delay, delay)
    HAL.driver.SetMemoryDelay(CORE_ID, bd.bdpars.BDMemId.FIFO_PG, delay, delay)

def build_net():
    """Build a network for testing"""
    net = graph.Network("net")
    net_input = net.create_input("i", DIM)
    bucket = net.create_bucket("b", DIM)
    net_output = net.create_output("o", DIM)
    net.create_connection("i_to_b", net_input, bucket, WEIGHT)
    net.create_connection("b_to_o", bucket, net_output, None)
    HAL.map(net)
    set_memory_delays(MEM_DELAY)
    return net_input

def test_sticky_bits():
    """Perform the test"""
    for _ in range(N_CYCLES):
        # Set memory delay
        set_memory_delays(MEM_DELAY)

        # Set memory with alternating bits of all 0s and all 1s every entry
        mem_entries = [0, 2**MEM_WIDTH-1]*int(NUM_MEM_ENTRIES/2)
        HAL.driver.SetMem(CORE_ID, MEM_TYPE, mem_entries, 0)

        # Allow some time for memory to finish writing  (Do I need to do this?)
        time.sleep(SLEEP_TIME)

        # Dump memory num_dump_times over and over
        num_repeat_range = 1
        print("Memory Dump:")
        for _ in range(num_repeat_range):
            mem_dump = np.array(HAL.driver.DumpMem(CORE_ID, MEM_TYPE))
            mem_range = len(mem_dump)
            for idx in range(int(mem_range/2)):
                mem_entry1 = mem_dump[idx*2]
                mem_entry2 = mem_dump[idx*2+1]
                print("{:04}: {:0{width}b}\t".format(idx*2, mem_entry1, width=MEM_WIDTH) +
                      "{:04}: {:0{width}b}".format(idx*2+1, mem_entry2, width=MEM_WIDTH))

#        # Dump just one entry in memory over and over again
#        mem_range_start = 5
#        num_repeat_range = 10
#        for i in range(num_repeat_range):
#            mem_dump = np.array(HAL.driver.DumpMemRange(
#                CORE_ID, MEM_TYPE, mem_range_start, mem_range_start+1))
#            mem_range = len(mem_dump)
#            print("{:02}: {:0{width}b}".format(mem_range_start, mem_dump[0], width=MEM_WIDTH))

        # TODO Add assertions to check for correctness

if __name__ == "__main__":
    test_sticky_bits()
