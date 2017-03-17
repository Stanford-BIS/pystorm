#include "CommSoft.h"

namespace PyStorm
{
namespace BDDriver
{
namespace BDComm
{


void CommSoft::ReadPacketsIn()
{
    std::unique_ptr<PacketGeneratorCallbackData> cb = 
        std::unique_ptr<PacketGeneratorCallbackData>(
        new PacketGeneratorCallbackData());
    cb->comm = this;
    m_pg.Read(std::move(cb));
}

void CommSoft::WritePacketsOut()
{
    std::unique_ptr<COMMWordStream> wordstream;
    bool retValue = m_write_queue.try_pop(std::ref(wordstream));
    if (true == retValue)
    {
        
        std::unique_ptr<PacketGeneratorCallbackData> cb = 
            std::unique_ptr<PacketGeneratorCallbackData>(
            new PacketGeneratorCallbackData());
        cb->comm = this;
        cb->buf = std::move(wordstream);

        m_pg.Write(std::move(cb));
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


CommSoft::CommSoft(std::string& read_file_name, std::string& write_file_name) :
    m_in_file_name(read_file_name),
    m_out_file_name(write_file_name)
{
    m_in_stream = std::ifstream(m_out_file_name);
    m_out_stream = std::ofstream(m_out_file_name);
    m_state = CommStreamState::STOPPED;
}

CommSoft::~CommSoft()
{
    StopStreaming();
    SetStreamState(CommStreamState::STOPPED);
    if (m_control_thread.joinable())
        m_control_thread.join();
    
    m_in_stream.close();
    m_out_stream.close();
}

void CommSoft::StartStreaming()
{
    if (CommStreamState::STOPPED == GetStreamState())
    {
        SetStreamState(CommStreamState::STARTED);

        if (!m_in_stream.is_open())
        {
            m_in_stream.open(m_in_file_name);
        }

        if (!m_out_stream.is_open())
        {
            m_out_stream.open(m_in_file_name);
        }

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

    m_in_stream.close();
    m_out_stream.close();
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

void CommSoft::ReadCallback(std::unique_ptr<PacketGeneratorCallbackData> cb)
{
    m_read_queue.push(std::ref(cb->buf));
}

void CommSoft::WriteCallback(std::unique_ptr<PacketGeneratorCallbackData> cb)
{
}


} // BDComm namespace
} // BDDriver namespace
} // PyStorm namespace
