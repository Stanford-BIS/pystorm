import sys
sys.path.append("../../../../lib/")
sys.path.append("../../../../lib/Release")
sys.path.append("../../../../lib/Release/PyOK")

from enum import Enum
import PyOK as ok

class OP():

    BDHORN          = '00'
    FPGA_REG        = '10'
    FPGA_CHANNEL    = '11'

    BD_LEAF = Enum('BDLeafId',[
        'ADC',
        'DAC0',
        'DAC10',
        'DAC11',
        'DAC12',
        'DAC1',
        'DAC2',
        'DAC3',
        'DAC4',
        'DAC5',
        'DAC6',
        'DAC7',
        'DAC8',
        'DAC9',
        'DELAY0',
        'DELAY1',
        'DELAY2',
        'DELAY3',
        'DELAY4',
        'DELAY5',
        'DELAY6',
        'INIT_FIFO_DCT',
        'INIT_FIFO_HT',
        'NEURONCONFIG',
        'NEURONDUMPTOGGLE',
        'NEURONINJECT',
        'PROG_AMMM',
        'PROG_PAT',
        'PROG_TAT0',
        'PROG_TAT1',
        'RI',
        'TOGGLE_POST_FIFO0',
        'TOGGLE_POST_FIFO1',
        'TOGGLE_PRE_FIFO',
        'INVALID'
        ], start=0)

    def endpointword(opcode, subcode, payload):
        _op1 = getattr(OP, opcode)
        _op2 = None
        _val = "{0:024b}".format(int(payload, 0))
        if opcode == 'BDHORN':
            _op2 = "{0:06b}".format(OP.BD_LEAF[subcode].value)
        else:
            _op2 = "{0:06b}".format(int(subcode, 0))
        return int('0b' + _op1 + _op2 + _val, 0).to_bytes(4, byteorder='little')

def __default_commands__():
    OPS = [
        ['BDHORN', 'DAC0', '0x000'],
        ['BDHORN', 'DAC1', '0x000'],
        ['BDHORN', 'DAC2', '0x000'],
        ['BDHORN', 'DAC3', '0x000'],
        ['BDHORN', 'DAC4', '0x000'],
        ['BDHORN', 'DAC5', '0x000'],
        ['BDHORN', 'DAC6', '0x000'],
        ['BDHORN', 'DAC7', '0x000'],
        ['BDHORN', 'DAC8', '0x000'],
        ['BDHORN', 'DAC9', '0x000'],
        ['BDHORN', 'DAC10', '0x000'],
        ['BDHORN', 'DAC11', '0x000']
    ]
    return [OP.endpointword(*_item) for _item in OPS]


def parse_command_file(FH):
    lines = FH.read().splitlines()
    cmds = [l.split() for l in lines]
    return [OP.endpointword(*_item) for _item in cmds]


if __name__ ==  "__main__":
################################################################################
# Parse the command line arguments.
# Normally, don't have to modify this code.
################################################################################
    import argparse
    PARSER = argparse.ArgumentParser(description='Program Braindrop with OpalKelly FrontPanel interface', formatter_class=argparse.RawTextHelpFormatter)
    PARSER.add_argument('BITFILE', type=str, help="Bitfile to program the FPGA with")
    PARSER.add_argument('-f', '--cmdfile', metavar='FILE', type=argparse.FileType('r'), dest='command_file',
                        help="""File with downstream instructions.
Each line has three entries:

BDHORN|FPGA_REG|FPGA_CHANNEL <ID> <PAYLOAD>

<ID> is any BDLeafId enum for 'BDHORN' or
an integer for FPGA_REG and FPGA_CHANNEL.
<PAYLOAD> is a 24-bit number that can be
written in binary ('0bnnnn'), decimal or
hex ('0xnnnn') format.""")
    PARSER.add_argument('-i', '--ep_in', metavar='EP', default='0x08',
                        help="Downstream endpoint (PC->FPGA) (default=0x08)")
    PARSER.add_argument('-o', '--ep_out', metavar='EP', default='0xa0',
                        help="Upstream endpoint (PC<-FPGA) (default=0xa0)")
    PARSER.add_argument('-b', '--block', metavar='SIZE', default='512',
                        help="""Block size in bytes (default=512).
Must be a power of 2 with a minimum value of 16.
""")
    PARSER.add_argument('-v', action='store_true', dest='verbose',
                        help="Print payload (default=disabled)")
    ARGS = PARSER.parse_args()
    # `args.BITFILE`        : bitfile
    # `args.command_file`   : command file

    payload = None
    if ARGS.command_file is None:
        print("No command file specified. Using default commands.")
        payload = __default_commands__()
    else:
        print("Using command file.")
        payload = parse_command_file(ARGS.command_file)

################################################################################
# Parameters for Braindrop communication.
# Normally, don't have to modify this code.
################################################################################
    ep_in = int(ARGS.ep_in, 0)
    ep_out = int(ARGS.ep_out, 0)
    block_size = int(ARGS.block, 0)

    print("Configuration:")
    print("  FPGA bitflie: %s" % ARGS.BITFILE)
    print("  Downstream endpoint: 0x%X" % ep_in)
    print("  Upstream endpoint: 0x%X" % ep_out)
    print("  Block size: %d" % block_size)

    if ARGS.verbose is True:
        print("  Payload (lsB -> msB):")
        for idx, word in enumerate(payload):
            print(("    %d: " + ", ".join(list(("0x" + "{0:02X}".format(num) for num in list(word))))) % idx)

################################################################################
# Handle to OpalKelly FrontPanel
################################################################################

    dev = ok.InitOK(ARGS.BITFILE)

################################################################################
# Interactions with Braindrop begins here.
# Modify as required
################################################################################
    if dev is not None:
        dev.WriteToBlockPipeIn(ep_in, block_size, payload)
    else:
        print("Device is unavailable")