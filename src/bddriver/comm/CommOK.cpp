#include "CommOK.h"
#include <chrono>

using namespace std::chrono_literals;
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

void CommOK::CommController() {
  while (CommStreamState::STARTED == GetStreamState()) {
    ReadFromDevice();
    WriteToDevice();
    std::this_thread::sleep_for(1us);
  }
}

void CommOK::StartStreaming() {
  if (CommStreamState::STOPPED == GetStreamState()) {
    m_state = CommStreamState::STARTED;
    m_control_thread = std::thread(&CommOK::CommController, this);
  }
}

void CommOK::StopStreaming() {
  if (CommStreamState::STARTED == GetStreamState()) {
    m_state = CommStreamState::STOPPED;
  }

  if (m_control_thread.joinable()) m_control_thread.join();
}

int CommOK::WriteToDevice() {
  auto vectorOfCOMMWords = m_write_buffer->PopVect(WRITE_SIZE, DEFAULT_BUFFER_TIMEOUT);
  long data_length = vectorOfCOMMWords.size();

  if (vectorOfCOMMWords.size() > 0) {
      write_buffer_ = reinterpret_cast<unsigned char*>(vectorOfCOMMWords.data());
      return dev.WriteToPipeIn(PIPE_IN_ADDR, data_length, write_buffer_);
  }
}

int CommOK::ReadFromDevice() {
    int num_bytes = dev.ReadFromPipeOut(PIPE_OUT_ADDR, READ_SIZE, read_buffer_);
    if (num_bytes > 0) {
      std::vector<COMMWord> vecOfCWS(read_buffer_, read_buffer_ + num_bytes);
      m_read_buffer->Push(vecOfCWS, DEFAULT_BUFFER_TIMEOUT);
    }
    return num_bytes;
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