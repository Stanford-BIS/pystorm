import sys
from enum import Enum
import PyOK as ok

sys.path.append("../../../../lib/")
sys.path.append("../../../../lib/Release")

ErrorCode = dict({
      0 : 'NoError'                    ,
     -1 : 'Failed'                     ,
     -2 : 'Timeout'                    ,
     -3 : 'DoneNotHigh'                ,
     -4 : 'TransferError'              ,
     -5 : 'CommunicationError'         ,
     -6 : 'InvalidBitstream'           ,
     -7 : 'FileError'                  ,
     -8 : 'DeviceNotOpen'              ,
     -9 : 'InvalidEndpoint'            ,
    -10 : 'InvalidBlockSize'           ,
    -11 : 'I2CRestrictedAddress'       ,
    -12 : 'I2CBitError'                ,
    -13 : 'I2CNack'                    ,
    -14 : 'I2CUnknownStatus'           ,
    -15 : 'UnsupportedFeature'         ,
    -16 : 'FIFOUnderflow'              ,
    -17 : 'FIFOOverflow'               ,
    -18 : 'DataAlignmentError'         ,
    -19 : 'InvalidResetProfile'        ,
    -20 : 'InvalidParameter'
})

def InitOK(fpga_bitcode="counter.rbf"):
    dev = ok.okCFrontPanel()
    m_devInfo = ok.okTDeviceInfo()

    if ok.okFrontPanelDLL_LoadLib(None) is False:
        print("FrontPanel library could not be loaded");
        return -1

    lib_date, lib_time = ok.okFrontPanelDLL_GetVersion()
    print("FrontPanel library loaded")
    print("Built: %s, %s" % (lib_date, lib_time))

    if dev.OpenBySerial("") != ok.ErrorCode.NoError:
        print("Device could not be opened. Is one connected?")
        return -2

    dev.GetDeviceInfo(m_devInfo)
    print("Found a device: %s" % m_devInfo.productName)

    dev.LoadDefaultPLLConfiguration()

    print("Device firmware version: %d.%d" % (m_devInfo.deviceMajorVersion, m_devInfo.deviceMinorVersion))
    print("Device serial number: %s" % m_devInfo.serialNumber)
    print("Device product ID: %d" % m_devInfo.productID)

    if dev.ConfigureFPGA(fpga_bitcode) != ok.ErrorCode.NoError:
        print("FPGA configuration failed.")
        return -3

    if dev.IsFrontPanelEnabled():
        print("FrontPanel support is enabled")
    else:
        print("FrontPanel support is not enabled")
        return -4


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


if __name__ ==  "__main__":
    import argparse
    PARSER = argparse.ArgumentParser(description='Program Braindrop with OpalKelly FrontPanel interface')
    PARSER.add_argument('BITFILE', type=str, help="Bitfile to program the FPGA with")
    PARSER.add_argument('-f', '--cmdfile', type=argparse.FileType('r'), dest='command_file',
                        help="File with downstream instructions.\nThis file has one 32-bit word per line.")
    ARGS = PARSER.parse_args()
    # `args.BITFILE`        : bitfile
    # `args.command_file`   : command file

    if ARGS.command_file is None:
        print("No command file specified. Using default commands...")
        print(__default_commands__())
    #InitOK(args.BITFILE)
