"""
depth , route(bool) , route(int) , data width , serialization , chunk width , purpose
From Wiki: https://ng-hippocampus.stanford.edu/wiki/Funnel_and_Horn#Routing_Tables
"""
PIN_TO_CORE_WORD_LENGTH = 21

ADC               = (4, "1010"    , 10 , 3 , 1, 3 , "[ small(0)/large(1) current toggle for ADC 0 | small(0)/large(1) current toggle for ADC 1 | output on(1)/off(0) ]")
DAC_DIFF_G        = (8, "10110000", 176, 11, 1, 11, "DIFF_G DAC bias value")
DAC_SYN_INH       = (7, "1011101" , 93 , 11, 1, 11, "SYN_INH DAC bias value")
DAC_SYN_PU        = (7, "1011110" , 94 , 11, 1, 11, "SYN_PU DAC bias value")
DAC_UNUSED        = (7, "1011111" , 95 , 11, 1, 11, "UNUSED ('ghost DAC')")
DAC_DIFF_R        = (8, "10110001", 177, 11, 1, 11, "DIFF_R DAC bias value")
DAC_SOMA_OFFSET   = (8, "10110010", 178, 11, 1, 11, "SOMA_OFFSET DAC bias value")
DAC_SYN_LK        = (8, "10110011", 179, 11, 1, 11, "SYN_LK DAC bias value")
DAC_SYN_DC        = (8, "10110100", 180, 11, 1, 11, "SYN_DC DAC bias value")
DAC_SYN_PD        = (8, "10110101", 181, 11, 1, 11, "SYN_PD DAC bias value")
DAC_ADC_2         = (8, "10110110", 182, 11, 1, 11, "ADC_BIAS_2 DAC bias value")
DAC_ADC_1         = (8, "10110111", 183, 11, 1, 11, "ADC_BIAS_1 DAC bias value")
DAC_SOMA_REF      = (8, "10111000", 184, 11, 1, 11, "SOMA_REF DAC bias value")
DAC_SYN_EXC       = (8, "10111001", 185, 11, 1, 11, "SYN_EXC DAC bias value")
DELAY_DCTFIFO     = (7, "1000000" , 64 , 8 , 1, 8 , "FIFO:DCT delay line config")
DELAY_PGFIFO      = (7, "1000010" , 66 , 8 , 1, 8 , "FIFO:PG delay line config")
DELAY_TAT0        = (8, "10001110", 142, 8 , 1, 8 , "TAT 0 delay line config")
DELAY_TAT1        = (8, "10001111", 143, 8 , 1, 8 , "TAT 1 delay line config")
DELAY_PAT         = (6, "100100"  , 36 , 8 , 1, 8 , "PAT delay line config")
DELAY_MM          = (7, "1001100" , 76 , 8 , 1, 8 , "MM delay line config")
DELAY_AM          = (7, "1001101" , 77 , 8 , 1, 8 , "AM delay line config")
INIT_FIFO_DCT     = (7, "1000110" , 70 , 11, 1, 11, "inserts a tag into the DCT side of the FIFO with ct                                                            = (1 needed to clean initial FIFO state")
INIT_FIFO_HT      = (8, "10001000", 136, 1 , 1, 1 , "trigger sets FIFO head/tail register to empty state")
NeuronConfig      = (3, "110"     , 6  , 18, 1, 18, "programming input for neuron array tile SRAM")
NeuronDumpToggle  = (4, "1111"    , 15 , 2 , 1, 2 , "toggles data/dump traffic for neuron array output")
NeuronInject      = (4, "1110"    , 14 , 11, 1, 11, "direct spike injection to neuron array")
PROG_AMMM         = (6, "100111"  , 39 , 42, 4, 11, "AM/MM programming/diagnostic port")
PROG_PAT          = (6, "100101"  , 37 , 27, 4, 7 , "PAT programming/diagnostic port")
PROG_TAT0         = (7, "1000001" , 65 , 31, 4, 8 , "TAT 0 programming/diagnostic port")
PROG_TAT1         = (7, "1000011" , 67 , 31, 4, 8 , "TAT 1 programming/diagnostic port")
RI                = (1, "0"       , 0  , 20, 1, 20, "main tag input to FIFO")
TOGGLE_POST_FIFO0 = (8, "10001010", 138, 2 , 1, 2 , "toggles data/dump traffic for FIFO tag class 0 output")
TOGGLE_POST_FIFO1 = (8, "10001011", 139, 2 , 1, 2 , "toggles data/dump traffic for FIFO tag class 1 output")
TOGGLE_PRE_FIFO   = (8, "10001001", 137, 2 , 1, 2 , "toggles data/dump traffic for FIFO input")


def __create_input_word__(horn_route, payload):
    """
    Input word to Horn
    :param horn_route: Horn leaf ID
    :param payload: payload as integer or a base-prefixed string (e.g., '0bxxx')
    :return: binary string (without prefix) representing the HORN word
    """
    _payload_len = horn_route[3]
    _route = horn_route[1][::-1]
    _route_len = len(_route)
    _x_len = PIN_TO_CORE_WORD_LENGTH - (_payload_len + _route_len)

    if isinstance(payload, str):
        if payload[1] == '0' or payload[1] == '1':
            _payload = int(payload, 2)
        else:
            _payload = int(payload, 0)
    else:
        _payload = payload

    if _payload > int(''.join(['1' for idx in range(_payload_len)]), 2):
        raise AssertionError("Payload '%d' exceeds allowed bit length of '%d'" % (_payload, _payload_len))

    if _x_len > 0:
        input_word = ("{0:0%db}" % _x_len).format(0) + ("{0:0%db}" % _payload_len).format(_payload) + _route
    else:
        input_word = ("{0:0%db}" % _payload_len).format(_payload) + _route

    return input_word


def CreateInputWord(arg, *args):
    if isinstance(arg, dict):
        return __create_input_word__(arg['leaf_id'], arg['payload'])
    else:
        return __create_input_word__(arg, *args)
