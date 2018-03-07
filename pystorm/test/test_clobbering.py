"""Test what is causing packet clobbering for long width packets

Set up the following traffic flow:
spike generator -> TAT -> output

Vary the input spike rates and see if output packets get clobbered
"""
import os
import time
import argparse
import numpy as np
import matplotlib.pyplot as plt
np.set_printoptions(precision=1)

from pystorm.hal import HAL
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.PyDriver import bddriver as bd
from pystorm.calibration.utils.exp import clear_tags, compute_spike_gen_rates

CORE_ID = 0 # Set Core to 0, b/c Braindrop only has 1 core
DIM = 1 # 1 dimensional
WEIGHT = 1 # weight of connection from input to output
RUN_TIME = 0.01 # time to sample in s
INTER_RUN_TIME = 2.2 # time between samples in seconds

MEM_DELAY = 15

SPIKE_GEN_TIME_UNIT_NS = 100 # time unit of fpga spike generator
SPIKE_GEN_IDX = 0 # FPGA spike generator index
# Tag Action Table settings
TAT_IDX = 0
TAT_START_ADDR = 0

N_TAGS_TO_SEND = 10
N_TAGS_TO_SHOW = 10
N_CYCLES = 3
IGNORE_TAG_VAL = 2047

DOWNSTREAM_TIME = 100

DATA_DIR = "./data/" + os.path.basename(__file__)[:-3] + "/"
if not os.path.isdir(DATA_DIR):
    os.makedirs(DATA_DIR, exist_ok=True)

def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Characterize the soma max firing rates')
    parser.add_argument("-r", action="store_true", dest="use_saved_data", help='reuse cached data')
    args = parser.parse_args()
    return args

def set_memory_delays(delay):
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
    net.create_connection("i_to_b", net_input, bucket, WEIGHT)

    net_outputs = []
    for tag_idx in range(N_TAGS_TO_SEND):
        cur_net_output = net.create_output("o"+str(tag_idx), DIM)
        net_outputs.append(cur_net_output)
        net.create_connection("b_to_o"+str(tag_idx), bucket, cur_net_output, None)
    HAL.map(net)
    set_memory_delays(MEM_DELAY)
    return net_input, net_outputs

def program_TAT():
    """Program the TAT with tag words"""
    # Added circuitry to try and measure tag rates

#    tag_out = [int("01"*5 + "0", 2), int("10"*5 + "1", 2)] # tag returned by TAT

    output_tags = []
    for i in range(N_TAGS_TO_SEND):
        if i != (N_TAGS_TO_SEND-1):
            output_tags.append([255, i, 0])
        else:
            output_tags.append([255, i, 1])
    print("Output Tags: {}".format(output_tags))
    print("* Sending some tags")
    tag_words = [bd.PackWord([
        (bd.TATTagWord.STOP, to[2]),
        (bd.TATTagWord.GLOBAL_ROUTE, to[0]), # any gtag not 0 goes to PC
        (bd.TATTagWord.TAG, to[1])
        ]) for to in output_tags]

    HAL.driver.SetMem(CORE_ID, bd.bdpars.BDMemId.TAT0, tag_words, TAT_START_ADDR)

def program_spike_filter():
    HAL.driver.SetNumSpikeFilters(CORE_ID, N_TAGS_TO_SEND)

def toggle_spk_generator(rate, run_sleep_time=RUN_TIME, inter_run_sleep_time=INTER_RUN_TIME):
    """Toggle the spike generator and check for overflow"""
    clear_tags(HAL, inter_run_sleep_time, CORE_ID)
    HAL.driver.SetSpikeGeneratorRates(
        CORE_ID, [SPIKE_GEN_IDX], [TAT_IDX], [rate], time=0, flush=True)
    time.sleep(run_sleep_time)
    HAL.driver.SetSpikeGeneratorRates(
        CORE_ID, [SPIKE_GEN_IDX], [TAT_IDX], [0], time=0, flush=True)
    time.sleep(inter_run_sleep_time)

    overflow_0, _ = HAL.driver.GetFIFOOverflowCounts(CORE_ID)
    print("\tRate {:.1f}, overflow_count:{}".format(rate, overflow_0))

    sf_ids, sf_states, sf_times = np.array(HAL.driver.RecvSpikeFilterStates(CORE_ID, 1000))
    return sf_ids, sf_states, sf_times

