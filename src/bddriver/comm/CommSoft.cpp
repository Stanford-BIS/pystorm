#include "CommSoft.h"

namespace pystorm {
namespace bddriver {
namespace comm {

CommSoft::CommSoft(
    const std::string& in_file_name,
    const std::string& out_file_name,
    MutexBuffer<COMMWord>* read_buffer,
    MutexBuffer<COMMWord>* write_buffer)
    : m_emulator(nullptr), m_read_buffer(read_buffer), m_write_buffer(write_buffer), m_state(CommStreamState::STOPPED) {
  m_emulator = new Emulator(in_file_name, out_file_name);
}

void CommSoft::ReadFromDevice() {
  std::unique_ptr<EmulatorCallbackData> cb = std::unique_ptr<EmulatorCallbackData>(new EmulatorCallbackData());

  auto wordstream = std::unique_ptr<COMMWordStream>(new COMMWordStream());

  wordstream->resize(READ_SIZE);

  cb->client = this;
  cb->buf    = std::unique_ptr<COMMWordStream>(new COMMWordStream());
  cb->buf->resize(READ_SIZE);

  m_emulator->Read(std::move(cb));
}

void CommSoft::WriteToDevice() {
  std::unique_ptr<std::vector<COMMWord>> vectorOfCOMMWords = m_write_buffer->Pop(DEFAULT_BUFFER_TIMEOUT);

  if (vectorOfCOMMWords->size() > 0) {
    std::unique_ptr<EmulatorCallbackData> cb = std::unique_ptr<EmulatorCallbackData>(new EmulatorCallbackData());

    cb->client = this;
    cb->buf    = std::move(vectorOfCOMMWords);

    m_emulator->Write(std::move(cb));
  }
}

void CommSoft::CommSoftController() {
  while (CommStreamState::STARTED == GetStreamState()) {
    ReadFromDevice();
    WriteToDevice();
  }
}

CommSoft::~CommSoft() {
  StopStreaming();
  m_state = CommStreamState::STOPPED;
  if (m_control_thread.joinable()) m_control_thread.join();
  delete m_emulator;
}

void CommSoft::StartStreaming() {
  if (CommStreamState::STOPPED == GetStreamState()) {
    m_state = CommStreamState::STARTED;
    m_emulator->Start();
    m_control_thread = std::thread(&CommSoft::CommSoftController, this);
  }
}

void CommSoft::StopStreaming() {
  {
    std::lock_guard<std::recursive_mutex> lck(m_state_mutex);
    if (CommStreamState::STARTED == GetStreamState()) {
      m_state = CommStreamState::STOPPED;
    }
  }

  if (m_control_thread.joinable()) m_control_thread.join();

  m_emulator->Stop();
}

void CommSoft::ReadCallback(std::unique_ptr<EmulatorCallbackData> cb) {
  std::unique_ptr<std::vector<COMMWord>> vecOfCWS(new std::vector<COMMWord>(*(cb->buf)));

  m_read_buffer->Push(std::move(vecOfCWS));
}

void CommSoft::WriteCallback(std::unique_ptr<EmulatorCallbackData> cb) {}

}  // comm namespace
}  // bddriver namespace
}  // pystorm namespace
