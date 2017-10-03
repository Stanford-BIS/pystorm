from . _PyOK import *

ErrorNames = dict({
      0 : 'NoError'             ,
     -1 : 'Failed'              ,
     -2 : 'Timeout'             ,
     -3 : 'DoneNotHigh'         ,
     -4 : 'TransferError'       ,
     -5 : 'CommunicationError'  ,
     -6 : 'InvalidBitstream'    ,
     -7 : 'FileError'           ,
     -8 : 'DeviceNotOpen'       ,
     -9 : 'InvalidEndpoint'     ,
    -10 : 'InvalidBlockSize'    ,
    -11 : 'I2CRestrictedAddress',
    -12 : 'I2CBitError'         ,
    -13 : 'I2CNack'             ,
    -14 : 'I2CUnknownStatus'    ,
    -15 : 'UnsupportedFeature'  ,
    -16 : 'FIFOUnderflow'       ,
    -17 : 'FIFOOverflow'        ,
    -18 : 'DataAlignmentError'  ,
    -19 : 'InvalidResetProfile' ,
    -20 : 'InvalidParameter'
})


def InitOK(fpga_bitcode):
    """
    Initialize OpalKelly board and return a handler to the FrontPanel
    """
    dev = okCFrontPanel()
    m_devInfo = okTDeviceInfo()

    if okFrontPanelDLL_LoadLib(None) is False:
        print("FrontPanel library could not be loaded");
        return None

    lib_date, lib_time = okFrontPanelDLL_GetVersion()
    print("FrontPanel library loaded")
    print("Built: %s, %s" % (lib_date, lib_time))

    if dev.OpenBySerial("") != ErrorCode.NoError:
        print("Device could not be opened. Is one connected?")
        return None

    dev.GetDeviceInfo(m_devInfo)
    print("Found a device: %s" % m_devInfo.productName)

    dev.LoadDefaultPLLConfiguration()

    print("Device firmware version: %d.%d" % (m_devInfo.deviceMajorVersion, m_devInfo.deviceMinorVersion))
    print("Device serial number: %s" % m_devInfo.serialNumber)
    print("Device product ID: %d" % m_devInfo.productID)

    if dev.ConfigureFPGA(fpga_bitcode) != ErrorCode.NoError:
        print("FPGA configuration failed.")
        return None

    if dev.IsFrontPanelEnabled():
        print("FrontPanel support is enabled")
    else:
        print("FrontPanel support is not enabled")
        return None
    return dev