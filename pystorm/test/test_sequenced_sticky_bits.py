"""Test packet clobbering and sticky bits at the same time

Set up the following traffic flow:
    spike generator -> TAT -> output
    (where the TAT values aren't just sequenced, but are actually mostly 0s or 1s
    with a little sequencing at the end)

Vary the input spike rates and see if output packets get clobbered while
also checking for sticky bits
"""
import time
import numpy as np
from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.PyDriver import bddriver as bd
from pystorm.calibration.utils.exp import clear_tags, clear_overflows
HAL = HAL()
np.set_printoptions(precision=1)

CORE_ID = 0 # Set Core to 0, b/c Braindrop only has 1 core
DIM = 1                 # 1 dimensional
WEIGHT = 1              # weight of connection from input to output
RUN_TIME = 0.0001       # time to sample (s)
INTER_RUN_TIME = 2.2    # time between samples (s)
DOWNSTREAM_TIME = 100   # FPGA operation time (ns)

GTAG = 16383

MEM_DELAY = 15

SPIKE_GEN_IDX = 0       # FPGA spike generator index

# Tag Action Table settings
TAT_IDX = 0
TAT_START_ADDR = 0
TAG_BIT_WIDTH = 11      # The number bits in the data of the memory

MEM_TYPE = bd.bdpars.BDMemId.TAT1
NUM_MEM_ENTRIES = 32
MEM_WIDTH = 34

N_TAGS_TO_SEND = 16     # ideally a power of two, to see bits iterate to a given digit
N_TAGS_TO_SHOW = 20
IGNORE_TAG_VAL = 2047

# Various Flags to change the modes of this test
TEST_MODE = False       # TEST_MODE = True when we want this script to be able to run with pytest
                        # Use TEST_MODE = False for debugging or characterization of a chip
USE_RECV_TAGS = True    # USE_RECV_TAGS uses RecvTags to get data, as opposed to get_outputs
DUMP_MEM = True        # DUMP_MEM = True to dump memory for debugging
INCR_EACH = False       # Increment tag number for each entry (vs every other entry)

def set_memory_delays(delay):
    """Set memory delays"""
    HAL.driver.SetMemoryDelay(CORE_ID, bd.bdpars.BDMemId.PAT, delay, delay)
    HAL.driver.SetMemoryDelay(CORE_ID, bd.bdpars.BDMemId.AM, delay, delay)
    HAL.driver.SetMemoryDelay(CORE_ID, bd.bdpars.BDMemId.MM, delay, delay)
    HAL.driver.SetMemoryDelay(CORE_ID, bd.bdpars.BDMemId.TAT0, delay, delay)
    HAL.driver.SetMemoryDelay(CORE_ID, bd.bdpars.BDMemId.TAT1, delay, delay)
    HAL.driver.SetMemoryDelay(CORE_ID, bd.bdpars.BDMemId.FIFO_DCT, delay, delay)
    HAL.driver.SetMemoryDelay(CORE_ID, bd.bdpars.BDMemId.FIFO_PG, delay, delay)

def dump_mem():
    """Dump the memory (for debugging purposes)"""
    mem_dump_output = []
    mem_dump = np.array(HAL.driver.DumpMemRange(CORE_ID, MEM_TYPE, 0, NUM_MEM_ENTRIES))
    print("    : {:0{width}b}\t".format(2**14, width=MEM_WIDTH) +
          "    : {:0{width}b}".format(2**14, width=MEM_WIDTH))
    for idx in range(int(NUM_MEM_ENTRIES/2)):
        mem_entry1 = mem_dump[idx*2]
        mem_entry2 = mem_dump[idx*2+1]
        mem_dump_output.append("{:0{width}b}".format(mem_entry1, width=MEM_WIDTH))
        mem_dump_output.append("{:0{width}b}".format(mem_entry2, width=MEM_WIDTH))
        print("{:04}: {:0{width}b}\t".format(idx*2, mem_entry1, width=MEM_WIDTH) +
              "{:04}: {:0{width}b}".format(idx*2+1, mem_entry2, width=MEM_WIDTH))

def build_net():
    """Build a network for testing"""
    net = graph.Network("net")
    net_input = net.create_input("i", DIM)
    bucket = net.create_bucket("b", DIM)
    net.create_connection("i_to_b", net_input, bucket, WEIGHT)

    net_outputs = []
    for tag_idx in range(N_TAGS_TO_SEND):
        cur_net_output = net.create_output("o"+str(tag_idx), DIM)
        net_outputs.append(cur_net_output)
        net.create_connection("b_to_o"+str(tag_idx), bucket, cur_net_output, None)
    HAL.map(net)
    set_memory_delays(MEM_DELAY)
    return net_input, net_outputs

