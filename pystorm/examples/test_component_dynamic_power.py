import numpy as np
import matplotlib.pyplot as plt
import time

from pystorm.hal import HAL # HAL is a singleton, importing immediately sets up a HAL and its C Driver

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

np.random.seed(0)


###########################################
# default network size parameters

width = 16
height = 16
Din = 1
Dint = 1
Dout = 1
width_height = (width, height)
N = width * height
d_range=(1,1)
t_range=(1,1)

CORE = 0

###########################################
# misc driver parameters
downstream_time_res = 10000 # ns
upstream_time_res = 1000000 # ns

HAL.set_time_resolution(downstream_time_res, upstream_time_res)


def create_decode_network(width=width, height=height, Din=Din, Dout=Dout, d_range=d_range):
"""
data flow with traffic on:

input IO -> 
tag horn ->

(pre-fifo valve) ->
FIFO ->
(post-fifo valve) ->

TAT -> 

AER_tx -> 
neurons -> 
AER_rx -> 
(neuron output valve) -> 

PAT ->
accumulator ->

(pre-fifo valve) ->
FIFO ->
(post-fifo valve) ->

TAT ->

tag funnel ->
output IO
"""

    net = graph.Network("net")

    min_d, max_d = d_range
    decoders = np.ones((Dint, N)) * (max_d - min_d) - min_d

    tap_matrix = np.zeros((N, Din))
    if Din == 1:
        # one synapse per 4 neurons
        for x in range(0, width, 2):
            for y in range(0, height, 2):
                n = y * width + x
                if x < width // 2:
                    tap_matrix[n, 0] = 1
                else:
                    tap_matrix[n, 0] = -1
    else:
        print("need to implement reasonable taps for Din > 1")
        assert(False)

    i1 = net.create_input("i1", Din)
    p1 = net.create_pool("p1", tap_matrix)
    b1 = net.create_bucket("b1", Dout)
    o1 = net.create_output("o1", Dout)

    net.create_connection("c_i1_to_p1", i1, p1, None)
    decoder_conn = net.create_connection("c_p1_to_b1", p1, b1, decoders)
    net.create_connection("c_b1_to_o1", b1, o1, None)

    return net

def create_trans_network(width=width, height=height, Din=Din, Dint=Dint, Dout=Dout, d_range=d_range, t_range=t_range):

      net = graph.Network("net")

      min_d, max_d = d_range
      decoders = np.ones((Dint, N)) * (max_d - min_d) - min_d

      min_t, max_t = t_range
      trains = np.ones((Dout, Dint)) * (max_t - min_t) - min_t
      
      tap_matrix = np.zeros((N, Din))
      if Din == 1:
          # one synapse per 4 neurons
          for x in range(0, width, 2):
              for y in range(0, height, 2):
                  n = y * width + x
                  if x < width // 2:
                      tap_matrix[n, 0] = 1
                  else:
                      tap_matrix[n, 0] = -1
      else:
          print("need to implement reasonable taps for Din > 1")
          assert(False)

      i1 = net.create_input("i1", Din)
      p1 = net.create_pool("p1", tap_matrix, xy=(width, height))
      b1 = net.create_bucket("b1", Dint)
      b2 = net.create_bucket("b1", Dout)
      o1 = net.create_output("o1", Dout)

      net.create_connection("c_i1_to_p1", i1, p1, None)
      net.create_connection("c_p1_to_b1", p1, b1, decoders)
      net.create_connection("c_b1_to_b2", b1, b2, trans)
      net.create_connection("c_b2_to_o1", b2, o1, None)

      return net

###########################################
# define different experiments, each measuring the throughput of a different component

def ns_to_sec(ns):
    return ns * 1e-9

def names_to_dict(names):
  return dict( (name,eval(name)) for name in names )

