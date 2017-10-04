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
    elif (this_word[0] == 128 and this_word[1] == 0 and this_word[2] == 0 and this_word[3] == 0):
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

    TAT0_FO_WI = [
            # fanout to tag 3
            #                         X     data[2:0] fo   stop      op     route
            "0b010000" + "00000" + "000000" + "011" + "10" + "1"  + "01" + "1000001",
            "0b010000" + "00000" + "000000" + "00000000"                 + "1000001",
            "0b010000" + "00000" + "000000" + "00000000"                 + "1000001",
            #                                                v give it nonzero GRT so it comes out of RO_TAT
            "0b010000" + "00000" + "000000" + "0" + "000" + "1000"       + "1000001"] 

    TAT0_ACC_WI = [
            # acc to AM addr 0, MM addr 0
            #                         X     data[2:0] acc   stop      op     route
            "0b010000" + "00000" + "000000" + "000" + "00" + "1"  + "01" + "1000001",
            "0b010000" + "00000" + "000000" + "00000000"                 + "1000001",
            "0b010000" + "00000" + "000000" + "00000000"                 + "1000001",
            "0b010000" + "00000" + "000000" + "0" + "0000000"            + "1000001"] 

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
        "0b010000" + "00000" + "00000" + "1000000000"           + "111001"]
    
    MM_WI = [
        # weight = 64
        #                         X       data[7:0]   op    MM     route
        "0b010000" + "00000" + "0000" + "01000000" + "01" + "1" + "111001",
        #                                    X                 
        "0b010000" + "00000" + "00000" + "0000000000"           + "111001",
        "0b010000" + "00000" + "0000" + "00000000000"           + "111001",
        "0b010000" + "00000" + "00000" + "1000000000"           + "111001"]

    MM_RI = [
        #                         X          X        op    MM     route
        "0b010000" + "00000" + "0000" + "00000000" + "10" + "1" + "111001",
        "0b010000" + "00000" + "00000" + "0000000000"           + "111001",
        "0b010000" + "00000" + "0000" + "00000000000"           + "111001",
        "0b010000" + "00000" + "00000" + "1000000000"           + "111001"]

    # AM return route: 100100000000000
    # bug in AM, funnel serializer reads LSBs twice
    def AM_addr(addr):
        bits = "{0:010b}".format(addr)
        bits_lo = bits[0:8]
        bits_hi = bits[8:10]
        return [
        #                         X    addr[7:0]   op    AM      route
        "0b010000" + "00000" + "0000" + bits_lo + "00" + "0"    + "111001",
        #                                    X       addr[9:8] 
        "0b010000" + "00000" + "00000" + "00000000" + bits_hi   + "111001",
        "0b010000" + "00000" + "0000" + "00000000000"           + "111001",
        #                                stop
        "0b010000" + "00000" + "00000" + "1" + "000000000"      + "111001"]
    
    AM_RW1 = [
        #                         X       data[7:0]   op    AM     route
        "0b010000" + "00000" + "0000" + "11111111" + "01" + "0" + "111001",
        #                                 data[17:8] 
        "0b010000" + "00000" + "00000" + "1111111111"           + "111001",
        #                                 data[28:18] 
        "0b010000" + "00000" + "0000" + "11111111111"           + "111001",
        #                               stop    data[37:29] 
        "0b010000" + "00000" + "00000" + "1" + "111111111"      + "111001"]

    AM_RW2 = [
        # threshold = 128
        #                         X       val[7:0]   op    AM     route
        "0b010000" + "00000" + "0000" + "00000000" + "01" + "0" + "111001",
        #                                 thr    val[14:8] 
        "0b010000" + "00000" + "00000" + "001" + "0000000"           + "111001",
        #                                   na         AM stop  
        "0b010000" + "00000" + "0000" + "1111111111" + "1"           + "111001",
        #                           AMMM stop       na 
        "0b010000" + "00000" + "11111" + "1" + "111111111"           + "111001"]


    AM_inc = [
        #                         X          X        op    AM     route
        "0b010000" + "00000" + "0000" + "00000000" + "10" + "0" + "111001",
        #                                    X               
        "0b010000" + "00000" + "00000" + "0000000000"           + "111001",
        "0b010000" + "00000" + "0000" + "00000000000"           + "111001",
        #                               stop     
        "0b010000" + "00000" + "00000" + "1" + "000000000"      + "111001"]

    # going to make these functions from now on
    BD_header = "0b010000" + "00000"
    BD_header_len = len(BD_header) - 2

    def TOGGLE_FIFO(fifo_id, traffic, dump):
        if fifo_id == "pre":
            route = "10010001"
        elif fifo_id == "post0":
            route = "01010001"
        elif fifo_id == "post1":
            route = "11010001"
        in_word = BD_header + "0"*(32-BD_header_len-len(route)-2)  + str(dump) + str(traffic) + route
        return [in_word]

    def TAG(tag, ct=1):
        return [BD_header + "{0:011b}".format(tag) + "{0:09b}".format(ct) + "0"]

    i = 0
    while(i < 4):

        #key = 'n'
        #while(key is not 'y'):

        #######################################
        ## test 1: program pat
        #SendWords(dev, PAT_write_to_0)
        #SendWords(dev, PAT_read_from_0)

        #######################################
        ## test 2: program tat0
        #SendWords(dev, TAT0_addr0 + TAT0_WI)
        #SendWords(dev, TAT0_addr0 + TAT0_RI)

        #######################################
        ## test 3: program tat1
        #SendWords(dev, TAT1_addr0 + TAT1_WI)
        #SendWords(dev, TAT1_addr0 + TAT1_RI)

        #######################################
        ## test 4: program mm
        #SendWords(dev, MM_addr0 + MM_WI)
        #SendWords(dev, MM_addr0 + MM_RI)

        #######################################
        ## test 5: program am
        # XXX this is a little weird, think the serializer is broken
        #if (i % 2 == 0):
        #    print('writing 1111s')
        #    SendWords(dev, AM_addr(0) + AM_RW1)
        #else:
        #    print('writing 1000s')
        #    SendWords(dev, AM_addr(0) + AM_RW2)

        #######################################
        ## test 6: test RI -> PRE_FIFO_DUMP
        ## tag inputs should show up, but not actually go into FIFO
        #SendWords(dev, TOGGLE_FIFO("pre", 0, 1))
        #SendWords(dev, TAG(3)) # tag id 3, ct 1

        #######################################
        ## test 7: test RI -> fifo -> POST_FIFO_DUMP0
        ## turn on fifo input, dump fifo output0
        #SendWords(dev, TOGGLE_FIFO("pre", 1, 0))
        #SendWords(dev, TOGGLE_FIFO("post0", 0, 1))
        #SendWords(dev, TAG(3)) # tag id 3, ct 1

        #######################################
        ## test 8: test RI -> fifo -> POST_FIFO_DUMP1
        ## turn on fifo input, dump fifo output1
        #SendWords(dev, TOGGLE_FIFO("pre", 1, 0))
        #SendWords(dev, TOGGLE_FIFO("post1", 0, 1))
        #SendWords(dev, TAG(1025)) 

        ########################################
        ## test 9: test RI -> fifo -> tat0 fanout -> RO_TAT
        ## turn on fifo input, dump fifo output0
        ## program TAT0
        #SendWords(dev, TAT0_addr0 + TAT0_FO_WI)
        ##SendWords(dev, TAT0_addr0 + TAT0_RI) 

        ## drain FIFO tag 0
        #SendWords(dev, TOGGLE_FIFO("pre", 1, 0))
        #SendWords(dev, TOGGLE_FIFO("post0", 0, 0))
        #SendWords(dev, TOGGLE_FIFO("post1", 0, 0))
        #SendWords(dev, TAG(0)) 

        #time.sleep(.5)

        ## turn fifo post0 traffic on
        #SendWords(dev, TOGGLE_FIFO("post0", 1, 0))
        ## send a tag, should go through TAT and come out with tag 3, grt MSB=1
        #SendWords(dev, TAG(0))

        #######################################
        # test 10: test RI -> fifo -> tat0 acc -> RO_ACC
        
        # drain FIFO tag 0
        SendWords(dev, TOGGLE_FIFO("pre", 1, 0))
        SendWords(dev, TOGGLE_FIFO("post0", 0, 0))
        SendWords(dev, TOGGLE_FIFO("post1", 0, 0))
        SendWords(dev, TAG(0)) 

        # program TAT0
        SendWords(dev, TAT0_addr0 + TAT0_ACC_WI)
        #SendWords(dev, TAT0_addr0 + TAT0_RI) 

        # program MM with weight of 64
        SendWords(dev, MM_addr0 + MM_WI)
        #SendWords(dev, MM_addr0 + MM_RI)

        # program ACC with threshold of 128
        SendWords(dev, AM_addr(0) + AM_RW2)

        SendWords(dev, TOGGLE_FIFO("post0", 1, 0))
        print('STARTING RUN')
        for j in range(10):
            # get the memory programming read and one output!
            SendWords(dev, TAG(0)*2)

            out_buf = bytearray(block_size*4)
            dev.ReadFromPipeOut(ep_up, out_buf)
            print("------------RECEIVED------------")
            PrintBytearrayAs32b(out_buf)
            print("--------------------------------")
            print(i)
            time.sleep(1)
            i += 1

        #######################################
        # common read

        out_buf = bytearray(block_size*4)
        dev.ReadFromPipeOut(ep_up, out_buf)
        print("------------RECEIVED------------")
        PrintBytearrayAs32b(out_buf)
        print("--------------------------------")
        print(i)
        time.sleep(1)
        i += 1

        time.sleep(1)

        #key = input('press y to continue')

        i += 1
