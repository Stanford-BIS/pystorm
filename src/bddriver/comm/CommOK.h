#pragma once

// Needs preprocessor definition `-DBD_COMM_TYPE_OPALKELLY`

#include "Comm.h"
#include "common/MutexBuffer.h"
#include "common/vector_util.h"
#include <okFrontPanelDLL.h>

namespace pystorm {
namespace bddriver {
namespace comm {

static const int PIPE_IN_ADDR = 0x80;  /// Endpoint to send to FPGA
static const int PIPE_OUT_ADDR = 0xa0; /// Endpoint to read from FPGA

static const unsigned int WRITE_BLOCK_SIZE       = 1024;
static const unsigned int WRITE_SIZE             = 1024;
static const unsigned int READ_SIZE              = 512;
static const unsigned int DEFAULT_BUFFER_TIMEOUT = 1;

class CommOK : public Comm {
public:
    /// Constructor
    CommOK(MutexBuffer<COMMWord>* read_buffer, MutexBuffer<COMMWord>* write_buffer) :
        m_read_buffer(read_buffer), m_write_buffer(write_buffer), m_state(CommStreamState::STOPPED), deserializer_(new VectorDeserializer<COMMWord>(WRITE_SIZE)) {};
    /// Default copy constructor
    CommOK(const CommOK&) = delete;
    /// Default move constructor
    CommOK(CommOK&&) = delete;
    /// Default copy assignment
    CommOK& operator=(const CommOK&) = delete;
    /// Default move assignment
    CommOK& operator=(CommOK&&) = delete;
    /// Default destructor
    ~CommOK() { delete deserializer_; }

    /// Initialization
    int Init(const std::string bitfile, const std::string serial);

    /// data_length: length in bytes
    int WriteToDevice();
    int ReadFromDevice();

    void StartStreaming();
    void StopStreaming();
    CommStreamState GetStreamState() { return m_state; }
    MutexBuffer<COMMWord>* getReadBuffer() { return m_read_buffer; }
    MutexBuffer<COMMWord>* getWriteBuffer() { return m_write_buffer; }

protected:
  MutexBuffer<COMMWord>* m_read_buffer;
  MutexBuffer<COMMWord>* m_write_buffer;
  std::atomic<CommStreamState> m_state;
  std::thread m_control_thread;
  VectorDeserializer<COMMWord> * deserializer_; 
  std::vector<COMMWord> deserialized_; // continuosly write into here

  void CommController();

private:
    bool InitializeFPGA(const std::string bitfile, const std::string serial);
    bool InitializeUSB();

    // Variables
    okCFrontPanel dev;
    okTDeviceInfo m_devInfo;
};
}
}
}
