def CorePars():
    # copied from test_TopLevel.act:
    # if you change the above, you can keep the below the same

    SELF_ROUTE = 0; # value of route to self
    MTAG = 6; # tags
    MGRT = 12; # global route
    MCT = MTAG - 2; # counts

    # 8x8 neuron array, 16 tap points, 4 neurons to a pool
    MTAP = 4; # tap points
    MNRNY = 3; # neurons y entries
    MNRNX = 3; # neurons x entries
    MPOOL = 2; # neurons in a pool

    # AM = 16 entries, MM = 4*2 * 8 = 64 entries
    MAMA = 4; # acc memory entries
    MMMAY = 3; # main memory y entries
    MMMAX = 3; # main memory x entries

    # acc datapath is still hardcoded, so
    # 8 bit weights, 15 bit val, 3 bit thr
    MW = 8; # weight
    MVAL = 15; # acc bucket
    MTHR = 3; # acc thr positions

    pars = {}
    pars['SELF_ROUTE'] = SELF_ROUTE
    pars['MTAG'] = MTAG
    pars['MGRT'] = MGRT
    pars['MCT'] = MCT
    pars['MTAP'] = MTAP
    pars['MNRNY'] = MNRNY
    pars['MNRNX'] = MNRNX
    pars['MPOOL'] = MPOOL
    pars['MAMA'] = MAMA
    pars['MMMAY'] = MMMAY
    pars['MMMAX'] = MMMAX
    pars['MW'] = MW
    pars['MVAL'] = MVAL
    pars['MTHR'] = MTHR

    return pars
