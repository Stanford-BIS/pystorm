#include "CommOK.h"
#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

using std::cout;
using std::endl;

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
    cout << "running once" << endl;
    ReadFromDevice();
    WriteToDevice();
    std::this_thread::sleep_for(1us);
  }
}

void CommOK::StartStreaming() {
  if (CommStreamState::STOPPED == GetStreamState()) {
    m_state = CommStreamState::STARTED;
    cout << "about ot start streaming" << endl;
    m_control_thread = std::thread(&CommOK::CommController, this);
    cout << "after start streaming" << endl;
  }
}

void CommOK::StopStreaming() {
  if (CommStreamState::STARTED == GetStreamState()) {
    m_state = CommStreamState::STOPPED;
  }

  if (m_control_thread.joinable()) m_control_thread.join();
}

int CommOK::WriteToDevice() {

    cout << "writing to device" << endl;
    // Pop one vector of COMMWords from the MutexBuffer
    // blocks for 1 us
    std::unique_ptr<std::vector<COMMWord>> popped_vect = m_write_buffer->PopAll(1);

    // use the deserializer to build up blocks of WRITE_SIZE elements
    deserializer_.NewInput(popped_vect.get());

    cout << "popped size " << popped_vect->size() << endl;

    std::vector<COMMWord> deserialized; // continuosly write into here

    int last_status = -1;
    deserializer_.GetOneOutput(&deserialized);
    while (deserialized.size() > 0) {
        cout << "hi3" << endl;
        deserializer_.GetOneOutput(&deserialized);
        last_status = dev.WriteToBlockPipeIn(PIPE_IN_ADDR, WRITE_SIZE, WRITE_SIZE, &deserialized[0]);
        if (last_status != WRITE_SIZE) {
            printf("*WARNING*: Read from MB: %d, Written to OK: %d", WRITE_SIZE, last_status);
        }
    }
    cout << "hi4" << endl;
    return last_status;
}

int CommOK::ReadFromDevice() {
    cout << "reading from device" << endl;
    std::unique_ptr<std::vector<COMMWord>> read_buffer(new std::vector<COMMWord>(READ_SIZE, 0));
    COMMWord * raw_data = &(*read_buffer)[0];
    int num_bytes = dev.ReadFromBlockPipeOut(PIPE_OUT_ADDR, READ_SIZE, READ_SIZE, raw_data);
    if (num_bytes > 0) {
      m_read_buffer->Push(std::move(read_buffer));
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
