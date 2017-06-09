#ifndef COMMSOFT_H
#define COMMSOFT_H

#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <cstring>

#include "Comm.h"
#include "Emulator.h"
#include "common/DriverTypes.h"
#include "common/MutexBuffer.h"

namespace pystorm {
namespace bddriver {
namespace comm {

class CommSoft : public Comm, EmulatorClientIfc {
 public:
  CommSoft(
      const std::string& in_file_name,
      const std::string& out_file_name,
      MutexBuffer<COMMWord>* read_buffer,
      MutexBuffer<COMMWord>* write_buffer);
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
  virtual CommStreamState GetStreamState() { return m_state; }

  MutexBuffer<COMMWord>* getReadBuffer() { return m_read_buffer; }

  MutexBuffer<COMMWord>* getWriteBuffer() { return m_write_buffer; }

  // EmulatorCallbackIfc interface

  ///
  /// Method called by the Emulator after data is read
  ///
  virtual void ReadCallback(std::unique_ptr<EmulatorCallbackData> cb);

  ///
  /// Method called by the Emulator after data is written
  ///
  virtual void WriteCallback(std::unique_ptr<EmulatorCallbackData> cb);

  static const int WRITE_SIZE             = 512;
  static const int READ_SIZE              = 512;
  static const int DEFAULT_BUFFER_TIMEOUT = 10;

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

  Emulator* m_emulator;
  MutexBuffer<COMMWord>* m_read_buffer;
  MutexBuffer<COMMWord>* m_write_buffer;
  std::atomic<CommStreamState> m_state;
  std::recursive_mutex m_state_mutex;

  std::thread m_control_thread;
};

}  // comm namespace
}  // bddriver namespace
}  // pystorm namespace
#endif
