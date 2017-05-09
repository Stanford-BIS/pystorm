#ifndef COMMSOFT_H
#define COMMSOFT_H

#include <vector>
#include <deque>
#include <iostream>
#include <thread>
#include <mutex>
#include <memory>

#include <cstring>

#include "common/MutexBuffer.h"
#include "common/DriverTypes.h"
#include "Comm.h"
#include "Emulator.h"

namespace pystorm {
namespace bddriver {
namespace comm {

class CommSoft : public Comm, EmulatorClientIfc {
public:
    CommSoft(const std::string& in_file_name, const std::string& out_file_name);
    ~CommSoft();
    CommSoft(const CommSoft&) = delete;

    // Comm interface

    ///
    /// Start streaming packets between Comm and the software emulator
    ///
    virtual void StartStreaming();

    ///
    /// Stop streaming packets between Comm and the software emulator
    ///
    virtual void StopStreaming();

    ///
    /// Get the current streaming state
    ///
    virtual CommStreamState GetStreamState() {
        return m_state;
    }

    MutexBuffer<COMMWordStream> * getReadBuffer() {
        return m_read_buffer;
    }

    MutexBuffer<COMMWordStream> * getWriteBuffer() {
        return m_write_buffer;
    }

    // EmulatorCallbackIfc interface

    ///
    /// Method called by the Emulator after data is read
    ///
    virtual void ReadCallback(std::unique_ptr<EmulatorCallbackData> cb);

    ///
    /// Method called by the Emulator after data is written
    ///
    virtual void WriteCallback(std::unique_ptr<EmulatorCallbackData> cb);

    ///
    /// The number of elements the read and write buffers can store before 
    /// blocking.
    ///
    static const unsigned int CAPACITY = 10000; 

protected:
    ///
    /// Entry point for CommSofts thread
    /// 
    void CommSoftController();

    ///
    /// Read packets in from file and place them on Decoder input buffer
    ///
    void ReadFromDevice();

    ///
    // Read packets from Encoder output buffer and place them into file
    ///
    void WriteToDevice();

    Emulator * m_emulator;
    MutexBuffer<COMMWordStream> * m_read_buffer;
    MutexBuffer<COMMWordStream> * m_write_buffer;
    std::atomic<CommStreamState> m_state;

    std::thread m_control_thread;

    std::recursive_mutex m_state_mutex;
   
    static const int MAX_POP = 4;
    static const int DEFAULT_BUFFER_TIMEOUT = 10;
};

} // comm namespace
} // bddriver namespace
} // pystorm namespace
#endif
