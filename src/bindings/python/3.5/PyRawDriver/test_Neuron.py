import sys
import time
import PyRawDriver as bd
from PyDriver.pystorm.bddriver import bdpars



driver = bd.Driver(debug=True)
#self.__noout__ = True
#self.__buffered__ = False

driver.InitBD()

#driver.SetSpikeDumpState(1)
#driver.SetSpikeDumpState(1)
##time.sleep(1)
#driver.SetDACValue(bd.HORN.DAC_SOMA_OFFSET, 512)
##time.sleep(1)
#driver.SetDACValue(bd.HORN.DAC_SOMA_OFFSET, 512)
##time.sleep(1)
#driver.SetDACValue(bd.HORN.DAC_SOMA_REF, 8)
##time.sleep(1)
#driver.SetDACValue(bd.HORN.DAC_SOMA_REF, 8)
##time.sleep(1)

driver.EnableSoma(1)

#for _idx in range(4096):
#    driver.EnableSoma(_idx)
#    driver.SetSomaGain(_idx, bdpars.SomaGainId.ONE)
#    driver.SetSomaOffsetSign(_idx, bdpars.SomaOffsetSignId.POSITIVE)
#    driver.SetSomaOffsetMultiplier(_idx, bdpars.SomaOffsetMultiplierId.ZERO)
#
#for _idx in range(1024):
#    driver.DisableSynapse(_idx)
#    driver.DisableSynapseADC(_idx)
#
for _idx in range(256):
    driver.OpenDiffusorAllCuts(_idx)

bd.ByteUtils.PrettyPrintBytearray(driver.__make_byte_array__(driver.BUFFER), grouping=4)

#while(1):
#    print(bd.ByteUtils.PrintBytearrayAs32b(driver.ReceiveWords()))
#    time.sleep(1)