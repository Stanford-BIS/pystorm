"""Programs TAT0[0] to send excitatory spikes to 
all synapses in order. It then sends some tags in.
"""
from pystorm.PyDriver import bddriver as bd
__bd_exists__ = False

try:
    BDDriver
except NameError:
    pass
else:
    if BDDriver.__class__ == bd.Driver:
        __bd_exists__ = True

if __bd_exists__:
    bd_driver = BDDriver
else:
    bd_driver = bd.Driver()

    bd_driver.Start() # starts bd_driver threads

    print("[INFO] Resetting BD")
    bd_driver.ResetBD()

    print("[INFO] Init the FIFO (also turns on traffic)")
    bd_driver.InitFIFO(0)

tag = 0
# this programs tat0[tag] -> every tap point
# then sends tag, hitting every tap point

tat_tap_entries = []

N = 32; # 32x32 array of synapses
for y in range(0, N, 1): 
    for x in range(0, N, 2): # each tap point entry goes to two synapses
        yx0 = y * N + x
        yx1 = y * N + x + 1
        syn_addr_0 = bd_driver.GetSynAERAddr(yx0)
        syn_addr_1 = bd_driver.GetSynAERAddr(yx1)
        sign0 = 0 # NOTE TO BEN: exc=0, inh=1
        sign1 = 0 
        stop = y is N - 1 and x is N - 2 # tells the TAT this is the last entry for the TAG

        # pack TAT entry
        entry = bd.PackWord([
            (bd.TATSpikeWord.STOP, stop),
            (bd.TATSpikeWord.SYNAPSE_ADDRESS_0, syn_addr_0),
            (bd.TATSpikeWord.SYNAPSE_SIGN_0, sign0),
            (bd.TATSpikeWord.SYNAPSE_ADDRESS_1, syn_addr_1),
            (bd.TATSpikeWord.SYNAPSE_SIGN_1, sign1)
            ])

        tat_tap_entries.append(entry)

# the tag we're using is the start addr
print("[INFO] Programming memory")
bd_driver.SetMem(0, bd.bdpars.BDMemId.TAT0, tat_tap_entries, tag)

# set time unit to 10 us
print("[INFO] Setting FPGA time units")
FPGA_unit = 10 # us
bd_driver.SetTimeUnitLen(FPGA_unit)
# set upstream HB time to .1 ms
bd_driver.SetTimePerUpHB(100)

print("[INFO] Reset FPGA clock")
# set FPGA clock to 0 before we start sending timed tags
bd_driver.ResetFPGATime()

# send tags
tag_rate = 100 # Hz
exp_duration = 2 # seconds

# time vector is in FPGA time units, convert
def sec_to_FPGA_unit(sec):
    return sec / (FPGA_unit * 1e-6)

N_tags = exp_duration * tag_rate
times = [int(i * sec_to_FPGA_unit(1/tag_rate)) for i in range(N_tags)]
 
print("[INFO] Sending tags")
bd_driver.SetTagTrafficState(0, True) # have to turn on tags first

bd_driver.SendTags(0, [tag]*N_tags, times, True)

# now you want to call bd_driver.SetSpikeTrafficState, bd_driver.RecvSpikes probably, to measure some effect
