import nengo
import time
import numpy as np
import sys
from math import floor

sys.path.append(".")

import PyRawDriver as bd
from PyDriver.pystorm.bddriver import bdpars

AER_TO_XY = np.zeros((4096, 2), dtype=int)
for _idx in range(4096):
    x = 0
    y = 0
    word = "{0:012b}".format(_idx)[::-1]
    for _n in range(6):
        x += (1 << _n) * (int(word[2 * _n]) ^ int(word[2 * _n + 1]))
        y += (1 << _n) * int(word[2 * _n + 1])
    AER_TO_XY[_idx] = (x, y)



def singleton_driver():
    if not hasattr(nengo, 'bddriver'):
        nengo.bddriver = bd.Driver()
        nengo.bddriver.InitBD()

        nengo.bddriver.__buffered__=True

        # Disable all Somas
        for _idx in range(4096):
            nengo.bddriver.DisableSoma(_idx)
            nengo.bddriver.DisableSoma(_idx)
            nengo.bddriver.DisableSoma(_idx)
            nengo.bddriver.DisableSoma(_idx)
            nengo.bddriver.SetSomaGain(_idx, bdpars.SomaGainId.ONE)
            nengo.bddriver.SetSomaGain(_idx, bdpars.SomaGainId.ONE)
            nengo.bddriver.SetSomaGain(_idx, bdpars.SomaGainId.ONE)
            nengo.bddriver.SetSomaGain(_idx, bdpars.SomaGainId.ONE)
            nengo.bddriver.SetSomaOffsetSign(_idx, bdpars.SomaOffsetSignId.POSITIVE)
            nengo.bddriver.SetSomaOffsetSign(_idx, bdpars.SomaOffsetSignId.POSITIVE)
            nengo.bddriver.SetSomaOffsetSign(_idx, bdpars.SomaOffsetSignId.POSITIVE)
            nengo.bddriver.SetSomaOffsetSign(_idx, bdpars.SomaOffsetSignId.POSITIVE)
            nengo.bddriver.SetSomaOffsetMultiplier(_idx, bdpars.SomaOffsetMultiplierId.ZERO)
            nengo.bddriver.SetSomaOffsetMultiplier(_idx, bdpars.SomaOffsetMultiplierId.ZERO)
            nengo.bddriver.SetSomaOffsetMultiplier(_idx, bdpars.SomaOffsetMultiplierId.ZERO)
            nengo.bddriver.SetSomaOffsetMultiplier(_idx, bdpars.SomaOffsetMultiplierId.ZERO)
            nengo.bddriver.FlushBDBuffer()

        # Disable all Synapses
        for _idx in range(1024):
            nengo.bddriver.DisableSynapse(_idx)
            nengo.bddriver.DisableSynapse(_idx)
            nengo.bddriver.DisableSynapse(_idx)
            nengo.bddriver.DisableSynapse(_idx)
            nengo.bddriver.DisableSynapseADC(_idx)
            nengo.bddriver.DisableSynapseADC(_idx)
            nengo.bddriver.DisableSynapseADC(_idx)
            nengo.bddriver.DisableSynapseADC(_idx)
            nengo.bddriver.FlushBDBuffer()

        # Open all Diffusors
        for _idx in range(256):
            nengo.bddriver.OpenDiffusorAllCuts(_idx)
            nengo.bddriver.OpenDiffusorAllCuts(_idx)
            nengo.bddriver.OpenDiffusorAllCuts(_idx)
            nengo.bddriver.OpenDiffusorAllCuts(_idx)
            nengo.bddriver.FlushBDBuffer()

        
    return nengo.bddriver



