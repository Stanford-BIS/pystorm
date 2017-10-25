"""
depth, route(bool), route(int), data width, serialization, chunk width, purpose
From Wiki: https://ng-hippocampus.stanford.edu/wiki/Funnel_and_Horn#Funnel
"""
CORE_TO_PIN_WORD_LENGTH = 34


# from enum import Enum
# FIELD = Enum('FIELD', (
#    'DUMP_AM '
#    'DUMP_MM '
#    'DUMP_PAT '
#    'DUMP_POST_FIFO '
#    'DUMP_PRE_FIFO '
#    'DUMP_TAT '
#    'NRNI '
#    'OVFLW '
#    'RO_ACC '
#    'RO_TAT '
# )
# )

class DUMP_AM():
    pass


class DUMP_MM():
    pass


class DUMP_PAT():
    pass


class DUMP_POST_FIFO0():
    pass


class DUMP_POST_FIFO1():
    pass


class DUMP_PRE_FIFO():
    pass


class DUMP_TAT0():
    pass


class DUMP_TAT1():
    pass


class NRNI():
    pass


class OVFLW0():
    pass


class OVFLW1():
    pass


class RO_ACC():
    pass


class RO_TAT():
    pass


IDS = dict({
#                                    data       ser. data
#                  route             width  ser.  width    description
DUMP_AM         : ("100100000000000", 38,    2,    19,     "AM diagnostic read output"),
DUMP_MM         : ("100100000000001", 8 ,    1,    8 ,     "MM diagnostic read output"),
DUMP_PAT        : ("10010000000001" , 20,    1,    20,     "PAT diagnostic read output"),
DUMP_POST_FIFO0 : ("100100000001100", 19,    1,    19,     "copy of tag class 0 traffic exiting FIFO"),
DUMP_POST_FIFO1 : ("100100000001101", 19,    1,    19,     "copy of tag class 1 traffic exiting FIFO"),
DUMP_PRE_FIFO   : ("10010000000101" , 20,    1,    20,     "copy of traffic entering FIFO"),
DUMP_TAT0       : ("10000"          , 29,    1,    29,     "TAT 0 diagnostic read output"),
DUMP_TAT1       : ("10001"          , 29,    1,    29,     "TAT 1 diagnostic read output"),
NRNI            : ("101"            , 12,    1,    12,     "copy of traffic exiting neuron array"),
OVFLW0          : ("100100000001000", 1 ,    1,    1 ,     "class 0 FIFO overflow warning"),
OVFLW1          : ("100100000001001", 1 ,    1,    1 ,     "class 1 FIFO overflow warning"),
RO_ACC          : ("01"             , 28,    1,    28,     "tag output from accumulator"),
RO_TAT          : ("00"             , 32,    1,    32,     "tag output from TAT"),
})

# only one output has a serializer, the DUMP_AM word, which is 38 bits, but comes out as two 19-bit pieces
# this circuit doesn't seem to work anyway

def GetOutputWord(data):
    """
    Decodes BD parallel data to the appropriate funnel's payload
    :param data:
    :return: (Leaf ID, Sub-leaf, Payload (binary), Payload (int))
    """
    if isinstance(data, str):
        if data[1] == '0' or data[1] == '1':
            _data = data
        else:
            _data = ("{0:0%db}" % CORE_TO_PIN_WORD_LENGTH).format(int(data, 0))
    else:
        _data = ("{0:0%db}" % CORE_TO_PIN_WORD_LENGTH).format(data)

    _route = None
    for key, value in IDS.items():
        _froute = value[0]
        _rlen = len(_froute)
        if _froute == data[:_rlen]:
            _route = key
            break

    _rfield = IDS[_route]
    _payload_len = CORE_TO_PIN_WORD_LENGTH - _rlen
    _payload = _data[-_payload_len:]

    return ({
        'leaf': _route,
        'bin_payload': _payload,
        'int_payload': int(_payload, 2)
    })