def program_tat():
    """Program the TAT with tag words"""
    # Added circuitry to try and measure tag rates
    output_tags = []
    for i in range(N_TAGS_TO_SEND):
        if i%2 == 0:
            gtag = GTAG
        else:
            gtag = GTAG+1

        if INCR_EACH:
            # Send completely sequential tags
            # seq_count val is the number to non-inclusively count up to (0..seq_count_val-1)
            seq_count_val = N_TAGS_TO_SEND
            seq_bit_val = int(i)%seq_count_val
            tag_value = seq_bit_val%seq_count_val
        else:
            # Increment tags every other tag (to see odd vs even behavior)
            # seq_count val is the number to non-inclusively count up to (0..seq_count_val-1)
            seq_count_val = int(N_TAGS_TO_SEND/2)
            seq_bit_val = int(i/2)%seq_count_val
            if i%2 == 0:
                tag_value = seq_bit_val%seq_count_val# + 2**(TAG_BIT_WIDTH)
            else:
                tag_value = 2**TAG_BIT_WIDTH-seq_count_val+seq_bit_val
        #print("{}".format(seq_bit_val))
        gtag = 255  # Hack to avoid needing to rewrite the HDL to handle invalid routes
        # This means we don't get to test as many bits of stickiness

        #output_tags.append([gtag, tag_value, 1])
        if i != (N_TAGS_TO_SEND-1):
            output_tags.append([gtag, tag_value, 0])
        else:
            output_tags.append([gtag, tag_value, 1])

    print("Output Tags: {}".format(output_tags))
    print("* Sending some tags")
#    tag_packed_words = HAL.driver.PackTATTagWords(list(range(10)), [255]*10, [0]*9+[1])
    tag_packed_words = [bd.PackWord([
        (bd.TATTagWord.STOP, to[2]),
        (bd.TATTagWord.GLOBAL_ROUTE, to[0]), # any gtag not 0 goes to PC
        (bd.TATTagWord.TAG, to[1])
        ]) for to in output_tags]

    HAL.driver.SetMem(CORE_ID, MEM_TYPE, tag_packed_words, TAT_START_ADDR)

    if DUMP_MEM:
        dump_mem()
    return output_tags

def print_unpacked_tags(tags):
    """Print unpacked tags at integers and binary"""
    for tag_idx, tag in enumerate(tags):
        print("{:04}: ({:04})\t{:0{width}b}".format(tag_idx, tag, tag, width=11))

def get_unpacked_recv_tags(net_outputs, rates, rate_idx):
    """Use RecvUnpackedTags to get outputs from HAL"""
    rate = rates[rate_idx]
    print("\n[OUTPUT] Checking rate {}".format(rate))
    n_rates = len(rates)
    measured_rates_tags = np.zeros(n_rates)

    tag_outputs = np.array(HAL.driver.RecvUnpackedTags(CORE_ID, 1000)).T

    filtered_tags = tag_outputs[tag_outputs[:, 0] > 0]
    tag_counts = filtered_tags[:, 0]
    tag_tags = filtered_tags[:, 1]
    tag_routes = filtered_tags[:, 2]
    tag_times = filtered_tags[:, 3]
    #print("[OUTPUT] [{}] Sample of tag counts: {}".format(rate, tag_counts[:N_TAGS_TO_SHOW]))
    #print("[OUTPUT] [{}] Sample of tag tags: {}".format(rate, tag_tags[:N_TAGS_TO_SHOW]))
    #print("[OUTPUT] [{}] Sample of tag routes: {}".format(rate, tag_routes[:N_TAGS_TO_SHOW]))
    #print("[OUTPUT] [{}] Sample of tag times: {}".format(rate, tag_times[:N_TAGS_TO_SHOW]))

    print_unpacked_tags(tag_tags)

    # Get and print output information
    if tag_times.shape[0] > 0:
        measured_time = (tag_times[-1] - tag_times[0])/1e9
    else:
        measured_time = 1e9
    total_tags = len(tag_tags)
    measured_rates_tags[rate_idx] = 1.*total_tags/measured_time

    print("\nReceived Tags for Rate {}:".format(rate))
    #print("[OUTPUT] [{}]\t{}".format(rate, tag_tags[:N_TAGS_TO_SEND*4]))

    if len(set(tag_tags)) != len(net_outputs):
        print("WARNING: Some of the output tags never got sent")

    for tag_idx, cur_tag in enumerate(sorted(list(set(tag_tags)))):
        cur_total_tags = np.sum(filtered_tags[filtered_tags[:, 1] == cur_tag, 0])
        print("[OUTPUT] [{}]\tCount of output [{}]\t".format(rate, cur_tag)+
              "tags: {} ({:5.2f}%)".format(cur_total_tags, cur_total_tags/total_tags*100))
    print("[OUTPUT] [{}]\tMeasured time: {}\t".format(rate, measured_time) +
          "Num of tags: {}\tSum of tag counts: {}".format(total_tags, np.sum(tag_counts)))
    print("[OUTPUT] [{}]\t".format(rate) +
          "Measured rate: {}".format(measured_rates_tags[rate_idx]))

    print("[OUTPUT] [{}]\tTotal overflows: {}\n".format(rate, HAL.get_overflow_counts()))

    return measured_rates_tags, tag_tags


