#include "Decoder.h"

#include <chrono>
#include <cstdint>
#include <random>
#include <vector>
#include <iostream>
#include <thread>

#include "MutexBuffer.h"
#include "BDPars.h"
#include "BDWord.h"
#include "gtest/gtest.h"
#include "test_util/Producer_Consumer.h"

using std::cout;
using std::endl;
using namespace pystorm;
using namespace bddriver;
using namespace bdpars;

typedef std::vector<DecInput> DIVect;
typedef std::vector<DecOutput> DOVect;

// construct a set of inputs and outputs
// a simple clone of the Decoder code
std::pair<DIVect, std::unordered_map<uint8_t, DOVect>> MakeDecInputAndOutputs(unsigned int N, const BDPars * pars) {

  // return values
  DIVect dec_inputs;
  std::unordered_map<uint8_t, DOVect> dec_outputs;
  std::vector<uint8_t> up_eps = pars->GetUpEPs();
  for (auto& it : up_eps) {
    dec_outputs.insert({it, {}});
  }

  // rng
  std::default_random_engine generator(0);
  std::uniform_int_distribution<uint8_t> input_dist(0, UINT8_MAX);

  for (unsigned int i = 0; i < N; i++) {
    // make random input data
    uint8_t b[4];
    for (unsigned int j = 0; j < 4; j++) {
      b[i] = input_dist(generator);
      dec_inputs.push_back(b[i]);
    }

    // pack
    uint32_t packed = PackWord<FPGABYTES>(
        {{FPGABYTES::B0, b[0]}, 
         {FPGABYTES::B1, b[1]}, 
         {FPGABYTES::B2, b[2]}, 
         {FPGABYTES::B3, b[3]}});

    // extract code, payload
    uint8_t code = GetField(packed, FPGAIO::EP_CODE);
    uint32_t payload = GetField(packed, FPGAIO::PAYLOAD);
    BDTime time = 0; // XXX temporary

    DecOutput to_push;
    to_push.payload = payload;
    to_push.time = time;

    dec_outputs.at(code).push_back(to_push);
  }

  return make_pair(dec_inputs, dec_outputs);
}

// run the decoder against producer and consumers threads
TEST(DecoderTest, MainDecoderTest) {

  BDPars pars;

  MutexBuffer<DecInput> buf_in;
  std::unordered_map<uint8_t, MutexBuffer<DecOutput> *> bufs_out;
  
  std::vector<uint8_t> up_eps = pars.GetUpEPs();
  for (auto& it : up_eps) {
    bufs_out.insert({it, new MutexBuffer<DecOutput>()});
  }
  
  unsigned int M = 100;
  unsigned int N = 100;

  std::vector<DIVect> all_inputs;
  std::unordered_map<uint8_t, std::vector<DOVect>> all_outputs;
  for (unsigned int i = 0; i < N; i++) {
    auto in_and_out = MakeDecInputAndOutputs(M, &pars);
    all_inputs.push_back(in_and_out.first);
    for (auto& it : in_and_out.second) {
      if (all_outputs.count(it.first) == 0)
        all_outputs.insert({it.first, {}});
      all_outputs.at(it.first).push_back(it.second);
    }
  }
  
  std::thread producer = std::thread(Produce<DecInput>, &buf_in, all_inputs);
  std::vector<std::thread> consumers;

  std::unordered_map<uint8_t, std::vector<DOVect>> recvd_outputs;
  for (auto& it : bufs_out) {
    recvd_outputs.insert({it.first, {}});
    consumers.push_back(std::thread(Consume<DecOutput>, it.second, &recvd_outputs.at(it.first), N, 0));
  }
  
  // need nonzero timeout so we can stop ourselves
  Decoder dec(&buf_in, bufs_out, 1000);
  dec.Start();

  producer.join();
  cout << "producer joined" << endl;
  unsigned int i = 0;
  for (auto& it : consumers) {
    it.join();
    cout << "consumer " << i++ << " joined" << endl;
  }
  
  // because of dumb gtest reasons, we can't just define equality for DecOutput
  // and have ASSERT_EQ work, so we can't use ConsumeAndCheck
  for (auto& it : all_outputs) {
    ASSERT_EQ(all_outputs.at(it.first).size(), recvd_outputs.at(it.first).size());
    for (unsigned int i = 0; i < all_outputs.at(it.first).size(); i++) {
      ASSERT_EQ(all_outputs.at(it.first).at(i).size(), recvd_outputs.at(it.first).at(i).size());
      for (unsigned int j = 0; j < all_outputs.at(it.first).at(i).size(); j++) {
        ASSERT_EQ(all_outputs.at(it.first).at(i).at(j).payload, recvd_outputs.at(it.first).at(i).at(j).payload);
        ASSERT_EQ(all_outputs.at(it.first).at(i).at(j).time, recvd_outputs.at(it.first).at(i).at(j).time);
      }
    }
  }

  dec.Stop();
  
  for (auto& it : up_eps) {
    delete bufs_out.at(it);
  }
}

