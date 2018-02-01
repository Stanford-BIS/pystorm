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
print("Setting Core_id to 0")

# Set all the DACs to some default values
# Use this function to determine what the default DAC values should be
def SetDefaultDACValues():
    print("[INFO] Setting Default DAC Values")
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

#Disabling all DAC to ADC connections
def DisconnectAllDACtoADCConnections():
    print("[INFO] Disconnecting all DAC to ADC connections")
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

# Toggle between two current values to see frequency shifting
def toggle_DAC_values(toggleCount, toggleSleepTime, DAC_ID, DACval1, DACval2):
    for i in range(toggleCount):
#    print("Setting DAC Value to %d" % 10**i)
        print("[INFO] Switching to low DAC setting: %d" % DACval1)
        HAL.driver.SetDACCount(CORE_ID, DAC_ID, DACval1)
        time.sleep(toggleSleepTime)
        print("[INFO] Switching to high DAC setting %d" % DACval2)
        HAL.driver.SetDACCount(CORE_ID, DAC_ID, DACval2)
        time.sleep(toggleSleepTime)

# Toggle between two large and small current ADC for a given DAC setting
def toggle_small_large_adc(adc_id, toggleCount, toggleSleepTime):
    HAL.driver.SetADCScale(CORE_ID, adc_id, "large")
    for i in range(toggleCount):
        HAL.driver.SetADCScale(CORE_ID, adc_id, "small")
        print("[INFO] Using ADC %d, Small Current" % adc_id)
        time.sleep(toggleSleepTime)
        HAL.driver.SetADCScale(CORE_ID, adc_id, "large")
        print("[INFO] Using ADC %d, Large Current" % adc_id)
        time.sleep(toggleSleepTime)

# Sweep the given DAC for the full range between 1 and 1024
def sweep_DAC_range(DAC_ID):
    for i in range(1024):
        print("{INFO} Setting DAC to %d" % (i+1))
        HAL.driver.SetDACCount(CORE_ID, DAC_ID, i+1)
        time.sleep(0.1)


# Reset the board to a known reset/refresh state
SetDefaultDACValues()
DisconnectAllDACtoADCConnections()

# Disable all Synapse to ADC connections
print("[INFO] Disconnecting all Synapse to ADC connections")
for addr in range(1024):
    HAL.driver.DisableSynapseADC(CORE_ID, addr)

# Disable all Somas and set them to minimum gain
print("[INFO] Disabling all Somas")
for addr in range(4096):
    HAL.driver.DisableSoma(CORE_ID, addr)
    HAL.driver.SetSomaGain(CORE_ID, addr, bd.bdpars.SomaGainId.ONE)
    HAL.driver.SetSomaOffsetSign(CORE_ID, addr, bd.bdpars.SomaOffsetSignId.POSITIVE)
    HAL.driver.SetSomaOffsetMultiplier(CORE_ID, addr, bd.bdpars.SomaOffsetMultiplierId.ONE)

# Turn on ADCs, assuming they are off
HAL.driver.SetADCTrafficState(CORE_ID, True)
time.sleep(3)

def SetADCsScale(scale):
    if(scale=="large" or scale=="small"):
        print("[INFO] Setting ADC Scale to %s for ADC: 0" % scale)
        HAL.driver.SetADCScale(CORE_ID, 0, scale)
        print("[INFO] Setting ADC Scale to %s for ADC: 1" % scale)
        HAL.driver.SetADCScale(CORE_ID, 1, scale)
    else:
        print("Error: Invalid Scale value; Doing Nothing")

SetADCsScale("large")
#adc_id = 0
#print("[INFO] Setting ADC Scale to large for ADC: %d" % adc_id)
#HAL.driver.SetADCScale(CORE_ID, adc_id, "large")
#adc_id = 1
#print("[INFO] Setting ADC Scale to small for ADC: %d" % adc_id)
#HAL.driver.SetADCScale(CORE_ID, adc_id, "small")

# ADC Biases don't connect to the ADCs, so we shouldn't see changing them affect
#   ADC output at all
#       bd.bdpars.BDHornEP.DAC_ADC_BIAS_1  1pA to 1nA
#       bd.bdpars.BDHornEP.DAC_ADC_BIAS_2  1pA to 1nA

# Approximate current values for each of the 12 DACs to the Neuron Array
# Current values to the ADC should be approximately 1pA and 1nA, subject to mismatch
#       bd.bdpars.BDHornEP.DAC_SYN_EXC     250fA to 250pA
#       bd.bdpars.BDHornEP.DAC_SYN_DC      125fA to 125pA
#       bd.bdpars.BDHornEP.DAC_SYN_INH     8fA to 8pA
#       bd.bdpars.BDHornEP.DAC_SYN_LK      6.25fA to 6.25pA
#       bd.bdpars.BDHornEP.DAC_SYN_PD      1pA to 1nA
#       bd.bdpars.BDHornEP.DAC_SYN_PU      1pA to 1nA
#       bd.bdpars.BDHornEP.DAC_DIFF_G      1pA to 1nA
#       bd.bdpars.BDHornEP.DAC_DIFF_R      1pA to 1nA
#       bd.bdpars.BDHornEP.DAC_SOMA_OFFSET 250fA to 250pA
#       bd.bdpars.BDHornEP.DAC_SOMA_REF    1pA to 1nA

DAC_ID = bd.bdpars.BDHornEP.DAC_SYN_EXC
tsleep = 4

# Enable the one DAC to ADC Connection that we want
def ConnectDACtoADC(DAC_ID):
    DisconnectAllDACtoADCConnections()
    print("Connecting to %s" % DAC_ID)
    HAL.driver.SetDACtoADCConnectionState(CORE_ID, DAC_ID, True)
ConnectDACtoADC(DAC_ID)

# Sweep the DAC through it's full range of 1024 values
#sweep_DAC_range(DAC_ID)

# Toggle between two DAC values
#toggle_DAC_values(5, tsleep, DAC_ID, 10, 100)

# Toggle between two large and small current ADC for a given DAC setting
#adc_id = 0
print("[INFO] Setting DAC Count to 10")
HAL.driver.SetDACCount(CORE_ID, DAC_ID, 10)
#toggle_small_large_adc(adc_id, 7, tsleep):

#HAL.driver.SetADCTrafficState(CORE_ID, False)

