import numpy as np
from copy import copy
from bitutils import *
from Resources import *

class NetObj(object):

    def __repr__(self):
        return self.name

    def MapResources(self):
        pass # no default impl

    def ConnectResources(self):
        for name, R in self.R.iteritems():
            if isinstance(R, list):
                for R_el in R:
                    R_el.tgts = NetTgtToResTgt(R_el.tgts)
            else:
                R.tgts = NetTgtToResTgt(R.tgts)

    def InitRates(self, n_iter):
        self.rates = np.zeros((n_iter, self.D))

    def UpdateRates(self, tidx):
        pass # this is fine for guys that just sink spikes or source spikes

    def GetResourceRates(self, tidx):
        return None

class Node(NetObj):
    idx = 0

    def __init__(self, D, name=''):
        self.D = D
        self.islocal = None
        self.input_obj = None

        self.idx = Node.idx
        Node.idx += 1
        self.name = "Node" + str(self.idx) + ' ' * (name != '') + name

        # connections
        self.conns = []
        self.Ws = []

        # resources
        self.R = {}

    def Connect(self, tgt, W=None):
        if W is None:
            assert self.D == tgt.D
        else:
            assert W.shape[0] == tgt.D
        self.conns.append(tgt)
        self.Ws.append(W)

    def MapResources(self):
        self.R['trans'] = [Matrix(W, False, [tgt]) for W, tgt in zip(self.Ws, self.conns) if W is not None]
        self.R['trans_acc'] = [ACC([t]) for t in self.R['trans']]

        tgts = copy(self.R['trans_acc']) # we know the resources for our own transforms already
        for W, tgt in zip(self.Ws, self.conns):
            if W is None: # we already have the ones that aren't None
                tgts += [tgt] # we don't know for the arbitrary fanouts

        if len(tgts) == 1:
            self.input_obj = tgts[0] # this can be a NetObj, in which case this is a bypass
        elif len(tgts) > 1:
            self.R['fo'] = FO(tgts)
            self.input_obj = self.R['fo']
        else:
            assert False

    def UpdateRates(self, tidx):
        for W, tgt in zip(self.Ws, self.conns):
            if W is not None:
                tgt.rates[tidx+1] += W.dot(self.rates[tidx])
            else:
                tgt.rates[tidx+1] += self.rates[tidx]

class Pool(NetObj):
    idx = 0

    def __init__(self, taps, dec, name=''):
        self.taps = taps
        self.dec = dec
        self.N = dec.shape[1]
        self.D = taps.shape[1]
        self.DO = dec.shape[0]
        self.islocal = None
        self.input_obj = None

        self.idx = Pool.idx
        Pool.idx += 1
        self.name = "Pool" + str(self.idx) + ' ' * (name != '') + name

        # connections
        self.conns = []
        self.Ws = []

        # resources
        self.R = {}

        # for simulation
        self.spk_rate = 1.

    def Connect(self, tgt, W=None):
        if W is None:
            assert self.DO == tgt.D
        else:
            assert W.shape[0] == tgt.D

        self.conns.append(tgt)
        self.Ws.append(W)

    def MapResources(self):
        self.R['arr'] = NRNArray(self.N)
        self.R['dec'] = Matrix(self.dec, True)
        self.R['pat'] = PAT(self.R['arr'], self.R['dec'])
        taps = np.abs(self.taps)
        tap_signs = ((-np.sign(self.taps)).astype(int) + 1)/2
        self.R['enc'] = NRN(taps, tap_signs, self.R['arr'])

        self.R['trans'] = [Matrix(W, False, [tgt]) for W, tgt in zip(self.Ws, self.conns) if W is not None]
        self.R['trans_acc'] = [ACC([t]) for t in self.R['trans']]

        tgts = copy(self.R['trans_acc']) # we know the resources for our own transforms already
        for W, tgt in zip(self.Ws, self.conns):
            if W is None: # we already have the ones that aren't None
                tgts += [tgt] # we don't know for the arbitrary fanouts

        if len(tgts) == 1:
            self.R['dec'].tgts = tgts
        elif len(tgts) > 1:
            self.R['fo'] = FO(tgts)
            self.R['dec'].tgts = [self.R['fo']]
        else:
            assert False

        self.input_obj = self.R['enc']

    def UpdateRates(self, tidx):
        # don't have to do anything with the encoder inputs, those just get sunk
        ninp = self.spk_rate * np.ones((self.N,1))
        dec = self.dec.dot(ninp)
        for W, tgt in zip(self.Ws, self.conns):
            if W is not None:
                tgt.rates[tidx + 1] += W.dot(dec).flatten()
            else:
                tgt.rates[tidx + 1] += dec.flatten()

    def GetResourceRates(self, tidx):
        rates = self.rates[tidx]
        tap_rates = {}
        for d in xrange(rates.shape[0]):
            r = rates[d]
            taps = self.R['enc'].taps[:,d]
            signs = self.R['enc'].signs[:,d]
            mapped_taps = self.R['enc'].mapped_taps[:,d]
            for t, s, t_mapped in zip(taps, signs, mapped_taps):
                if t_mapped not in tap_rates:
                    tap_rates[t_mapped] = 0
                tap_rates[t_mapped] += r * (-2*s + 1)
        return tap_rates


