import numpy as np
from hardware_resources import (
    AMBuckets, MMWeights, Neurons, Sink, Source, TATAccumulator, TATFanout, TATTapPoint)

R = []

# neurons -> weights -> bucket
def GetN1():
    D = 2
    My = 10
    Mx = 20
    M = My*Mx

    N = Neurons(My, Mx)
    W = MMWeights(np.random.rand(D, M))
    B = AMBuckets(D)

    N.connect(W)
    W.connect(B)

    return [N, W, B]

# (neurons -> weights -> bucket) *2
def GetN2():
    return GetN1() + GetN1()

# source -> TATTapPoint -> neurons -> weights -> bucket -> sink
def GetN3():
    D = 2
    K = 2
    My = 10
    Mx = 20
    M = My*Mx

    I = Source(D)
    TP = TATTapPoint(np.random.randint(M, size=(K, D)), np.random.randint(1, size=(K, D))*2 - 1, M)
    N = Neurons(My, Mx)
    W = MMWeights(np.random.rand(D, M))
    B = AMBuckets(D)
    O = Sink(D)

    I.connect(TP)
    TP.connect(N)
    N.connect(W)
    W.connect(B)
    B.connect(O)

    return [I, TP, N, W, B, O]

#         source -> TATAccumulator -> weights ->
# source -> TATTapPoint -> neurons -> weights -> bucket -> TATFanout -> sink
#                                                                    -> sink
def GetN4():
    D = 2
    K = 2
    My = 10
    Mx = 20
    M = My*Mx

    I1 = Source(D)
    TP = TATTapPoint(np.random.randint(M, size=(K, D)), np.random.randint(1, size=(K, D))*2 - 1, M)
    N = Neurons(My, Mx)
    W1 = MMWeights(np.random.rand(D, M))
    B = AMBuckets(D)
    O1 = Sink(D)

    I2 = Source(D)
    TA = TATAccumulator(D)
    W2 = MMWeights(np.random.rand(D, D))

    TF = TATFanout(D)
    O2 = Sink(D)

    I1.connect(TP)
    TP.connect(N)
    N.connect(W1)
    W1.connect(B)
    B.connect(TF)
    TF.connect(O1)

    I2.connect(TA)
    TA.connect(W2)
    W2.connect(B)

    TF.connect(O2)

    return [I1, I2, TP, TA, TF, N, W1, W2, B, O1, O2]

if __name__ == "__main__":

    print("creating Resource graphs")
    print("all this is testing is that no assertions are thrown, expect no further output")

    N1 = GetN1()
    N2 = GetN2()
    N3 = GetN3()
    N4 = GetN4()
