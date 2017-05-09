#include "CommSoft.h"

namespace pystorm {
namespace bddriver {
namespace comm {

CommSoft::CommSoft(const std::string& in_file_name, 
    const std::string& out_file_name) :
        m_emulator(nullptr),
        m_read_buffer(nullptr),
        m_write_buffer(nullptr),
        m_state(CommStreamState::STOPPED) {
    m_emulator = new Emulator(in_file_name, out_file_name);
    m_read_buffer = new MutexBuffer<COMMWordStream>(CommSoft::CAPACITY);
    m_write_buffer = new MutexBuffer<COMMWordStream>(CommSoft::CAPACITY);
}

void CommSoft::ReadFromDevice() {
    std::unique_ptr<EmulatorCallbackData> cb = 
        std::unique_ptr<EmulatorCallbackData>(new EmulatorCallbackData());

    cb->client = this;

    m_emulator->Read(std::move(cb));
}

void CommSoft::WriteToDevice() {
    auto vectorOfCOMMWordStreams =
        m_write_buffer->PopVect(MAX_POP, DEFAULT_BUFFER_TIMEOUT);

    if (vectorOfCOMMWordStreams.size() > 0) {
        auto wordstream = std::unique_ptr<COMMWordStream>(
            new COMMWordStream());

        for (auto stream:vectorOfCOMMWordStreams) {
            wordstream->insert(wordstream->end(),stream.begin(), stream.end());
        }

        std::unique_ptr<EmulatorCallbackData> cb = 
            std::unique_ptr<EmulatorCallbackData>(
            new EmulatorCallbackData());

        cb->client = this;
        cb->buf = std::move(wordstream);

        m_emulator->Write(std::move(cb));
    }
}

void CommSoft::CommSoftController() {
    while (CommStreamState::STARTED == GetStreamState())
    {
        ReadFromDevice();
        WriteToDevice();
    }
}

CommSoft::~CommSoft() {
    StopStreaming();
    m_state = CommStreamState::STOPPED;
    if (m_control_thread.joinable())
        m_control_thread.join();
    delete m_write_buffer;
    delete m_read_buffer;
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

    if (m_control_thread.joinable())
        m_control_thread.join();

    m_emulator->Stop();
}

void CommSoft::ReadCallback(std::unique_ptr<EmulatorCallbackData> cb) {

    std::vector<COMMWordStream> vecOfCWS;
    vecOfCWS.push_back(*(cb->buf));

    while ((m_read_buffer->Push(vecOfCWS,DEFAULT_BUFFER_TIMEOUT) == false) &&
           (CommStreamState::STOPPED != m_state)) {
    }
}

void CommSoft::WriteCallback(std::unique_ptr<EmulatorCallbackData> cb) {
}


} // comm namespace
} // bddriver namespace
} // pystorm namespace
