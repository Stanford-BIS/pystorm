import time
import pickle
from numpy import diff, sort, median, array, zeros, linspace
import numpy as np
import matplotlib
matplotlib.use('Agg')

from pystorm.hal import HAL # HAL is a singleton, importing immediately sets up a HAL and its C Driver

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network
from pystorm.PyDriver import bddriver as bd

HAL.driver.SetADCTrafficState(0, True)
