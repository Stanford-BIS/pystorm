#ifndef PACKETGEN_H
#define PACKETGEN_H

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

namespace pystorm
{
namespace bddriver
{
namespace bdcomm
{

class PacketGeneratorClientIfc;

struct PacketGeneratorCallbackData                                                             
{                                                                               
    PacketGeneratorClientIfc* comm;                                                                 
    std::unique_ptr<COMMWordStream> buf;                                        
};                   

class PacketGeneratorClientIfc
{
public:
    virtual void ReadCallback(std::unique_ptr<PacketGeneratorCallbackData> cb) = 0;
    virtual void WriteCallback(std::unique_ptr<PacketGeneratorCallbackData> cb) = 0;
};

class PacketGenerator
{
public:
    int m_count;
    PacketGenerator()
    {
        m_count = 0;
    }

    ~PacketGenerator()
    {
    }

    void Read(std::unique_ptr<PacketGeneratorCallbackData> cb)
    {
        // Need to modify this to so that we can create parameterized 
        // streams setting delays and stream sized
        if (m_count < 2)
        {
        const char * testdata = "Good morning america.";         
        unsigned char * buffer = (unsigned char *) testdata;                        
        pystorm::bddriver::bdcomm::COMMWordStream::size_type size = strlen((const char *) buffer);
                                                                                    
        std::unique_ptr<pystorm::bddriver::bdcomm::COMMWordStream> wordstream =     
            std::unique_ptr<pystorm::bddriver::bdcomm::COMMWordStream>(             
                new pystorm::bddriver::bdcomm::COMMWordStream(buffer, buffer + size));

        cb->buf = std::move(wordstream);
        cb->comm->ReadCallback(std::move(cb));
        m_count++;
        }
    }

    void Write(std::unique_ptr<PacketGeneratorCallbackData> cb)
    {
        // Need to modify this to so that we can write to a file
        std::unique_ptr<COMMWordStream> wordstream = std::move(cb->buf);
        for (auto& i : *wordstream)
        {
            std::cout << i << std::endl;
        }

        cb->comm->WriteCallback(std::move(cb));
    }
};

} // bdcomm namespace
} // bddriver namespace
} // pystorm namespace
#endif
