from pystorm.PyDriver import bddriver as bd
import time

CORE = 0

range_len = 3
tag_in = [i for i in range(range_len)] # tag we use to make a loopback entry
tag_in = [0] * range_len
tag_out = [i for i in range(range_len)] # tag returned by TAT
tag_rate = [100 for i in range(range_len)] # Hz
experiment_duration = 1 # sec

D = bd.Driver()

comm_state = D.Start()
if (comm_state < 0):
    print("* Driver failed to start!")
    exit(-1)

print("* Resetting BD")
D.ResetBD()

# set time unit (SF/SG update interval) to .1 ms
print("* Setting FPGA time units")
FPGA_unit = 10000 # ns
D.SetTimeUnitLen(FPGA_unit)

# set upstream HB time
D.SetTimePerUpHB(100000) # ns

print("* Init the FIFO")
D.InitFIFO(CORE)

print("* Enable tag traffic")
D.SetTagTrafficState(CORE, True, True)

print("* Programming TAT for loopback through tag 0")
#lb_entries = [bd.PackWord([
#    (bd.TATTagWord.STOP, 1),
#    (bd.TATTagWord.GLOBAL_ROUTE, 1), # any gtag not 0 goes to PC
#    (bd.TATTagWord.TAG, t)
#    ]) for t in tag_out]
lb_entries = [bd.PackWord([
    (bd.TATTagWord.STOP, 0),
    (bd.TATTagWord.GLOBAL_ROUTE, 1), # any gtag not 0 goes to PC
    (bd.TATTagWord.TAG, t)
    ]) for t in tag_out]
lb_entries[-1] = bd.PackWord([
    (bd.TATTagWord.STOP, 1),
    (bd.TATTagWord.GLOBAL_ROUTE, 1), # any gtag not 0 goes to PC
    (bd.TATTagWord.TAG, tag_out[-1])
    ])
D.SetMem(CORE, bd.bdpars.BDMemId.TAT0, lb_entries, tag_in[0])

print("* Reading TAT")
mem_vals = D.DumpMem(CORE, bd.bdpars.BDMemId.TAT0)

if len(mem_vals) >= 1 and sum([i == j for i,j in zip(mem_vals, lb_entries)]) == range_len:
    print("* TAT matches!")
else:
    print("* TAT didn't match")
    if len(mem_vals) == 0:
        print("we read nothing!")
    for entry, lb_entry in zip(mem_vals[:range_len], lb_entries):
        print("we programmed:")
        print("{0:b}".format(lb_entry))
        print("we read:")
        print("{0:b}".format(entry))
        print("stop", bd.GetField(entry, bd.TATTagWord.STOP))
        print("groute", bd.GetField(entry, bd.TATTagWord.GLOBAL_ROUTE))
        print("tag", bd.GetField(entry, bd.TATTagWord.TAG))
    D.Stop()
    exit(-1)

print("trigger scope now!")
time.sleep(1)

N_tags_to_send = 100
print("* Sending", N_tags_to_send, "tags directly")
tag_words = [bd.PackWord([
    (bd.InputTag.TAG, t),
    (bd.InputTag.COUNT, 1) 
    ]) for t in tag_in]
all_tag_words = tag_words*(N_tags_to_send//range_len)
interval = 100000 # ns

# send out 1s in future
curr_FPGA_time = D.GetFPGATime()
print("current time:", curr_FPGA_time)
start_time = curr_FPGA_time + int(1e9)

times = [i + start_time for i in range(0, len(all_tag_words)*interval, interval)]
D.SendTags(CORE, all_tag_words, times)

time.sleep(1)

def print_queue_states(D):
    print("printing driver queue states:")
    for ep_code, ct in D.GetOutputQueueCounts():
        if ct > 0:
            print("  ep_code", ep_code, "had", ct, "entries")

print("queue states after sending tags:")
print_queue_states(D)

def print_tat_tags(tags, times, max_to_print=20):
    print("* Received", len(tags), "TAT tags")
    cts = {}
    for tag_idx, (tag, time) in enumerate(zip(tags, times)):
        ltag = bd.GetField(tag, bd.TATOutputTag.TAG)
        gtag = bd.GetField(tag, bd.TATOutputTag.GLOBAL_ROUTE)
        ct = bd.GetField(tag, bd.TATOutputTag.COUNT)

        key = (gtag, ltag)
        if key not in cts:
            cts[key] = 0
        cts[key] += ct

        if (tag_idx < max_to_print):
            print(" ", tag_idx, ": ltag", ltag,
                  "| gtag", gtag,
                  "| ct", ct,
                  "| time", time)

    if (len(tags) > max_to_print):
        print("  ...")

    print("")
    print("  ------------")
    print("  total counts")
    print(cts)

tags, times = D.RecvTags(CORE, 1000)
print_tat_tags(tags, times, 100)

D.Stop()
