import numpy as np
import matplotlib.pyplot as plt
import time

from pystorm.hal import HAL # HAL is a singleton, importing immediately sets up a HAL and its C Driver

from pystorm.hal.neuromorph import graph # to describe HAL/neuromorph network

from pystorm.PyDriver import bddriver as bd # expose Driver functions directly for debug (cool!)

from sys import exit

np.random.seed(0)

###########################################
# default network size parameters

width = 16
height = 16
Din = 1
Dint = 1
Dout = 1
width_height = (width, height)
d_range=(1,1)
t_range=(1,1)
exp_duration = 1
taps_per_dim = 8

CORE = 0

###########################################
# misc driver parameters
downstream_time_res = 10000 # ns
upstream_time_res  = 100000 # ns

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

    N = width * height

    net = graph.Network("net")

    min_d, max_d = d_range
    decoders = np.ones((Dout, N)) * (max_d - min_d) + min_d

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

def create_transform_network(width=width, height=height, Din=Din, Dint=Dint, Dout=Dout, d_range=d_range, t_range=t_range):

    N = width * height

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

def create_decode_encode_network(width=width, height=height, Dint=Dint, d_range=d_range, taps_per_dim=taps_per_dim, measure_tags=False, is_recur=False):

    N = width * height

    net = graph.Network("net")

    min_d, max_d = d_range
    decoders = np.ones((Dint, N)) * (max_d - min_d) + min_d

    tap_matrix = np.zeros((N, Dint))
    if Dint == 1:
        # one synapse per 4 neurons
        for x in range(0, width, 2):
            for y in range(0, height, 2):
                n = y * width + x
                if x < width // 2:
                    tap_matrix[n, 0] = 1
                else:
                    tap_matrix[n, 0] = -1
    else:
        for d in range(Dint):
            for s in [-1, 1]:
                if s == -1:
                    num_taps = taps_per_dim // 2
                else:
                    num_taps = taps_per_dim - taps_per_dim // 2

                for t in range(num_taps):
                    while True:
                        x = np.random.randint(width//2)
                        y = np.random.randint(height//2)
                        n = (2*y) * width + (2*x)
                        if np.all(tap_matrix[n, :] == 0): # keep trying until an unused synapse is found
                            tap_matrix[n, d] = s
                            break

    p1 = net.create_pool("p1", tap_matrix)
    b1 = net.create_bucket("b1", Dint)
    if not is_recur:
        p2 = net.create_pool("p2", tap_matrix)

    if measure_tags:
        o1 = net.create_output("o1", Dint)

    net.create_connection("c_p1_to_b1", p1, b1, decoders)

    if not is_recur:
        net.create_connection("c_b1_to_p2", b1, p2, None)
    else:
        net.create_connection("c_b1_to_p1", b1, p1, None)

    if measure_tags:
        net.create_connection("c_b1_to_o1", b1, o1, None)

    return net

###########################################
# define different experiments, each measuring the throughput of a different component

def ns_to_sec(ns):
    return ns * 1e-9

def names_to_dict(names, locs):
  return dict( (name, locs[name]) for name in names )

class Experiment(object):

    duration = exp_duration # seconds

    def run(self):
        assert(False and "derived class must implement run()")

    # utility functions, commonly used by derived experiments
    def kill_all_neurons(self):
        for i in range(4096):
            HAL.driver.DisableSoma(CORE, i)
        HAL.driver.Flush()

    def make_enabled_neurons_spike(self, bias):
        HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SOMA_OFFSET, bias)
        for i in range(4096):
            HAL.driver.SetSomaGain(CORE, i, bd.bdpars.SomaGainId.ONE)
            HAL.driver.SetSomaOffsetSign(CORE, i, bd.bdpars.SomaOffsetSignId.POSITIVE)
            HAL.driver.SetSomaOffsetMultiplier(CORE, i, bd.bdpars.SomaOffsetMultiplierId.THREE)


    def check_outputs(self, outputs, obj, max_Dout):
        if not np.all(outputs[:,1] == obj):
            print('ERROR: got unexpected output object')
            exit(-1)
        if not np.all(outputs[:,2] <= max_Dout):
            print('ERROR: got unexpected output dimension')
            exit(-1)

    def compute_output_rate(self, outputs, obj, max_Dout):
        if outputs.shape[0] > 0:
            self.check_outputs(outputs, obj, max_Dout)

            min_time = np.min(outputs[:,0])
            max_time = np.max(outputs[:,0])

            total_count = np.sum(outputs[:,3])

            rate = total_count / ns_to_sec(max_time - min_time)
        else:
            rate = 0

        return rate

    def make_fast_synapse(self):
        HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PD, 1024)
        HAL.driver.SetDACCount(CORE, bd.bdpars.BDHornEP.DAC_SYN_PU, 1024)

