# BD driver is available as `driver`
import time
from pystorm.PyDriver import bddriver as bd

try:
    driver
except NameError:
    driver = bd.Driver()
else:
    pass

CORE = 0
downstream_time_res = 10 * 1000 # ns
upstream_time_res = downstream_time_res * 10 # ns

print("[INFO] Starting BD")
driver.Start() # starts driver threads
print("[INFO] Resetting BD")
driver.ResetBD()
print("[INFO] Setting downstream time resolution to [color=ffffff]%d[/color]ns" % downstream_time_res)
driver.SetTimeUnitLen(downstream_time_res)
print("[INFO] Setting upstream time resolution to [color=ffffff]%d[/color]ns" % upstream_time_res)
driver.SetTimePerUpHB(upstream_time_res)
print("[INFO] Init the FIFO (also turns on traffic)")
driver.InitFIFO(0)
print("[INFO] Enable tag traffic")
driver.SetTagTrafficState(CORE, True, True)

time.sleep(2)

print("[INFO] Initializing DAC")
driver.InitDAC(CORE)

## magical DAC settings (DC is the most important, with the default, inhibition doesn't work)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_EXC     , 512)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_DC      , 544)  # Alex says 900 required for balancing
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_INH     , 512)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_LK      , 10)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_PD      , 10)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_PU      , 10)  # Alex uses 1023
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_DIFF_G      , 1024)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_DIFF_R      , 1)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , 1)
driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SOMA_REF    , 10)

for i in range(4096):
    driver.DisableSoma(CORE, i)
    driver.SetSomaGain(CORE, i, bd.bdpars.SomaGainId.ONE)
    driver.SetSomaOffsetSign(CORE, i, bd.bdpars.SomaOffsetSignId.NEGATIVE)
    driver.SetSomaOffsetMultiplier(CORE, i, bd.bdpars.SomaOffsetMultiplierId.ZERO)

for i in range(1024):
    driver.DisableSynapse(CORE, i)
    driver.DisableSynapseADC(CORE, i)

for i in range(256):
    driver.OpenDiffusorAllCuts(CORE, i)

driver.SetSpikeDumpState(CORE, True, True)
driver.SetSpikeDumpState(CORE, True, True)
driver.SetSpikeDumpState(CORE, True, True)

driver.Flush()

print("[INFO] BDDriver available using variable '[color=ffffff]driver[/color]'")
