from Resources import *

R = []

# weights -> bucket
def GetN1():
    D = 2

    W = MMWeights(np.random.rand(D, D))
    B = AMBuckets(D)

    W.Connect(B)

    return [W, B]

# neurons -> weights -> bucket
def GetN2():
    D = 2
    M = 10

    N = Neurons(M)
    W = MMWeights(np.random.rand(D, M))
    B = AMBuckets(D)

    N.Connect(W)
    W.Connect(B)

    return [N, W, B]

# source -> TATTapPoint -> neurons -> weights -> bucket -> sink
def GetN3():
    D = 2
    K = 2
    M = 10
    
    I = Source(D)
    TP = TATTapPoint(np.random.randint(M, size=(K, D)), np.random.randint(1, size=(K, D))*2 - 1, M)
    N = Neurons(M)
    W = MMWeights(np.random.rand(D, M))
    B = AMBuckets(D)
    O = Sink(D)

    I.Connect(TP)
    TP.Connect(N)
    N.Connect(W)
    W.Connect(B)
    B.Connect(O)

    return [I, TP, N, W, B, O]

#         source -> TATAccumulator -> weights -> 
# source -> TATTapPoint -> neurons -> weights -> bucket -> TATFanout -> sink
#                                                                    -> sink
def GetN4():
    D = 2
    K = 2
    M = 10
    
    I1 = Source(D)
    TP = TATTapPoint(np.random.randint(M, size=(K, D)), np.random.randint(1, size=(K, D))*2 - 1, M)
    N = Neurons(M)
    W1 = MMWeights(np.random.rand(D, M))
    B = AMBuckets(D)
    O1 = Sink(D)

    I2 = Source(D)
    TA = TATAccumulator(D)
    W2 = MMWeights(np.random.rand(D, D))

    TF = TATFanout(D)
    O2 = Sink(D)

    I1.Connect(TP)
    TP.Connect(N)
    N.Connect(W1)
    W1.Connect(B)
    B.Connect(TF)
    TF.Connect(O1)

    I2.Connect(TA)
    TA.Connect(W2)
    W2.Connect(B)
    
    TF.Connect(O2)

    return [I1, I2, TP, TA, TF, N, W1, W2, B, O1, O2]


N1 = GetN1()
N2 = GetN2()
N3 = GetN3()
N4 = GetN4()