class Probe(NetObj):
    idx = 0

    def __init__(self, D, name=''):
        self.D = D
        self.R = {}

        self.idx = Probe.idx
        Probe.idx += 1
        self.name = "Probe" + str(self.idx) + ' ' * (name != '') + name

    def MapResources(self):
        self.R['sink'] = Sink(self.D)
        self.input_obj = self.R['sink']

    def GetResourceRates(self, tidx):
        tags = self.R['sink'].in_tags
        #return {t:r for t,r, in zip(tags, self.rates[tidx])}
        rval = {}
        for t,r, in zip(tags, self.rates[tidx]):
          rval[t] = r
        return rval


class Input(NetObj):
    idx = 0
    def __init__(self, D, name=''):
        self.D = D
        self.R = {}

        self.idx = Input.idx
        Input.idx += 1
        self.name = "Input" + str(self.idx) + ' ' * (name != '') + name

        # connections
        self.conns = []

        # resources
        self.R = {}

        # for rate simulation
        self.dim_rate = 10.

    def Connect(self, tgt):
        assert self.D == tgt.D
        self.conns.append(tgt)

    def MapResources(self):
        self.R['src'] = Source(self.D, self.conns)

    def UpdateRates(self, tidx):
        dinp = np.ones((self.D,)) * self.dim_rate
        self.rates[tidx] += dinp
        for tgt in self.conns:
            tgt.rates[tidx + 1] += self.rates[tidx]


class Network(object):

    def __init__(self, net_objs):
        self.net_objs = net_objs

    def __repr__(self):
        string = ''
        for obj in self.net_objs:
            string += obj.__repr__() + '\n'
        return string
    
    def FlattenResources(self):
        R = {}
        for obj in self.net_objs:
            for k, v in obj.R.iteritems():
                if v == []: # get rid of the empty-list resources here
                    pass
                elif isinstance(v, list):
                    for idx, v_el in enumerate(v):
                        R[obj.name + '_' + k + '_' + str(idx)] = v_el
                else:
                    R[obj.name + '_' + k] = v
        return R

    def Synthesize(self):
        for obj in self.net_objs:
            obj.MapResources()
        for obj in self.net_objs:
            obj.ConnectResources()
        return self.FlattenResources()

    def SimulateRates(self, inputs, n_iter):
        vals = {}
        for obj in self.net_objs:
            obj.InitRates(n_iter)
        for it in xrange(n_iter-1):
            for k,v in inputs.iteritems():
                vals[k] += v # add in inputs before each step
            for obj in self.net_objs:
                obj.UpdateRates(it)
    
    def GetResourceRates(self, tidx):
        all_rates = {}
        for obj in self.net_objs:
            rates = obj.GetResourceRates(tidx)
            if rates is not None:
                all_rates[obj.name] = rates
        return all_rates


def NetTgtToResTgt(tgts):
    # get all resource targets
    if tgts is not None:
        res_tgts = []
        for tgt in tgts:
            if isinstance(tgt, NetObj):
                res = tgt.input_obj
                while isinstance(res, NetObj): # sometimes this happens with "passthrough" Nodes
                    res = res.input_obj
            elif isinstance(tgt, Resource):
                res = tgt
            else:
                assert False
            assert res is not None # could signify an unspecified input_obj
            res_tgts += [res]
        assert sum([isinstance(rt, Resource) for rt in res_tgts]) == len(res_tgts)
        assert len(res_tgts) != 0
        tgts = res_tgts
    return tgts


