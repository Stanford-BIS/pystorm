import PyOK as ok

from enum import Enum
class OP():

    BDHORN          = '00'
    FPGA_REG        = '10'
    FPGA_CHANNEL    = '11'

    BD_LEAF = Enum('BDLeafId',[
        'ADC',
        'DAC0',
        'DAC10',
        'DAC11',
        'DAC12',
        'DAC1',
        'DAC2',
        'DAC3',
        'DAC4',
        'DAC5',
        'DAC6',
        'DAC7',
        'DAC8',
        'DAC9',
        'DELAY0',
        'DELAY1',
        'DELAY2',
        'DELAY3',
        'DELAY4',
        'DELAY5',
        'DELAY6',
        'INIT_FIFO_DCT',
        'INIT_FIFO_HT',
        'NEURONCONFIG',
        'NEURONDUMPTOGGLE',
        'NEURONINJECT',
        'PROG_AMMM',
        'PROG_PAT',
        'PROG_TAT0',
        'PROG_TAT1',
        'RI',
        'TOGGLE_POST_FIFO0',
        'TOGGLE_POST_FIFO1',
        'TOGGLE_PRE_FIFO',
        'INVALID'
        ], start=0)

    def endpointword(opcode, subcode, payload):
        _op1 = getattr(OP, opcode)
        _op2 = None
        _val = "{0:024b}".format(int(payload, 0))
        if opcode == 'BDHORN':
            _op2 = "{0:06b}".format(OP.BD_LEAF[subcode].value)
        else:
            _op2 = "{0:06b}".format(int(subcode, 0))
        return int('0b' + _op1 + _op2 + _val, 0).to_bytes(4, byteorder='little')

# there's a better way to do this in python3, like the above
def ToBytes(s):
  v = int(s, 2)
  b = bytearray()
  while v:
    b.append(v & 0xff)
    v >>= 8
  return b[::-1]

nop_up_code = 64
nop_down_code = 128 + 33
HB_up_code = 13

import sys

# pretty print of a bytearray, ignores upstream nops
def PrintBytearrayAs32b(buf_out, ignore_nop=True, ignore_HBs=True):
  nop_count = 0
  HB_count = 0
  for idx in range(len(buf_out) // 4):
    this_word_flipped = buf_out[4*idx:4*(idx+1)]
    this_word = this_word_flipped[::-1]
    if ((this_word[0] == nop_up_code or this_word[0] == nop_down_code) and ignore_nop):
      nop_count += 1
    elif (this_word[0] == HB_up_code and ignore_HBs):
      HB_count += 1
    else:
      for j in range(4):
        to_print = ""
        elcopy = this_word[j]
        for i in range(8):
          to_print += str(elcopy % 2)
          elcopy = elcopy >> 1
        to_print += " "
        sys.stdout.write(str(to_print[::-1]))
      sys.stdout.write('\n')
  print("plus " + str(nop_count) + " NOPs")
  print("plus " + str(HB_count) + " HBs")



bitfile = "OKCoreBD.rbf"
block_size = 128

ep_dn = 0x80 # BTPipeIn ep num
ep_up = 0xa0 # PipeOut ep num

# for padding downstream block transmissions
nop_down = ['FPGA_REG', '33', '0']

# pads [codes] with NOPS (flushes block)
def SendWords(dev, codes):
    outputs = []
    for code in codes:
        outputs.append(OP.endpointword(*code))

    while len(outputs) < block_size:
        outputs.append(OP.endpointword(*nop_down))

    buf = bytearray()
    for el in outputs:
        buf.extend(el)

    print("============SENDING=============")
    PrintBytearrayAs32b(buf, ignore_HBs=False)
    print("================================")

    print(dev.WriteToBlockPipeIn(ep_dn, block_size, buf))

import time

if __name__ ==  "__main__":

    dev = ok.InitOK(bitfile)

    time.sleep(.1)

    # reset sequence
    BD_Reset0 = [['FPGA_REG', '31', '3']]
    BD_Reset1 = [['FPGA_REG', '31', '2']]
    BD_Reset2 = [['FPGA_REG', '31', '0']]

    # run through reset cycle, with some delays beween each phase
    SendWords(dev, BD_Reset0)
    time.sleep(.5)
    SendWords(dev, BD_Reset1)
    time.sleep(.5)
    SendWords(dev, BD_Reset2)
    time.sleep(.5)

    print("STARTING")

    ## gives alex a second to hit trigger on scope
    time.sleep(2)

    ## reset DACs (should already be reset)
    #DACs = [
    #    ['BDHORN', 'DAC0', '0x000'],
    #    ['BDHORN', 'DAC1', '0x000'],
    #    ['BDHORN', 'DAC2', '0x000'],
    #    ['BDHORN', 'DAC3', '0x000'],
    #    ['BDHORN', 'DAC4', '0x000'],
    #    ['BDHORN', 'DAC5', '0x000'],
    #    ['BDHORN', 'DAC6', '0x000'],
    #    ['BDHORN', 'DAC7', '0x000'],
    #    ['BDHORN', 'DAC8', '0x000'],
    #    ['BDHORN', 'DAC9', '0x000'],
    #    ['BDHORN', 'DAC10', '0x000'],
    #    ['BDHORN', 'DAC11', '0x000']]
    #SendWords(dev, DACs)

    ## send lots of DAC words
    ## some DACs seem to take a really long time to ack
    #for i in range(12):
    #  DACs = [['BDHORN', 'DAC' + str(i), '0x000']]*100
    #  SendWords(dev, DACs)
    #  time.sleep(5)

    ## turns on the neurons!
    #SendWords(dev, [['BDHORN', 'NEURONDUMPTOGGLE', '0x002']])

    # read from PAT address 0
    # '0 will do this
    # has a serializer, need to send four words
    # should get something out of DUMP_PAT

    PAT_all_0 = [['BDHORN', 'PROG_PAT', '0x000']]*4
    SendWords(dev, PAT_all_0)

    #BD_leaves = [
    #    'ADC',
    #    'DAC0',
    #    'DAC10',
    #    'DAC11',
    #    'DAC12',
    #    'DAC1',
    #    'DAC2',
    #    'DAC3',
    #    'DAC4',
    #    'DAC5',
    #    'DAC6',
    #    'DAC7',
    #    'DAC8',
    #    'DAC9',
    #    'DELAY0',
    #    'DELAY1',
    #    'DELAY2',
    #    'DELAY3',
    #    'DELAY4',
    #    'DELAY5',
    #    'DELAY6',
    #    'INIT_FIFO_DCT',
    #    'INIT_FIFO_HT',
    #    'NEURONCONFIG',
    #    'NEURONDUMPTOGGLE',
    #    'NEURONINJECT',
    #    'PROG_AMMM',
    #    'PROG_PAT',
    #    'PROG_TAT0',
    #    'PROG_TAT1',
    #    'RI',
    #    'TOGGLE_POST_FIFO0',
    #    'TOGGLE_POST_FIFO1',
    #    'TOGGLE_PRE_FIFO']
    #for leaf in BD_leaves:
    #    SendWords(dev, [['BDHORN', leaf, '0x000']]*12)

    # read upstream traffic
    i = 0
    while(True):
        out_buf = bytearray(block_size*4)
        dev.ReadFromPipeOut(ep_up, out_buf)
        print("------------RECEIVED------------")
        PrintBytearrayAs32b(out_buf)
        print("--------------------------------")
        print(i)
        i += 1
        time.sleep(.5)


