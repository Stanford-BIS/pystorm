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

void PrintBinaryAsStr(uint32_t b, unsigned int N) {
  std::vector<bool> bits;
  // pop LSBs
  for (unsigned int i = 0; i < N; i++) {
    bool low_bit = b % 2 == 1;
    b = b >> 1;
    bits.push_back(low_bit);
  }
  for (unsigned int i = 0; i < N; i++) {
    if (bits.at(N - 1 - i))
      cout << "1";
    else
      cout << "0";
  }
  cout << endl;
}

int CommOK::WriteToDevice() {

    // Pop one vector of COMMWords from the MutexBuffer
    // blocks for 1 us
    std::vector<std::unique_ptr<std::vector<COMMWord>>> popped_vect = m_write_buffer->PopAll(1);

    if (popped_vect.size() > 0) 
      cout << "popped this many vects: " << popped_vect.size() << endl;
    // use the deserializer to build up blocks of WRITE_SIZE elements
    int size = 0;
    for (unsigned int i = 0; i < popped_vect.size(); i++) {
      size += popped_vect.at(i)->size();
      deserializer_->NewInput(std::move(popped_vect.at(i)));
    }
    if (popped_vect.size() > 0) 
      cout << "  total size " << size << endl;

    std::vector<COMMWord> deserialized; // continuosly write into here

    int last_status = -1;
    deserializer_->GetOneOutput(&deserialized);
    while (deserialized.size() > 0) {
        assert(deserialized.size() == WRITE_SIZE);

        //cout << "Comm writing words:" << endl;
        //for (unsigned int i = 0; i < WRITE_SIZE; i++) {
        //  PrintBinaryAsStr(deserialized[i], 8);
        //}

        last_status = dev.WriteToBlockPipeIn(PIPE_IN_ADDR, WRITE_SIZE, WRITE_SIZE, &deserialized[0]);
        if (last_status != WRITE_SIZE) {
            printf("*WARNING*: Read from MB: %d, Written to OK: %d", WRITE_SIZE, last_status);
        }
        cout << "Comm wrote " << WRITE_SIZE << " words" << endl;
        deserializer_->GetOneOutput(&deserialized);
    }
    return last_status;
}

int CommOK::ReadFromDevice() {
    std::unique_ptr<std::vector<COMMWord>> read_buffer(new std::vector<COMMWord>(READ_SIZE, 0));
    COMMWord * raw_data = &(*read_buffer)[0];
    int num_bytes = dev.ReadFromPipeOut(PIPE_OUT_ADDR, READ_SIZE, raw_data);

    //cout << "Comm reading words:" << endl;
    //for (unsigned int i = 0; i < READ_SIZE; i++) {
    //  PrintBinaryAsStr(raw_data[i], 8);
    //}

    assert(num_bytes == READ_SIZE);
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
