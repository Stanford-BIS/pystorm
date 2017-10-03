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

    # PAT return route: 10010000000001
    PAT_read_from_0 = [
            #                     op             
            #                     ||addr||rt  |
            "0b010000" + "00000" + "000000000000000101001",
            "0b010000" + "00000" + "000000000000000101001",
            "0b010000" + "00000" + "000000000000000101001",
            "0b010000" + "00000" + "000000000000000101001"]

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

    # TAT0 return route: 10000
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

    # TAT1 return route: 10001
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

    # MM return route: 100100000000001
    MM_addr0 = [
        #                         X       addr[7:0]   op    MM     route
        "0b010000" + "00000" + "0000" + "00000000" + "00" + "1" + "111001",
        #                                X      addr[15:8] 
        "0b010000" + "00000" + "00000" + "00" + "00000000"      + "111001",
        "0b010000" + "00000" + "0000" + "00000000000"           + "111001",
        "0b010000" + "00000" + "00000" + "0000000000"           + "111001"]
    
    MM_WI = [
        #                         X       data[7:0]   op    MM     route
        "0b010000" + "00000" + "0000" + "00110011" + "01" + "1" + "111001",
        #                                    X                 
        "0b010000" + "00000" + "00000" + "0000000000"           + "111001",
        "0b010000" + "00000" + "0000" + "00000000000"           + "111001",
        "0b010000" + "00000" + "00000" + "0000000000"           + "111001"]

    MM_RI = [
        #                         X          X        op    MM     route
        "0b010000" + "00000" + "0000" + "00000000" + "10" + "1" + "111001",
        "0b010000" + "00000" + "00000" + "0000000000"           + "111001",
        "0b010000" + "00000" + "0000" + "00000000000"           + "111001",
        "0b010000" + "00000" + "00000" + "0000000000"           + "111001"]

    # AM return route: 100100000000000
    # bug in AM, funnel serializer reads LSBs twice
    def AM_addr(addr):
        bits = "{0:010b}".format(addr)
        bits_lo = bits[0:8]
        bits_hi = bits[8:10]
        return [
        #                         X    addr[7:0]   op    AM     route
        "0b010000" + "00000" + "0000" + bits_lo + "00" + "0" + "111001",
        #                                    X       addr[9:8] 
        "0b010000" + "00000" + "00000" + "00000000" + bits_hi   + "111001",
        "0b010000" + "00000" + "0000" + "00000000000"           + "111001",
        "0b010000" + "00000" + "00000" + "0000000000"           + "111001"]
    
    AM_RW1 = [
        #                         X       data[7:0]   op    AM     route
        "0b010000" + "00000" + "0000" + "11111111" + "01" + "0" + "111001",
        #                                 data[17:8] 
        "0b010000" + "00000" + "00000" + "1111111111"           + "111001",
        #                                 data[28:18] 
        "0b010000" + "00000" + "0000" + "11111111111"           + "111001",
        #                               stop    data[37:29] 
        "0b010000" + "00000" + "00000" + "0" + "111111111"      + "111001"]

    AM_RW2 = [
        #                         X       data[7:0]   op    AM     route
        "0b010000" + "00000" + "0000" + "10001000" + "01" + "0" + "111001",
        #                                 data[17:8] 
        "0b010000" + "00000" + "00000" + "0010001000"           + "111001",
        #                                 data[28:18] 
        "0b010000" + "00000" + "0000" + "01000100010"           + "111001",
        #                               stop    data[37:29] 
        "0b010000" + "00000" + "00000" + "0" + "001000100"      + "111001"]


    AM_inc = [
        #                         X          X        op    AM     route
        "0b010000" + "00000" + "0000" + "00000000" + "10" + "0" + "111001",
        #                                    X               
        "0b010000" + "00000" + "00000" + "0000000000"           + "111001",
        "0b010000" + "00000" + "0000" + "00000000000"           + "111001",
        "0b010000" + "00000" + "00000" + "0000000000"           + "111001"]
    

    i = 0
    while(True):

        #key = 'n'
        #while(key is not 'y'):

        #SendWords(dev, PAT_write_to_0)
        #SendWords(dev, PAT_read_from_0)

        #SendWords(dev, TAT0_addr0 + TAT0_WI)
        #SendWords(dev, TAT0_addr0 + TAT0_RI)

        #SendWords(dev, TAT1_addr0 + TAT1_WI)
        #SendWords(dev, TAT1_addr0 + TAT1_RI)

        #SendWords(dev, MM_addr0 + MM_WI)
        #SendWords(dev, MM_addr0 + MM_RI)

        if (i % 2 == 0):
            print('writing 1111s')
            SendWords(dev, AM_addr(0) + AM_RW1)
        else:
            print('writing 1000s')
            SendWords(dev, AM_addr(0) + AM_RW2)

        time.sleep(.1)

        out_buf = bytearray(block_size*4)
        dev.ReadFromPipeOut(ep_up, out_buf)
        print("------------RECEIVED------------")
        PrintBytearrayAs32b(out_buf)
        print("--------------------------------")
        print(i)
        print("sleeping")
        time.sleep(1)

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



