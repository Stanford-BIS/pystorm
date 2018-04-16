import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import time

from pystorm.hal import HAL

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

from pystorm.hal.neuromorph.core_pars import CORE_PARAMETERS

np.random.seed(0)

###########################################
# network parameters

Din = 1
Dout = 10
fmax = 1000
num_training_points_per_dim = 5
training_hold_time = .1

###########################################
# specify network using HAL

# net = graph.Network("net")

# if Din == 1 and Dout == 1:
#     transform = np.array([[1.0]])
# elif Din == 2 and Dout == 1:
#     transform = np.array([[1.0, .25]])
# elif Din == 1 and Dout == 2:
#     transform = np.array([[1.0], [.1]])
# elif Din == 2 and Dout == 2:
#     transform = np.array([[1.0, .25], [.1, .2]])
# elif Din == 2 and Dout == 4:
#     transform = np.array([[1.0, .25], [.1, .4], [.1, .1], [.25, 1.0]])
# elif Din == 1 and Dout > 4:
#     transform = np.linspace(0, 1, Dout).reshape((Dout, Din))
# else:
#     assert(False and "write a matrix for this Din/Dout combo")

# i1 = net.create_input("i1", Din)
# b1 = net.create_bucket("b1", Dout)
# b2 = net.create_bucket("b2", Dout)
# o1 = net.create_output("o1", Dout)

# net.create_connection("c_i1_to_b1", i1, b1, transform)
# net.create_connection("c_b1_to_b2", b1, b2, None)
# net.create_connection("c_b2_to_o1", b2, o1, None)

def create_decode_encode_network(width, height, d_val):
    
    N = width * height
    net = graph.Network("net")
    decoders = np.ones((1, N)) * d_val
    tap_list = []
    for y in range(0, height, 2):
        for x in range(0, width, 2):
            idx = y * width + x
            tap_list.append((idx, 1))

    for y in range(0, height, 2):
        for x in range(0, width, 2):
            idx = y * width + x
            tap_list.append((idx, -1))
        
    i1 = net.create_input("i1", 1)
    p1 = net.create_pool("p1", (N, [tap_list]))
    b1 = net.create_bucket("b1", 1)
    p3 = net.create_pool("p3", (N, [tap_list]))
    b2 = net.create_bucket("b2", 1)
    p2 = net.create_pool("p2", (N, [tap_list]))
    b4 = net.create_bucket("b4", 1)
    o1 = net.create_output("o1", 1)
    o2 = net.create_output("o2", 1)
    
    net.create_connection("i1_to_p1", i1, p1, None)
    net.create_connection("p1_to_b1", p1, b1, decoders)
    net.create_connection("b1_to_p3", b1, p3, None)
    net.create_connection("p3_to_b2", p3, b2, decoders)
    net.create_connection("b2_to_o1", b2, o1, None)
    net.create_connection("b2_to_p2", b2, p2, None)
    net.create_connection("p2_to_b4", p2, b4, decoders)
    net.create_connection("b4_to_o2", b4, o2, None)
    
    return net

# TAT has only 1024 entries. Since we are hitting the same synapse twice, we can only use 1024 somas or 512 synapses.
# Hence use 32x32 somas
#net = create_decode_encode_network(32, 32, 1.)
net = create_decode_encode_network(8, 8, 1.)
net.create_hardware_resources()

###########################################
# invoke HAL.map(), make tons of neuromorph/driver calls under the hood

# map network
print("calling parition")
net.map(CORE_PARAMETERS, num_cores = 3, verbose = True)
