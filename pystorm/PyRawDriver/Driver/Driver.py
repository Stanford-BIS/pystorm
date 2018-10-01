"""
Python version of BDDriver
"""
import time
import pystorm.PyOK as ok
from . import HORN
from .ConfigMemory import ConfigMemory
from ..Utils import ByteUtils
#import FUNNEL

import logging

logger = logging.getLogger(__name__)

__dac_list__ = {HORN.DAC_DIFF_G, HORN.DAC_SYN_INH, HORN.DAC_SYN_PU, HORN.DAC_UNUSED, HORN.DAC_DIFF_R,
                HORN.DAC_SOMA_OFFSET, HORN.DAC_SYN_LK, HORN.DAC_SYN_DC, HORN.DAC_SYN_PD, HORN.DAC_ADC_2, HORN.DAC_ADC_1,
                HORN.DAC_SOMA_REF, HORN.DAC_SYN_EXC}


class Driver(ConfigMemory):
    def __init__(self, bit_file="OKBD.rbf", block_size=16 * 4, debug=False):
        super().__init__()

        self.EP_DN = 0x80  # OK PipeIn EP num
        self.EP_UP = 0xa0  # OK PipeOut EP num
        self.BIT_FILE = bit_file
        self.BLOCK_SIZE = block_size  # in bytes
        self.NOP_DOWN = "0x80000000"
        self.BD_PREFIX = "0b010000"
        self.__nop_bytes__ = int(self.NOP_DOWN, 0).to_bytes(4, byteorder='little')
        self.dev = None
        self.__dbg__ = debug
        self.BUFFER = []
        self.__buffered__ = True

    def __make_byte_array__(self, code_list):
        from math import ceil, floor

        if isinstance(code_list[0], int):
            ok_bytes = [int(_c).to_bytes(4, byteorder='little') for _c in code_list]
        else:
            ok_bytes = [int(_c, 0).to_bytes(4, byteorder='little') for _c in code_list]
        len_code = len(ok_bytes)

        if len_code % self.BLOCK_SIZE == 0:
            ok_out = b''.join(ok_bytes)
        else:
            num_blocks = ceil(len_code * 4 / self.BLOCK_SIZE)
            rem_bytes = num_blocks * self.BLOCK_SIZE - len_code * 4
            ok_bytes.extend([self.__nop_bytes__] * int(floor(rem_bytes / 4)))
            ok_out = b''.join(ok_bytes)

        assert (len(ok_out) % self.BLOCK_SIZE == 0)
        return ok_out

    def __create_bd_word__(self, payload):
        return self.BD_PREFIX + "00000" + payload

    def SendOKWords(self, word_list):
        if isinstance(word_list, str):
            word_list = (word_list, )
        out_bytes = self.__make_byte_array__(word_list)
        if self.__dbg__:
            logger.debug(ByteUtils.PrettyPrintBytearray(out_bytes, grouping=4, downstream=True))
        else:
            ret_code = self.dev.WriteToBlockPipeIn(self.EP_DN, self.BLOCK_SIZE, out_bytes)
            if ret_code < 0:
                logger.critical("OK Write Failure - '%s'" % ok.ErrorNames[ret_code])
            #else:
            #    logger.info("OK Write successful [%d bytes]" % ret_code)

    def SendBDWords(self, horn_id, payload_list):
        bd_data = [self.__create_bd_word__(HORN.CreateInputWord(horn_id, _buf)) for _buf in payload_list]
        self.SendOKWords(bd_data)
        
    def BufferBDWord(self, horn_id, payload):
        self.BUFFER.append((horn_id, payload))

    def FlushBDBuffer(self):
        if len(self.BUFFER) > 0:
            bd_data = [self.__create_bd_word__(HORN.CreateInputWord(_v[0], _v[1])) for _v in self.BUFFER]
            self.SendOKWords(bd_data)
        self.BUFFER = []

    def ReceiveWords(self):
        out_buf = bytearray(self.BLOCK_SIZE)
        if self.__dbg__:
            logger.info("Reading from BD disabled")
        else:
            self.dev.ReadFromPipeOut(self.EP_UP, out_buf)
        return out_buf

    def InitBD(self):
        # Initialize OpalKelly
        self.dev = ok.InitOK(self.BIT_FILE)

        # Send reset
        # pReset, sReset ON
        self.SendOKWords(("0x20000001", "0x10000001"))
        time.sleep(1.)
        # pReset OFF
        self.SendOKWords(("0x20000000",))
        time.sleep(0.5)
        # sReset OFF
        self.SendOKWords(("0x10000000",))

    def SetSpikeDumpState(self, state):
        self.SendBDWords(HORN.NeuronDumpToggle, (state * 2, ))

    def SetDACValue(self, leaf_id, value):
        assert leaf_id in __dac_list__
        assert value > 0 and value <= 1024
        self.SendBDWords(leaf_id, (value - 1, ))
