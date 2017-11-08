"""Memory write-read loopback test
Programs all entries of the PAT
Reads those entries back and checks equivalence
"""
from pystorm.PyDriver import bddriver as bd
import numpy as np
import os
import time

CORE = 0

def TestMem(D, mem, pattern="alternate", delay=0):

    D.SetMemoryDelay(CORE, mem, delay, delay)

    mem_size = D.GetBDPars().mem_info_[mem].size

    if (mem == bd.bdpars.BDMemId.PAT):
        name = "PAT"
        word_size = 20
        sleep_for = 0
    elif (mem == bd.bdpars.BDMemId.TAT0):
        name = "TAT0"
        word_size = 29
        sleep_for = 0
    elif (mem == bd.bdpars.BDMemId.TAT1):
        name = "TAT1"
        word_size = 29
        sleep_for = 0
    elif (mem == bd.bdpars.BDMemId.MM):
        name = "MM"
        word_size = 8
        sleep_for = 1.5
    elif (mem == bd.bdpars.BDMemId.AM):
        name = "AM"
        word_size = 38
        sleep_for = 0
    else:
        assert(False and "bad mem ID")

    # typically, you would use PackWord to set memory word values
    # we don't care about the fields since we aren't actually using the memories

    if pattern == "random":
        vals = [np.random.randint(2**word_size) for i in range(mem_size)]

    elif pattern == "ones":
        vals = [2**word_size-1 for i in range(mem_size)]

    elif pattern == "zeros":
        vals = [0 for i in range(mem_size)]

    elif pattern == "ones_after_zeros":
        #11..1, 00..0, etc.
        vals = []
        for i in range(mem_size):
            if i % 2 == 0:
                vals.append(2**word_size-1)
            else:
                vals.append(0)

    elif pattern == "alternate":
        # 0101..01, 1010..10, etc.
        vals = []
        for i in range(mem_size):
            if i % 2 == 0:
                vals.append(int("10"*(word_size//2), 2))
            else:
                vals.append(int("01"*(word_size//2), 2))

    elif pattern == "zero_flip":
        vals = [i%2 for i in range(mem_size)]

    else: 
        print("unknown bit pattern requested", pattern)
        exit(1)


    D.SetMem(CORE, mem, vals, 0)

    time.sleep(sleep_for)
    print("NOW")
    time.sleep(2)

    dumped  = D.DumpMem(CORE, mem)
    dumped2 = D.DumpMem(CORE, mem)

    #dumped2 = []
    #for i in range(mem_size):
    #    dumped2 += D.DumpMemRange(CORE, mem, i, i+1)

    # write to file, in case we want to compare
    directory = "write_read"
    if not os.path.exists(directory):
        os.makedirs(directory)

    fvals = open(directory + "/" + name + "_vals.txt", "w")
    for v in vals:
        fvals.write(("{0:0" + str(word_size) +"b}").format(v) + "\n")
    fvals.close()

    fdump = open(directory + "/" + name + "_dump.txt", "w")
    for v in dumped:
        fdump.write(("{0:0" + str(word_size) +"b}").format(v) + "\n")
    fdump.close()

    fdump2 = open(directory + "/" + name + "_dump2.txt", "w")
    for v in dumped2:
        fdump2.write(("{0:0" + str(word_size) +"b}").format(v) + "\n")
    fdump2.close()

    # do comparison, print result
    same = [i == j for i,j in zip(vals, dumped)]
    if(sum(same) == len(vals)):
        print("-->", name, "write/read test passed!")
    else:
        print("-->", name, "write/read test FAILED!")

    # do comparison, print result
    same = [i == j for i,j in zip(vals, dumped2)]
    if(sum(same) == len(vals)):
        print("-->", name, "write/read test passed!")
    else:
        print("-->", name, "write/read test FAILED!")

D = bd.Driver()

D.Start()
print("* Driver Started")

D.ResetBD()
print("* Sent reset")

#TestMem(D, bd.bdpars.BDMemId.PAT, "ones")
TestMem(D, bd.bdpars.BDMemId.PAT, "ones_after_zeros")
#TestMem(D, bd.bdpars.BDMemId.PAT, "zero_flip")
#TestMem(D, bd.bdpars.BDMemId.PAT)


#TestMem(D, bd.bdpars.BDMemId.TAT0)
#TestMem(D, bd.bdpars.BDMemId.TAT1)
#TestMem(D, bd.bdpars.BDMemId.AM)
#TestMem(D, bd.bdpars.BDMemId.MM)

print("* Test over!")

D.Stop()
