#include "Encoder.h"

#include <chrono>
#include <cstdint>
#include <random>
#include <iostream>
#include <thread>

#include "gtest/gtest.h"
#include "MutexBuffer.h"
#include "binary_util.h"
#include "test_util/Producer_Consumer.h"

#define BYTES_PER_WORD 3

using std::cout;
using std::endl;
using namespace pystorm;
using namespace bddriver;
using namespace bdpars;

std::pair<std::vector<EncInput>, std::vector<EncOutput> > MakeEncInputAndOutput(BDPars * pars, unsigned int N, unsigned int max_leaf_idx_to_use) {

  std::vector<EncInput> enc_inputs;
  std::vector<EncOutput> enc_outputs;

  // rng
  assert(max_leaf_idx_to_use < HornLeafIdCount);
  std::default_random_engine generator(0);
  std::uniform_int_distribution<unsigned int> leaf_idx_distribution(0, max_leaf_idx_to_use);
  std::uniform_int_distribution<uint32_t> payload_distribution(0, UINT32_MAX);

  for (unsigned int i = 0; i < N; i++) {
    unsigned int leaf_idx = leaf_idx_distribution(generator);
    uint32_t payload_val  = payload_distribution(generator);

    // look up data width and input width
    //unsigned int BD_input_width = pars->Width(BD_input);
    unsigned int payload_width = pars->Width(static_cast<HornLeafId>(leaf_idx));

    // mask payload
    payload_val = payload_val % (1 << payload_width);

    // make EncInput
    // chip id 0, leaf, payload N
    EncInput enc_input = {0, leaf_idx, payload_val};

    // look up route val
    uint32_t route_val;
    unsigned int route_len;
    std::tie(route_val, route_len) = pars->HornRoute(leaf_idx); 

    // pack enc output word
    // msb <- lsb
    // [ X | payload | route ]
    uint32_t enc_output_packed = PackV32(
        {route_val, payload_val}, 
        {route_len, payload_width}
    );
    //cout << route_val << "(" << route_len << "), " << payload_val << "(" << payload_width << ")" << endl;
    //cout << UintAsString(enc_output_packed, 32) << endl;
    
    // now unpack output into uint8_ts
    std::vector<unsigned int> byte_widths (BYTES_PER_WORD, 8);
    std::vector<uint32_t> enc_output = UnpackV32(enc_output_packed, byte_widths);

    // push to vectors
    for (auto& it : enc_output) {
       enc_outputs.push_back(static_cast<uint8_t>(it));
       //cout << UintAsString(it, 8) << endl;
    }
    enc_inputs.push_back(enc_input);
  }

  return std::make_pair(enc_inputs, enc_outputs);
}

class EncoderFixture : public testing::Test {
  public:
    void SetUp() {
      buf_in = new MutexBuffer<EncInput>(buf_depth);
      buf_out = new MutexBuffer<EncOutput>(buf_depth);

      pars = new BDPars(); // filename unused for now

      std::tie(enc_inputs, enc_outputs) = MakeEncInputAndOutput(pars, N, HornLeafIdCount-1);
    }

    void TearDown() {
      delete buf_in;
      delete buf_out;
      delete pars;
    }

    unsigned int N = 10e6;
    unsigned int M = 1000;
    unsigned int buf_depth = 100000;

    // XXX set this to something meaningful later, compiler flags?
    const double fastEnough = .2;

    MutexBuffer<EncInput> * buf_in;
    MutexBuffer<EncOutput> * buf_out;

    BDPars * pars;

    std::vector<EncInput> enc_inputs;
    std::vector<EncOutput> enc_outputs;

    std::thread producer;
    std::thread consumer;
};


TEST_F(EncoderFixture, Test1xEncoder) {
  Encoder enc(pars, buf_in, buf_out, M);

  // start producer/consumer threads
  std::vector<EncOutput> consumed;
  producer = std::thread(ProduceN<EncInput>, buf_in, &enc_inputs[0], N, M, 0);
  consumer = std::thread(ConsumeVectN<EncOutput>, buf_out, &consumed, N * BYTES_PER_WORD, M, 0);

  // start encoder, sources from producer through buf_in, sinks to consumer through buf_out
  auto t0 = std::chrono::high_resolution_clock::now();
  enc.Start();
  
  producer.join();
  consumer.join();

  auto tend = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(tend - t0).count();
  double throughput = static_cast<double>(N) / diff; // in million entries/sec
  cout << "throughput: " << throughput << " Mwords/s" << endl;

  // eventually, encoder will time out and thread will join
  enc.Stop();

  EXPECT_GT(throughput, fastEnough);

  ASSERT_EQ(consumed.size(), enc_outputs.size());
  for (unsigned int i = 0; i < consumed.size(); i++) {
    //cout << "got " << UintAsString(consumed[i], 8) << endl;
    //cout << "exp " << UintAsString(enc_outputs[i], 8) << endl;
    EXPECT_EQ(consumed[i], enc_outputs[i]);
  }

}

TEST_F(EncoderFixture, Test2xEncoder) {
  // two identical Encoders in this test
  Encoder enc0(pars, buf_in, buf_out, M);
  Encoder enc1(pars, buf_in, buf_out, M);

  // start producer/consumer threads
  std::vector<EncOutput> consumed;
  producer = std::thread(ProduceN<EncInput>, buf_in, &enc_inputs[0], N, M, 0);
  consumer = std::thread(ConsumeVectN<EncOutput>, buf_out, &consumed, N * BYTES_PER_WORD, M, 0);

  // start encoder, sources from producer through buf_in, sinks to consumer through buf_out
  auto t0 = std::chrono::high_resolution_clock::now();
  enc0.Start();
  enc1.Start();
  
  producer.join();
  consumer.join();

  auto tend = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(tend - t0).count();
  double throughput = static_cast<double>(N) / diff; // in million entries/sec
  cout << "throughput: " << throughput << " Mwords/s" << endl;

  // eventually, encoder will time out and thread will join
  enc0.Stop();
  enc1.Stop();

  EXPECT_GT(throughput, fastEnough);

  // it's hard to check output correctness, the order might be jumbled
}