def toggle_hal(net_input, rate):
    """Toggle the spike input and output recording"""
    HAL.set_time_resolution(upstream_ns=100000, downstream_ns=DOWNSTREAM_TIME)
    HAL.start_traffic(flush=False)
    HAL.enable_output_recording(flush=True)
    t0 = HAL.driver.GetFPGATime()
#    print("FPGA Time: {}".format(t0))
    fudge = int(1e9) # add a 1 s (1e9 ns) fudge
#    print("FPGA Time+Fudge: {}".format(t0+fudge))
    HAL.set_input_rate(net_input, 0, rate, time=t0+fudge, flush=True)
#    HAL.driver.SendTags(CORE_ID, tag_words, tag_times)
#    time.sleep(RUN_TIME)
    duration = int(RUN_TIME * 1e9)
    HAL.set_input_rate(net_input, 0, 0, time=t0+fudge+duration, flush=True)
#    print("FPGA Time+Fudge+duration: {}".format(t0+fudge+duration))
#    HAL.driver.SendTags(CORE_ID, tag_words, tag_times)
    HAL.stop_traffic(flush=False)
    HAL.disable_output_recording(flush=True)
    time.sleep(INTER_RUN_TIME)
#    HAL.set_time_resolution(upstream_ns=1000000, downstream_ns=DOWNSTREAM_TIME)

def sweep_tag_input_rates(parsed_args):
    """Perform the test"""
    use_saved_data = parsed_args.use_saved_data
    if use_saved_data:
        print("Should be using saved data, but does nothing in this script at the moment")
    else:
        HAL.disable_spike_recording(flush=True)
#        HAL.set_time_resolution(upstream_ns=100000, downstream_ns=DOWNSTREAM_TIME)

#        program_spike_filter()
#        program_TAT()
        rates = np.arange(800000, 900000, 10000) # specify rates in units of Hz
        rates = np.arange(100000, 900000, 100000) # specify rates in units of Hz
        rates = np.array([769230, 980000])
#        rates = np.logspace(4.0, 6.0, num=5).astype(int)
        n_rates = len(rates)
        print("Checking {} rates: {}".format(n_rates, rates))
#        measured_rates = np.zeros(n_rates)
        measured_rates = np.zeros(N_CYCLES*n_rates)
#        measured_rates_tags = np.zeros(n_rates)
        measured_rates_tags = np.zeros(N_CYCLES*n_rates)

        HAL.driver.SetSpikeFilterDebug(CORE_ID, True)
        net_input, net_outputs = build_net()

        for cycle_idx in range(N_CYCLES):
            for rate_idx, rate in enumerate(rates):
                toggle_hal(net_input, rate)
