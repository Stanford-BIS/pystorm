"""
depth, route(bool), route(int), data width, serialization, chunk width, purpose
From Wiki: https://ng-hippocampus.stanford.edu/wiki/Funnel_and_Horn#Funnel
"""
CORE_TO_PIN_WORD_LENGTH = 34

#from enum import Enum
#FIELD = Enum('FIELD', (
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
#)
#)

class DUMP_AM():
	pass
class DUMP_MM():
	pass
class DUMP_PAT():
	pass
class DUMP_POST_FIFO():
	pass
class DUMP_PRE_FIFO():
	pass
class DUMP_TAT():
	pass
class NRNI():
	pass
class OVFLW():
	pass
class RO_ACC():
	pass
class RO_TAT():
	pass

IDS = dict({
DUMP_AM           : ((6, "101000" , 40, 38, 2, 19, "AM diagnostic read output"), ),
DUMP_MM           : ((6, "101001" , 41, 8 , 1, 8 , "MM diagnostic read output"), ),
DUMP_PAT          : ((5, "10101"  , 21, 20, 1, 20, "PAT diagnostic read output"), ),
DUMP_POST_FIFO    : (
                            (6, "101110" , 46, 19, 1, 19, "copy of tag class 0 traffic exiting FIFO"),
                            (6, "101111" , 47, 19, 1, 19, "copy of tag class 1 traffic exiting FIFO")
),
DUMP_PRE_FIFO     : ((6, "101101" , 45, 20, 1, 20, "copy of traffic entering FIFO"), ),
DUMP_TAT          : (
                            (4, "1000"   , 8 , 29, 1, 29, "TAT 0 diagnostic read output"),
                            (4, "1001"   , 9 , 29, 1, 29, "TAT 1 diagnostic read output")
),
NRNI              : ((2, "11"     , 3 , 12, 1, 12, "copy of traffic exiting neuron array"), ),
OVFLW             : (
                            (7, "1011000", 88, 1 , 1, 1 , "class 0 FIFO overflow warning"),
                            (7, "1011001", 89, 1 , 1, 1 , "class 1 FIFO overflow warning")
),
RO_ACC            : ((2, "01"     , 1 , 28, 1, 28, "tag output from accumulator"), ),
RO_TAT            : ((2, "00"     , 0 , 32, 1, 32, "tag output from TAT"), ),
})

def GetOutputWord(data):
    """
    Decodes BD parallel data to the appropriate funnel's payload
    :param data:
    :return: (Leaf ID, Sub-leaf, Payload (binary), Payload (int))
    """
    _data = None
    if(isinstance(data, str)):
        if(data[1] == '0' or data[1] == '1'):
            _data = data
        else:
            _data = ("{0:0%db}" % CORE_TO_PIN_WORD_LENGTH).format(int(data, 0))
    else:
        _data = ("{0:0%db}" % CORE_TO_PIN_WORD_LENGTH).format(data)

    _route = None
    _subroute = None
    for key, value in IDS.items():
        for _idx, _rf in enumerate(value):
            _froute = _rf[1]
            _rlen = len(_froute)
            if(_froute == data[:_rlen]):
                _route = key
                _subroute = _idx
                break
            
        if _route is not None:
            break
            
    _rfield = IDS[_route][_subroute]
    _payload_len = _rfield[3]
    _payload = _data[-_payload_len:]
    
    return({
        'leaf'        : _route,
        'sub_leaf'    : _subroute,
        'bin_payload' : _payload,
        'int_payload' : int(_payload, 2)
    })