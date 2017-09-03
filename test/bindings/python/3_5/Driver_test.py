import BDDriver as bd

driver = bd.pystorm.bddriver.BDModelDriver()
model = driver.GetBDModel()

driver.Start()
driver.InitBD()

kCoreId = 0

pars = driver.GetBDPars()
size = pars.Size(bd.pystorm.bddriver.bdpars.PAT)
data = [bd.pystorm.bddriver.BDWord(64),]
print("* Setting Memory")
driver.SetMem(kCoreId, bd.pystorm.bddriver.bdpars.PAT, data, 0)
print("* Dumping Memory")
dumped = driver.DumpMem(kCoreId, bd.pystorm.bddriver.bdpars.PAT)
print("* Set Data")
print(data)
print("* First BDWord's value")
print(dumped[0].Packed())
print("* Dumped Data")
print(dumped)