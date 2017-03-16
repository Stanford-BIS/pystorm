#ifndef COMM_H
#define COMM_H

#include <vector>
#include <deque>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>

#include "common/TSQueue.h"

namespace PyStorm
{
namespace BDDriver
{
namespace BDComm
{

enum class CommStreamState { STARTED=0, STOPPED=1 };

template <typename Enumeration>
auto as_integer(Enumeration const value)
    -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

typedef unsigned char COMMWord;
typedef std::vector<COMMWord> COMMWordStream;
typedef std::deque<std::unique_ptr<COMMWordStream> > COMMWordStreamQueue;

// Comm Interface
class Comm
{
public:
    virtual void startStreaming() = 0;
    virtual void stopStreaming() = 0;
    CommStreamState getStreamState()
    {
        std::lock_guard<std::recursive_mutex> lck(m_state_mutex);
        return m_state;
    };

    void setStreamState(CommStreamState new_state)
    {
        std::lock_guard<std::recursive_mutex> lck(m_state_mutex);
        m_state = new_state;
    };

    virtual void Write(std::unique_ptr<COMMWordStream> wordStream) = 0;
    virtual std::unique_ptr<COMMWordStream> Read() = 0;
    
protected:

    CommStreamState m_state;
    std::recursive_mutex m_state_mutex;

};

class CommSoft : public Comm
{
public:
    CommSoft(std::string& read_file_name, std::string& write_file_name);
    ~CommSoft();
    CommSoft(const CommSoft&) = delete;

    virtual void startStreaming();

    virtual void stopStreaming();

    virtual void Write(std::unique_ptr<COMMWordStream> wordStream);

    virtual std::unique_ptr<COMMWordStream> Read();

protected:
    std::string   m_in_file_name;
    std::string   m_out_file_name;
    std::ofstream m_out_stream;
    std::ifstream m_in_stream;
    TSQueue<std::unique_ptr<COMMWordStream> > m_read_queue;
    TSQueue<std::unique_ptr<COMMWordStream> > m_write_queue;

    std::thread m_control_thread;

    // We want to implement a software comm controller that functions similar 
    // to the usb comm controller will in that it will pull data from a file, 
    // and add it to the read queue and pull data from the write queue and 
    // output it to a file (or std::cout).
    void CommSoftController();
};

}
}
}
#endif
