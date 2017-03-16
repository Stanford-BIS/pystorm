#include "Comm.h"

namespace PyStorm
{
namespace BDDriver
{
namespace BDComm
{

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
    // check that the thread joined
    
    m_in_stream.close();
    m_out_stream.close();
}

void CommSoft::startStreaming()
{
    std::lock_guard<std::mutex> lck(m_state_mutex);
    if (CommStreamState::STOPPED == m_state)
    {
        // open the streams
        m_controlThread = std::thread(&CommSoft::CommSoftController,this);
    }
}

void CommSoft::stopStreaming()
{
    std::lock_guard<std::mutex> lck(m_state_mutex);
    if (CommStreamState::STARTED == m_state)
    {
        m_state = CommStreamState::STOPPED;
        // check that the thread joined
        // close the streams
        // clean up the queues
    }
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

void CommSoft::CommSoftController()
{
    std::cout << "In CommSoftController" << std::endl;
    // while CommStreamState::STARTED == getStreamState()
    // {
    //  read 
    //      then 
    //  write
    // }
}

}
}
}
