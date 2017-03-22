#include "CommSoft.h"

namespace pystorm
{
namespace bddriver
{
namespace bdcomm
{


void CommSoft::ReadPacketsIn()
{
    std::unique_ptr<EmulatorCallbackData> cb = 
        std::unique_ptr<EmulatorCallbackData>(
        new EmulatorCallbackData());

    cb->comm = this;

    m_emulator->Read(std::move(cb));
}

void CommSoft::WritePacketsOut()
{
    std::unique_ptr<COMMWordStream> wordstream;
    bool retValue = m_write_queue.try_pop(std::ref(wordstream));
    if (true == retValue)
    {
        
        std::unique_ptr<EmulatorCallbackData> cb = 
            std::unique_ptr<EmulatorCallbackData>(
            new EmulatorCallbackData());
        cb->comm = this;
        cb->buf = std::move(wordstream);

        m_emulator->Write(std::move(cb));
    }
}

void CommSoft::CommSoftController()
{
    while (CommStreamState::STARTED == GetStreamState())
    {
        ReadPacketsIn();
        WritePacketsOut();        
    }
}


CommSoft::CommSoft(std::string& in_file_name, std::string& out_file_name)
{
    m_emulator = std::unique_ptr<Emulator>(new Emulator(in_file_name, out_file_name));
    m_state = CommStreamState::STOPPED;
}

CommSoft::~CommSoft()
{
    StopStreaming();
    SetStreamState(CommStreamState::STOPPED);
    if (m_control_thread.joinable())
        m_control_thread.join();
}

void CommSoft::StartStreaming()
{
    if (CommStreamState::STOPPED == GetStreamState())
    {
        SetStreamState(CommStreamState::STARTED);

        m_emulator->Start();

        m_control_thread = std::thread(&CommSoft::CommSoftController, this);
    }
}

void CommSoft::StopStreaming()
{
    {
        std::lock_guard<std::recursive_mutex> lck(m_state_mutex);
        if (CommStreamState::STARTED == GetStreamState())
        {
            SetStreamState(CommStreamState::STOPPED);
        }
    }

    if (m_control_thread.joinable())
        m_control_thread.join();

    m_emulator->Stop();
}

void CommSoft::Write(std::unique_ptr<COMMWordStream> wordStream)
{
    m_write_queue.push(wordStream);
}

std::unique_ptr<COMMWordStream> CommSoft::Read()
{
    std::unique_ptr<COMMWordStream> wordstream;
    auto retValue = m_read_queue.try_pop(wordstream);
    if (retValue)
        return wordstream;
    else
        return nullptr;
}

void CommSoft::ReadCallback(std::unique_ptr<EmulatorCallbackData> cb)
{
    m_read_queue.push(std::ref(cb->buf));
}

void CommSoft::WriteCallback(std::unique_ptr<EmulatorCallbackData> cb)
{
}


} // bdcomm namespace
} // bddriver namespace
} // pystorm namespace
