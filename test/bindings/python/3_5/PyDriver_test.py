import PyDriver as bd

driver = bd.pystorm.bddriver.BDModelDriver()
model = driver.GetBDModel()

# Calling driver.Start() results in segfault when exiting Python
#driver.Start()
#driver.InitBD()

#kCoreId = 0

#pars = driver.GetBDPars()
#size = pars.mem_info_[bd.bdpars.BDMemId.PAT].size;
#data = [64, 63]
#print("* Setting Memory")
#driver.SetMem(kCoreId, bd.bdpars.BDMemId.PAT, data, 0)
#print("* Dumping Memory")
#dumped = driver.DumpMem(kCoreId, bd.bdpars.BDMemId.PAT)
#print("* Set Data")
#print(data)
#print("* First BDWord's value")
#print(dumped[0])
#print("* Dumped Data")
#print(dumped)

exit(0)