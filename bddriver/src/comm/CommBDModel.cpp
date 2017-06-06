#include "CommBDModel.h"

#include <chrono>
#include <thread>

#include "encoder/Encoder.h"

namespace pystorm {
namespace bddriver {
namespace comm {

CommBDModel::CommBDModel(
    bdmodel::BDModel * model,
    MutexBuffer<COMMWord>* read_buffer,
    MutexBuffer<COMMWord>* write_buffer) {
  model_ = model;
  read_buffer_ = read_buffer;
  write_buffer_ = write_buffer;
}

void CommBDModel::RunOnce() {
  std::vector<COMMWord> inputs = write_buffer_->PopVect(kMaxToRead, kTryForUS, Encoder::bytesPerOutput);
  model_->ParseInput(inputs);
  std::vector<COMMWord> outputs = model_->GenerateOutputs();
  read_buffer_->Push(outputs, kTryForUS);
}

void CommBDModel::Run() {
  while (GetStreamState() == CommStreamState::STARTED) {
    RunOnce();
    std::this_thread::sleep_for(std::chrono::milliseconds(kSleepMS));
  }
}

void CommBDModel::StartStreaming() {
  if (GetStreamState() == CommStreamState::STOPPED) {
    run_state_ = CommStreamState::STARTED;
    thread_ = std::thread(&CommBDModel::Run, this);
  }
}

void CommBDModel::StopStreaming() {
  if (GetStreamState() == CommStreamState::STARTED) {
    run_state_ = CommStreamState::STOPPED;
  }

  if (thread_.joinable()) thread_.join();
}

}  // comm namespace
}  // bddriver namespace
}  // pystorm namespace

