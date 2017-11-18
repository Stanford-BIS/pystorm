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
    int last_read_status = ReadFromDevice();

    // determine if downstream queue is almost full, if it isn't write
    if (FIFO_DEPTH - DS_queue_count_ * 4 > MAX_WRITE_BLOCKS * WRITE_BLOCK_SIZE){
      int last_write_status = WriteToDevice();

      if (last_write_status < 0) {
        cout << "ERROR: CommOK: write failed, with code " << last_write_status << ". Stopping thread" << endl;
        StopStreaming();
      }
    }

    if (last_read_status < 0) {
      cout << "ERROR: CommOK: read failed. Got code " << last_read_status << ". Stopping." << endl;
      StopStreaming();
    } else if (last_read_status != READ_SIZE) {
      cout << "WARNING: CommOK: didn't read " << READ_SIZE << " bytes. Instead got " << last_read_status << endl;
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

  // potentially blocks for 1 us
  if (m_write_buffer->TotalSize() > 0) {
    std::unique_ptr<std::vector<COMMWord>> blocks = m_write_buffer->Pop();

    // each block should be WRITE_BLOCK_SIZE * N elements long
    // nothing gets through the driver and encoder without a flush, 
    // encoder flush pads to the correct length

    assert(blocks->size() % WRITE_BLOCK_SIZE == 0);

    auto start = std::chrono::high_resolution_clock::now();
    int last_status = dev.WriteToBlockPipeIn(PIPE_IN_ADDR, WRITE_BLOCK_SIZE, blocks->size(), blocks->data());
    auto end = std::chrono::high_resolution_clock::now();
    cout << "CommOK::WriteToDevice : write took " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " us" << endl;

    if (last_status < 0) {
      cout << "WARNING: CommOK::WriteToDevice: tried writing " << blocks->size() << " got error code " << last_status << endl;
    } else if (static_cast<unsigned int>(last_status) != blocks->size()) {
      cout << "WARNING: CommOK::WriteToDevice: tried writing " << blocks->size() << " but only wrote " << last_status << ". Lost data!" << endl;
    }

    return last_status;
  } else {
    return 0;
  }
}

int CommOK::ReadFromDevice() {
    std::unique_ptr<std::vector<COMMWord>> read_buffer(new std::vector<COMMWord>(READ_SIZE, 0));
    COMMWord * raw_data = read_buffer->data();
    int num_bytes = dev.ReadFromBlockPipeOut(PIPE_OUT_ADDR, READ_BLOCK_SIZE, READ_SIZE, raw_data);

    // peek at the first word to get the number of things in the downstream queue
    // use this to determine whether we should WriteToDevice or not
    
    // FIFO depth is 2**14, so tack together first two words
    DS_queue_count_ = (raw_data[1] << 8) | raw_data[0];
    //if (DS_queue_count_ > 0) {
    //  cout << "DS_queue_count_ measured as " << DS_queue_count_ << endl;
    //}
    assert(raw_data[3] == 128);
    
    //cout << "Comm reading words:" << endl;
    //cout << "comm read: " << num_bytes << endl;
    //for (unsigned int i = 0; i < 16; i++) {
    //  PrintBinaryAsStr(raw_data[i], 8);
    //}

    //assert(num_bytes == READ_SIZE);

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
    //dev.SetTimeout(1);
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
