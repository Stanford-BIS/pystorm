"""
Python version of BDDriver
"""
import time
import PyOK as ok
from . import HORN
#import FUNNEL


class Driver:
    def __init__(self, bit_file="OKBD.rbf", block_size=16 * 4):
        self.EP_DN = 0x80  # OK PipeIn EP num
        self.EP_UP = 0xa0  # OK PipeOut EP num
        self.BIT_FILE = bit_file
        self.BLOCK_SIZE = block_size  # in bytes
        self.NOP_DOWN = "0x80000000"
        self.BD_PREFIX = "0b010000"
        self.__nop_bytes__ = int(self.NOP_DOWN, 0).to_bytes(4, byteorder='little')
        self.dev = None

    def __make_byte_array__(self, code_list):
        from math import ceil

        ok_bytes = [int(_c, 0).to_bytes(4, byteorder='little') for _c in code_list]
        len_code = len(ok_bytes)

        if len_code % self.BLOCK_SIZE == 0:
            ok_out = b''.join(ok_bytes)
        else:
            num_blocks = ceil(len_code / self.BLOCK_SIZE)
            rem_bytes = num_blocks * self.BLOCK_SIZE - len_code * 4
            ok_bytes.extend([self.__nop_bytes__] * int(rem_bytes / 4))
            ok_out = b''.join(ok_bytes)

        assert (len(ok_out) % self.BLOCK_SIZE == 0)
        return ok_out

    def __creat_bd_word__(self, payload):
        return self.BD_PREFIX + "00000" + payload

    def SendWords(self, word_list):
        if isinstance(word_list, str):
            word_list = (word_list, )
        self.dev.WriteToBlockPipeIn(self.EP_DN, self.BLOCK_SIZE, self.__make_byte_array__(word_list))

    def ReceiveWords(self):
        out_buf = bytearray(self.BLOCK_SIZE)
        self.dev.ReadFromPipeOut(self.EP_UP, out_buf)
        return out_buf

    def InitBD(self):
        # Initialize OpalKelly
        self.dev = ok.InitOK(self.BIT_FILE)

        # Send reset
        # pReset, sReset ON
        self.SendWords(("0x20000001", "0x10000001"))
        time.sleep(1.)
        # pReset OFF
        self.SendWords(("0x20000000",))
        time.sleep(0.5)
        # sReset OFF
        self.SendWords(("0x10000000",))

    def SetSpikeDumpState(self, state):
        ok_data = self.__creat_bd_word__(HORN.CreateInputWord(HORN.NeuronDumpToggle, state * 2))
        self.SendWords(ok_data)

    def SetDACValue(self, leaf_id, value):
        ok_data = self.__creat_bd_word__(HORN.CreateInputWord(leaf_id, value))
        self.SendWords(ok_data)

    def EnableSoma(self, _idx):
        pass

    def SetSomaGain(self, _idx, gain_enum):
        pass

    def SetSomaOffsetSign(self, _idx, offset_sign_enum):
        pass

    def SetSomaOffsetMultiplier(self, _idx, offset_mult_enum):
        pass
