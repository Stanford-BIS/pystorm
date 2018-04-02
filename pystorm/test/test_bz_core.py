import numpy as np
from pystorm.PyDriver import bddriver as bd
import time
import sys

D = bd.Driver()

# print("* Done with reset sequence")
# time.sleep(1)

D.SetOKBitFile("../../FPGA/quartus/output_files/BZ_host_core.rbf")
#D.SetOKBitFile("../../FPGA/quartus/output_files/OKCoreBD.rbf")

comm_state = D.Start()
if (comm_state < 0):
    print("* Driver failed to start!")
    exit(-1)

# D.InitBD()

# print(D.GetDefaultAMEntries());
# print(D.DumpMem(0, bd.bdpars.BDMemId.AM))

# print("* FPGA init")
# D.InitFPGA()
time.sleep(1)
print("* Resetting BD")
D.ResetBD()

time.sleep(2)


# set time unit (SF/SG update interval) to .1 ms
time_unit_ns=10000
upstream_hb_ns=100000
print("* Setting FPGA time units")
D.SetTimeUnitLen(time_unit_ns)
D.SetTimePerUpHB(upstream_hb_ns)

time.sleep(3)


print("* Check FPGA time")
old_time = D.GetFPGATime()
for n in range(0, 3):
	time.sleep(1)
	new_time = D.GetFPGATime()
	print(new_time - old_time)
	old_time = new_time

print("* Disable tag & spike traffic")
D.SetTagTrafficState(0, False, False)
D.SetSpikeTrafficState(0, False, False)

# print("* setup memory delays")

# for mem in {bd.bdpars.BDMemId.AM, bd.bdpars.BDMemId.MM, bd.bdpars.BDMemId.FIFO_PG, bd.bdpars.BDMemId.FIFO_DCT, bd.bdpars.BDMemId.TAT0, bd.bdpars.BDMemId.TAT1, bd.bdpars.BDMemId.PAT}:
#   D.SetMemoryDelay(0, mem, 0, 0);

# print("* Init the FIFO")
# D.InitFIFO(0)
# # print("* waiting")
# # time.sleep(1)
# # print(D.DumpMem(0, bd.bdpars.BDMemId.PAT))
# # time.sleep(1)


print("* initalize PAT")
D.SetMem(0 , bd.bdpars.BDMemId.PAT  , D.GetDefaultPATEntries() , 0);
time.sleep(1)
print("PAT:")
dump = D.DumpMem(1, bd.bdpars.BDMemId.PAT);
print(dump)# # time.sleep(1)

print("* initalize TAT0")
D.SetMem(0 , bd.bdpars.BDMemId.TAT0  , range(1024)   , 0);
time.sleep(1)
print("TAT0:")
dump = D.DumpMem(1, bd.bdpars.BDMemId.TAT0);
print(dump)
if (dump != range(1024)):
	print("data bad :(")

print("* initalize AM")
D.SetMem(0 , bd.bdpars.BDMemId.AM  , range(1024)  , 0);

time.sleep(5)

print("* stopping driver")
D.Stop();
print("* done")
