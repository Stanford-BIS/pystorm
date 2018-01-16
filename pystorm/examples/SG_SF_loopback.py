"""Programs SG[0] to emit tags at some rate to tag 0
TAT0[0] is configured to forward tag 0 out as a global tag 0
SF[0] is configured to filter this with a 10 ms tau
"""
from pystorm.PyDriver import bddriver as bd
import time

CORE = 0

range_len = 4
tag_in = [i for i in range(range_len)] # tag we use to make a loopback entry
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
FPGA_unit = 100000 # ns
D.SetTimeUnitLen(FPGA_unit)

D.SetTimePerUpHB(100000) # ns

print("* Init the FIFO")
D.InitFIFO(CORE)

print("* Enable tag traffic")
D.SetTagTrafficState(CORE, True, True)

print("* Programming TAT for loopback through tag 0")
lb_entries = [bd.PackWord([
    (bd.TATTagWord.STOP, 1),
    (bd.TATTagWord.GLOBAL_ROUTE, 1), # any gtag not 0 goes to PC
    (bd.TATTagWord.TAG, t)
    ]) for t in tag_out]

#lb_entries = [
#    bd.PackWord([
#    (bd.TATTagWord.STOP, 0),
#    (bd.TATTagWord.GLOBAL_ROUTE, 1), # any gtag not 0 goes to PC
#    (bd.TATTagWord.TAG, 0)
#    ]),
#    bd.PackWord([
#    (bd.TATTagWord.STOP, 0),
#    (bd.TATTagWord.GLOBAL_ROUTE, 1), # any gtag not 0 goes to PC
#    (bd.TATTagWord.TAG, 0)
#    ]),
#    bd.PackWord([
#    (bd.TATTagWord.STOP, 0),
#    (bd.TATTagWord.GLOBAL_ROUTE, 1), # any gtag not 0 goes to PC
#    (bd.TATTagWord.TAG, 0)
#    ]),
#    bd.PackWord([
#    (bd.TATTagWord.STOP, 1),
#    (bd.TATTagWord.GLOBAL_ROUTE, 1), # any gtag not 0 goes to PC
#    (bd.TATTagWord.TAG, 1)
#    ])]
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

N_tags_to_send = 10
print("* Sending", N_tags_to_send, "tags directly")
tag_word = bd.PackWord([
    (bd.InputTag.TAG, tag_in[0]),
    (bd.InputTag.COUNT, 1)
    ])
D.SendTags(CORE, [tag_word]*N_tags_to_send, [])

time.sleep(.1)

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
print_tat_tags(tags, times)

print("queue states after manually sending tags:")
print_queue_states(D)

## observe tag dumps before FIFO
# XXX this seems to have unusual effects
#D.SetPreFIFODumpState(CORE, True)
#D.SetPostFIFODumpState(CORE, True)

print("trigger scope now!")
time.sleep(2)

print("* Setting up spike generator")
print("  rate = ", tag_rate)
#D.SetSpikeGeneratorRates(CORE, [i for i in range(range_len)], tag_in, tag_rate)
D.SetSpikeGeneratorRates(CORE, [0], [tag_in[0]], [tag_rate[0]])
#D.SetSpikeGeneratorRates(CORE, [1], [tag_in[1]], [tag_rate[1]+97])
#D.SetSpikeGeneratorRates(CORE, [1], [tag_in[1]], [tag_rate[1]])
#D.SetSpikeGeneratorRates(CORE, [1], [tag_in[1]], [tag_rate[1]])
#D.SetSpikeGeneratorRates(CORE, [1], [tag_in[0]], [tag_rate[1]])

print("* Sleeping for", experiment_duration, "seconds during experiment")
time.sleep(experiment_duration)

print("queue states after experiment:")
print_queue_states(D)

D.SetSpikeGeneratorRates(CORE, [0], [tag_in[0]], [0])

time.sleep(.1)

tags, times = D.RecvTags(CORE, 1000)
print_tat_tags(tags, times)

