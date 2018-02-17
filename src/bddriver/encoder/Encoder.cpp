#include "Encoder.h"

#include <cstdint>
#include <thread>
#include <unordered_map>
#include <vector>

#include "common/DriverPars.h"
#include "common/BDPars.h"
#include "common/BDWord.h"
#include "common/MutexBuffer.h"

#include <iostream>
#include <bitset>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

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

  output_block_->push_back(b0);
  output_block_->push_back(b1);
  output_block_->push_back(b2);
  output_block_->push_back(b3);

  // if we've got a lot of blocks, break it up
  if (output_block_->size() == driverpars::MAX_WRITE_SIZE) {
    FlushWords();
  }
}

inline void Encoder::PadNopsAndFlush() {
  // figure out how man nops are needed to pad
  unsigned int curr_size_in_frame = output_block_->size() % driverpars::WRITE_BLOCK_SIZE;
  unsigned int to_complete_block = (driverpars::WRITE_BLOCK_SIZE - curr_size_in_frame) % driverpars::WRITE_BLOCK_SIZE;

  // construct FPGA nop word
  uint8_t nop_code = bd_pars_->DnEPCodeFor(bdpars::FPGARegEP::NOP);
  BDWord nop = PackWord<FPGAIO>({{FPGAIO::PAYLOAD, 0}, {FPGAIO::EP_CODE, nop_code}, {FPGAIO::ROUTE, 0}});

  // push nops
  for (unsigned int i = 0; i < to_complete_block / 4; i++) { // 4 words per nop, so / 4
    PushWord(nop);
  }

  // and flush
  FlushWords();
}

// flush code, pad nops to complete block
inline void Encoder::FlushWords() {

  assert(driverpars::WRITE_BLOCK_SIZE % 4 == 0);
  assert(output_block_->size() % driverpars::WRITE_BLOCK_SIZE == 0);

  // move output_block_
  out_buf_->Push(std::move(output_block_)); 

  // construct new output_block_
  output_block_ = std::make_unique<std::vector<EncOutput>>();
}

//converts the core ID to the route
inline unsigned int Encoder::toRoute(unsigned int core_id) {
  if(bd_pars_->NumCores == 1)
    return 2; //THIS IS TEMPORARY, SHOULD BE 0
  return core_id+1; //for multicore system, just add 1 for now
}

void Encoder::Encode(const std::unique_ptr<std::vector<EncInput>> inputs) {

  // if multiple flushes are in the same set of inputs, just flush once
  bool flush_pending = false;

  for (auto& it : *inputs) {
    // unpack data
    unsigned int core_id      = it.core_id;
    unsigned int FPGA_ep_code = it.FPGA_ep_code;
    uint32_t     payload      = it.payload;
    BDTime       time         = it.time;

    if (FPGA_ep_code == EncInput::kFlushCode) {
      flush_pending = true;

    } else {
      unsigned int route = toRoute(core_id); // convert core ID to route
      (void)time;    // XXX this is where you would do something with time

      // pack into 32 bits
      // FPGA word format:
      //  MSB                  LSB
      //    5b      7b      20b
      // [ route | code | payload ]
      uint32_t FPGA_encoded = PackWord<FPGAIO>({{FPGAIO::PAYLOAD, payload}, {FPGAIO::EP_CODE, FPGA_ep_code}, {FPGAIO::ROUTE, route}});

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
          PushWord(PackWord<FPGAIO>({{FPGAIO::PAYLOAD, time_chunk[i]}, {FPGAIO::EP_CODE, HB_ep_code[i]}, {FPGAIO::ROUTE, host_board_route}}));
        }
      }

      // cout<<std::bitset<32>(FPGA_encoded)<<endl;
      // serialize to bytes 
      PushWord(FPGA_encoded);

      //if (FPGA_ep_code != bd_pars_->DnEPCodeFor(bdpars::FPGARegEP::NOP))
      //  PrintBinaryAsStr(FPGA_encoded, 32);
    }
  }

  if (flush_pending) {
    // pad frame to block size multiple and send to comm
    PadNopsAndFlush();
  }
}

}  // bddriver
}  // pystorm