#                # Use get_outputs
#                print("\n[OUTPUT] Checking rate {}".format(rate))
#                binned_tags = HAL.get_outputs()
#                #print("[OUTPUT] Shape of outputs: {}".format(binned_tags.shape))
#                measured_time = (binned_tags[-1, 0] - binned_tags[0, 0])/1e9
#                #print("measured time: {}".format(measured_time))
#                total_tags = np.sum(binned_tags[:, 3])
#                measured_rates[rate_idx] = total_tags/measured_time
#                #measured_rates[cycle_idx*len(rates)+rate_idx] = total_tags/measured_time
#                filtered_tags = binned_tags[binned_tags[:, 3] > 0]
#                print("Output Tags:")
#                print(filtered_tags[:, 1:4])
#                for tag_idx, cur_output in enumerate(net_outputs):
#                    cur_total_tags = np.sum(filtered_tags[filtered_tags[:, 1] == tag_idx, 0])
#                    print("[OUTPUT] [{}]\tCount of output [{}]\t".format(rate, tag_idx)+
#                          "tags: {} ({:5.2f}%)".format(cur_total_tags, cur_total_tags/total_tags*100))
#                print("[OUTPUT] [{}]\tNumber of output tags: {}".format(rate, total_tags))
#                print("[OUTPUT] [{}]\tMeasured rate: {}".format(rate, measured_rates[rate_idx]))
#                #print("[OUTPUT] [{}]\tMeasured rate: {}".format(rate, measured_rates[cycle_idx*len(rates)+rate_idx]))

                # Use RecvTags instead of get_outputs
                tag_outputs = np.array(HAL.driver.RecvUnpackedTags(CORE_ID, 1000)).T
                #print("[OUTPUT] [{}] Sample of raw tag outputs: {}".format(rate, tag_outputs[:12]))
                filtered_tags = tag_outputs[tag_outputs[:, 0] > 0]
                #print("[OUTPUT] [{}] Sample of filtered tag outputs: {}".format(rate, tag_outputs[:12]))

                tag_counts = filtered_tags[:, 0]
                tag_tags = filtered_tags[:, 1]
                #tag_routes = filtered_tags[:, 2]
                tag_times = filtered_tags[:, 3]
                #print("[OUTPUT] [{}] Sample of tag counts: {}".format(rate, tag_counts[:12]))
                #print("[OUTPUT] [{}] Sample of tag tags: {}".format(rate, tag_tags[:12]))
                #print("[OUTPUT] [{}] Sample of tag routes: {}".format(rate, tag_routes[:12]))
                #print("[OUTPUT] [{}] Sample of tag times: {}".format(rate, tag_times[:12]))

                if(len(tag_times) > 0):
                    measured_time = (tag_times[-1] - tag_times[0])/1e9
                else:
                    measured_time = 1e9
                total_tags = len(tag_tags)
                #measured_rates_tags[rate_idx] = 1.*total_tags/measured_time
                measured_rates_tags[cycle_idx*len(rates)+rate_idx] = 1.*total_tags/measured_time

                print("\nReceived Tags for Rate {}:".format(rate))

                for tag_idx, cur_output in enumerate(net_outputs):
                    cur_total_tags = np.sum(filtered_tags[filtered_tags[:, 1] == tag_idx, 0])
                    print("[OUTPUT] [{}]\tCount of output [{}]\t".format(rate, tag_idx)+
                          "tags: {} ({:5.2f}%)".format(cur_total_tags, cur_total_tags/total_tags*100))
                print("[OUTPUT] [{}]\tMeasured time: {}\t".format(rate, measured_time) +
                      "Num of tags: {}\tSum of tag counts: {}".format(total_tags, np.sum(tag_counts)))
                print("[OUTPUT] [{}]\t".format(rate) +
                      #"Measured rate: {}".format( measured_rates_tags[rate_idx]))
                      "Measured rate: {}".format( measured_rates_tags[cycle_idx*len(rates)+rate_idx]))

                print("[OUTPUT] [{}]\tTotal overflows: {}\n".format(rate, HAL.get_overflow_counts()))
                print("[OUTPUT] [{}]\t{}\n".format(rate, tag_tags[:N_TAGS_TO_SEND*4]))

#                # Assert test for automatic pytest compatibility
#                non_padded_tag_idx = 0
#                input_tag_list = np.array(range(N_TAGS_TO_SEND))
#                for tag_i, tag in enumerate(tag_tags):
#                    if tag == IGNORE_TAG_VAL:
#                        #print("Ignored a padded tag output entry")
#                        pass
#                    else:
#                        cur_tag_idx = non_padded_tag_idx % N_TAGS_TO_SEND
#                        if tag_i < N_TAGS_TO_SHOW:
#                            assert input_tag_list[cur_tag_idx] == tag, (
#                                "Received unexpected tag value of {} ".format(tag) +
#                                "(expected tag value of {}) at index {}\n".format(input_tag_list[cur_tag_idx], tag_i) +
#                                "Tag Stream starting @ index 0:\n\t" +
#                                "{}".format(tag_tags[0:tag_i+N_TAGS_TO_SHOW]))
#                        else:
#                            assert input_tag_list[cur_tag_idx] == tag, (
#                                "Received unexpected tag value of {} ".format(tag) +
#                                "(expected tag value of {}) at index {}\n".format(input_tag_list[cur_tag_idx], tag_i) +
#                                "Tag Stream centered @ index {}:\n\t".format(tag_i) +
#                                "{}".format(tag_tags[tag_i-N_TAGS_TO_SHOW:tag_i+N_TAGS_TO_SHOW+1]))
#                        non_padded_tag_idx += 1


if __name__ == "__main__":
    sweep_tag_input_rates(parse_args())
