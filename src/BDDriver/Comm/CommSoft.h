#ifndef COMMSOFT_H
#define COMMSOFT_H

#include <vector>
#include <deque>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <memory>

#include <cstring>

#include "common/TSQueue.h"
#include "Comm.h"
#include "PacketGenerator.h"

namespace pystorm
{
namespace bddriver
{
namespace bdcomm
{

class CommSoft : public Comm, PacketGeneratorClientIfc
{
public:
    CommSoft(std::string& read_file_name, std::string& write_file_name);
    ~CommSoft();
    CommSoft(const CommSoft&) = delete;

    // Comm interface

    virtual void StartStreaming();

    virtual void StopStreaming();

    virtual void Write(std::unique_ptr<COMMWordStream> wordStream);

    virtual std::unique_ptr<COMMWordStream> Read();

    // PacketGeneratorCallbackIfc interface

    virtual void ReadCallback(std::unique_ptr<PacketGeneratorCallbackData> cb);
    virtual void WriteCallback(std::unique_ptr<PacketGeneratorCallbackData> cb);

protected:
    void CommSoftController();

    // Read packets in from file and place them on read queue
    void ReadPacketsIn();

    // Read packets from write queue and place them into file
    void WritePacketsOut();

    std::string   m_in_file_name;
    std::string   m_out_file_name;

    std::ofstream m_out_stream;
    std::ifstream m_in_stream;

    TSQueue<std::unique_ptr<COMMWordStream> > m_read_queue;
    TSQueue<std::unique_ptr<COMMWordStream> > m_write_queue;

    std::thread m_control_thread;

    PacketGenerator m_pg;

};

} // bdcomm namespace
} // bddriver namespace
} // pystorm namespace
#endif
