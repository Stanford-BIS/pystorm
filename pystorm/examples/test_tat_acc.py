import numpy as np
import matplotlib
import time

from pystorm.PyDriver import bddriver as bd
import driver_util
from driver_util import s_to_ns, ns_to_s

num_tags_in = 8000
duration = 4 # seconds

D = driver_util.standard_startup()

CORE = 0

print("* Init'ing DACs")
time.sleep(1)
D.InitDAC(CORE)

print("* Programming AM")
acc_entry = bd.PackWord([
  (bd.AMWord.STOP, 1),
  (bd.AMWord.THRESHOLD, 1),
  (bd.AMWord.NEXT_ADDRESS, 2**11)])
D.SetMem(CORE, bd.bdpars.BDMemId.AM, [acc_entry], 0)

print("* Programming MM")
D.SetMem(CORE, bd.bdpars.BDMemId.MM, [64], 0)

print("* Programming TAT to target acc")
TAT_entry = bd.PackWord([
  (bd.TATAccWord.STOP, 1),
  (bd.TATAccWord.AM_ADDRESS, 0),
  (bd.TATAccWord.MM_ADDRESS, 0)])
D.SetMem(CORE, bd.bdpars.BDMemId.TAT0, [TAT_entry], 0)

tags = [bd.PackWord([(bd.InputTag.TAG, 0), (bd.InputTag.COUNT, 1)])] * num_tags_in

time.sleep(.5)

start_time = ns_to_s(D.GetFPGATime()) + .5 # s
end_time = start_time + duration
tag_times = np.array(s_to_ns(np.linspace(start_time, end_time, num_tags_in)))

print("* Sending tags")
D.SendTags(CORE, tags, tag_times)
D.Flush()

time.sleep(duration + 2)

print(D.GetOutputQueueCounts())
tags_out, tags_out_times = D.RecvTags(CORE)
print("* after sending", num_tags_in, "tags, got", len(tags_out), "back")
cts = {}
for tag in tags_out:
  tag_id = bd.GetField(tag, bd.AccOutputTag.TAG)
  ct = bd.GetField(tag, bd.AccOutputTag.COUNT)
  if tag_id not in cts:
    cts[tag_id] = 0
  cts[tag_id] += ct
print(cts)

print("* Done")
D.Stop()
