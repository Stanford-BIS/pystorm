#include "CommOK.h"

namespace pystorm {
namespace bddriver {
namespace comm {

int CommOK::Init(const std::string bitfile, const std::string serial) {
    char lib_date[32], lib_time[32];

    if (FALSE == okFrontPanelDLL_LoadLib(NULL)) {
        std::cout << "FrontPanel library could not be loaded" << std::endl;
        return -1;
    }

    okFrontPanelDLL_GetVersion(lib_date, lib_time);
    std::cout << "FrontPanel library loaded\n"
        << "Built: " << lib_date << ", " << lib_time << std::endl;

    okCFrontPanel dev;

	if (false == InitializeFPGA(bitfile, serial)) {
        std::cout << "FPGA could not be initialized" << std::endl;
        return -1;
	}

    if (false == InitializeUSB()) {
        std::cout << "USB3 interface not available." << std::endl;
        return -1;
    }

    return 0;
}

int CommOK::Write(long data_length, unsigned char* data) {
    return dev.WriteToPipeIn(PIPE_IN_ADDR, data_length, data);
}

int CommOK::Read(long data_length, unsigned char* data) {
    return dev.ReadFromPipeOut(PIPE_OUT_ADDR, data_length, data);
}

bool CommOK::InitializeFPGA(const std::string bitfile, const std::string serial) {
    if (okCFrontPanel::NoError != dev.OpenBySerial(serial)) {
        std::cout << "Device could not be opened.  Is one connected?" << std::endl;
        return(false);
    }

    dev.GetDeviceInfo(&m_devInfo);
    std::cout << "Found a device: " << m_devInfo.productName << std::endl;

    dev.LoadDefaultPLLConfiguration();    

    // Get some general information about the XEM.
    std::cout << "Device firmware version: " 
        << m_devInfo.deviceMajorVersion << "." << m_devInfo.deviceMinorVersion << std::endl;
    std::cout << "Device serial number: " << m_devInfo.serialNumber << std::endl;
    std::cout << "Device device ID: " << m_devInfo.productID << std::endl;

    // Download the configuration file.
    if (okCFrontPanel::NoError != dev.ConfigureFPGA(bitfile)) {
        std::cout << "FPGA configuration failed." << std::endl;
        return(false);
    }

    // Check for FrontPanel support in the FPGA configuration.
    if (dev.IsFrontPanelEnabled()){
        std::cout << "FrontPanel support is enabled." << std::endl;
    } else {
        std::cout << "FrontPanel support is not enabled." << std::endl;
        return(false);
    }
    return(true);
}

bool CommOK::InitializeUSB() {
    if (!(OK_INTERFACE_USB3 == m_devInfo.deviceInterface)) {
        return false;
    }
}

}
}
}