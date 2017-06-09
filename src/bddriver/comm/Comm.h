#ifndef COMM_H
#define COMM_H

#include <atomic>
#include <deque>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "common/MutexBuffer.h"

namespace pystorm {
namespace bddriver {
namespace comm {

enum class CommStreamState { STARTED = 0, STOPPED = 1 };

template <typename Enumeration>
auto as_integer(Enumeration const value) -> typename std::underlying_type<Enumeration>::type {
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

typedef unsigned char COMMWord;
typedef std::vector<COMMWord> COMMWordStream;

/// An interface for classes that send words to and receive words from
/// a device.
///
/// Comm describes the functionality needed by bddriver module when
/// communicating with external devices such as the hosts USB controller.
/// Another example could be the hosts TCP/IP interface or a socket interface.
/// In all cases, bddriver needs the ability to start and stop streams
/// between itself and the device. In addition, bddriver needs the ability
/// to query the streaming state as well as read words from and write
/// words to the device.
///
/// A user of Comm writes to the device by getting Comm's write buffer and
/// and pushing COMMWordStreams to it.
/// For example, using the CommSoft implementation of Comm, a user would
/// write data as follows:
///
///     unsigned int write_timeout = 0;
///     std::string output_file("somefilename.bin");
///     std::string input_file("someotherfilename.bin");

///     auto comm_instance = new CommSoft(input_file, output_file);
///     auto write_buffer  = comm_instance->getWriteBuffer();
///     COMMWordStream wordstream;
///
///         ... Populate the wordstream
///
///     write_buffer->Push(wordstream,write_timeout);
///
/// To read data coming from the interface, a user could do the following:
///
///     unsigned int read_timeout = 0;
///     unsigned int max_read_buffer_size = CommSoft::CAPACITY;
///     auto read_buffer = comm_instance->getReadBuffer();
///     auto inputstream = read_buffer(max_read_buffer_size, read_timeout);
///
///         ... Do work with the stream
///
class Comm {
 public:
  /// Sets the Comm to a streaming state where words can read and written
  virtual void StartStreaming() = 0;

  /// Sets the Comm to a non-streaming state where words are no longer  read
  /// or written.
  virtual void StopStreaming() = 0;

  /// Returns the streaming state.
  virtual CommStreamState GetStreamState() = 0;

  /// Returns the read buffer
  virtual MutexBuffer<COMMWord>* getReadBuffer() = 0;

  /// Returns the write buffer
  virtual MutexBuffer<COMMWord>* getWriteBuffer() = 0;
};

}  // comm namespace
}  // bddriver namespace
}  // pystorm namespace
#endif
