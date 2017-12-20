#include "CommBDModel.h"

#include <chrono>
#include <thread>
#include <memory>

#include "encoder/Encoder.h"

#include <iostream>
using std::cout;
using std::endl;

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
  stream_state_ = CommStreamState::STOPPED;
}

CommBDModel::~CommBDModel() {

}

void CommBDModel::RunOnce() {
  // pop from MB
  unsigned int try_for_us = driverpars::BDMODELCOMM_TRY_FOR_US;
  std::vector<std::unique_ptr<std::vector<COMMWord>>> inputs = write_buffer_->PopAll(try_for_us);

  // shouldn't need a deserializer, there's no USB to break up the transmission

  // parse inputs
  for (auto& input : inputs) {
    model_->ParseInput(*input);
  }

  // get outputs
  std::unique_ptr<std::vector<COMMWord>> outputs(new std::vector<COMMWord>);
  *outputs = model_->GenerateOutputs();

  // push to MB
  read_buffer_->Push(std::move(outputs));
}

void CommBDModel::Run() {
  while (GetStreamState() == CommStreamState::STARTED) {
    RunOnce();
    unsigned int sleep_for_us = driverpars::BDMODELCOMM_SLEEP_FOR_US;
    std::this_thread::sleep_for(std::chrono::microseconds(sleep_for_us));
  }
}

void CommBDModel::StartStreaming() {
  if (GetStreamState() == CommStreamState::STOPPED) {
    stream_state_ = CommStreamState::STARTED;
    thread_ = std::thread(&CommBDModel::Run, this);
  }
}

void CommBDModel::StopStreaming() {
  if (GetStreamState() == CommStreamState::STARTED) {
    stream_state_ = CommStreamState::STOPPED;
  }

  if (thread_.joinable()) thread_.join();
}

}  // comm namespace
}  // bddriver namespace
}  // pystorm namespace

