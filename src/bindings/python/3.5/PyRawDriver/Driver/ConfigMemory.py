from PyDriver.pystorm.bddriver import bdpars
from PyDriver.pystorm.bddriver import PackWord
from PyDriver.pystorm.bddriver import NeuronConfig
from . import HORN
from math import floor

__config_mem__ = dict({
    'SOMA'    : bdpars.BDPars.config_soma_mem_,
    'SYNAPSE' : bdpars.BDPars.config_synapse_mem_,
    'DIFF'    : bdpars.BDPars.config_diff_cut_mem_
})

__gain_vals__ = dict({
    bdpars.SomaGainId.ONE_FOURTH: (0, 0),
    bdpars.SomaGainId.ONE_THIRD : (0, 1),
    bdpars.SomaGainId.ONE_HALF  : (1, 0),
    bdpars.SomaGainId.ONE       : (1, 1)
})

__offset_mult_vals__ = dict({
    bdpars.SomaOffsetMultiplierId.ZERO : (0, 0),
    bdpars.SomaOffsetMultiplierId.ONE  : (0, 1),
    bdpars.SomaOffsetMultiplierId.TWO  : (1, 0),
    bdpars.SomaOffsetMultiplierId.THREE: (1, 1)
})

class ConfigMemory(object):
    def __init__(self):
        self.BUFFER = []
        self.__buffered__ = True

    def SendBDWords(self, horn_id, payload_list):
        pass

    def __set_config_memory__(self, elem_type, elem_id, config_type, config_value):
        """
        sets BD's neuron config memory
        :param elem_type: One of: 'SOMA', 'SYN' or 'DIFF'
        :param elem_id: A flat index, [0, 4095] for Soma, [0, 1023] for Synapse and [0, 255]
        :param config_type: One of: 'ConfigSomaID', 'ConfigSynapseID' or 'DiffusorCutLocatinoID'
        :param config_value: Bool
        :return:
        """
        mem = __config_mem__[elem_type]
        mem_sub = mem[config_type]
        num_per_tile = len(mem_sub)
        tile_id = int(floor(elem_id / num_per_tile))
        intra_tile_id = elem_id % num_per_tile
        tile_mem_loc = mem_sub[intra_tile_id]
        row = tile_mem_loc % 8
        column = int(floor(tile_mem_loc / 16))
        bit_select = int(floor((tile_mem_loc % 16) / 8))
        config_word = PackWord([
            (NeuronConfig.ROW_HI, (row >> 1) & 0x03),
            (NeuronConfig.ROW_LO, row & 0x01),
            (NeuronConfig.COL_HI, (column >> 2) & 0x01),
            (NeuronConfig.COL_LO, column & 0x03),
            (NeuronConfig.BIT_SEL, bit_select & 0x01),
            (NeuronConfig.BIT_VAL, config_value),
            (NeuronConfig.TILE_ADDR, tile_id)
        ])
        if self.__buffered__:
            self.BUFFER.append((HORN.NeuronConfig, config_word))
        else:
            self.SendBDWords(HORN.NeuronConfig, (config_word, ))

    def ZeroConfigMemory(self):
        config_words = []
        for tile_id in range(256):
            for tile_mem_loc in range(128):
                row = tile_mem_loc % 8
                column = int(floor(tile_mem_loc / 16))
                bit_select = int(floor((tile_mem_loc % 16) / 8))
                config_words.append(PackWord([
                    (NeuronConfig.ROW_HI, (row >> 1) & 0x03),
                    (NeuronConfig.ROW_LO, row & 0x01),
                    (NeuronConfig.COL_HI, (column >> 2) & 0x01),
                    (NeuronConfig.COL_LO, column & 0x03),
                    (NeuronConfig.BIT_SEL, bit_select & 0x01),
                    (NeuronConfig.BIT_VAL, 0),
                    (NeuronConfig.TILE_ADDR, tile_id)
                ]))
        self.SendBDWords(HORN.NeuronConfig, config_words)

    def SetSomaConfigMemory(self, elem_id, config_type, config_value):
        self.__set_config_memory__('SOMA', elem_id, config_type, config_value)

    def SetSomaEnableStatus(self, elem_id, status):
        if status == bdpars.SomaStatusId.DISABLED:
            bit_val = 0
        else:
            bit_val = 1
        self.SetSomaConfigMemory(elem_id, bdpars.ConfigSomaID.ENABLE, bit_val)

    def EnableSoma(self, elem_id):
        self.SetSomaEnableStatus(elem_id, bdpars.SomaStatusId.ENABLED)

    def DisableSoma(self, elem_id):
        self.SetSomaEnableStatus(elem_id, bdpars.SomaStatusId.DISABLED)

    def SetSomaGain(self, elem_id, gain):
        _gconf = __gain_vals__[gain]
        self.SetSomaConfigMemory(elem_id, bdpars.ConfigSomaID.GAIN_0, _gconf[1])
        self.SetSomaConfigMemory(elem_id, bdpars.ConfigSomaID.GAIN_1, _gconf[0])

    def SetSomaOffsetSign(self, elem_id, sign):
        if sign == bdpars.SomaOffsetSignId.POSITIVE:
            bit_val = 0
        else:
            bit_val = 1
        self.SetSomaConfigMemory(elem_id, bdpars.ConfigSomaID.SUBTRACT_OFFSET, bit_val)

    def SetSomaOffsetMultiplier(self, elem_id, multiplier):
        _oconf = __offset_mult_vals__[multiplier]
        self.SetSomaConfigMemory(elem_id, bdpars.ConfigSomaID.OFFSET_0, _oconf[1])
        self.SetSomaConfigMemory(elem_id, bdpars.ConfigSomaID.OFFSET_1, _oconf[0])

    def SetSynapseConfigMemory(self, elem_id, config_type, config_value):
        self.__set_config_memory__('SYNAPSE', elem_id, config_type, config_value)

    def SetSynapseEnableStatus(self, elem_id, status):
        if status == bdpars.SynapseStatusId.ENABLED:
            bit_val = 0
        else:
            bit_val = 1
        self.SetSynapseConfigMemory(elem_id, bdpars.ConfigSynapseID.SYN_DISABLE, bit_val)

    def EnableSynapse(self, elem_id):
        self.SetSynapseEnableStatus(elem_id, bdpars.SynapseStatusId.ENABLED)

    def DisableSynapse(self, elem_id):
        self.SetSynapseEnableStatus(elem_id, bdpars.SynapseStatusId.DISABLED)

    def SetSynapseADCStatus(self, elem_id, status):
        if status == bdpars.SynapseStatusId.ENABLED:
            bit_val = 0
        else:
            bit_val = 1
        self.SetSynapseConfigMemory(elem_id, bdpars.ConfigSynapseID.ADC_DISABLE, bit_val)

    def EnableSynapseADC(self, elem_id):
        self.SetSynapseADCStatus(elem_id, bdpars.SynapseStatusId.ENABLED)

    def DisableSynapseADC(self, elem_id):
        self.SetSynapseADCStatus(elem_id, bdpars.SynapseStatusId.DISABLED)

    def SetDiffusorCutStatus(self, tile_id, location, status):
        if status == bdpars.DiffusorCutStatusId.CLOSE:
            bit_val = 0
        else:
            bit_val = 1
        self.__set_config_memory__('DIFF', tile_id, location, bit_val)

    def OpenDiffusorCut(self, tile_id, location):
        self.SetDiffusorCutStatus(tile_id, location, bdpars.DiffusorCutStatusId.OPEN)

    def CloseDiffusorCut(self, tile_id, location):
        self.SetDiffusorCutStatus(tile_id, location, bdpars.DiffusorCutStatusId.CLOSE)

    def SetDiffusorAllCutsStatus(self, tile_id, status):
        if status == bdpars.DiffusorCutStatusId.CLOSE:
            bit_val = 0
        else:
            bit_val = 1
        self.__set_config_memory__('DIFF', tile_id, bdpars.DiffusorCutLocationId.NORTH_LEFT, bit_val)
        self.__set_config_memory__('DIFF', tile_id, bdpars.DiffusorCutLocationId.NORTH_RIGHT, bit_val)
        self.__set_config_memory__('DIFF', tile_id, bdpars.DiffusorCutLocationId.WEST_TOP, bit_val)
        self.__set_config_memory__('DIFF', tile_id, bdpars.DiffusorCutLocationId.WEST_BOTTOM, bit_val)

    def OpenDiffusorAllCuts(self, tile_id):
        self.SetDiffusorAllCutsStatus(tile_id, bdpars.DiffusorCutStatusId.OPEN)

    def CloseDiffusorAllCuts(self, tile_id):
        self.SetDiffusorAllCutsStatus(tile_id, bdpars.DiffusorCutStatusId.CLOSE)
