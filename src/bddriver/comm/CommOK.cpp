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
        cout << "FrontPanel library could not be loaded" << endl;
        return -1;
    }

    okFrontPanelDLL_GetVersion(lib_date, lib_time);
    cout << "FrontPanel library loaded\n"
        << "Built: " << lib_date << ", " << lib_time << endl;

    if (false == InitializeFPGA(bitfile, serial)) {
          cout << "FPGA could not be initialized" << endl;
          return -1;
    }

    if (false == InitializeUSB()) {
        cout << "Could not open USB connection" << endl;
        return -1;
    }

    return 0;
}

void CommOK::CommController() {
  while (CommStreamState::STARTED == GetStreamState()) {
    ReadFromDevice();
    int last_write_status = WriteToDevice();
    if (last_write_status < 0) {
      cout << "CommOK: write failed, with code " << last_write_status << ". Stopping thread" << endl;
      StopStreaming();
    }
    //std::this_thread::sleep_for(1us);
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
    // potentially blocks for 1 us
    std::vector<std::unique_ptr<std::vector<COMMWord>>> popped_vect = m_write_buffer->PopAll(1);

    // use the deserializer to build up blocks of WRITE_SIZE elements
    int size = 0;
    for (unsigned int i = 0; i < popped_vect.size(); i++) {
      size += popped_vect.at(i)->size();
      deserializer_->NewInput(std::move(popped_vect.at(i)));
    }

    if (deserialized_.size() == 0) {
      deserializer_->GetOneOutput(&deserialized_);
    }

    //cout << "Comm writing words:" << endl;
    //for (unsigned int i = 0; i < WRITE_SIZE; i++) {
    //  PrintBinaryAsStr(deserialized[i], 8);
    //}

    if (deserialized_.size() > 0) {
      assert(deserialized_.size() == WRITE_SIZE);

      //cout << "comm about to write" << endl;
      int last_status = dev.WriteToBlockPipeIn(PIPE_IN_ADDR, WRITE_BLOCK_SIZE, WRITE_SIZE, &deserialized_[0]);
      //cout << "comm wrote " << last_status << " words" << endl;

      if (last_status == WRITE_SIZE) {
        deserialized_.clear();
      } else {
        printf("*WARNING*: Read from MB: %d, Written to OK: %d", WRITE_SIZE, last_status);
      }

      return last_status;
    } else {
      return 0;
    }

}

int CommOK::ReadFromDevice() {
    std::unique_ptr<std::vector<COMMWord>> read_buffer(new std::vector<COMMWord>(READ_SIZE, 0));
    COMMWord * raw_data = &(*read_buffer)[0];
    int num_bytes = dev.ReadFromPipeOut(PIPE_OUT_ADDR, READ_SIZE, raw_data);

    //cout << "Comm reading words:" << endl;
    //for (unsigned int i = 0; i < READ_SIZE; i++) {
    //  PrintBinaryAsStr(raw_data[i], 8);
    //}

    //assert(num_bytes == READ_SIZE);
    //cout << "comm read: " << num_bytes << endl;
    if (num_bytes > 0) {
      m_read_buffer->Push(std::move(read_buffer));
    }
    return num_bytes;
}

bool CommOK::InitializeFPGA(const std::string bitfile, const std::string serial) {
    if (okCFrontPanel::NoError != dev.OpenBySerial(serial)) {
        cout << "Device could not be opened.  Is one connected?" << endl;
        return(false);
    }

    dev.GetDeviceInfo(&m_devInfo);
    cout << "Found a device: " << m_devInfo.productName << endl;

    dev.LoadDefaultPLLConfiguration();    

    // Get some general information about the XEM.
    cout << "Device firmware version: " 
        << m_devInfo.deviceMajorVersion << "." << m_devInfo.deviceMinorVersion << endl;
    cout << "Device serial number: " << m_devInfo.serialNumber << endl;
    cout << "Device device ID: " << m_devInfo.productID << endl;

    // Download the configuration file.
    if (okCFrontPanel::NoError != dev.ConfigureFPGA(bitfile)) {
        cout << "FPGA configuration failed." << endl;
        return(false);
    }

    // Check for FrontPanel support in the FPGA configuration.
    if (dev.IsFrontPanelEnabled()){
        cout << "FrontPanel support is enabled." << endl;
    } else {
        cout << "FrontPanel support is not enabled." << endl;
        return(false);
    }

    // If the BTPipeIn (downstream) isn't ready, we want to return ASAP
    dev.SetTimeout(1);
    dev.SetBTPipePollingInterval(1);

    return(true);
}

bool CommOK::InitializeUSB() {
    auto ifc = m_devInfo.deviceInterface;
    if (ifc == OK_INTERFACE_USB3) {
        return true;
    } else if (ifc == OK_INTERFACE_USB2) {
        cout << "USB3 interface not available. Using USB2" << endl;
        return true;
    }
    return false;
}

}
}
}
