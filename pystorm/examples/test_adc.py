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
toggleCount = 5

HAL.driver.SetADCTrafficState(CORE_ID, False)
time.sleep(3)
HAL.driver.SetADCTrafficState(CORE_ID, True)


def SetDefaultDACValues():
    driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_ADC_BIAS_1  , 512)
    driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_ADC_BIAS_2  , 512)
    driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_EXC     , 512)
    driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_DC      , 544)
    driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_INH     , 512)
    driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_LK      , 10)
    driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_PD      , 100)
    driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SYN_PU      , 1024)
    driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_DIFF_G      , 1024)
    driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_DIFF_R      , 1)
    driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , 1024)
    driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SOMA_REF    , 3)

def DisconnectAllDACtoADCConnections():
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_ADC_BIAS_1  , False, flush=flush)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_ADC_BIAS_2  , False, flush=flush)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_EXC     , False, flush=flush)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_DC      , False, flush=flush)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_INH     , False, flush=flush)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_LK      , False, flush=flush)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_PD      , False, flush=flush)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SYN_PU      , False, flush=flush)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_DIFF_G      , False, flush=flush)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_DIFF_R      , False, flush=flush)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET , False, flush=flush)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, bd.bdpars.BDHornEP.DAC_SOMA_REF    , False, flush=flush)

def toggle_small_large_adc(adc_id, toggleCount, toggleSleepTime):
    HAL.driver.SetADCScale(CORE_ID, adc_id, "large")
    for i in range(toggleCount):
        HAL.driver.SetADCScale(CORE_ID, adc_id, "small")
        print("[INFO] Using ADC %d, Small Current" % adc_id)
        time.sleep(toggleSleepTime)
        HAL.driver.SetADCScale(CORE_ID, adc_id, "large")
        print("[INFO] Using ADC %d, Large Current" % adc_id)
        time.sleep(toggleSleepTime)

toggle_small_large_adc(0, toggleCount, 2)
toggle_small_large_adc(1, toggleCount, 2)

