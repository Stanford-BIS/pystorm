import time
import pickle
from numpy import diff, sort, median, array, zeros, linspace
import numpy as np
import matplotlib
matplotlib.use('Agg')

from pystorm.hal import HAL # HAL is a singleton, importing immediately sets up a HAL and its C Driver
from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.PyDriver import bddriver as bd

CORE_ID = 0

def SetDefaultDACValues():
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_ADC_BIAS_1  , 512)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_ADC_BIAS_2  , 512)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_EXC     , 512)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_DC      , 544)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_INH     , 512)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_LK      , 10)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_PD      , 100)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SYN_PU      , 1024)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_DIFF_G      , 1024)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_DIFF_R      , 1)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , 1024)
    HAL.driver.SetDACCount(CORE_ID , bd.bdpars.BDHornEP.DAC_SOMA_REF    , 3)

def DisconnectAllDACtoADCConnections():
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_ADC_BIAS_1  , False)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_ADC_BIAS_2  , False)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_EXC     , False)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_DC      , False)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_INH     , False)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_LK      , False)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_PD      , False)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_PU      , False)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_DIFF_G      , False)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_DIFF_R      , False)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , False)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SOMA_REF    , False)

def toggle_small_large_adc(adc_id, toggleCount, toggleSleepTime):
    HAL.driver.SetADCScale(CORE_ID, adc_id, "large")
    for i in range(toggleCount):
        HAL.driver.SetADCScale(CORE_ID, adc_id, "small")
        print("[INFO] Using ADC %d, Small Current" % adc_id)
        time.sleep(toggleSleepTime)
        HAL.driver.SetADCScale(CORE_ID, adc_id, "large")
        print("[INFO] Using ADC %d, Large Current" % adc_id)
        time.sleep(toggleSleepTime)

def sweep_DAC_value(DAC_ID):
    for i in range(1024):
        print("{INFO} Setting DAC to %d" % (i+1))
        HAL.driver.SetDACCount(CORE_ID, DAC_ID, i+1)
        time.sleep(0.1)

toggleCount = 2
adc_id = 1

print("[INFO] Starting Experiments")
SetDefaultDACValues()
print("[INFO] Disconnecting all DAC to ADC connections")
DisconnectAllDACtoADCConnections()
#time.sleep(3)
HAL.driver.SetADCTrafficState(CORE_ID, True)
#toggle_small_large_adc(adc_id, toggleCount, 2)
HAL.driver.SetADCScale(CORE_ID, adc_id, "large")

#HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_ADC_BIAS_1  , True)
#HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_ADC_BIAS_2  , True)
#HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_EXC     , True)
#HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_DC      , True)
#HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_INH     , True)
#HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_LK      , True)
#HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_PD      , True)
HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_PU      , True)
#HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_DIFF_G      , True)
#HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_DIFF_R      , True)
#HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , True)
#HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SOMA_REF    , True)

#sweep_DAC_value(bd.bdpars.BDHornEP.DAC_SYN_PU)
DAC_ID = bd.bdpars.BDHornEP.DAC_SYN_PU
tsleep = 5
HAL.driver.SetDACCount(CORE_ID, DAC_ID, 1)
time.sleep(tsleep)
for i in range(10):
    HAL.driver.SetDACCount(CORE_ID, DAC_ID, 512)
    time.sleep(tsleep)
    HAL.driver.SetDACCount(CORE_ID, DAC_ID, 1024)
    time.sleep(tsleep)
HAL.driver.SetADCTrafficState(CORE_ID, False)