class Experiment(object):

    duration = 4 # seconds

    def run(self):
        assert(False and "derived class must implement run()")

    # utility functions, commonly used by derived experiments
    def kill_all_neurons(self):
        for i in range(4096):
            HAL.driver.DisableSoma(CORE, i)

    def make_enabled_neurons_spike(self, bias):
        for i in range(4096):
            HAL.driver.SetSomaGain(CORE, i, bd.bdpars.SomaGainId.ONE)
            HAL.driver.SetSomaOffsetSign(CORE, i, bd.bdpars.SomaOffsetSignId.POSITIVE)
            HAL.driver.SetSomaOffsetMultiplier(CORE, i, bd.bdpars.SomaOffsetMultiplierId.THREE)
            HAL.driver.SetDACCount(CORE , bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, bias)

    def check_outputs(self, outputs, obj, max_Dout):
        if !np.all(outputs[:,0] == obj):
            print('got unexpected output')
        if !np.all(outputs[:,1] <= max_Dout):
            print('got unexpected output dimension')

    def compute_output_rate(self, outputs, obj, max_Dout):
        self.check_outputs(outputs, obj, max_Dout)

        min_time = np.min(outputs[:,3])
        max_time = np.max(outputs[:,3])

        total_count = np.sum(outputs[:,2])

        rate = total_count / ns_to_sec(max_time - min_time)

        return rate
            

class AERRX(Experiment):
    # power setup:
    # neurons -> (stopped at post-neuron valve)
    # counting setup:
    # default

    def __init__(self, soma_bias=50, duration=Experiment.duration):
        self.pars = names_to_dict(["soma_bias", "duration"])
        self.description = "measure AER xmitter power. Pars: " + str(pars)

    def run(self):

        net = create_decoder_network()
        HAL.map(net)
        
        # give the neurons some juice
        self.make_enabled_neurons_spike(self.pars["soma_bias"])

        # turn on traffic, count spikes
        print("enabling traffic, counting spikes")
        HAL.start_traffic()

        time.sleep(self.pars["duration"])

        HAL.stop_traffic()
        # all decoders and tranforms are 1, output count is spike count
        outputs = HAL.get_outputs()
        rate = self.compute_output_rate(outputs, net.get_outputs[0], 0)
        print("measured", rate, "spikes per second")

        print("only neurons and AER RX active, measure voltage now")

        time.sleep(self.pars["duration"])


class Decode(Experiment):
    # counting setup:
    # default
    # power setup:
    # neurons -> AERRX -> PAT -> accumulator -> stopped at pre-FIFO valve

    def __init__(self, soma_bias=50, d_val=.1, Dout=10, duration=Experiment.duration):
        self.pars = names_to_dict(["soma_bias", "duration", "d_val", "Dout"])
        self.description = "measure power for decode operation: AER xmitter + PAT + accumulator. Pars: " + str(self.pars)

    def run(self):
        net = create_decoder_network(d_range=(d_val, d_val))
        HAL.map(net)
        
        # give the neurons some juice
        self.make_enabled_neurons_spike(self.pars["soma_bias"])

        # turn on traffic
        print("enabling traffic, counting tags out")
        HAL.start_traffic()

        time.sleep(self.pars["duration"])

        HAL.stop_traffic()
        # all decoders and tranforms are 1, output count is spike count
        outputs = HAL.get_outputs()
        tag_rate = self.compute_output_rate(outputs, net.get_outputs[0], self.pars["Dout"])
        print("measured", tag_rate, "accumulator outputs per second")
        spike_rate = tag_rate / self.pars["d_val"] / self.pars["Dout"]
        print("inferred", spike_rate, "spikes per second")

        print("disabling pre-FIFO valve, measure power now")
        HAL.start_traffic()
        HAL.driver.SetPreFIFOTrafficState(CORE, False)

        time.sleep(self.pars["duration"])


tests = [
  AERRX(soma_bias=10),
  AERRX(soma_bias=50),
  Decode(soma_bias=50, d_val=.1, Dout=1),
  Decode(soma_bias=50, d_val=1., Dout=1),
  Decode(soma_bias=50, d_val=.1, Dout=10),
  ]
  
for test in tests:
  test.run()


