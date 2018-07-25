from . _PyOK import *

import loggging

logger = logging.(__name__)

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
        logger.critical("FrontPanel library could not be loaded");
        return None

    lib_date, lib_time = okFrontPanelDLL_GetVersion()
    logger.info("FrontPanel library loaded")
    logger.info("Built: %s, %s" % (lib_date, lib_time))

    if dev.OpenBySerial("") != ErrorCode.NoError:
        logger.critical("Device could not be opened. Is one connected?")
        return None

    dev.GetDeviceInfo(m_devInfo)
    logger.info("Found a device: %s" % m_devInfo.productName)

    dev.LoadDefaultPLLConfiguration()

    logger.info("Device firmware version: %d.%d" % (m_devInfo.deviceMajorVersion, m_devInfo.deviceMinorVersion))
    logger.info("Device serial number: %s" % m_devInfo.serialNumber)
    logger.info("Device product ID: %d" % m_devInfo.productID)

    if dev.ConfigureFPGA(fpga_bitcode) != ErrorCode.NoError:
        logger.critical("FPGA configuration failed.")
        return None

    if dev.IsFrontPanelEnabled():
        logger.info("FrontPanel support is enabled")
    else:
        logger.critical("FrontPanel support is not enabled")
        return None
    return dev


def GetFPGASerialNumber():
    dev = okCFrontPanel()
    m_devInfo = okTDeviceInfo()

    if dev.OpenBySerial("") != ErrorCode.NoError:
        logger.critical("Device could not be opened. Is one connected?")
        return None

    dev.GetDeviceInfo(m_devInfo)
    logger.info("Received FPGA Serial Number: %s" % m_devInfo.serialNumber)
    return m_devInfo.serialNumber


