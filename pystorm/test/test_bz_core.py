import numpy as np
from pystorm.PyDriver import bddriver as bd
import time
import sys

D = bd.Driver()

D.SetOKBitFile("../../FPGA/quartus/output_files/OKCoreBD.rbf")

comm_state = D.Start()
if (comm_state < 0):
    print("* Driver failed to start!")
    exit(-1)

# print("* FPGA init")
# D.InitFPGA()
# print(D.GetFPGATime())
# time.sleep(1)
# print(D.GetFPGATime())
# time.sleep(1)
# print(D.GetFPGATime())

print("* Resetting BD")
D.ResetBD()

# set time unit (SF/SG update interval) to .1 ms
time_unit_ns=10000
upstream_hb_ns=10000000
print("* Setting FPGA time units")
D.SetTimeUnitLen(time_unit_ns)
D.SetTimePerUpHB(upstream_hb_ns)

# print("* Disable tag & spike traffic")
# D.SetTagTrafficState(0, False, False)
# D.SetSpikeTrafficState(0, False, False)

# print("* setup memory delays")

# for mem in {bd.bdpars.BDMemId.AM, bd.bdpars.BDMemId.MM, bd.bdpars.BDMemId.FIFO_PG, bd.bdpars.BDMemId.FIFO_DCT, bd.bdpars.BDMemId.TAT0, bd.bdpars.BDMemId.TAT1, bd.bdpars.BDMemId.PAT}:
#   D.SetMemoryDelay(0, mem, 0, 0);

# print("* Init the FIFO")
# D.InitFIFO(0)
print("* waiting")
time.sleep(1)

print("* initalize PAT")
D.SetMem(0 , bd.bdpars.BDMemId.PAT  , D.GetDefaultPATEntries()  , 0);
print("* initalize TAT0")
D.SetMem(0 , bd.bdpars.BDMemId.TAT0  , D.GetDefaultTAT0Entries()  , 0);
print("* initalize TAT1")
D.SetMem(0 , bd.bdpars.BDMemId.TAT1  , D.GetDefaultTAT1Entries()  , 0);
print("* initalize MM")
D.SetMem(0 , bd.bdpars.BDMemId.MM  , D.GetDefaultMMEntries()  , 0);
print("* initalize AM")
D.SetMem(0 , bd.bdpars.BDMemId.AM  , D.GetDefaultAMEntries()  , 0);

# print("* waiting 2")
# time.sleep(1)
# print(D.GetDefaultMMEntries())
# print(D.DumpMem(0, bd.bdpars.BDMemId.MM))

time.sleep(5)

print("* stopping driver")
D.Stop();
print("* done")
