from cal_db import *
import numpy as np

# test with synthetic activations

cdb = CalibrationDB('testdb')
cdb.init_dataframes() # force clearing of data

act1 = np.random.randint(2, size=(4096,))
act1 = act1 / np.linalg.norm(act1)

act2 = np.random.randint(2, size=(4096,))
act2 = act2 / np.linalg.norm(act2)

cdb.add_new_chip(act1, 'chip1')
cdb.add_new_chip(act2, 'chip2')

try:
    cdb.add_new_chip(act2, 'chip3')
except ValueError:
    print('caught exception after trying to add same chip twice')
    
caldata1 = np.random.rand(1024)
caldata2 = np.random.rand(1024)
caldata3 = np.random.rand(1024)

cdb.add_calibration('chip1', 'synapse', 'tau', caldata1)
cdb.add_calibration('chip1', 'synapse', 'pulse_width', caldata2)
cdb.add_calibration('chip2', 'synapse', 'tau', caldata3)

try:
    cdb.add_calibration('chip1', 'synapse', 'tau', np.random.rand(1025))
except ValueError:
    print('caught exception after trying to add too-long synapse data')

hopefully_caldata1 = cdb.get_calibration('chip1', 'synapse', 'tau')

assert(np.sum(np.abs(hopefully_caldata1 - caldata1) < .001) == len(caldata1))

cdb2 = CalibrationDB('testdb')

hopefully_caldata1 = cdb.get_calibration('chip1', 'synapse', 'tau')

assert(np.sum(np.abs(hopefully_caldata1 - caldata1) < .001) == len(caldata1))

# test with HAL's activation-gathering functions
# just making sure that the data from this function is well-formed
# make sure that real key doesn't collide with a random one

from pystorm.hal import HAL
HAL = HAL()
real_act = HAL.get_unique_chip_activation()
cdb.add_new_chip(real_act, 'realchip', commit_now=True)