def get_packed_recv_tags(net_outputs, rates, rate_idx):
    """Use RecvTags to get outputs from HAL (this method 
    outputs less stats than get_unpacked_recv_tags)"""
    rate = rates[rate_idx]
    print("\n[OUTPUT] Checking rate {}".format(rate))
    n_rates = len(rates)

    packed_tags = HAL.driver.RecvTags(CORE_ID, 1000)
    for tag_idx, tag in enumerate(packed_tags[0]):
        print("{:04}: {:0{width}b}".format(tag_idx, tag, width=MEM_WIDTH))

    return packed_tags

def toggle_hal(net_input, rate, tags=None):
    """Toggle the spike input and output recording"""
    HAL.set_time_resolution(upstream_ns=100000, downstream_ns=DOWNSTREAM_TIME)
    HAL.start_traffic(flush=False)
    HAL.enable_output_recording(flush=True)
    t0 = HAL.driver.GetFPGATime()
#    print("FPGA Time: {}".format(t0))
    fudge = 1000000000
#    print("FPGA Time+Fudge: {}".format(t0+fudge))
    HAL.set_input_rate(net_input, 0, rate, time=t0+fudge, flush=True)
#    HAL.driver.SendTags(CORE_ID, tags, [])
#    time.sleep(RUN_TIME)
    duration = int(RUN_TIME * 1e9)
    HAL.set_input_rate(net_input, 0, 0, time=t0+fudge+duration, flush=True)
#    print("FPGA Time+Fudge+duration: {}".format(t0+fudge+duration))
#    HAL.driver.SendTags(CORE_ID, tags, tag_times)
    HAL.stop_traffic(flush=False)
    HAL.disable_output_recording(flush=True)
    time.sleep(INTER_RUN_TIME)

def test_sequenced_sticky_bits():
    """Perform the test"""
    HAL.disable_spike_recording(flush=True)

    rates = np.arange(100000, 400001, 100000) # specify rates in units of Hz
    print("Checking {} rates: {}".format(len(rates), rates))

    HAL.driver.SetSpikeFilterDebug(CORE_ID, True)
    net_input, net_outputs = build_net()

    output_tags = program_tat()
    tags = list(np.array(output_tags).T[1])
    tags = None

    for rate_idx, rate in enumerate(rates):
        toggle_hal(net_input, rate, tags)
        #measured_rates_tags, tag_tags = get_unpacked_recv_tags(net_outputs, rates, rate_idx)
        packed_tags = get_packed_recv_tags(net_outputs, rates, rate_idx)

        # Assert test for automatic pytest compatibility
        if TEST_MODE:
            non_padded_tag_idx = 0
            input_tag_list = np.array(range(N_TAGS_TO_SEND))
            for tag_i, tag in enumerate(tag_tags):
                if tag == IGNORE_TAG_VAL:
                    #print("Ignored a padded tag output entry")
                    pass
                else:
                    cur_tag_idx = non_padded_tag_idx % N_TAGS_TO_SEND
                    # If there aren't enough tags to show the history,
                    # show from the beginning of the tag stream
                    if tag_i < N_TAGS_TO_SHOW:
                        assert input_tag_list[cur_tag_idx] == tag, (
                            "Received unexpected tag value of {} ".format(tag) +
                            "(expected tag value of {})".format(input_tag_list[cur_tag_idx]) +
                            " at index {}\n".format(tag_i) +
                            "Tag Stream starting @ index 0:\n\t" +
                            "{}".format(tag_tags[0:tag_i+N_TAGS_TO_SHOW]))
                    # Else, show N_TAGS_TO_SHOW both forward and backwards, with the center value
                    # showing the first errant value.
                    else:
                        assert input_tag_list[cur_tag_idx] == tag, (
                            "Received unexpected tag value of {} ".format(tag) +
                            "(expected tag value of {})".format(input_tag_list[cur_tag_idx]) +
                            " at index {}\n".format(tag_i) +
                            "Tag Stream centered @ index {}:\n\t".format(tag_i) +
                            "{}".format(tag_tags[tag_i-N_TAGS_TO_SHOW:tag_i+N_TAGS_TO_SHOW+1]))
                    non_padded_tag_idx += 1


if __name__ == "__main__":
    test_sequenced_sticky_bits()
