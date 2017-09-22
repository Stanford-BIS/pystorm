#include "Encoder.h"

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
  EOVect enc_outputs;

  // rng
  std::default_random_engine generator(0);
  std::uniform_int_distribution<uint32_t> payload_dist(0, UINT32_MAX);
  std::uniform_int_distribution<uint8_t> ep_code_dist(0, UINT8_MAX);

  for (unsigned int i = 0; i < N; i++) {
    // make random input data
    EncInput input;
    input.payload = payload_dist(generator);
    input.FPGA_ep_code = ep_code_dist(generator);
    input.core_id = 0; // XXX
    input.time = 0; // XXX
    enc_inputs.push_back(input);

    // pack
    uint32_t packed = PackWord<FPGAIO>({{FPGAIO::EP_CODE, input.FPGA_ep_code}, {FPGAIO::PAYLOAD, input.payload}});

    // serialize
    uint8_t b[4];
    b[0] = GetField(packed, FPGABYTES::B0);
    b[1] = GetField(packed, FPGABYTES::B1);
    b[2] = GetField(packed, FPGABYTES::B2);
    b[3] = GetField(packed, FPGABYTES::B3);

    for (unsigned int j = 0; j < 4; j++) {
      enc_outputs.push_back(b[j]);
    }
  }

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

  Encoder enc(&buf_in, &buf_out, 0);
  enc.Start();

  producer.join();
  consumer.join();

  enc.Stop();
}

