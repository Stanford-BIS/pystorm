from Resources import *
from Core import *
from default_pars import CorePars
from test_Resources import *

N1 = GetN1()
N2 = GetN2()
N3 = GetN3()
N4 = GetN4()

for idx, n in enumerate([N1, N2, N3, N4]):
#for idx, n in enumerate([N1]):

    print("NETWORK " + str(idx))
    pars = CorePars()
    core = Core(pars)

    for r in n:
        r.PreTranslate(core)
        
    for r in n:
        r.AllocateEarly(core)

    core.MM.alloc.SwitchToTrans() # switch allocation mode of MM
    for r in n:
        r.Allocate(core)

    for r in n:
        r.PostTranslate(core)

    core.Print()