def print_pre_fifo_tags(tags, max_to_print=20):
    print("* Received", len(tags), "pre-fifo tags")
    cts = {}
    for tag_idx, tag in enumerate(tags[:max_to_print]):
        tag = bd.GetField(tag, bd.PreFIFOTag.TAG)
        ct = bd.GetField(tag, bd.PreFIFOTag.COUNT)
        print(" ", tag_idx, ": ltag", tag,
              "| ct", ct)
        if tag not in cts:
            cts[tag] = 0
        cts[tag] += ct

    if (len(tags) > max_to_print):
        print("  ...")

    print("total counts")
    print(cts)

def print_post_fifo_tags(tags, max_to_print=20):
    print("* Received", len(tags), "post-fifo tags")
    for tag_idx, tag in enumerate(tags[:max_to_print]):
        print(" ", tag_idx, ": ltag", bd.GetField(tag, bd.PostFIFOTag.TAG),
              "| ct", bd.GetField(tag, bd.PostFIFOTag.COUNT))
    if (len(tags) > max_to_print):
        print("  ...")

pre_tags = D.GetPreFIFODump(CORE)
post_tags0, post_tags1 = D.GetPostFIFODump(CORE)

print_pre_fifo_tags(pre_tags)
print_post_fifo_tags(post_tags0)
print_post_fifo_tags(post_tags1)

print("* turn off SG, try sending tags manually again")

time.sleep(1)

N_tags_to_send = 10
print("* Sending", N_tags_to_send, "tags directly")
tag_word = bd.PackWord([
    (bd.InputTag.TAG, tag_in[0]),
    (bd.InputTag.COUNT, 1)
    ])
D.SendTags(CORE, [tag_word]*N_tags_to_send, [i for i in range(N_tags_to_send)])

time.sleep(.1)

tags, times = D.RecvTags(CORE, 1000)
print_tat_tags(tags, times)

#print("* Resetting FPGA clock")
## set FPGA clock to 0 before we start sending timed tags
#D.ResetFPGATime()

#print("* Setting up spike filter")
## spike filter in binned count mode
#D.SetSpikeFilterIncrementConst(CORE, 1, flush=False)
#D.SetSpikeFilterDecayConst(CORE, 0, flush=False)
#D.SetNumSpikeFilters(CORE, 1, flush=True)

#
##print("* Sending some tags directly")
##N_tags_to_send = 100
##tag_word = bd.PackWord([
##    (bd.InputTag.TAG, tag_in),
##    (bd.InputTag.COUNT, 1)
##    ])
##D.SendTags(CORE, [tag_word]*N_tags_to_send, [i for i in range(N_tags_to_send)])
#
#time.sleep(experiment_duration)
#
#states, times = D.RecvSpikeFilterStates(CORE, 1)
#print("got ", len(states), " SF outputs")
#N_to_print = 10
#stride = len(states) // N_to_print
#print("printing nonzero samples")
#total_count = 0
#for idx, (s, t) in enumerate(zip(states, times)):
#    filt_idx = bd.GetField(s, bd.FPGASFWORD.FILTIDX)
#    state = bd.GetField(s, bd.FPGASFWORD.STATE)
#    if (state != 0):
#        print("time: ", t, " (FPGA units)")
#        print("SF idx: ", filt_idx)
#        print("SF state (count): ", state)
#
#    total_count += state
#
#print("total count = ", total_count)
#print("after exp:", D.GetOutputQueueCounts())
#
#tags, times = D.RecvTags(CORE, 1000)
#print(len(tags))
#for tag in tags[0:10]:
#    print("ltag", bd.GetField(tag, bd.TATOutputTag.TAG),
#          "gtag", bd.GetField(tag, bd.TATOutputTag.GLOBAL_ROUTE),
#          "ct", bd.GetField(tag, bd.TATOutputTag.COUNT))
#
#words = D.GetPreFIFODump(CORE)
#for word in words[0:200]:
#    print("ct", bd.GetField(word, bd.PreFIFOTag.COUNT),
#          "tag", bd.GetField(word, bd.PreFIFOTag.TAG))

D.Stop()
