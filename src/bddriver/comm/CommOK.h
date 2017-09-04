#pragma once

// Needs preprocessor definition `-DBD_COMM_TYPE_OPALKELLY`

#include "Comm.h"
#include "common/MutexBuffer.h"
#include <okFrontPanelDLL.h>

namespace pystorm {
namespace bddriver {
namespace comm {

static const int PIPE_IN_ADDR = 0x80;  /// Endpoint to send to FPGA
static const int PIPE_OUT_ADDR = 0xa0; /// Endpoint to read from FPGA

class CommOK : public Comm {
public:
    /// Constructor
    CommOK() = default;
    /// Default copy constructor
    CommOK(const CommOK&) = delete;
    /// Default move constructor
    CommOK(CommOK&&) = delete;
    /// Default copy assignment
    CommOK& operator=(const CommOK&) = delete;
    /// Default move assignment
    CommOK& operator=(CommOK&&) = delete;
    /// Default destructor
    ~CommOK() = default;

    /// Initialization
    int Init(const std::string bitfile, const std::string serial);

    /// data_length: length in bytes
    int Write(long data_length, unsigned char* data);
    int Read(long data_length, unsigned char* data);

    void StartStreaming() {}
    void StopStreaming() {}
    CommStreamState GetStreamState() { return m_state; }
    MutexBuffer<COMMWord>* getReadBuffer() { return m_read_buffer; }
    MutexBuffer<COMMWord>* getWriteBuffer() { return m_write_buffer; }

protected:
  MutexBuffer<COMMWord>* m_read_buffer;
  MutexBuffer<COMMWord>* m_write_buffer;
  std::atomic<CommStreamState> m_state;

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