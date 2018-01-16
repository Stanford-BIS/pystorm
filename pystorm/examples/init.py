from pystorm.PyDriver import bddriver as bd

CORE = 0

D = bd.Driver()

comm_state = D.Start()
if (comm_state < 0):
    print("* Driver failed to start!")
    exit(-1)

D.InitBD();

D.Stop()
