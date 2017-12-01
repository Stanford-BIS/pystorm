import sys
import time
import PyRawDriver as bd
from PyDriver.pystorm.bddriver import bdpars
import pprint

# In debug mode, no data is sent to OK (default = send to OK)
#  This is done by setting driver.__dbg__ = True or during instantiation
driver = bd.Driver(debug=True)

driver.InitBD()

time.sleep(1)
driver.SetDACValue(bd.HORN.DAC_SOMA_OFFSET, 8)
time.sleep(1)
driver.SetDACValue(bd.HORN.DAC_SOMA_OFFSET, 8)
time.sleep(1)
driver.SetDACValue(bd.HORN.DAC_SOMA_REF, 8)
time.sleep(1)
driver.SetDACValue(bd.HORN.DAC_SOMA_REF, 8)
time.sleep(1)

# ConfigMemory calls can be buffered (default = buffered)
# This is done by setting driver.__buffered__ = True
driver.__buffered__ = True

# Disable all Somas
for _idx in range(4096):
    driver.DisableSoma(_idx)
    driver.SetSomaGain(_idx, bdpars.SomaGainId.ONE)
    driver.SetSomaOffsetSign(_idx, bdpars.SomaOffsetSignId.POSITIVE)
    driver.SetSomaOffsetMultiplier(_idx, bdpars.SomaOffsetMultiplierId.THREE)

# Disable all Synapses
for _idx in range(1024):
    driver.DisableSynapse(_idx)
    driver.DisableSynapseADC(_idx)

# Open all Diffusors
for _idx in range(256):
    driver.OpenDiffusorAllCuts(_idx)

# In case of buffering, call this to flush the buffer
driver.FlushBDBuffer()

# Enable spike dump
driver.SetSpikeDumpState(1)
driver.SetSpikeDumpState(1)
time.sleep(1)

# Enable one soma
driver.EnableSoma(127)

while(1):
    print(bd.ByteUtils.PrintBytearrayAs32b(driver.ReceiveWords()))
    time.sleep(1)
