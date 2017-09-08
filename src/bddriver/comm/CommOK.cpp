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
    // There is still no guarantee that m_write_buffer has at least WRITE_SIZE elements
    // after GetCount().
    // So, this could potentially fail, but it is unlikely.
    // So, lock the buffer first, and do the rest.
    int status = -1;
    if (m_write_buffer->LockFrontSimple(DEFAULT_BUFFER_TIMEOUT)) {
        unsigned int data_length = m_write_buffer->GetCount();
        if (data_length >= WRITE_SIZE) {
            m_write_buffer->PopSimple(write_buffer_, WRITE_SIZE);
            status = dev.WriteToBlockPipeIn(PIPE_IN_ADDR, WRITE_SIZE, WRITE_SIZE, write_buffer_);
            if (status != WRITE_SIZE) {
                printf("*WARNING*: Read from MB: %d, Written to OK: %d", WRITE_SIZE, status);
            }
        } 
    }
    m_write_buffer->UnlockFront(false);
    return status;
}

int CommOK::ReadFromDevice() {
    int num_bytes = dev.ReadFromBlockPipeOut(PIPE_OUT_ADDR, READ_SIZE, READ_SIZE, read_buffer_);
    if (num_bytes > 0) {
      // Write in blocking fashion
      m_read_buffer->Push(read_buffer_, num_bytes, 0);
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