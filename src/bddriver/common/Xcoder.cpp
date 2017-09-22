#include "Xcoder.h"

#include <atomic>
#include <thread>

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

Xcoder::Xcoder() {
  do_run_ = false;
  thread_ = nullptr;
}

Xcoder::~Xcoder() {
  if (do_run_) {
    Stop();
  }
}

void Xcoder::Run() {
  while (do_run_) {
    RunOnce();
  }
}

void Xcoder::Start() {
  do_run_ = true;
  thread_ = new std::thread([this] { this->Run(); });
}

void Xcoder::Stop() {
  do_run_ = false;
  if (thread_ != nullptr) {
    if (thread_->joinable()) {
      thread_->join();
    }
    delete thread_;
  }
}

}  // bddriver
}  // pystorm
