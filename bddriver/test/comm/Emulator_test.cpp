#include <iostream>
#include <vector>
#include <cstdio>
#include <fstream>

#include "comm/Emulator.h"
#include "common/MutexBuffer.h"
#include "gtest/gtest.h"

namespace pystorm
{
namespace bddriver
{
namespace comm
{

///
/// Helper class to access protected member variables in the
/// test cases.
///
class EmulatorFixture : public Emulator {
public:
    EmulatorFixture(std::string in_file, std::string out_file) :
        Emulator(in_file, out_file) {
    }

    COMMWordStream& getWordStreamVec() {
        return m_word_stream;
    }

    std::ifstream& getInStream() {
        return m_in_stream;
    }

    std::ofstream& getOutStream() {
        return m_out_stream;
    }
};

///
/// Emulator client used in the Read and Write test cases.
///
class EmulatorClient : public EmulatorClientIfc {
public:
    EmulatorClient() {
    }
    
    void ReadCallback(std::unique_ptr<EmulatorCallbackData> cb) {
        m_read_buffer.insert(m_read_buffer.end(),
            cb->buf->begin(), cb->buf->end());
    }

    void WriteCallback(std::unique_ptr<EmulatorCallbackData> cb) {
    }

    std::vector<char> m_read_buffer;
};

///
/// Test that the constructor fails if the input and output files
/// do not exist.
///
TEST(EmulatorTests, testConstructionFails) {
    std::string infile("./infile.txt");
    std::string outfile("./outfile.txt");

    ASSERT_DEATH(new Emulator(infile, outfile),"");
}

///
/// Test that the constructor works if the input and output
/// files exist.
///
TEST(EmulatorTests, testConstructionPasses) {
    // create the temp files using a unique string
    std::string input_file_name = std::tmpnam(nullptr);
    std::string output_file_name = std::tmpnam(nullptr);
    std::ofstream input_file(input_file_name);
    std::ofstream output_file(output_file_name);

    EXPECT_NO_THROW(new Emulator(input_file_name, output_file_name));

    // close and delete the files
    input_file.close();
    output_file.close();
    std::remove(input_file_name.c_str());
    std::remove(output_file_name.c_str());
}

///
/// Test that the constructor reads the input file and constructs
/// the word stream properly. This test passes if the content and
/// size of the stream read matches the contents and size of the
/// input file created.
///
TEST(EmulatorTests, testConstructionCreatesInputStream) {
    unsigned int INPUT_FILE_SIZE = Emulator::READ_SIZE;
    std::vector<char> stream_read;

    // create the temp files
    std::string input_file_name = std::tmpnam(nullptr);
    std::string output_file_name = std::tmpnam(nullptr);
    std::ofstream input_file(input_file_name);
    std::ofstream output_file(output_file_name);

    // populate the input file
    for (unsigned int i = 0; i < INPUT_FILE_SIZE;i++) {
        input_file << 'x' << std::flush;
    }

    // read the contents of m_word_streams;
    EmulatorFixture em(input_file_name, output_file_name);
    auto wordStream = em.getWordStreamVec();

    for (auto byte : wordStream) {
        stream_read.push_back(byte);
    }

    ASSERT_EQ(stream_read.size(), INPUT_FILE_SIZE);

    for (auto byte : stream_read) {
        ASSERT_EQ(byte,'x');
    }

    // close and remove the temp files
    input_file.close();
    output_file.close();
    std::remove(input_file_name.c_str());
    std::remove(output_file_name.c_str());
}

///
/// Test that the Read method. This test passes if the content and
/// size of the stream read matches the contents and size of the
/// input file created.
///
TEST(EmulatorTests, testRead) {
    unsigned int INPUT_FILE_SIZE = Emulator::READ_SIZE;
    EmulatorClient emc;

    // create the temp files
    std::string input_file_name = std::tmpnam(nullptr);
    std::string output_file_name = std::tmpnam(nullptr);
    std::ofstream input_file(input_file_name);
    std::ofstream output_file(output_file_name);

    // populate the input file
    for (unsigned int i = 0; i < INPUT_FILE_SIZE;i++) {
        input_file << 'x' << std::flush;
    }

    // Read the contents of the input file
    EmulatorFixture em(input_file_name, output_file_name);

    while(emc.m_read_buffer.size()<INPUT_FILE_SIZE) {
        auto emcb = 
            std::unique_ptr<EmulatorCallbackData>(new EmulatorCallbackData());
        emcb->client = &emc;
        emcb->buf = std::unique_ptr<COMMWordStream>(new COMMWordStream());
        emcb->buf->resize(INPUT_FILE_SIZE);
        em.Read(std::move(emcb));
    }

    ASSERT_EQ(emc.m_read_buffer.size(),INPUT_FILE_SIZE);

    for (auto byte : emc.m_read_buffer) {
        ASSERT_EQ(byte,'x');
    }

    // close and remove the temp files
    input_file.close();
    output_file.close();
    std::remove(input_file_name.c_str());
    std::remove(output_file_name.c_str());
}

/// Test the Write method. This test passes if the contents of the
/// output file match the size of the characters written to the
/// Emulator.
///
TEST(EmulatorTests, testWrite) {
    unsigned int OUTPUT_FILE_SIZE = 1000;
    unsigned int OUTPUT_VEC_SIZE = 512;
    std::vector<char> stream_read;
    EmulatorClient emc;

    // create the temp files
    std::string input_file_name = std::tmpnam(nullptr);
    std::string output_file_name = std::tmpnam(nullptr);
    std::ofstream input_file(input_file_name);
    std::ofstream output_file(output_file_name);

    // write the data
    EmulatorFixture em(input_file_name, output_file_name);
    for (unsigned int i = 0; i < OUTPUT_FILE_SIZE;i+=OUTPUT_VEC_SIZE) {
        auto emcb = 
            std::unique_ptr<EmulatorCallbackData>(new EmulatorCallbackData());
        emcb->client = &emc;
        auto chars = 
            std::unique_ptr<COMMWordStream>(
            new COMMWordStream(
                std::min(OUTPUT_FILE_SIZE-i,OUTPUT_VEC_SIZE),'x'));
        emcb->buf=std::move(chars);
        em.Write(std::move(emcb));
    }

    // read data back in from the output file and check that the
    // size and contents are correct.
    std::ifstream out_stream(output_file_name);
    std::istream_iterator<char> start(out_stream),end;
    std::vector<char> data(start,end);

    for (auto byte : data) {
        ASSERT_EQ(byte,'x');
    }

    ASSERT_EQ(data.size(),OUTPUT_FILE_SIZE);

    // close and remove the temp files
    out_stream.close();
    input_file.close();
    output_file.close();
    std::remove(input_file_name.c_str());
    std::remove(output_file_name.c_str());
}

///
/// Test the Start method. This test passes if the input and
/// output file streams are open.
///
TEST(EmulatorTests, testStart)
{
    // create the temp files
    std::string input_file_name = std::tmpnam(nullptr);
    std::string output_file_name = std::tmpnam(nullptr);
    std::ofstream input_file(input_file_name);
    std::ofstream output_file(output_file_name);

    // write the data
    EmulatorFixture em(input_file_name, output_file_name);

    em.Start();

    ASSERT_TRUE(em.getInStream().is_open());
    ASSERT_TRUE(em.getOutStream().is_open());

    // close and remove the temp files
    input_file.close();
    output_file.close();
    std::remove(input_file_name.c_str());
    std::remove(output_file_name.c_str());

}

///
/// Test the Stop method. This test passes if the input and
/// output file streams are closed.
///
TEST(EmulatorTests, testStop)
{
    // create the temp files
    std::string input_file_name = std::tmpnam(nullptr);
    std::string output_file_name = std::tmpnam(nullptr);
    std::ofstream input_file(input_file_name);
    std::ofstream output_file(output_file_name);

    // write the data
    EmulatorFixture em(input_file_name, output_file_name);

    em.Stop();

    ASSERT_FALSE(em.getInStream().is_open());
    ASSERT_FALSE(em.getOutStream().is_open());

    // close and remove the temp files
    input_file.close();
    output_file.close();
    std::remove(input_file_name.c_str());
    std::remove(output_file_name.c_str());
}

}
}
}
