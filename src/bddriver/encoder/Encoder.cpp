#include "Encoder.h"

#include <cstdint>
#include <thread>
#include <unordered_map>
#include <vector>
#include <chrono>

#include "common/DriverPars.h"
#include "common/BDPars.h"
#include "common/BDWord.h"
#include "common/MutexBuffer.h"

#include <iostream>
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

void Encoder::ResetBaseTime() {
  base_time_ = std::chrono::high_resolution_clock::now();
}

BDTime Encoder::GetRelativeTime() const {
  auto time_point_now = std::chrono::high_resolution_clock::now();
  auto ns_now = std::chrono::duration_cast<std::chrono::nanoseconds>(time_point_now - base_time_);
  return ns_now.count();
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

  assert(driverpars::WRITE_BLOCK_SIZE % 4 == 0);
  unsigned int this_block_size = output_block_->size();
  assert(this_block_size % driverpars::WRITE_BLOCK_SIZE == 0);

  // move output_block_
  out_buf_->Push(std::move(output_block_)); 
  output_block_ = std::make_unique<std::vector<EncOutput>>();

  // figure out if USB is too slow
  /*
  have to figure out if the contents of the MB between us
  and the Comm contains stale traffic, based on its current
  capacity and our record of the times associated with the traffic
  that we've sent and the times we've sent them

  words_outstanding_ has data like: 

  [pushed M_0 words for ts_0,
   pushed M_1 words for ts_1,
     ...
   pushed M_NCURR words for ts_NCURR]

  if current MB capacity is X:
  
  find min N s.t. : (X - sum_N_NCURR_downto_N(Mi)) > 0
  ts_N is then the stalest time in the buffer

  if t_wall_time - ts_N > tolerance, emit a warning that the USB is slow

  we can then discard words_outstanding_[:N]
  */

  block_sizes_times_outstanding_.push_back({this_block_size, last_HB_sent_at_});
  total_outstanding_ += this_block_size;

  unsigned int MB_size = out_buf_->TotalSize();

  // trim from the end of our deque until we're only slightly greater than the current size
  BDTime block_time; // will record stalest time when done
  while (true) {
    auto front_block = block_sizes_times_outstanding_.front();
    block_time = front_block.first;
    unsigned int block_size = front_block.second;

    if (total_outstanding_ - block_size < MB_size) {
      break; // gone too far
    } else {
      total_outstanding_ -= block_size;
      block_sizes_times_outstanding_.pop_front();
    }
  }
  BDTime wall_time = GetRelativeTime();
  BDTime stale_time = wall_time - block_time;
  if (stale_time > driverpars::WRITE_LAG_WARNING_TIME_NS) {
      cout << "WARNING: bddriver::Encoder: Downstream USB is too slow. Running " << stale_time << " ns behind" << endl;
  }
  
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
        // make sure we're not sending something stale
        // if we are, emit warning
        BDTime wall_time = GetRelativeTime();
        BDTime stale_time = wall_time - time;
        if (stale_time > driverpars::WRITE_LAG_WARNING_TIME_NS) {
            cout << "WARNING: bddriver::Encoder: Encoder (downstream data processing) is too slow. Running " << stale_time << " ns behind" << endl;
        }

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

  if (flush_pending) {
    // pad frame to block size multiple and send to comm
    PadNopsAndFlush();
  }
}

}  // bddriver
}  // pystorm
