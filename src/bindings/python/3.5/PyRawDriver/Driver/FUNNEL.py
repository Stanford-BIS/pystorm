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
DUMP_AM         : "100100000000000"
DUMP_MM         : "100100000000001"
DUMP_PAT        : "10010000000001"
DUMP_POST_FIFO0 : "100100000001100"
DUMP_POST_FIFO1 : "100100000001101"
DUMP_PRE_FIFO   : "10010000000101"
DUMP_TAT0       : "10000"
DUMP_TAT1       : "10001"
NRNI            : "101"
OVFLW0          : "100100000001000"
OVFLW1          : "100100000001001"
RO_ACC          : "01"
RO_TAT          : "00"
})

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
        _froute = value
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
