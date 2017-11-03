from pystorm.PyDriver import bddriver as bd

D = bd.Driver()

D.Start()
print("* Started")

D.ResetBD()
print("* Sent reset")

PATSize = 64
PAT_vals = [i for i in range(PATSize)] # typically, you would use PackWord to set memory word values
D.SetMem(0, bdpars.BDMemId.PAT, PAT_vals, 0)
print("* Set Memory")

dumped = D.DumpMem(0, bdpars.BDMemId.PAT)
print("* Dumped Memory")
print(dumped)

D.Stop()
