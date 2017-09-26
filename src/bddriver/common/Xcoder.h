#ifndef XCODER_H
#define XCODER_H

#include <atomic>
#include <thread>

namespace pystorm {
namespace bddriver {

class Xcoder {
  // pure-virtual base class for encoders/decoders. Derived classes must define RunOnce()
 public:
  Xcoder();
  virtual ~Xcoder();

  void Start();
  void Stop();

 protected:
  std::thread *thread_;       // pointer to thread which will be launched with Start()
  std::atomic<bool> do_run_;  // used to join thread on destruction

  void Run();
  virtual void RunOnce() = 0;
};

}  // bddriver
}  // pystorm

#endif
