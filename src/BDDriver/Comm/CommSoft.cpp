#include "Comm.h"

namespace PyStorm
{
namespace BDDriver
{
namespace BDComm
{

void CommSoft::CommSoftController()
{
    std::unique_ptr<COMMWordStream> value;
    
    while (CommStreamState::STARTED == getStreamState())
    {
        // read

        auto retValue = m_read_queue.try_pop(value);
        if (true == retValue)
        {
        //    std::cout << "have value" << std::endl;
        }
        else
        {
        //    std::cout << "no value" << std::endl;
        }

        //  write
    }
    std::cout << "out" << std::endl;
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
    std::cout << "In destructor" << std::endl;
    stopStreaming();
    setStreamState(CommStreamState::STOPPED);
    if (m_control_thread.joinable())
        m_control_thread.join();
    
    m_in_stream.close();
    m_out_stream.close();
    std::cout << "Leaving destructor" << std::endl;
}

void CommSoft::startStreaming()
{
    std::lock_guard<std::recursive_mutex> lck(m_state_mutex);
    if (CommStreamState::STOPPED == m_state)
    {
        if (!m_in_stream.is_open())
        {
            m_in_stream.open(m_in_file_name);
        }

        if (!m_out_stream.is_open())
        {
            m_out_stream.open(m_in_file_name);
        }

        setStreamState(CommStreamState::STARTED);

        m_control_thread = std::thread(&CommSoft::CommSoftController, this);
    }
}

void CommSoft::stopStreaming()
{
    std::cout << "In stop streaming" << std::endl;
    {
        std::lock_guard<std::recursive_mutex> lck(m_state_mutex);
        if (CommStreamState::STARTED == getStreamState())
        {
            setStreamState(CommStreamState::STOPPED);
        }
    }

    // Wait for the thread to join
    // close the streams
    // clear the queues

    if (m_control_thread.joinable())
        m_control_thread.join();

    m_in_stream.close();
    m_out_stream.close();

    std::cout << "Leaving stop streaming" << std::endl;
    
}

void CommSoft::Write(std::unique_ptr<COMMWordStream> wordStream)
{
    m_write_queue.push(wordStream);
}

std::unique_ptr<COMMWordStream> CommSoft::Read()
{
    std::unique_ptr<COMMWordStream> ret_value;
    m_read_queue.wait_and_pop(ret_value);
    return ret_value;
}


}
}
}
