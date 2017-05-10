#ifndef EMULATOR_H
#define EMULATOR_H

#include <vector>
#include <deque>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <memory>
#include <chrono>

#include <cstring>

#include "Comm.h"

namespace pystorm {
namespace bddriver {
namespace comm {

class EmulatorClientIfc;

///
/// EmulatorCallbackData is passed to Emulator clients after a read or write
/// is performed.
///
struct EmulatorCallbackData {                                                                               
    EmulatorClientIfc* client;
    std::unique_ptr<COMMWordStream> buf;                                        
};                   

class EmulatorClientIfc {
public:
    virtual void ReadCallback(std::unique_ptr<EmulatorCallbackData> cb) = 0;
    virtual void WriteCallback(std::unique_ptr<EmulatorCallbackData> cb) = 0;
};

///
/// Emulator is used by CommSoft and emulates the libusb module by having the
/// client (i.e. an instance of CommSoft) create a callback method that is
/// invoked when Emulator has mimiced reading data from an input file or 
/// writing data to an output file.
///
class Emulator {
public:
    Emulator(const std::string& in_file_name, const std::string& out_file_name);

    Emulator() = delete;

    ~Emulator();

    ///
    /// Blocking call telling Emulator to read a stream of data
    ///
    void Read(std::unique_ptr<EmulatorCallbackData> cb);

    ///
    /// Blocking call telling Emulator to write a stream of data
    ///
    void Write(std::unique_ptr<EmulatorCallbackData> cb);

    ///
    /// Open the streams 
    ///
    void Start();

    ///
    /// Close the streams
    ///
    void Stop();

    /// The maximum number of characters in each element of m_word_streams.
    static const int READ_SIZE = 512;
protected:
    /// 
    /// Init is called by the constructor open the file handles to the 
    /// input and output files.
    void Init();

    ///
    /// Read the input file and build a vector of streams whose elements will
    /// will be passed to an emulator client when the emulator client calls 
    ///
    void BuildInputStream();

    /// The name of the input file read when the Emulator is started.
    std::string   m_in_file_name;                                               

    /// The name of the output file written to when the Emulator is started.
    std::string   m_out_file_name;                                              
       
    /// The input file stream handler                                                                         
    std::ifstream m_in_stream; 

    /// The output file stream handler                                                                         
    std::ofstream m_out_stream;                                                 

    /// The word stream that the Emulator reads from the input file.
    COMMWordStream m_word_stream;

    /// The index of the next element of m_word_stream to read from.
    unsigned int m_current_word_stream_pos;
};

} // comm namespace
} // bddriver namespace
} // pystorm namespace
#endif
