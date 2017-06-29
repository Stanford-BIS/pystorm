from enum import Enum

# XXX this isn't permanent implementation
# the boost objects for the C++ BDPars.h enums should use the same syntax as this

class AMField(Enum):
    ACCUMULATOR_VALUE = 0
    THRESHOLD = 1
    STOP = 2
    NEXT_ADDRESS = 3

class MMField(Enum):
    WEIGHT = 0

class PATField(Enum):
    AM_ADDRESS = 0
    MM_ADDRESS_LO = 1
    MM_ADDRESS_HI = 2

class TATAccField(Enum):
    STOP = 0
    FIXED_0 = 1
    AM_ADDRESS = 2
    MM_ADDRESS_LO = 3
    MM_ADDRESS_HI = 4

class TATSpikeField(Enum):
    STOP = 0
    FIXED_1 = 1
    SYNAPSE_ADDRESS_0 = 1
    SYNAPSE_SIGN_0 = 2
    SYNAPSE_ADDRESS_1 = 3
    SYNAPSE_SIGN_1 = 4
    UNUSED = 5

class TATTagField(Enum):
    STOP = 0
    FIXED_2 = 1
    TAG = 2
    GLOBAL_ROUTE = 3
    UNUSED = 4

