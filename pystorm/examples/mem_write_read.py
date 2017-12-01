"""Memory write-read loopback test
Programs all entries of the PAT
Reads those entries back and checks equivalence
"""
from pystorm.PyDriver import bddriver as bd
import numpy as np
import os
import time

CORE = 0

def TestMem(D, mem, pattern="random", delay=0):

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
        sleep_for = 3
    elif (mem == bd.bdpars.BDMemId.AM):
        name = "AM"
        word_size = 38
        sleep_for = 0
    else:
        assert(False and "bad mem ID")

    # typically, you would use PackWord to set memory word values
    # we don't care about the fields since we aren't actually using the memories

    if pattern == "count":
        vals = [i % 2**word_size for i in range(mem_size)]

    elif pattern == "random":
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
    #print("NOW")
    #time.sleep(2)

    dumped  = D.DumpMem(CORE, mem)
    dumped2 = D.DumpMem(CORE, mem)

    #dumped2 = []
    #for i in range(mem_size):
    #    dumped2 += D.DumpMemRange(CORE, mem, i, i+1)

    # write to file, in case we want to compare
    directory = "write_read"
    if not os.path.exists(directory):
        os.makedirs(directory)

    def write_to_file(fname, vals):
        fvals = open(directory + "/" + fname, "w")
        for v in vals:
            fvals.write(("{0:0" + str(word_size) +"b}").format(v) + "\n")
        fvals.close()

    write_to_file(name + "_vals.txt", vals)
    write_to_file(name + "_dump.txt", dumped)
    write_to_file(name + "_dump2.txt", dumped2)

    # do comparison, print results
    def compare(vals0, vals1, verbose=False):
        same = [i == j for i,j in zip(vals0, vals1)]
        if(sum(same) == len(vals0)):
            print("-->", name, "write/read test passed!")
        else:
            print("-->", name, "write/read test FAILED!")
            if verbose:
                for idx, (s, v, d) in enumerate(zip(same, vals0, vals1)):
                    if s is False:
                        print(idx, ":", ("{0:0" + str(word_size) +"b}").format(v), "vs", ("{0:0" + str(word_size) +"b}").format(d))
    
    # AM serializer has a bug, use different (incomplete) comparison
    def compare_AM(vals, dumped, verbose=False):
        half_size = word_size // 2

        v_lsbs = []
        v_lsbs = [v % 2**half_size for v in vals]

        d_msbs = []
        d_msbs = [d >> half_size   for d in dumped]

        compare(v_lsbs, d_msbs, verbose)

    if name != "AM":
        compare(vals, dumped, verbose=True)
        compare(vals, dumped2)
    else:
        compare_AM(vals, dumped, verbose=True)
        compare_AM(vals, dumped2)

D = bd.Driver()

comm_state = D.Start()
if (comm_state < 0):
    print("* Driver failed to start!")
    exit(-1)

print("* Driver Started")

D.ResetBD()
print("* Sent reset")

#TestMem(D, bd.bdpars.BDMemId.PAT, "ones")
#TestMem(D, bd.bdpars.BDMemId.PAT, "ones_after_zeros")
#TestMem(D, bd.bdpars.BDMemId.PAT, "zero_flip")
#TestMem(D, bd.bdpars.BDMemId.PAT, "alternate")
#TestMem(D, bd.bdpars.BDMemId.PAT, "count")

## XXX random is a hard test because of the unresolved output(?) iffyness
#TestMem(D, bd.bdpars.BDMemId.PAT)
#TestMem(D, bd.bdpars.BDMemId.TAT0)
#TestMem(D, bd.bdpars.BDMemId.TAT1)
#TestMem(D, bd.bdpars.BDMemId.MM)
#TestMem(D, bd.bdpars.BDMemId.AM)

# count is easier, should generally pass
TestMem(D , bd.bdpars.BDMemId.PAT  , "count")
TestMem(D , bd.bdpars.BDMemId.TAT0 , "count")
TestMem(D , bd.bdpars.BDMemId.TAT1 , "count")
TestMem(D , bd.bdpars.BDMemId.MM   , "count")
TestMem(D , bd.bdpars.BDMemId.AM   , "count")

print("* Test over!")

D.Stop()