###########################################
# Get baseline static power

class Static(Experiment):

    def __init__(self, duration=Experiment.duration):
        self.pars = names_to_dict(["duration"], locals())
        self.results = {}
        self.description = "don't map any network, just measure baseline power"

    def run(self):
        HAL.driver.InitBD()
        # nothing to do, neurons should be killed without mapping
        print("Unmapped network for baseline power, measure power now")
        time.sleep(self.pars["duration"])

###########################################
# Get AER rx power

class AERRX(Experiment):
    # counting setup:
    # default
    # power setup:
    # neurons -> (stopped at post-neuron valve)

    def __init__(self, soma_bias=2, duration=Experiment.duration):
        self.pars = names_to_dict(["soma_bias", "duration"], locals())
        self.results = {}
        self.description = "measure AER xmitter power. Pars: " + str(self.pars)

    def run(self):

        net = create_decode_network()
        HAL.map(net)

        # give the neurons some juice
        self.make_enabled_neurons_spike(self.pars["soma_bias"])

        # turn on traffic, count spikes
        print("enabling traffic, counting spikes")
        HAL.start_traffic(flush=False)
        HAL.enable_output_recording()

        time.sleep(self.pars["duration"])

        HAL.stop_traffic() # and keep it off
        # all decoders and tranforms are 1, output count is spike count
        outputs = HAL.get_outputs()
        rate = self.compute_output_rate(outputs, net.get_outputs()[0], 0)
        print("measured", rate, "spikes per second")
        self.results["rate"] = rate

        print("only neurons and AER RX active, measure voltage now")

        time.sleep(self.pars["duration"])

        outputs = HAL.get_outputs()
        rate = self.compute_output_rate(outputs, net.get_outputs()[0], 0)
        print("sanity check: measured", rate, "spikes per second (expect 0)")

###########################################
# Get decode operation (PAT + Acc) power

class Decode(Experiment):
    # counting setup:
    # default
    # power setup:
    # neurons -> AERRX -> PAT -> accumulator -> stopped at pre-FIFO valve

    def __init__(self, soma_bias=2, d_val=.1, Dout=10, duration=Experiment.duration):
        self.pars = names_to_dict(["soma_bias", "duration", "d_val", "Dout"], locals())
        self.results = {}
        self.description = "measure power for decode operation: AER xmitter + PAT + accumulator. Pars: " + str(self.pars)

    def count_after_experiment(self, net):
        HAL.stop_traffic()
        time.sleep(.1)
        # all decoders are 1, output count is spike count
        outputs = HAL.get_outputs()
        tag_rate = self.compute_output_rate(outputs, net.get_outputs()[0], self.pars["Dout"])
        print("measured", tag_rate, "accumulator outputs per second")
        spike_rate = tag_rate / self.pars["d_val"] / self.pars["Dout"]
        print("inferred", spike_rate, "spikes per second")
        return spike_rate, tag_rate

    def run(self):
        net = create_decode_network(width=32, height=32, Dout=self.pars["Dout"], d_range=(self.pars["d_val"], self.pars["d_val"]))
        HAL.map(net)

        # give the neurons some juice
        self.make_enabled_neurons_spike(self.pars["soma_bias"])

        # turn on traffic
        print("enabling traffic, counting tags out")
        HAL.start_traffic()
        HAL.enable_output_recording()

        time.sleep(self.pars["duration"])

        spike_rate, tag_rate = self.count_after_experiment(net)
        self.results["spike_rate"] = spike_rate
        self.results["tag_rate"] = tag_rate

        print("disabling pre-FIFO valve, measure power now")
        HAL.start_traffic(flush=False)
        HAL.driver.SetPreFIFOTrafficState(CORE, False)
        HAL.enable_output_recording(flush=True)

        time.sleep(self.pars["duration"])

        print("sanity check: should expect no outputs with pre-FIFO valve closed")
