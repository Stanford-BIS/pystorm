def main():
    import PyOK as ok
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

    if dev.ConfigureFPGA("counters.rbf") != ok.ErrorCode.NoError:
        print("FPGA configuration failed.")
        return -3

    if dev.IsFrontPanelEnabled():
        print("FrontPanel support is enabled")
    else:
        print("FrontPanel support is not enabled")
        return -4

if __name__ ==  "__main__":
    main();
