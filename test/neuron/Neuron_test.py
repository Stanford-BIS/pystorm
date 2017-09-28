import pprint
import PyDriver.pystorm.bddriver as bd
from PyDriver.pystorm.bddriver.bdpars import BDHornEP as HornID
from PyDriver.pystorm.bddriver.bdpars import SomaStatusId as SomaStatusID
from PyDriver.pystorm.bddriver.bdpars import SomaGainId as SomaGainID
from PyDriver.pystorm.bddriver.bdpars import SomaOffsetSignId as SomaOffsetSignID
from PyDriver.pystorm.bddriver.bdpars import SomaOffsetMultiplierId as SomaOffsetMultiplierID
from PyDriver.pystorm.bddriver.bdpars import SynapseStatusId as SynapseStatusID
from PyDriver.pystorm.bddriver.bdpars import DiffusorCutStatusId as DiffusorCutStatusID
from PyDriver.pystorm.bddriver.bdpars import DiffusorCutLocationId as DiffusorCutLocationID

driver = bd.Driver()
dac_ids = [
  HornID.DAC_DIFF_G,
  HornID.DAC_SYN_INH,
  HornID.DAC_SYN_PU,
  HornID.DAC_DIFF_R,
  HornID.DAC_SOMA_OFFSET,
  HornID.DAC_SYN_LK,
  HornID.DAC_SYN_DC,
  HornID.DAC_SYN_PD,
  HornID.DAC_ADC_BIAS_2,
  HornID.DAC_ADC_BIAS_1,
  HornID.DAC_SOMA_REF,
  HornID.DAC_SYN_EXC,
]

for _dac in dac_ids:
    _default_count = driver.GetDACDefaultCount(_dac);
    print("Setting %s to %g" % (_dac, _default_count))
    driver.SetDACValue(0, _dac, _default_count);

# 4096 Somas
for _idx in range(4096):
    driver.DisableSoma(0, _idx)
    driver.SetSomaGain(0, _idx, SomaGainID.ONE)
    driver.SetSomaOffsetSign(0, _idx, SomaOffsetSignID.POSITIVE)
    driver.SetSomaOffsetMultiplier(0, _idx, SomaOffsetMultiplierID.ZERO)

# Enable just one
driver.EnableSoma(0, 1023)

# 1024 Synapses
for _idx in range(1024):
    driver.DisableSynapse(0, _idx)
    driver.DisableSynapseADC(0, _idx)

# Enable just one
driver.EnableSynapse(0, 255)

# 256 tiles. Open all diffusor cuts
for _idx in range(0, 256):
    driver.OpenDiffusorAllCuts(0, _idx)

# Check state
state = driver.GetState(0)
soma_mem = state.GetSomaConfigMem()
syn_mem = state.GetSynapseConfigMem()
diff_mem = state.GetDiffusorConfigMem()

print("Tile 63:")
print("Soma Config:")
pprint.pprint(soma_mem[63])
print("Synapse Config:")
pprint.pprint(syn_mem[63])
print("Diffusor Config:")
pprint.pprint(diff_mem[63])
