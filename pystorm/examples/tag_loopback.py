from pystorm.PyDriver import bddriver as bd
import time

"""TAT0[0] is configured to forward one input tag to another out
We then send some tags and record the outputs
"""

CORE = 0
tag_in_start = 0
tag_out = [int("01"*5 + "0", 2), int("10"*5 + "1", 2)] # tag returned by TAT
#count = [int("01"*4 + "0", 2), int("10"*4 + "1", 2)] # counts passed through
count = [1, 511] # counts passed through
N_tags_to_send = 100

print("* Starting BD")
D = bd.Driver()
D.Start() # starts driver threads
D.ResetBD()
D.InitFIFO(CORE)
print("* Setting FPGA time units")
FPGA_unit = 10 # us
D.SetTimeUnitLen(FPGA_unit)
D.ResetFPGATime()

print("* Programming TAT for loopback")
lb_entries = [bd.PackWord([
    (bd.TATTagWord.STOP, 1),
    (bd.TATTagWord.GLOBAL_ROUTE, 1), # any gtag not 0 goes to PC
    (bd.TATTagWord.TAG, to)
    ]) for to in tag_out]

D.SetMem(CORE, bd.bdpars.BDMemId.TAT0, lb_entries, tag_in_start)
#D.DumpMem(CORE, bd.bdpars.BDMemId.TAT0)

print("* Enabling tag traffic")
D.SetTagTrafficState(CORE, True, True)

input_tags      = [tag_in_start + (i % len(tag_out)) for i in range(N_tags_to_send)]
exp_output_tags = [tag_out[i % len(tag_out)] for i in range(N_tags_to_send)]
input_cts       = [  count[i % len(tag_out)] for i in range(N_tags_to_send)]
input_cts       = [(i+1)%512 for i in range(N_tags_to_send)]
exp_output_cts  = input_cts

print("* Sending some tags")
tag_words = [bd.PackWord([
    (bd.InputTag.TAG, inp_tag),
    (bd.InputTag.COUNT, inp_ct)
    ]) for inp_tag, inp_ct in zip(input_tags, input_cts)]
#tag_times = [i*FPGA_unit for i in range(N_tags_to_send)]
tag_times = []

print("NOW")
time.sleep(2)
D.SendTags(CORE, tag_words[:50], tag_times[:50])
D.SendTags(CORE, tag_words[50:], tag_times[50:])
#D.SendTags(CORE, tag_words, tag_times)

time.sleep(.5)

print("driver queue states after experiment:")
print("  (upstream ep code, count)")
for tup in D.GetOutputQueueCounts():
    print(" ", tup)

print("* Getting returned tags")
tags, times = D.RecvTags(0, 1000)
print(len(tags))
rec_output_tags = []
rec_output_cts = []
for tag in tags:
    ltag = bd.GetField(tag, bd.TATOutputTag.TAG)
    gtag = bd.GetField(tag, bd.TATOutputTag.GLOBAL_ROUTE)
    count = bd.GetField(tag, bd.TATOutputTag.COUNT)
    #print("ltag :", "{0:011b}".format(ltag))
    #print("gtag :", "{0:012b}".format(gtag))
    #print("ct   :", "{0:09b}".format(count))
    rec_output_tags.append(ltag)
    rec_output_cts.append(count)

# discard initial all-zero-words
while rec_output_cts[0] != exp_output_cts[0]:
    rec_output_tags = rec_output_tags[1:]
    rec_output_cts = rec_output_cts[1:]

#for rec, exp in zip(rec_output_tags, exp_output_tags):
#    if rec == exp:
#        match = "     "
#    else:
#        match = "FAIL "
#    print(match, "{0:011b}".format(rec), "vs", "{0:011b}".format(exp))
    
had_XXXX = False
seen = {}

for rec, exp in zip(rec_output_cts, exp_output_cts):
    rec_bin = "{0:09b}".format(rec)
    exp_bin = "{0:09b}".format(exp)

    if rec == exp:
        match = "     "
    elif rec != exp_output_cts[0] and rec != exp_output_cts[1]:
        match = "XXXX "
        had_XXXX = True
    else:
        match = "FAIL "

    if rec not in seen:
        seen[rec] = 0
    seen[rec] += 1

    print(match, rec_bin, "(", rec, ")", "vs", exp_bin, "(", exp, ")")

if had_XXXX:
    print("REPRODUCED PROBLEM")
else:
    print("bit flip problem not observed")

print(seen)

D.Stop()

