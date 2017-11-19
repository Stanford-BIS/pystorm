from pystorm.PyDriver import bddriver as bd

def standard_startup(time_unit_ns, upstream_hb_ns):
    CORE = 0

    D = bd.Driver()

    comm_state = D.Start()
    if (comm_state < 0):
        print("* Driver failed to start!")
        exit(-1)

    print("* Resetting BD")
    D.ResetBD()

    # set time unit (SF/SG update interval) to .1 ms
    print("* Setting FPGA time units")
    D.SetTimeUnitLen(time_unit_ns)
    D.SetTimePerUpHB(upstream_hb_ns)

    print("* Init the FIFO")
    D.InitFIFO(CORE)

    print("* Enable tag traffic")
    D.SetTagTrafficState(CORE, True, True)

    return D

def compare_TAT_words(progged, dumped):
    if len(dumped) >= 1 and sum([i == j for i,j in zip(dumped, progged)]) == len(progged):
        print("* TAT matches!")
        return 0
    else:
        print("* TAT didn't match")
        if len(dumped) == 0:
            print("we read nothing!")
        for entry, progged_entry in zip(dumped[:len(progged)], progged):
            print("we proggedrammed:")
            print("{0:b}".format(progged_entry))
            print("we read:")
            print("{0:b}".format(entry))
            print("stop", bd.GetField(entry, bd.TATTagWord.STOP))
            print("groute", bd.GetField(entry, bd.TATTagWord.GLOBAL_ROUTE))
            print("tag", bd.GetField(entry, bd.TATTagWord.TAG))
        return -1