#        self.count_after_experiment(net)

###########################################
# Get Input IO/horn

class InputIO(Experiment):
    # no counting setup, we use the SG to produce an exact rate
    # power setups:
    # IO/horn:
    # IO -> horn -> (closed pre-FIFO valve)
    def __init__(self, input_rate=1000, duration=Experiment.duration):
        self.pars = names_to_dict(["input_rate", "duration"], locals())
        self.results = {}
        self.description = "Measure input IO + tag horn power. Pars: " + str(self.pars)

    def run(self):
        net = create_decode_network()
        HAL.map(net)

        # don't want any neuron power
        self.kill_all_neurons()

        time.sleep(.1)

        # sanity check, make sure there are no spikes
        HAL.enable_spike_recording()
        time.sleep(.5)
        spikes = HAL.get_spikes()
        print("sanity check: got", len(spikes), "spikes (expect 0)")

        # leave traffic off, that keeps the pre-FIFO valve closed
        inp = net.get_inputs()[0]
        HAL.set_input_rate(inp, 0, self.pars["input_rate"])

        print("measure power now")
        time.sleep(self.pars["duration"])

###########################################
# FIFO

class FIFO(Experiment):
    # no counting setup, we use the SG to produce an exact rate
    # FIFO:
    # IO -> horn -> FIFO -> (closed post-FIFO valve)
    def __init__(self, input_rate=1000, duration=Experiment.duration):
        self.pars = names_to_dict(["input_rate", "duration"], locals())
        self.results = {}
        self.description = "Measure FIFO power. Pars: " + str(self.pars)

    def run(self):
        net = create_decode_network()
        HAL.map(net)

        # don't want any neuron power
        self.kill_all_neurons()

        # turn off post-fifo traffic
        HAL.start_traffic(flush=False)
        HAL.driver.SetPostFIFOTrafficState(CORE, False)

        time.sleep(.1)

        inp = net.get_inputs()[0]
        HAL.set_input_rate(inp, 0, self.pars["input_rate"])

        # sanity check, monitor pre-fifo spikes
        HAL.driver.SetPreFIFODumpState(CORE, True)
        time.sleep(.5)
        print("trying to get pre-fifo dump")
        dumped = HAL.driver.GetPreFIFODump(CORE)
        print("sanity check: with pre-fifo dump on, got", len(dumped), "pre-FIFO events (expect", self.pars["input_rate"]*.5, ")")
        #HAL.driver.SetPreFIFODumpState(CORE, False)
        time.sleep(.1)


        print("measure power now")
        time.sleep(self.pars["duration"])

        dumped = HAL.driver.GetPreFIFODump(CORE)
        print("sanity check: make sure pre-FIFO dump actually turned off. Got", len(dumped), "pre-FIFO events (expect 0)")

###########################################
# tap point/txmitter  power
class TapPointAndAERTX(Experiment):
    # no counting setup, we use the SG to produce an exact rate
    # tap point/AER TX
    # IO -> horn -> FIFO -> TAT -> AER tx
    def __init__(self, input_rate=1000, width=width, height=height, duration=Experiment.duration):
        self.pars = names_to_dict(["input_rate", "width", "height", "duration"], locals())
        self.results = {}
        self.description = "Measure tap point/AER tx power. Pars: " + str(self.pars)

    def run(self):
        net = create_decode_network(width=self.pars["width"], height=self.pars["height"])
        HAL.map(net)

        # don't want any neuron power
        self.kill_all_neurons()

        HAL.start_traffic(flush=False)

        time.sleep(.1)

        inp = net.get_inputs()[0]
        HAL.set_input_rate(inp, 0, self.pars["input_rate"])

        print("measure power now")
        time.sleep(self.pars["duration"])

