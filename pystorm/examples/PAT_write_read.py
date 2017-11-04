from pystorm.PyDriver import bddriver as bd

D = bd.Driver()

D.Start()
print("* Started")

D.ResetBD()
print("* Sent reset")

PATSize = 64
PAT_vals = [i for i in range(PATSize)] # typically, you would use PackWord to set memory word values
D.SetMem(0, bd.bdpars.BDMemId.PAT, PAT_vals, 0)
print("* Set Memory")

dumped = D.DumpMem(0, bd.bdpars.BDMemId.PAT)
print("* Dumped Memory")

print("* Test over!")
print("we wrote:")
print(PAT_vals)
print("we read:")
print(dumped)

same = [i == j for i,j in zip(PAT_vals, dumped)]
if(sum(same) == len(PAT_vals)):
    print("--> test passed!")
else:
    print("--> test FAILED!")

D.Stop()

exit(0)
