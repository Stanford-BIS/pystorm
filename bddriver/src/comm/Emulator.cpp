#include <vector>
#include <deque>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <memory>
#include <iterator>
#include <assert.h>

#include <cstring>

#include "Emulator.h"

namespace pystorm {
namespace bddriver {
namespace comm {

Emulator::Emulator(const std::string& in_file_name, const std::string& out_file_name):
    m_in_file_name(in_file_name),
    m_out_file_name(out_file_name),
    m_current_word_stream_pos(0) {
    Init();
}

void Emulator::Init() {
    // reading the infile, build a vector of streams that would get returned 
    // when a read call is made to the emulator.
    m_in_stream = std::ifstream(m_in_file_name,std::ios::binary);
    assert(m_in_stream.good());

    m_out_stream = std::ofstream(m_out_file_name,std::ios::binary);
    assert(m_out_stream.good());

    BuildInputStream();
}

void Emulator::BuildInputStream() {
    std::istreambuf_iterator<char> beg(m_in_stream), end;

    m_current_word_stream_pos = 0;

    m_word_stream.clear();
    m_word_stream.resize(0);

    while(beg!=end) {
        m_word_stream.push_back(*beg);
        beg++;
    }
}

Emulator::~Emulator() {
    m_in_stream.close();
    m_out_stream.close();
}

void Emulator::Read(std::unique_ptr<EmulatorCallbackData> cb) {
    if (m_word_stream.size() > 0) {
        for (int i = 0; i < cb->buf->size(); i++) {
            cb->buf->at(i) = m_word_stream.at(m_current_word_stream_pos++);
            if (m_current_word_stream_pos == m_word_stream.size()) {
                m_current_word_stream_pos = 0;
            }
        }

        cb->client->ReadCallback(std::move(cb));
    }
}

void Emulator::Write(std::unique_ptr<EmulatorCallbackData> cb) {
    std::unique_ptr<COMMWordStream> wordstream = std::move(cb->buf);
    for (auto& i : *wordstream) {
        m_out_stream << i;
    }

    m_out_stream << std::flush;
    cb->client->WriteCallback(std::move(cb));
}

void Emulator::Start() {
    if (!m_in_stream.is_open()) {
        m_in_stream.open(m_in_file_name, std::ios::in | std::ios::binary);
    } else {
        m_in_stream.seekg(0,m_in_stream.beg);
    }

    BuildInputStream();

    if (!m_out_stream.is_open()) {                                                                       
        m_out_stream.open(m_in_file_name, std::ios::in | std::ios::binary);
    } else {
        m_out_stream.close();
        m_out_stream.open(m_out_file_name, std::ios::out | std::ios::binary);
    }
}

void Emulator::Stop() {
    m_in_stream.close();
    m_out_stream.close();
}

} // comm namespace
} // bddriver namespace
} // pystorm namespace
