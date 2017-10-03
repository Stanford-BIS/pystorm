import PyOK as ok

import time
import sys

def FormatBits(bitstr):
    #print(bitstr)
    #print(str(len(bitstr)) + " chars")
    #print(int(bitstr, 0))
    #binary_bitstr = "{0:032b}".format(int(bitstr, 0))
    #print(binary_bitstr)
    #print(type(binary_bitstr))
    #print(int(binary_bitstr))
    return int(bitstr, 0).to_bytes(4, byteorder='little')


# pretty print of a bytearray, ignores upstream nops
def PrintBytearrayAs32b(buf_out):
  nop_count = 0
  for idx in range(len(buf_out) // 4):
    this_word_flipped = buf_out[4*idx:4*(idx+1)]
    this_word = this_word_flipped[::-1]
    if (this_word[0] == 64 and this_word[1] == 0 and this_word[2] == 0 and this_word[3] == 0):
      nop_count += 1
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

bitfile = "OKBD.rbf"
block_size = 16

ep_dn = 0x80 # BTPipeIn ep num
ep_up = 0xa0 # PipeOut ep num

# for padding downstream block transmissions
nop_down = "0x80000000" # that's 1 followed by a 31 0s

# pads [codes] with NOPS (flushes block)
def SendWords(dev, codes):
    outputs = []
    for code in codes:
        outputs.append(FormatBits(code))

    while len(outputs) < block_size:
        outputs.append(FormatBits(nop_down))

    buf = bytearray()
    for el in outputs:
        buf.extend(el)

    print("============SENDING=============")
    PrintBytearrayAs32b(buf)
    print("================================")

    written = dev.WriteToBlockPipeIn(ep_dn, block_size, buf)
    assert(written == block_size*4)


if __name__ ==  "__main__":

    dev = ok.InitOK(bitfile)


    # reset sequence
    time.sleep(.5)
    SendWords(dev, ["0b00100000000000000000000000000000"]) # pReset off
    time.sleep(.5)
    SendWords(dev, ["0b00010000000000000000000000000000"]) # sReset off

    print("STARTING")

    ## gives alex a second to hit trigger on scope
    time.sleep(2)

    PAT_read_from_0 = [
            #                     op             
            #                     ||addr||rt  |
            "0b010000" + "00000" + "000000000000000101001",
            "0b010000" + "00000" + "000000000000000101001",
            "0b010000" + "00000" + "000000000000000101001",
            "0b010000" + "00000" + "000000000000000101001"]

    # pat comes back as 10010000000001 route

    PAT_write_to_0 = [
            #                                    ---ser------
            #                                   op     addr       rt  
            "0b010000" + "00000" + "00000000" + "1" + "000000" + "101001",
            #                                    6     0
            #                                    ggggggg
            "0b010000" + "00000" + "00000000" + "0110011"      + "101001",
            #                                   13     7
            #                                    ggiggii         notes: lsb causes b7, b14 to sometimes go high? 
            "0b010000" + "00000" + "00000000" + "1100110"      + "101001",
            #                                    19    14
            #                                     gggggg
            "0b010000" + "00000" + "00000000" + "0001100"      + "101001"]

    TAT0_addr0 = [
            #                         X       addr[5:0]   op     route
            "0b010000" + "00000" + "000000" + "000000" + "00" + "1000001",
            #                                   X    addr[9:6]  
            "0b010000" + "00000" + "000000" + "0000" + "0000" + "1000001",
            "0b010000" + "00000" + "000000" + "00000000"      + "1000001",
            "0b010000" + "00000" + "000000" + "00000000"      + "1000001"]
    
    TAT0_WI = [
            #                         X       data[5:0]   op     route
            "0b010000" + "00000" + "000000" + "110011" + "01" + "1000001",
            #                                 data[13:6]  
            "0b010000" + "00000" + "000000" + "11001100"      + "1000001",
            #                                 data[21:14]  
            "0b010000" + "00000" + "000000" + "11001100"      + "1000001",
            #                                  X    data[28:22]  
            "0b010000" + "00000" + "000000" + "0" + "1001100" + "1000001"]

    TAT0_RI = [
            #                         X          X        op     route
            "0b010000" + "00000" + "000000" + "000000" + "10" + "1000001",
            #                                     X    
            "0b010000" + "00000" + "000000" + "00000000"      + "1000001",
            "0b010000" + "00000" + "000000" + "00000000"      + "1000001",
            "0b010000" + "00000" + "000000" + "00000000"      + "1000001"]

    TAT1_addr0 = [
            #                         X       addr[5:0]   op     route
            "0b010000" + "00000" + "000000" + "000000" + "00" + "1100001",
            #                                   X    addr[9:6]  
            "0b010000" + "00000" + "000000" + "0000" + "0000" + "1100001",
            "0b010000" + "00000" + "000000" + "00000000"      + "1100001",
            "0b010000" + "00000" + "000000" + "00000000"      + "1100001"]
    
    TAT1_WI = [
            #                         X       data[5:0]   op     route
            "0b010000" + "00000" + "000000" + "110011" + "01" + "1100001",
            #                                 data[13:6]  
            "0b010000" + "00000" + "000000" + "11001100"      + "1100001",
            #                                 data[21:14]  
            "0b010000" + "00000" + "000000" + "11001100"      + "1100001",
            #                                  X    data[28:22]  
            "0b010000" + "00000" + "000000" + "0" + "1001100" + "1100001"]

    TAT1_RI = [
            #                         X          X        op     route
            "0b010000" + "00000" + "000000" + "000000" + "10" + "1100001",
            #                                     X    
            "0b010000" + "00000" + "000000" + "00000000"      + "1100001",
            "0b010000" + "00000" + "000000" + "00000000"      + "1100001",
            "0b010000" + "00000" + "000000" + "00000000"      + "1100001"]

    MM_addr0 = [
        #                         X       addr[5:0]   op     route
        "0b010000" + "00000" + "000000" + "000000" + "00" + "1100001",
        #                                   X    addr[9:6]  
        "0b010000" + "00000" + "000000" + "0000" + "0000" + "1100001",
        "0b010000" + "00000" + "000000" + "00000000"      + "1100001",
        "0b010000" + "00000" + "000000" + "00000000"      + "1100001"]
    
    MM_WI = [
        #                         X       data[5:0]   op     route
        "0b010000" + "00000" + "000000" + "110011" + "01" + "1100001",
        #                                 data[13:6]  
        "0b010000" + "00000" + "000000" + "11001100"      + "1100001",
        #                                 data[21:14]  
        "0b010000" + "00000" + "000000" + "11001100"      + "1100001",
        #                                  X    data[28:22]  
        "0b010000" + "00000" + "000000" + "0" + "1001100" + "1100001"]

    MM_RI = [
        #                         X          X        op     route
        "0b010000" + "00000" + "000000" + "000000" + "10" + "1100001",
        #                                     X    
        "0b010000" + "00000" + "000000" + "00000000"      + "1100001",
        "0b010000" + "00000" + "000000" + "00000000"      + "1100001",
        "0b010000" + "00000" + "000000" + "00000000"      + "1100001"]



    #MM_addr0 = [
    #        ['BDHORN', 'PROG_AMMM', '0x000001'],
    #        ['BDHORN', 'PROG_AMMM', '0x0f0000']]

    #MM_WI = [
    #        ['BDHORN', 'PROG_AMMM', '0x000013'],
    #        ['BDHORN', 'PROG_AMMM', '0x000000']]

    #MM_RI = [
    #        ['BDHORN', 'PROG_AMMM', '0x000005'],
    #        ['BDHORN', 'PROG_AMMM', '0x000000']]

    #AM_addr0 = [
    #        ['BDHORN', 'PROG_AMMM', '0x000000'],
    #        ['BDHORN', 'PROG_AMMM', '0x000000']]

    #MM_RW = [
    #        ['BDHORN', 'PROG_AMMM', '0x000002'],
    #        ['BDHORN', 'PROG_AMMM', '0x000000']]


    i = 0
    while(True):

        #key = 'n'
        #while(key is not 'y'):

        SendWords(dev, PAT_write_to_0)
        SendWords(dev, PAT_read_from_0)

        #SendWords(dev, TAT0_addr0 + TAT0_WI)
        #SendWords(dev, TAT0_addr0 + TAT0_RI)

        #SendWords(dev, TAT1_addr0 + TAT1_WI)
        #SendWords(dev, TAT1_addr0 + TAT1_RI)

        out_buf = bytearray(block_size*4)
        dev.ReadFromPipeOut(ep_up, out_buf)
        print("------------RECEIVED------------")
        PrintBytearrayAs32b(out_buf)
        print("--------------------------------")
        print(i)
        print("sleeping")
        time.sleep(1)

        #key = input('press y to continue')

        i += 1



