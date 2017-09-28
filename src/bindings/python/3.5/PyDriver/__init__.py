from . _PyDriver import pystorm as pystorm

Driver = pystorm.bddriver.Driver
BDHornEP = pystorm.bddriver.bdpars.BDHornEP
ConfigSomaID = pystorm.bddriver.bdpars.ConfigSomaID
ConfigSynapseID = pystorm.bddriver.bdpars.ConfigSynapseID
SomaStatusId = pystorm.bddriver.bdpars.SomaStatusId
SomaGainId = pystorm.bddriver.bdpars.SomaGainId
SomaOffsetSignId = pystorm.bddriver.bdpars.SomaOffsetSignId
SomaOffsetMultiplierId = pystorm.bddriver.bdpars.SomaOffsetMultiplierId
SynapseStatusId = pystorm.bddriver.bdpars.SynapseStatusId
DiffusorCutStatusId = pystorm.bddriver.bdpars.DiffusorCutStatusId
DiffusorCutLocationId = pystorm.bddriver.bdpars.DiffusorCutLocationId

DAC_LIST = [
    BDHornEP.DAC_DIFF_G,
    BDHornEP.DAC_SYN_INH,
    BDHornEP.DAC_SYN_PU,
    BDHornEP.DAC_DIFF_R,
    BDHornEP.DAC_SOMA_OFFSET,
    BDHornEP.DAC_SYN_LK,
    BDHornEP.DAC_SYN_DC,
    BDHornEP.DAC_SYN_PD,
    BDHornEP.DAC_ADC_BIAS_2,
    BDHornEP.DAC_ADC_BIAS_1,
    BDHornEP.DAC_SOMA_REF,
    BDHornEP.DAC_SYN_EXC,
]
