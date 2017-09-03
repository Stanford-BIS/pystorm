#include "CommBDModel.h"

#include <chrono>
#include <thread>

#include "encoder/Encoder.h"

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {
namespace comm {

CommBDModel::CommBDModel(
    bdmodel::BDModel * model,
    const driverpars::DriverPars * driver_pars,
    MutexBuffer<COMMWord>* read_buffer,
    MutexBuffer<COMMWord>* write_buffer) {
  model_ = model;
  driver_pars_ = driver_pars;
  read_buffer_ = read_buffer;
  write_buffer_ = write_buffer;
  stream_state_ = CommStreamState::STOPPED;
}

CommBDModel::~CommBDModel() {

}

void CommBDModel::RunOnce() {
  unsigned int max_to_read = driver_pars_->Get(driverpars::BDMODELCOMM_MAX_TO_READ);
  unsigned int try_for_us = driver_pars_->Get(driverpars::BDMODELCOMM_TRY_FOR_US);
  std::vector<COMMWord> inputs = write_buffer_->PopVect(max_to_read, try_for_us, Encoder::bytesPerOutput);
  model_->ParseInput(inputs);
  std::vector<COMMWord> outputs = model_->GenerateOutputs();

  // have to make sure that we don't send something bigger than the buffer
  std::vector<COMMWord> output_chunk;
  unsigned int i = 0;
  while (i < outputs.size()) {
    output_chunk.push_back(outputs.at(i));
    i++;
    if (i % driver_pars_->Get(driverpars::DEC_BUF_IN_CAPACITY) == 0) {
      read_buffer_->Push(output_chunk);
      output_chunk.clear();
    }
  }
  read_buffer_->Push(output_chunk);
}

void CommBDModel::Run() {
  while (GetStreamState() == CommStreamState::STARTED) {
    RunOnce();
    unsigned int sleep_for_us = driver_pars_->Get(driverpars::BDMODELCOMM_SLEEP_FOR_US);
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

