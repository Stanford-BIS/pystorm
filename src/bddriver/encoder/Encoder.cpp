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
    std::unique_ptr<std::vector<EncOutput>> to_push_vect = Encode(std::move(popped_vect));
    out_buf_->Push(std::move(to_push_vect)); // push can't block indefinitely
  }
}

inline void PushWord(std::unique_ptr<std::vector<EncOutput>> & output, uint32_t word) {
  uint8_t b0 = GetField<FPGABYTES>(word, FPGABYTES::B0);
  uint8_t b1 = GetField<FPGABYTES>(word, FPGABYTES::B1);
  uint8_t b2 = GetField<FPGABYTES>(word, FPGABYTES::B2);
  uint8_t b3 = GetField<FPGABYTES>(word, FPGABYTES::B3);
  output->push_back(b0);
  output->push_back(b1);
  output->push_back(b2);
  output->push_back(b3);
}

std::unique_ptr<std::vector<EncOutput>> Encoder::Encode(const std::unique_ptr<std::vector<EncInput>> inputs) {

  std::unique_ptr<std::vector<EncOutput>> output (new std::vector<EncOutput>);

  static unsigned int n0 = 0;
  static unsigned int n1 = 0;

  for (auto& it : *inputs) {
    // unpack data
    unsigned int core_id      = it.core_id;
    unsigned int FPGA_ep_code = it.FPGA_ep_code;
    uint32_t     payload      = it.payload;
    BDTime       time         = it.time;

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
        PushWord(output, PackWord<FPGAIO>({{FPGAIO::PAYLOAD, time_chunk[i]}, {FPGAIO::EP_CODE, HB_ep_code[i]}}));
      }
    }

    if (FPGA_ep_code == bd_pars_->DnEPCodeFor(bdpars::BDHornEP::RI)) {
      unsigned int bit = payload % 2;
      if (bit == 0) n0++;
      if (bit == 1) n1++;
      cout << "p[0] = " << bit << ", n0 = " << n0 << ", n1 = " << n1 << ", p = " << payload << endl;
    }

    // serialize to bytes 
    PushWord(output, FPGA_encoded);

    //if (FPGA_ep_code != bd_pars_->DnEPCodeFor(bdpars::FPGARegEP::NOP))
    //  PrintBinaryAsStr(FPGA_encoded, 32);

  }

  //if (inputs->size() > 0)
  //  cout << "encoder working" << endl;

  return output;
}

}  // bddriver
}  // pystorm