def PrintBytearrayAs32b(buf_out):
    nop_count = 0 
    for idx in range(len(buf_out) // 4): 
        this_word_flipped = buf_out[4 * idx:4 * (idx + 1)] 
        this_word = this_word_flipped[::-1]
        if (this_word[0] == 64 and this_word[1] == 0 and this_word[2] == 0 and this_word[3] == 0): # upstream NOP
            nop_count += 1
        elif (this_word[0] == 128 and this_word[1] == 0 and this_word[2] == 0 and this_word[3] == 0): # downstream NOP
            nop_count += 1
        else:
            for j in range(4):
                to_print = ""
                elcopy = this_word[j]
                for i in range(8):
                    to_print += str(elcopy % 2)
                    elcopy = elcopy >> 1
                to_print += " " 
                sys.stdout.write(str(to_print[::-1]))
            sys.stdout.write('\n')
    print("plus " + str(nop_count) + " NOPs")


class BrainDropSpikes(nengo.Process):
    def __init__(self, n_neurons):
        self.spikes = np.zeros(n_neurons, dtype=float)
        self.n_neurons = n_neurons
        self._nengo_spike_node_ = True
        super(BrainDropSpikes, self).__init__(default_size_out=n_neurons)
        
    def make_step(self, size_in, size_out, dt, rng):
        # NOTE: this all gets called once when you press Play
        
        #driver = bd.Driver()
        driver = singleton_driver()
        #driver.InitBD()
        
        driver.SetSpikeDumpState(1)  
        driver.SetSpikeDumpState(1)  
        driver.SetSpikeDumpState(1)    
        driver.SetSpikeDumpState(1)    

        driver.SetDACValue(bd.HORN.DAC_SOMA_OFFSET, 1)
        driver.SetDACValue(bd.HORN.DAC_SOMA_OFFSET, 1)
        driver.SetDACValue(bd.HORN.DAC_SOMA_OFFSET, 1)
        driver.SetDACValue(bd.HORN.DAC_SOMA_OFFSET, 1)

        driver.SetDACValue(bd.HORN.DAC_SOMA_REF, 16)
        driver.SetDACValue(bd.HORN.DAC_SOMA_REF, 16)
        driver.SetDACValue(bd.HORN.DAC_SOMA_REF, 16)
        driver.SetDACValue(bd.HORN.DAC_SOMA_REF, 16)

        driver.SetDACValue(bd.HORN.DAC_DIFF_G, 1)
        driver.SetDACValue(bd.HORN.DAC_DIFF_G, 1)
        driver.SetDACValue(bd.HORN.DAC_DIFF_G, 1)
        driver.SetDACValue(bd.HORN.DAC_DIFF_G, 1)
        driver.SetDACValue(bd.HORN.DAC_DIFF_R, 10)
        driver.SetDACValue(bd.HORN.DAC_DIFF_R, 10)
        driver.SetDACValue(bd.HORN.DAC_DIFF_R, 10)
        driver.SetDACValue(bd.HORN.DAC_DIFF_R, 10)
        driver.SetDACValue(bd.HORN.DAC_SYN_EXC, (34 + 30) * 8)
        driver.SetDACValue(bd.HORN.DAC_SYN_EXC, (34 + 30) * 8)
        driver.SetDACValue(bd.HORN.DAC_SYN_EXC, (34 + 30) * 8)
        driver.SetDACValue(bd.HORN.DAC_SYN_EXC, (34 + 30) * 8)
        #driver.SetDACValue(bd.HORN.DAC_SYN_EXC, 1024)
        #driver.SetDACValue(bd.HORN.DAC_SYN_EXC, 1024)
        #driver.SetDACValue(bd.HORN.DAC_SYN_EXC, 1024)
        #driver.SetDACValue(bd.HORN.DAC_SYN_EXC, 1024)
        driver.SetDACValue(bd.HORN.DAC_SYN_DC, 34 * 16)
        driver.SetDACValue(bd.HORN.DAC_SYN_DC, 34 * 16)
        driver.SetDACValue(bd.HORN.DAC_SYN_DC, 34 * 16)
        driver.SetDACValue(bd.HORN.DAC_SYN_DC, 34 * 16)
        driver.SetDACValue(bd.HORN.DAC_SYN_INH, (34 - 30) * 128)
        driver.SetDACValue(bd.HORN.DAC_SYN_INH, (34 - 30) * 128)
        driver.SetDACValue(bd.HORN.DAC_SYN_INH, (34 - 30) * 128)
        driver.SetDACValue(bd.HORN.DAC_SYN_INH, (34 - 30) * 128)
        driver.SetDACValue(bd.HORN.DAC_SYN_PU, 1024)
        driver.SetDACValue(bd.HORN.DAC_SYN_PU, 1024)
        driver.SetDACValue(bd.HORN.DAC_SYN_PU, 1024)
        driver.SetDACValue(bd.HORN.DAC_SYN_PU, 1024)
        driver.SetDACValue(bd.HORN.DAC_SYN_PD, 1)
        driver.SetDACValue(bd.HORN.DAC_SYN_PD, 1)
        driver.SetDACValue(bd.HORN.DAC_SYN_PD, 1)
        driver.SetDACValue(bd.HORN.DAC_SYN_PD, 1)
        driver.SetDACValue(bd.HORN.DAC_SYN_LK, 1)
        driver.SetDACValue(bd.HORN.DAC_SYN_LK, 1)
        driver.SetDACValue(bd.HORN.DAC_SYN_LK, 1)
        driver.SetDACValue(bd.HORN.DAC_SYN_LK, 1)
        driver.FlushBDBuffer()
        
        in_idx = 528

        for _y in range(16, 48):
            for _x in range(16, 48):
                _idx = _x * 64 + _y
                driver.EnableSoma(_idx)
                driver.EnableSoma(_idx)
                driver.EnableSoma(_idx)
                driver.EnableSoma(_idx)
                driver.FlushBDBuffer()
        #driver.EnableSoma(in_idx * 4)
        #driver.EnableSoma(in_idx * 4)
        #driver.EnableSoma(in_idx * 4)
        #driver.EnableSoma(in_idx * 4)
        #driver.EnableSoma(in_idx * 4 + 1)
        #driver.EnableSoma(in_idx * 4 + 1)
        #driver.EnableSoma(in_idx * 4 + 1)
        #driver.EnableSoma(in_idx * 4 + 1)
        #driver.EnableSoma(in_idx * 4 + 2)
        #driver.EnableSoma(in_idx * 4 + 2)
        #driver.EnableSoma(in_idx * 4 + 2)
        #driver.EnableSoma(in_idx * 4 + 2)
        #driver.EnableSoma(in_idx * 4 + 3)
        #driver.EnableSoma(in_idx * 4 + 3)
        #driver.EnableSoma(in_idx * 4 + 3)
        #driver.EnableSoma(in_idx * 4 + 3)
        #driver.FlushBDBuffer()

        driver.EnableSynapse(in_idx)
        driver.EnableSynapse(in_idx)
        driver.EnableSynapse(in_idx)
        driver.EnableSynapse(in_idx)
        driver.FlushBDBuffer()

        def CreateSpikePacket(address, sign):
            from PyRawDriver.Driver import ConfigMemory  as cm
            _aer_address = cm.__syn_xyflat_to_aerflat__[address]
            from PyDriver.pystorm.bddriver import PackWord
            from PyDriver.pystorm.bddriver import InputSpike
            config_word = PackWord([
                (InputSpike.SYNAPSE_ADDRESS, _aer_address),
                (InputSpike.SYNAPSE_SIGN, sign),
            ])
            return config_word
        
        def SendSpike(address, sign):
            config_word = CreateSpikePacket(address, sign)
            driver.BufferBDWord(bd.HORN.NeuronInject, config_word)
            driver.BufferBDWord(bd.HORN.NeuronInject, config_word)
            driver.BufferBDWord(bd.HORN.NeuronInject, config_word)
            driver.BufferBDWord(bd.HORN.NeuronInject, config_word)
            driver.FlushBDBuffer()
            
        spike_word = CreateSpikePacket(in_idx, 1)
        spike_bd_word = driver.__create_bd_word__(bd.HORN.CreateInputWord(bd.HORN.NeuronInject, spike_word))
        spike_exc = driver.__make_byte_array__([spike_bd_word] * 4)

        spike_word = CreateSpikePacket(in_idx, 0)
        spike_bd_word = driver.__create_bd_word__(bd.HORN.CreateInputWord(bd.HORN.NeuronInject, spike_word))
        spike_inh = driver.__make_byte_array__([spike_bd_word] * 4)
        
        
        def step_braindrop_spikes(t):
            # NOTE: this gets called every timestep and should return
            #  a vector of length n_neurons that is non-zero for every
            #  neuron that has spiked since the last time this was called.
            #  A larger numerical value will make the spikes brighter
            #  in the GUI
            _exc = False
            if _exc:
                driver.dev.WriteToBlockPipeIn(driver.EP_DN, driver.BLOCK_SIZE, spike_exc)
            else:
                driver.dev.WriteToBlockPipeIn(driver.EP_DN, driver.BLOCK_SIZE, spike_inh)

            self.spikes[:] = 0  # reset everything to zeros
            
            w = driver.ReceiveWords()
            nrn_idx = []
            
            for i in range(0, len(w), 8):
                _x, _y = AER_TO_XY[w[i] + w[i + 1] * 256]
                nrn_idx.append(_x + 64 *_y)
                
            self.spikes[nrn_idx] = 10.0
            
            return self.spikes
        return step_braindrop_spikes

model = nengo.Network()
with model:
    braindrop = nengo.Node(BrainDropSpikes(n_neurons=4096))
    
    
    def subsample(t, x):
        subsample._nengo_spike_node_ = True
        return x[250:260]
    subset = nengo.Node(subsample, size_in=4096)
    nengo.Connection(braindrop, subset, synapse=None)