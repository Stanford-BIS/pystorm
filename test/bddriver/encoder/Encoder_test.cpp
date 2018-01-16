#include "Encoder.h"
#include "common/DriverPars.h"

#include <chrono>
#include <cstdint>
#include <random>
#include <iostream>
#include <thread>

#include "gtest/gtest.h"
#include "MutexBuffer.h"
#include "BDPars.h"
#include "BDWord.h"
#include "test_util/Producer_Consumer.h"

#define BYTES_PER_WORD 3

using std::cout;
using std::endl;
using namespace pystorm;
using namespace bddriver;
using namespace bdpars;

typedef std::vector<EncInput> EIVect;
typedef std::vector<EncOutput> EOVect;

// construct a set of inputs and outputs
// a simple clone of the Encoder code
std::pair<EIVect, EOVect> MakeEncInputAndOutputs(unsigned int N, const BDPars * pars) {

  // return values
  EIVect enc_inputs;
  std::vector<uint32_t> enc_outputs_packed;
  EOVect enc_outputs;

  // rng
  std::default_random_engine generator(0);
  std::uniform_int_distribution<> payload_dist(0, (1<<24)-1);
  std::uniform_int_distribution<> ep_code_dist(0, UINT8_MAX);

  for (unsigned int i = 0; i < N; i++) {
    // make random input data
    EncInput input;
    input.payload = payload_dist(generator);
    input.FPGA_ep_code = ep_code_dist(generator);
    input.core_id = 0; // XXX
    input.time = 0; // XXX we're not testing the HB generation

    // last word must be a flush so everything comes out
    if (i == N-1) {
      input.FPGA_ep_code = EncInput::kFlushCode;
    }

    // don't make HBs randomly
    if (input.FPGA_ep_code == pars->DnEPCodeFor(bdpars::FPGARegEP::TM_PC_TIME_ELAPSED0) || 
        input.FPGA_ep_code == pars->DnEPCodeFor(bdpars::FPGARegEP::TM_PC_TIME_ELAPSED1) || 
        input.FPGA_ep_code == pars->DnEPCodeFor(bdpars::FPGARegEP::TM_PC_TIME_ELAPSED2)) {

      // pass
      
    } else if (input.FPGA_ep_code == EncInput::kFlushCode) {
      // pad nops
      enc_inputs.push_back(input);

      const unsigned int kPackedPerBlock = driverpars::WRITE_BLOCK_SIZE / 4;
      unsigned int curr_size_in_frame = enc_outputs_packed.size() % kPackedPerBlock;
      //cout << curr_size_in_frame << endl;
      //cout << kPackedPerBlock << endl;
      unsigned int to_complete_block = (kPackedPerBlock - curr_size_in_frame) % kPackedPerBlock;
      //cout << to_complete_block << endl;

      uint32_t nop = PackWord<FPGAIO>({{FPGAIO::PAYLOAD, 0}, {FPGAIO::EP_CODE, pars->DnEPCodeFor(bdpars::FPGARegEP::NOP)}});
      for (unsigned int nop_idx = 0; nop_idx < to_complete_block; nop_idx++) {
        enc_outputs_packed.push_back(nop);
      }

    } else {
      // pack inputs
      enc_inputs.push_back(input);

      // pack
      uint32_t packed = PackWord<FPGAIO>({{FPGAIO::EP_CODE, input.FPGA_ep_code}, {FPGAIO::PAYLOAD, input.payload}});

      enc_outputs_packed.push_back(packed);
    }
  }

  // serialize
  for (auto& packed : enc_outputs_packed) {

    uint8_t b[4];
    b[0] = GetField(packed, FPGABYTES::B0);
    b[1] = GetField(packed, FPGABYTES::B1);
    b[2] = GetField(packed, FPGABYTES::B2);
    b[3] = GetField(packed, FPGABYTES::B3);

    for (unsigned int j = 0; j < 4; j++) {
      enc_outputs.push_back(b[j]);
    }
  }
  //cout << enc_outputs.size() << endl;

  return make_pair(enc_inputs, enc_outputs);
}

// run the encoder against producer and consumers threads
TEST(EncoderTest, MainEncoderTest) {

  BDPars pars;

  MutexBuffer<EncInput> buf_in;
  MutexBuffer<EncOutput> buf_out;
  
  unsigned int M = 100;
  unsigned int N = 100;

  std::vector<EIVect> all_inputs;
  std::vector<EOVect> all_outputs;
  for (unsigned int i = 0; i < N; i++) {
    auto in_and_out = MakeEncInputAndOutputs(M, &pars);
    all_inputs.push_back(in_and_out.first);
    all_outputs.push_back(in_and_out.second);
  }
  
  std::thread producer = std::thread(Produce<EncInput>, &buf_in, all_inputs);
  std::thread consumer = std::thread(ConsumeAndCheck<EncOutput>, &buf_out, all_outputs, 0);

  // need nonzero timeout so we can stop ourselves
  Encoder enc(&buf_in, &buf_out, &pars, 1000);
  enc.Start();

  producer.join();
  //cout << "producer joined" << endl;
  consumer.join();
  //cout << "consumer joined" << endl;

  enc.Stop();
}

