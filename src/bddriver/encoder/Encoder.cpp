#include "Encoder.h"

#include <cstdint>
#include <thread>
#include <unordered_map>
#include <vector>

#include "common/BDPars.h"
#include "common/BDWord.h"
#include "common/MutexBuffer.h"

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

static const unsigned int WORDS_PER_BLOCK = 512; // XXX should get from comm
static const unsigned int MAX_BLOCKS = 16; // break up blocks at this boundary

void PrintBinaryAsStr(uint32_t b, unsigned int N) {
  std::vector<bool> bits;
  // pop LSBs
  for (unsigned int i = 0; i < N; i++) {
    bool low_bit = b % 2 == 1;
    b = b >> 1;
    bits.push_back(low_bit);
  }
  for (unsigned int i = 0; i < N; i++) {
    if (bits.at(N - 1 - i))
      cout << "1";
    else
      cout << "0";
  }
  cout << endl;
}

void Encoder::RunOnce() {
  // we may time out for the Pop, (which can block indefinitely), giving us a chance to be killed
  std::unique_ptr<std::vector<EncInput>> popped_vect = in_buf_->Pop(timeout_us_);
  if (popped_vect->size() > 0) {
    Encode(std::move(popped_vect));
  }
}

inline void Encoder::PushWord(uint32_t word) {

  uint8_t b0 = GetField<FPGABYTES>(word, FPGABYTES::B0);
  uint8_t b1 = GetField<FPGABYTES>(word, FPGABYTES::B1);
  uint8_t b2 = GetField<FPGABYTES>(word, FPGABYTES::B2);
  uint8_t b3 = GetField<FPGABYTES>(word, FPGABYTES::B3);

  working_block_->push_back(b0);
  working_block_->push_back(b1);
  working_block_->push_back(b2);
  working_block_->push_back(b3);

  // if we've got a lot of blocks, break it up
  if (working_block_->size() == MAX_BLOCKS * WORDS_PER_BLOCK) {
    FlushWords();
  }
}

inline void Encoder::PadNopsAndFlush() {
  // figure out how man nops are needed to pad
  unsigned int curr_size_in_frame = working_block_->size() % WORDS_PER_BLOCK;
  unsigned int to_complete_block = (WORDS_PER_BLOCK - curr_size_in_frame) % WORDS_PER_BLOCK;

  // construct FPGA nop word
  uint8_t nop_code = bd_pars_->DnEPCodeFor(bdpars::FPGARegEP::NOP);
  BDWord nop = PackWord<FPGAIO>({{FPGAIO::PAYLOAD, 0}, {FPGAIO::EP_CODE, nop_code}});

  // push nops
  for (unsigned int i = 0; i < to_complete_block / 4; i++) { // 4 words per nop, so / 4
    PushWord(nop);
  }

  // and flush
  FlushWords();
}

// flush code, pad nops to complete block
inline void Encoder::FlushWords() {

  assert(WORDS_PER_BLOCK % 4 == 0);
  assert(working_block_->size() % WORDS_PER_BLOCK == 0);

  // move working_block_
  out_buf_->Push(std::move(working_block_)); 

  // construct new working_block_
  working_block_ = std::make_unique<std::vector<EncOutput>>();
}

void Encoder::Encode(const std::unique_ptr<std::vector<EncInput>> inputs) {

  for (auto& it : *inputs) {
    // unpack data
    unsigned int core_id      = it.core_id;
    unsigned int FPGA_ep_code = it.FPGA_ep_code;
    uint32_t     payload      = it.payload;
    BDTime       time         = it.time;

    if (FPGA_ep_code == EncInput::kFlushCode) {
      // pad frame to block size multiple and send to comm
      PadNopsAndFlush();

    } else {
      (void)core_id; // XXX this is where you would do something with core_id
      (void)time;    // XXX this is where you would do something with time

      // pack into 32 bits
      // FPGA word format:
      //  MSB          LSB
      //    8b      24b
      // [ code | payload ]
      uint32_t FPGA_encoded = PackWord<FPGAIO>({{FPGAIO::PAYLOAD, payload}, {FPGAIO::EP_CODE, FPGA_ep_code}});

      // if it's been more than DnTimeUnitsPerHB since we last sent a HB, 
      // package the event's time into a spike
      if (time - last_HB_sent_at_ >= bd_pars_->DnTimeUnitsPerHB) {
        last_HB_sent_at_ = time;

        // need to insert two words
        uint32_t time_chunk[3];
        time_chunk[0] = GetField<THREEFPGAREGS>(time, THREEFPGAREGS::W0);
        time_chunk[1] = GetField<THREEFPGAREGS>(time, THREEFPGAREGS::W1);
        time_chunk[2] = GetField<THREEFPGAREGS>(time, THREEFPGAREGS::W2);
        
        // manually compute offset from this code
        uint8_t HB_ep_code[3];
        HB_ep_code[0] = bd_pars_->DnEPCodeFor(bdpars::FPGARegEP::TM_PC_TIME_ELAPSED0);
        HB_ep_code[1] = bd_pars_->DnEPCodeFor(bdpars::FPGARegEP::TM_PC_TIME_ELAPSED1);
        HB_ep_code[2] = bd_pars_->DnEPCodeFor(bdpars::FPGARegEP::TM_PC_TIME_ELAPSED2);

        for (unsigned int i = 0; i < 3; i++) {
          PushWord(PackWord<FPGAIO>({{FPGAIO::PAYLOAD, time_chunk[i]}, {FPGAIO::EP_CODE, HB_ep_code[i]}}));
        }
      }

      // serialize to bytes 
      PushWord(FPGA_encoded);

      //if (FPGA_ep_code != bd_pars_->DnEPCodeFor(bdpars::FPGARegEP::NOP))
      //  PrintBinaryAsStr(FPGA_encoded, 32);

    }
  }
}

}  // bddriver
}  // pystorm
