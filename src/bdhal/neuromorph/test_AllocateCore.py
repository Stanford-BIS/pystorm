from Resources import *
from Core import *
from CoreParsPlaceholder import *
from test_Resources import *

N1 = GetN1()
N2 = GetN2()
N3 = GetN3()
N4 = GetN4()

for idx, N in enumerate([N1, N2, N3, N4]):
#for idx, n in enumerate([N1]):

    print("NETWORK " + str(idx))
    pars = CoreParsPlaceholder()
    core = Core(pars)
    core.Map(N, verbose=True)
    core.Print()
    core.WriteMemsToFile("net_dump/N" + str(idx))
