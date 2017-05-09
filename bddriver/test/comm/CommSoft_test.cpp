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

///
/// Test that the constructor fails if the input and output files
/// do not exist.
///
TEST(CommSoftTests, testConstructionFails) {
    std::string infile("./infile.txt");
    std::string outfile("./outfile.txt");

    ASSERT_DEATH(new CommSoft(infile, outfile),"");
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

    EXPECT_NO_THROW(new CommSoft(input_file_name, output_file_name));

    // close and delete the files
    input_file.close();
    output_file.close();
    std::remove(input_file_name.c_str());
    std::remove(output_file_name.c_str());
}

///
/// Test StartStreaming
///
/// After creating CommSoft and before starting the streams,
/// the CommSoft state should be stopped, the encoding mutex
/// buffers size should be be equal to it's initial size,
/// the decoding mutex buffers size should be zero,
/// and the output file should be empty.
///
TEST(CommSoftTests, testNoStreaming) {
    // Create the input/output files
    unsigned int commwordstream_size = 512;
    std::string input_file_name = std::tmpnam(nullptr);
    std::string output_file_name = std::tmpnam(nullptr);
    std::ofstream input_file(input_file_name);
    std::ofstream output_file(output_file_name);
    CommSoft cs(input_file_name, output_file_name);
    unsigned int expected_write_buffer_size = CommSoft::CAPACITY;
    unsigned int buffer_timeout = 10;
    unsigned int empty_buffer_size = 0;
    auto read_buffer = cs.getReadBuffer();
    auto write_buffer = cs.getWriteBuffer();
    std::vector<COMMWordStream> vecOfCWS;

    for (unsigned int i = 0; i < commwordstream_size;i++) {
        input_file << 'x' << std::flush;
    }

    // create the buffers

    for (unsigned int i = 0;i < expected_write_buffer_size; i++) {
        std::vector<COMMWordStream> vecOfCWS;
        vecOfCWS.push_back(COMMWordStream(commwordstream_size,'x'));
        write_buffer->Push(vecOfCWS,0);
    }

    // sleep for a couple of seconds 
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // There should be a specific number of entries in the buffer ...
    vecOfCWS = write_buffer->PopVect(expected_write_buffer_size,buffer_timeout);
    EXPECT_EQ(vecOfCWS.size(),expected_write_buffer_size);

    // ... and no more.
    vecOfCWS = write_buffer->PopVect(expected_write_buffer_size,buffer_timeout);
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

TEST(CommSoftTests, testWriting) {
    // Create the input/output files
    unsigned int commwordstream_size = 512;
    std::string input_file_name = std::tmpnam(nullptr);
    std::string output_file_name = std::tmpnam(nullptr);
    std::ofstream input_file(input_file_name);
    std::ofstream output_file(output_file_name);
    CommSoft cs(input_file_name, output_file_name);
    auto write_buffer = cs.getWriteBuffer();
    auto read_buffer = cs.getReadBuffer();
    std::vector<COMMWordStream> vecOfCWS;
    unsigned int expected_write_buffer_size = CommSoft::CAPACITY;
    unsigned int expected_read_buffer_size = CommSoft::CAPACITY;
    unsigned int buffer_timeout = 10;
    unsigned int empty_buffer_size = 0;

    // fill the input file so that Comm can Read and Write
    for (unsigned int i = 0; i < commwordstream_size;i++) {
        input_file << 'x' << std::flush;
    }

    for (unsigned int i = 0;i < expected_write_buffer_size; i++) {
        std::vector<COMMWordStream> vecOfCWS;
        vecOfCWS.push_back(COMMWordStream(commwordstream_size,'x'));
        write_buffer->Push(vecOfCWS,buffer_timeout);
    }

    // Now start streaming

    cs.StartStreaming();

    // Need to keep reading because Comm blocks if the read buffer
    // is full
    vecOfCWS = read_buffer->PopVect(expected_read_buffer_size,buffer_timeout);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    vecOfCWS = read_buffer->PopVect(expected_read_buffer_size,buffer_timeout);

    // Write buffer should be initially empty
    vecOfCWS = write_buffer->PopVect(expected_write_buffer_size,buffer_timeout);
    EXPECT_EQ(vecOfCWS.size(), empty_buffer_size);

    //Populate the write buffer
    for (unsigned int i = 0;i < expected_write_buffer_size; i++) {
        std::vector<COMMWordStream> vecOfCWS;
        vecOfCWS.push_back(COMMWordStream(commwordstream_size,'x'));
        write_buffer->Push(vecOfCWS,buffer_timeout);
    }

    vecOfCWS = read_buffer->PopVect(expected_read_buffer_size,buffer_timeout);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    vecOfCWS = read_buffer->PopVect(expected_read_buffer_size,buffer_timeout);

    // Write buffer should be empty
    vecOfCWS = write_buffer->PopVect(expected_write_buffer_size,buffer_timeout);
    EXPECT_EQ(vecOfCWS.size(), empty_buffer_size);

    // Stop streaming
    cs.StopStreaming();

    // Repopulate the write buffer
    for (unsigned int i = 0;i < expected_write_buffer_size; i++) {
        std::vector<COMMWordStream> vecOfCWS;
        vecOfCWS.push_back(COMMWordStream(512,'x'));
        write_buffer->Push(vecOfCWS,buffer_timeout);
    }

    // Write buffer should not be empty, and
    // output file should be populated 

    vecOfCWS = write_buffer->PopVect(expected_write_buffer_size,buffer_timeout);
    EXPECT_EQ(vecOfCWS.size(), (unsigned int) expected_write_buffer_size);

    // Output file should be populated 
    // Why multiple of 2? because we should've filled and drained the write 
    // buffer twice
    output_file.seekp(0,std::ios_base::end);
    EXPECT_EQ(output_file.tellp(), 
        (unsigned int) 2*expected_write_buffer_size*commwordstream_size);

    // clean up
    input_file.close();
    output_file.close();
    std::remove(input_file_name.c_str());
    std::remove(output_file_name.c_str());
}

TEST(CommSoftTests, testReading) {
    unsigned int commwordstream_size = 512;
    std::string input_file_name = std::tmpnam(nullptr);
    std::string output_file_name = std::tmpnam(nullptr);
    std::ofstream input_file(input_file_name);
    std::ofstream output_file(output_file_name);
    CommSoft cs(input_file_name, output_file_name);
    unsigned int expected_read_buffer_size = CommSoft::CAPACITY;
    auto read_buffer = cs.getReadBuffer();
    auto write_buffer = cs.getWriteBuffer();
    unsigned int buffer_timeout = 10;
    unsigned int empty_buffer_size = 0;
    unsigned int first_read_size, second_read_size, third_read_size;
    std::vector<COMMWordStream> vecOfCWS;

    // Populate the input file
    for (unsigned int i = 0; i < commwordstream_size;i++) {
        input_file << 'x' << std::flush;
    }

    // Since we have not started streaming, there should be no
    // entries in the read buffer
    vecOfCWS = read_buffer->PopVect(expected_read_buffer_size,
        buffer_timeout);
    first_read_size = vecOfCWS.size();
    EXPECT_EQ(first_read_size, empty_buffer_size);

    // Now start streaming, then stop
    cs.StartStreaming();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    cs.StopStreaming();

    // The read buffer should be larger than before 
    vecOfCWS = read_buffer->PopVect(expected_read_buffer_size,
        buffer_timeout);
    second_read_size = first_read_size + vecOfCWS.size();
    EXPECT_GT(second_read_size, first_read_size);

    // Knowing the commword sizes and the max buffer capacity, we 
    // can determine the number of characters we should have read
    unsigned int total_chars = 0;
    for (auto stream : vecOfCWS) {
        total_chars += stream.size();
    }

    EXPECT_EQ(total_chars, commwordstream_size * expected_read_buffer_size);

    // And since we are no longer streaming, the read buffer
    // should not change
    vecOfCWS = read_buffer->PopVect(expected_read_buffer_size,
        buffer_timeout);

    third_read_size = second_read_size + vecOfCWS.size();
    EXPECT_EQ(third_read_size, second_read_size);

    
    // Write buffer should not change

    vecOfCWS = write_buffer->PopVect(expected_read_buffer_size,
        buffer_timeout);
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
