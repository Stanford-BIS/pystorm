#include <vector>
#include <deque>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <memory>
#include <iterator>

#include <cstring>

#include "Emulator.h"

namespace pystorm
{
namespace bddriver
{
namespace bdcomm
{

Emulator::Emulator(std::string& in_file_name, std::string& out_file_name):
    m_in_file_name(in_file_name),
    m_out_file_name(out_file_name)
{
    m_in_stream = std::ifstream(m_out_file_name);                               
    m_out_stream = std::ofstream(m_out_file_name);    
    m_current_word_stream_pos = 0;
    Init();
}

void Emulator::Init()
{
    // reading the infile, build a vector of streams that would get returned 
    // when a read call is made to the emulator.
    BuildInputStream();
}

void Emulator::BuildInputStream()
{
    // Clear the current stream vector
    // get the length of the input file
    // populate the vector of streams

    m_word_streams.clear();
    std::istreambuf_iterator<char> beg(m_in_stream), end;

    int current_stream_pos = 0;
    int current_vec_pos = 0;

    while(beg!=end)
    {
        m_word_streams[current_vec_pos][current_stream_pos] = *beg;
        beg++;
        current_stream_pos++;
        if (READ_SIZE == current_stream_pos)
        {
            current_stream_pos = 0;
            current_vec_pos++;
        }
    }
}

Emulator::~Emulator()
{
    m_in_stream.close();
    m_out_stream.close();
}

void Emulator::Read(std::unique_ptr<EmulatorCallbackData> cb)
{
    if (m_word_streams.size() > 0)
    {
        if ((m_word_streams.size() - 1) == m_current_word_stream_pos)
        {
            m_current_word_stream_pos = 0;
        }

        auto wordstream =
            std::unique_ptr<pystorm::bddriver::bdcomm::COMMWordStream>(             
            new pystorm::bddriver::bdcomm::COMMWordStream());

        *wordstream = m_word_streams.at(m_current_word_stream_pos);

        cb->buf = std::move(wordstream);
        cb->comm->ReadCallback(std::move(cb));
    }
}

void Emulator::Write(std::unique_ptr<EmulatorCallbackData> cb)
{
    // Need to modify this to so that we write to the outfile
    std::unique_ptr<COMMWordStream> wordstream = std::move(cb->buf);
    for (auto& i : *wordstream)
    {
        std::cout << i;
    }
    std::cout << std::endl;

    cb->comm->WriteCallback(std::move(cb));
}

void Emulator::Start()
{
    if (!m_in_stream.is_open())                                             
    {                                                                       
        m_in_stream.open(m_in_file_name, std::ios::in | std::ios::binary);
    }                                                                       
    else
    {
        m_in_stream.seekg(0,m_in_stream.beg);
    }

    BuildInputStream();
        
                                                                          
    if (!m_out_stream.is_open())                                            
    {                                                                       
        m_out_stream.open(m_in_file_name, std::ios::in | std::ios::binary);
    }                                                                       
    else
    {
        m_out_stream.close();
        m_out_stream.open(m_out_file_name, std::ios::out | std::ios::binary);
    }
}

void Emulator::Stop()
{
    m_in_stream.close();
    m_out_stream.close();
}

} // bdcomm namespace
} // bddriver namespace
} // pystorm namespace
