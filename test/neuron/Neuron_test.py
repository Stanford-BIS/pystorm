import pprint
import PyDriver as bd

driver = bd.Driver()
dac_ids = [
  bd.BDHornEP.DAC_DIFF_G,
  bd.BDHornEP.DAC_SYN_INH,
  bd.BDHornEP.DAC_SYN_PU,
  bd.BDHornEP.DAC_DIFF_R,
  bd.BDHornEP.DAC_SOMA_OFFSET,
  bd.BDHornEP.DAC_SYN_LK,
  bd.BDHornEP.DAC_SYN_DC,
  bd.BDHornEP.DAC_SYN_PD,
  bd.BDHornEP.DAC_ADC_BIAS_2,
  bd.BDHornEP.DAC_ADC_BIAS_1,
  bd.BDHornEP.DAC_SOMA_REF,
  bd.BDHornEP.DAC_SYN_EXC,
]

for _dac in dac_ids:
    _default_count = driver.GetDACDefaultCount(_dac);
    print("Setting %s to %g" % (_dac, _default_count))
    driver.SetDACValue(0, _dac, _default_count);

# 4096 Somas
for _idx in range(4096):
    driver.DisableSoma(0, _idx)
    driver.SetSomaGain(0, _idx, bd.SomaGainId.ONE)
    driver.SetSomaOffsetSign(0, _idx, bd.SomaOffsetSignId.POSITIVE)
    driver.SetSomaOffsetMultiplier(0, _idx, bd.SomaOffsetMultiplierId.ZERO)

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
