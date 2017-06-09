#include <iostream>
#include <vector>
#include <cstdio>
#include <fstream>
#include <chrono>
#include <thread>

#include "comm/CommSoft.h"
#include "common/MutexBuffer.h"
#include "gtest/gtest.h"

namespace pystorm {
namespace bddriver {
namespace comm {

unsigned int COMMSOFT_BUFFER_CAP = 10000;

///
/// Test that the constructor fails if the input and output files
/// do not exist.
///
TEST(CommSoftTests, testConstructionFails) {
    std::string infile("./infile.txt");
    std::string outfile("./outfile.txt");
    MutexBuffer<COMMWord> * read_buffer = 
        new MutexBuffer<COMMWord>(COMMSOFT_BUFFER_CAP);
    MutexBuffer<COMMWord> * write_buffer = 
        new MutexBuffer<COMMWord>(COMMSOFT_BUFFER_CAP);

    ASSERT_DEATH(new CommSoft(infile, outfile,read_buffer, write_buffer),"");
}

///
/// Test that the constructor works if the input and output
/// files exist.
///
TEST(CommSoftTests, testConstructionPasses) {
    // create the temp files using a unique string
    std::string input_file_name = std::tmpnam(nullptr);
    std::string output_file_name = std::tmpnam(nullptr);
    std::ofstream input_file(input_file_name);
    std::ofstream output_file(output_file_name);
    MutexBuffer<COMMWord> * read_buffer = 
        new MutexBuffer<COMMWord>(COMMSOFT_BUFFER_CAP);
    MutexBuffer<COMMWord> * write_buffer = 
        new MutexBuffer<COMMWord>(COMMSOFT_BUFFER_CAP);

    EXPECT_NO_THROW(new CommSoft(input_file_name, output_file_name,
        read_buffer, write_buffer));

    // close and delete the files
    input_file.close();
    output_file.close();
    std::remove(input_file_name.c_str());
    std::remove(output_file_name.c_str());
}

///
/// Tests that read and write buffers are empty if streaming is not started.
///
TEST(CommSoftTests, testNoStreaming) {
    // Create the input/output files
    unsigned int commwordstream_size = 512;
    std::string input_file_name = std::tmpnam(nullptr);
    std::string output_file_name = std::tmpnam(nullptr);
    std::ofstream input_file(input_file_name);
    std::ofstream output_file(output_file_name);
    MutexBuffer<COMMWord> * read_buffer = 
        new MutexBuffer<COMMWord>(COMMSOFT_BUFFER_CAP);
    MutexBuffer<COMMWord> * write_buffer = 
        new MutexBuffer<COMMWord>(COMMSOFT_BUFFER_CAP);
    CommSoft cs(input_file_name, output_file_name, read_buffer, write_buffer);
    unsigned int buffer_timeout = 10;
    unsigned int empty_buffer_size = 0;
    COMMWordStream vecOfCWS;

    for (unsigned int i = 0; i < commwordstream_size;i++) {
        input_file << 'x' << std::flush;
    }

    // create the buffers

    vecOfCWS = COMMWordStream(COMMSOFT_BUFFER_CAP, 'x');
    write_buffer->Push(vecOfCWS,0);

    // sleep for a couple of seconds 
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // There should be a specific number of entries in the buffer ...
    vecOfCWS = write_buffer->PopVect(COMMSOFT_BUFFER_CAP, buffer_timeout);
    EXPECT_EQ(vecOfCWS.size(), COMMSOFT_BUFFER_CAP);

    // ... and no more.
    vecOfCWS = write_buffer->PopVect(COMMSOFT_BUFFER_CAP, buffer_timeout);
    EXPECT_EQ(vecOfCWS.size(), empty_buffer_size);

    // Try to read at least one entry from the read buffer
    // there should 0 entries since streaming is not started
    vecOfCWS = read_buffer->PopVect(1, buffer_timeout);
    EXPECT_EQ(vecOfCWS.size(),empty_buffer_size);

    // Try to read from the output file and 
    // nothing should be written
    output_file.seekp(0,std::ios_base::end);
    EXPECT_EQ(output_file.tellp(), empty_buffer_size);

    // clean up
    input_file.close();
    output_file.close();
    std::remove(input_file_name.c_str());
    std::remove(output_file_name.c_str());
}

void readBufferThread(MutexBuffer<COMMWord> * read_buffer, double timeout,
    unsigned int read_size)
{
    auto init_time = std::chrono::system_clock::now();
    auto time_now = std::chrono::system_clock::now();
    std::chrono::duration<double> time_diff = time_now - init_time;
    // buffer_timeout in microseconds
    unsigned int buffer_timeout = 10;
    COMMWordStream vecOfCWS;

    while (time_diff.count() < timeout) {
        vecOfCWS = read_buffer->PopVect(read_size, buffer_timeout);
        time_diff = std::chrono::system_clock::now() - init_time;
    }
}

/// Test Writing
///
/// Tests that when elements are pushed onto the write buffer, they wind up
/// in the output file.
///
/// Note that a separate thread is started to continuously pop from the read
/// buffer. If this is not done, the read buffer blocks and no writes occur.
///
TEST(CommSoftTests, testWriting) {
    // Create the input/output files
    unsigned int commwordstream_size = 512;
    std::string input_file_name = std::tmpnam(nullptr);
    std::string output_file_name = std::tmpnam(nullptr);
    std::ofstream input_file(input_file_name);
    std::ofstream output_file(output_file_name);
    MutexBuffer<COMMWord> * read_buffer = 
        new MutexBuffer<COMMWord>(COMMSOFT_BUFFER_CAP);
    MutexBuffer<COMMWord> * write_buffer = 
        new MutexBuffer<COMMWord>(COMMSOFT_BUFFER_CAP);
    CommSoft cs(input_file_name, output_file_name, read_buffer, write_buffer);
    COMMWordStream vecOfCWS;
    unsigned int buffer_timeout = 10;
    unsigned int empty_buffer_size = 0;
    double timeout = 5;

    // fill the input file so that Comm can Read and Write
    for (unsigned int i = 0; i < commwordstream_size;i++) {
        input_file << 'x' << std::flush;
    }

    vecOfCWS = COMMWordStream(COMMSOFT_BUFFER_CAP,'x');
    write_buffer->Push(vecOfCWS,0);

    // Now start streaming

    cs.StartStreaming();

    // Need to keep reading because Comm blocks if the read buffer
    // is full
    std::thread readThread(readBufferThread, read_buffer, timeout, 
        commwordstream_size);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Write buffer should be initially empty
    vecOfCWS = write_buffer->PopVect(COMMSOFT_BUFFER_CAP, buffer_timeout);
    EXPECT_EQ(vecOfCWS.size(), empty_buffer_size);

    //Populate the write buffer
    vecOfCWS = COMMWordStream(COMMSOFT_BUFFER_CAP, 'x');
    write_buffer->Push(vecOfCWS,0);

    std::this_thread::sleep_for(std::chrono::seconds(1));

   // Write buffer should be empty
    vecOfCWS = write_buffer->PopVect(COMMSOFT_BUFFER_CAP, buffer_timeout);
    EXPECT_EQ(vecOfCWS.size(), empty_buffer_size);

    // Stop streaming
    cs.StopStreaming();

    // Repopulate the write buffer
    vecOfCWS = COMMWordStream(COMMSOFT_BUFFER_CAP, 'x');
    write_buffer->Push(vecOfCWS, 0);

    // Write buffer should not be empty, and
    // output file should be populated 

    vecOfCWS = write_buffer->PopVect(COMMSOFT_BUFFER_CAP,buffer_timeout);
    EXPECT_EQ(vecOfCWS.size(), COMMSOFT_BUFFER_CAP);

    // Output file should be populated 
    // Why multiple of 2? because we should've filled and drained the write 
    // buffer twice
    output_file.seekp(0, std::ios_base::end);
    EXPECT_EQ(output_file.tellp(), 2*COMMSOFT_BUFFER_CAP);

    // clean up
    
    readThread.join();
    input_file.close();
    output_file.close();
    std::remove(input_file_name.c_str());
    std::remove(output_file_name.c_str());
}

/// Test Reading
///
/// Tests that elements are pushed onto the read buffer only when streaming
/// is started.
///
TEST(CommSoftTests, testReading) {
    unsigned int commwordstream_size = 512;
    unsigned int commwordstream_size_adjusted = 
        (COMMSOFT_BUFFER_CAP / commwordstream_size) * commwordstream_size;
    std::string input_file_name = std::tmpnam(nullptr);
    std::string output_file_name = std::tmpnam(nullptr);
    std::ofstream input_file(input_file_name);
    std::ofstream output_file(output_file_name);
    MutexBuffer<COMMWord> * read_buffer = 
        new MutexBuffer<COMMWord>(COMMSOFT_BUFFER_CAP);
    MutexBuffer<COMMWord> * write_buffer = 
        new MutexBuffer<COMMWord>(COMMSOFT_BUFFER_CAP);
    CommSoft cs(input_file_name, output_file_name, read_buffer, write_buffer);
    unsigned int buffer_timeout = 10;
    unsigned int empty_buffer_size = 0;
    unsigned int first_read_size, second_read_size, third_read_size;
    COMMWordStream vecOfCWS;

    // Populate the input file
    for (unsigned int i = 0; i < commwordstream_size;i++) {
        input_file << 'x' << std::flush;
    }

    // Since we have not started streaming, there should be no
    // entries in the read buffer
    vecOfCWS = read_buffer->PopVect(COMMSOFT_BUFFER_CAP, buffer_timeout);
    first_read_size = vecOfCWS.size();
    EXPECT_EQ(first_read_size, empty_buffer_size);

    // Now start streaming, then stop
    cs.StartStreaming();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    cs.StopStreaming();

    // The read buffer should be larger than before streaming
    vecOfCWS = read_buffer->PopVect(COMMSOFT_BUFFER_CAP, buffer_timeout);
    second_read_size = first_read_size + vecOfCWS.size();
    EXPECT_GT(second_read_size, first_read_size);

    // Knowing the commword sizes and the max buffer capacity, we 
    // can determine the number of characters we should have read

    EXPECT_EQ(vecOfCWS.size(), commwordstream_size_adjusted);

    // And since we are no longer streaming, the read buffer
    // should not change
    vecOfCWS = read_buffer->PopVect(COMMSOFT_BUFFER_CAP, buffer_timeout);

    third_read_size = second_read_size + vecOfCWS.size();
    EXPECT_EQ(third_read_size, second_read_size);
    
    // Write buffer should not change

    vecOfCWS = write_buffer->PopVect(COMMSOFT_BUFFER_CAP, buffer_timeout);
    EXPECT_EQ(vecOfCWS.size(), empty_buffer_size);

    // Output file should not have changed either
    output_file.seekp(0,std::ios_base::end);
    EXPECT_EQ(output_file.tellp(), empty_buffer_size);

    // clean up
    input_file.close();
    output_file.close();
    std::remove(input_file_name.c_str());
    std::remove(output_file_name.c_str());
}

}
}
}