###########################################
# decode-encode
class DecodeEncode(Experiment):
    # need to map twice for this
    # counting setup:
    # neurons -> AERRX -> PAT -> accumulator -> TAT -> funnel -> out
    #
    # power setup:
    # neurons -> AERRX -> PAT -> accumulator -> FIFO -> TAT -> AERTX -> neurons
    # take care that there is no FIFO overflow in this setup, which would indicate TAT/AERTX/synapse backup

    def __init__(self, width=32, height=32, soma_bias=2, d_val=.1, Dint=10, taps_per_dim=8, duration=Experiment.duration, is_recur=False):
        self.pars = names_to_dict(["soma_bias", "duration", "d_val", "Dint", "taps_per_dim", "width", "height", "is_recur"], locals())
        self.results = {}
        self.description = "measure power for decode operation: AER xmitter + PAT + accumulator. Pars: " + str(self.pars)

    def count_after_experiment(self, net):
        HAL.stop_traffic()
        time.sleep(.1)
        # all decoders are 1, output count is spike count
        outputs = HAL.get_outputs()
        output_obj = net.get_outputs()[0]
        tag_rate = self.compute_output_rate(outputs, output_obj, self.pars["Dint"])
        print("[OUTPUT] measured", tag_rate, "accumulator outputs per second")
        spike_rate = tag_rate / self.pars["d_val"] / self.pars["Dint"]
        print("[OUTPUT] inferred", spike_rate, "spikes per second")
        return spike_rate, tag_rate

    def run(self):
        #####################################
        # measure rates 
        measure_net = create_decode_encode_network(
                          width=self.pars["width"],
                          height=self.pars["height"],
                          Dint=self.pars["Dint"],
                          d_range=(self.pars["d_val"], self.pars["d_val"]),
                          taps_per_dim=self.pars["taps_per_dim"],
                          measure_tags=True,
                          is_recur=self.pars["is_recur"])
        HAL.map(measure_net)

        # give the neurons some juice
        self.make_enabled_neurons_spike(self.pars["soma_bias"])
        self.make_fast_synapse()

        # turn on traffic
        print("enabling traffic, counting tags out")
        HAL.start_traffic()
        HAL.enable_output_recording()

        time.sleep(self.pars["duration"])

        spike_rate, tag_rate = self.count_after_experiment(measure_net)
        self.results["spike_rate"] = spike_rate
        self.results["tag_rate"] = tag_rate

        #####################################
        # go to power measurement configuration, make sure not overflowing
        power_net = create_decode_encode_network(
                         width=self.pars["width"],
                         height=self.pars["height"],
                         Dint=self.pars["Dint"],
                         d_range=(self.pars["d_val"], self.pars["d_val"]),
                         taps_per_dim=self.pars["taps_per_dim"],
                         measure_tags=False,
                         is_recur=self.pars["is_recur"])
        HAL.map(power_net)

        # give the neurons some juice
        self.make_enabled_neurons_spike(self.pars["soma_bias"])
        self.make_fast_synapse()

        # turn on traffic
        HAL.start_traffic()
        HAL.enable_output_recording(flush=True)

        time.sleep(self.pars["duration"])

        print("[OUTPUT] sanity check: should expect no outputs with remapped network")
        outputs = HAL.get_outputs()
        total_count = np.sum(outputs[:,3])
        print("[OUTPUT] total outputs:", total_count)

        print("[OUTPUT] sanity check: FIFO should not overflow")
        print("[OUTPUT] total overflows:", HAL.get_overflow_counts())

        #####################################
        # remap network again in power measurement configuration, measure power
        HAL.map(power_net)

        # give the neurons some juice
        self.make_enabled_neurons_spike(self.pars["soma_bias"])
        self.make_fast_synapse()

        # turn on traffic
        HAL.start_traffic()
        HAL.enable_output_recording(flush=True)

        print("remapped network, measure power now")

###########################################
# Print the pickled file
import pickle

fname = "trial_data.pck"

def print_pickle(fname):
    pfile = open(fname, "rb")
    old_tests = pickle.load(pfile)
    pfile.close()

    print("Length of old_tests: %d" % len(old_tests))
    for idx, test in enumerate(old_tests):
        print("Test Description: " + test.description)
        print("Test Results: ")
        print(test.results)

print_pickle(fname)

