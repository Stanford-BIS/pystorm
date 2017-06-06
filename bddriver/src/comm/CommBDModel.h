#ifndef COMMBDMODEL_H
#define COMMBDMODEL_H

#include "Comm.h"

#include <atomic>

#include "model/BDModel.h"
#include "common/MutexBuffer.h"

namespace pystorm {
namespace bddriver {
namespace comm {

/// CommBDModel is a Comm object which uses a BDModel to parse and generate
/// traffic streams as if they were from the BD hardware.
/// Takes BDModel ptr as an argument, user controls BDModel directly to 
/// create upstream traffic.
class CommBDModel : public Comm {
 public:

  CommBDModel(
      bdmodel::BDModel * model,
      MutexBuffer<COMMWord>* read_buffer,
      MutexBuffer<COMMWord>* write_buffer);
  ~CommBDModel();

  void StartStreaming();
  void StopStreaming();

  CommStreamState GetStreamState() { return run_state_; }
  MutexBuffer<COMMWord>* getReadBuffer() { return read_buffer_; }
  MutexBuffer<COMMWord>* getWriteBuffer() { return write_buffer_; }

 private:
  // I can't think of how this would be subclassed, so private
  
  std::thread thread_; /// worker thread 
  
  std::atomic<CommStreamState> run_state_; // atomic because StartStreaming/StopStreaming don't gain lock
  bdmodel::BDModel * model_;

  MutexBuffer<COMMWord>* read_buffer_; /// output buffer
  MutexBuffer<COMMWord>* write_buffer_; /// input buffer

  const unsigned int kTryForUS = 1000;
  const unsigned int kMaxToRead = 1000;
  const unsigned int kSleepMS = 1;

  // feed write_buffer_ into BDModel, use BDModel to feed read_buffer_
  void Run();
  void RunOnce();
};

}  // comm namespace
}  // bddriver namespace
}  // pystorm namespace

#endif
