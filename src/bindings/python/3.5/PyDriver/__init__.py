from . _PyDriver import pystorm as _pystorm
from yaml import load, dump
try:
    from yaml import CLoader as Loader, CDumper as Dumper
except ImportError:
    from yaml import Loader, Dumper
bddriver = _pystorm.bddriver

__Driver__ = bddriver.Driver
BDHornEP = bddriver.bdpars.BDHornEP
ConfigSomaID = bddriver.bdpars.ConfigSomaID
ConfigSynapseID = bddriver.bdpars.ConfigSynapseID
SomaStatusId = bddriver.bdpars.SomaStatusId
SomaGainId = bddriver.bdpars.SomaGainId
SomaOffsetSignId = bddriver.bdpars.SomaOffsetSignId
SomaOffsetMultiplierId = bddriver.bdpars.SomaOffsetMultiplierId
SynapseStatusId = bddriver.bdpars.SynapseStatusId
DiffusorCutStatusId = bddriver.bdpars.DiffusorCutStatusId
DiffusorCutLocationId = bddriver.bdpars.DiffusorCutLocationId

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

class Driver(__Driver__):
    """
    Add some extra helper functions to vanilla Driver
    """
    dac_current_range = dict()
    dac_current_values = dict()
    def __init__(self):
        super().__init__()
        for _dacid in DAC_LIST:
            _unit_current = self.GetDACUnitCurrent(_dacid)
            self.dac_current_range[_dacid] = (_unit_current, _unit_current * 1024.)
            self.dac_current_values[_dacid] = (_unit_current * (idx + 1) for idx in range(1024))

    def SaveToYAML(self, file_name, type):
        """
        Save BD state to a YAML file.

        `type` is one of 'DAC', 'Soma', 'Synapse' or 'Diffusor'
        """
        _dfunc = {
            'Soma': self.GetSomaConfigMem,
            'Synapse': self.GetSynapseConfigMem,
            'Diffusor': self.GetDiffusorConfigMem
        }

        with open(file_name, 'w') as FO:
            dump(_dfunc[type](), stream=FO, Dumper=Dumper)

    def LoadFromYAML(self, file_name, type):
        """
        Load BD state from a YAML file

        `type` is one of 'DAC', 'Soma', 'Synapse' or 'Diffusor'
        """
        _config = None
        with open(file_name, 'r') as FI:
            _config = load(FI, Loader=Loader)
        return _config