from test_hardware_resources import *
from core import Core
from core_pars import *
from neuromorph import map_resources_to_core

print("calling Map for various Resource graphs")
print("writing results to net_dump/")
print("inspection for correctness can be done manually")

N1 = GetN1()
N2 = GetN2()
N3 = GetN3()
N4 = GetN4()

for idx, N in enumerate([N1, N2, N3, N4]):

    print("NETWORK " + str(idx))
    pars = get_core_pars()
    core = Core(pars)
    map_resources_to_core(N, core, True)
    core.Print()
    core.WriteMemsToFile("net_dump/N" + str(idx))
