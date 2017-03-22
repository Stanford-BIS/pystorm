#ifndef EMULATOR_H
#define EMULATOR_H

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

class EmulatorClientIfc;

struct EmulatorCallbackData                                                             
{                                                                               
    EmulatorClientIfc* comm;                                                                 
    std::unique_ptr<COMMWordStream> buf;                                        
};                   

class EmulatorClientIfc
{
public:
    virtual void ReadCallback(std::unique_ptr<EmulatorCallbackData> cb) = 0;
    virtual void WriteCallback(std::unique_ptr<EmulatorCallbackData> cb) = 0;
};

class Emulator
{
public:
    Emulator(std::string& in_file_name, std::string& out_file_name);

    ~Emulator();

    /* Read
     * 
     * Blocking call telling Emulator to read a stream of data
     */
    void Read(std::unique_ptr<EmulatorCallbackData> cb);

    /* Read
     * 
     * Blocking call telling Emulator to write a stream of data
     */
    void Write(std::unique_ptr<EmulatorCallbackData> cb);

    /* Start
     * 
     * Open the streams 
     */
    void Start();

    /*
     * Stop
     *
     * Close the streams
     */
    void Stop();

protected:
    void Init();

    /*
     * BuildInputStream
     *
     * Read the input file and build a vector of streams whose elements will
     * will be passed to an emulator client when the emulator client calls 
     * Read.
     */
    void BuildInputStream();

    std::string   m_in_file_name;                                               
    std::string   m_out_file_name;                                              
                                                                                
    std::ofstream m_out_stream;                                                 
    std::ifstream m_in_stream; 

    std::vector<COMMWordStream> m_word_streams;
    int m_current_word_stream_pos;
    static const int READ_SIZE = 512;
};

} // bdcomm namespace
} // bddriver namespace
} // pystorm namespace
#endif
