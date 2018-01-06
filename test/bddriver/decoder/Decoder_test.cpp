#include "Decoder.h"
#include "common/DriverPars.h"

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
    //cout << int(it) << endl;
    dec_outputs.insert({it, {}});
  }

  // rng
  std::default_random_engine generator(0);

  // number non-nop to send
  std::uniform_int_distribution<> num_messages_dist(0, N);
  unsigned int num_messages = num_messages_dist(generator);

  std::uniform_int_distribution<> payload_dist(0, UINT8_MAX);
  std::uniform_int_distribution<> code_dist(0, up_eps.size()-1);

  // note that initialization only occurs on the first function invocation
  static BDTime last_time = 0;
  static BDTime last_time_p1 = 0;
  static BDTime last_time_p2 = 0;

  static unsigned int last_HB_LSB_recvd;

  for (unsigned int i = 0; i < N; i++) {
    // make random input data
    uint8_t b[4];
    for (unsigned int j = 0; j < 3; j++) {
      b[j] = payload_dist(generator);
      dec_inputs.push_back(b[j]);
    }

    uint8_t code_idx = code_dist(generator);
    assert(code_idx < up_eps.size());
    uint8_t code = up_eps.at(code_idx);

    // XXX don't send nop if i < num_messages
    if (i < num_messages) {
      if (code == pars->UpEPCodeFor(bdpars::FPGAOutputEP::NOP)) {
        code = pars->UpEPCodeFor(bdpars::FPGAOutputEP::NOP) - 1;
      } 
    // otherwise only send nops
    } else {
      code = pars->UpEPCodeFor(bdpars::FPGAOutputEP::NOP);
    }

    b[3] = code;
    dec_inputs.push_back(code);

    // pack
    uint32_t packed = PackWord<FPGABYTES>(
        {{FPGABYTES::B0, b[0]}, 
         {FPGABYTES::B1, b[1]}, 
         {FPGABYTES::B2, b[2]}, 
         {FPGABYTES::B3, b[3]}});
    //cout << " b[0] " << int(b[0]) << endl;
    //cout << " b[1] " << int(b[1]) << endl;
    //cout << " b[2] " << int(b[2]) << endl;
    //cout << " b[3] " << int(b[3]) << endl;
    //cout << "packed " << packed << endl;
    //cout << " at0 " << GetField(packed, FPGABYTES::B0) << endl;
    //cout << " at1 " << GetField(packed, FPGABYTES::B1) << endl;
    //cout << " at2 " << GetField(packed, FPGABYTES::B2) << endl;
    //cout << " at3 " << GetField(packed, FPGABYTES::B3) << endl;

    // extract code, payload
    assert(GetField(packed, FPGAIO::EP_CODE) == code);
    uint32_t payload = GetField(packed, FPGAIO::PAYLOAD);
    //cout << " code " << int(code) << endl;
    //cout << " payload " << payload << endl;
    
    // update time if necessary 
    // (no guarantee that time ascends in this test, 
    // so sequence may be unusual)
    if (code == pars->UpEPCodeFor(bdpars::FPGAOutputEP::UPSTREAM_HB_LSB)) {
      last_HB_LSB_recvd = payload;
    } else if (code == pars->UpEPCodeFor(bdpars::FPGAOutputEP::UPSTREAM_HB_MSB)) {
      last_time = PackWord<TWOFPGAPAYLOADS>(
          {{TWOFPGAPAYLOADS::MSB, payload},
           {TWOFPGAPAYLOADS::LSB, last_HB_LSB_recvd}});
    }

    if (code != pars->UpEPCodeFor(bdpars::FPGAOutputEP::NOP) && 
        code != pars->UpEPCodeFor(bdpars::FPGAOutputEP::DS_QUEUE_CT)) {
      DecOutput to_push;
      to_push.payload = payload;
      to_push.time = last_time_p2;
      last_time_p2 = last_time_p1;
      last_time_p1 = last_time;

      if (dec_outputs.count(code) == 0) {
        cout << int(code) << endl;
        assert(false);
      }

      dec_outputs.at(code).push_back(to_push);
    }

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
  
  unsigned int M = driverpars::READ_BLOCK_SIZE * 4; // must be a multiple of READ_BLOCK_SIZE
  unsigned int N = 100;

  // this is super confusing, turn the map<vector<>> into a map<vector<vector<>>> for outputs
  std::vector<DIVect> all_inputs;
  std::unordered_map<uint8_t, std::vector<DOVect>> all_outputs;

  for (unsigned int i = 0; i < N; i++) {

    auto in_and_out = MakeDecInputAndOutputs(M, &pars);

    all_inputs.push_back(in_and_out.first);
    for (auto& it : in_and_out.second) {
      if (all_outputs.count(it.first) == 0)
        all_outputs.insert({it.first, {}});
      if (it.second.size() > 0) 
        all_outputs.at(it.first).push_back(it.second);
    }
  }
  
  std::thread producer = std::thread(Produce<DecInput>, &buf_in, all_inputs);

  // one consumer per output
  std::vector<std::thread> consumers;
  std::unordered_map<uint8_t, std::vector<DOVect>> recvd_outputs;
  for (auto& it : bufs_out) {
    uint8_t ep_code = it.first;
    MutexBuffer<DecOutput> * buf = it.second;
    recvd_outputs.insert({ep_code, {}});
    unsigned int num_vect_to_consume = all_outputs.at(ep_code).size(); // should be N or close to it
    //cout << "ep " << int(ep_code) << " needs to consume " << num_vect_to_consume << endl;
    consumers.push_back(std::thread(Consume<DecOutput>, buf, &recvd_outputs.at(ep_code), num_vect_to_consume, 0));
  }
  
  // need nonzero timeout so we can stop ourselves
  Decoder dec(&buf_in, bufs_out, &pars, 1000);
  dec.Start();

  producer.join();
  //cout << "producer joined" << endl;
  unsigned int i = 0;
  for (auto& it : consumers) {
    it.join();
    //cout << "consumer " << i << " joined" << endl;
    i++;
  }
  
  // because of dumb gtest reasons, we can't just define equality for DecOutput
  // and have ASSERT_EQ work, so we can't use ConsumeAndCheck
  for (auto& it : all_outputs) {
    uint8_t ep_code = it.first;
    ASSERT_EQ(
          all_outputs.at(ep_code).size(), 
        recvd_outputs.at(ep_code).size());
    for (unsigned int i = 0; i < all_outputs.at(ep_code).size(); i++) {
      ASSERT_EQ(
            all_outputs.at(ep_code).at(i).size(), 
          recvd_outputs.at(ep_code).at(i).size());
      for (unsigned int j = 0; j < all_outputs.at(ep_code).at(i).size(); j++) {
        ASSERT_EQ(
              all_outputs.at(ep_code).at(i).at(j).payload, 
            recvd_outputs.at(ep_code).at(i).at(j).payload);
        if (
              all_outputs.at(ep_code).at(i).at(j).time != 
            recvd_outputs.at(ep_code).at(i).at(j).time) cout << "ep " << int(ep_code) << " i " << i << " j " << j << endl;
        ASSERT_EQ(
              all_outputs.at(ep_code).at(i).at(j).time, 
            recvd_outputs.at(ep_code).at(i).at(j).time);
      }
    }
  }

  dec.Stop();
  
  for (auto& it : up_eps) {
    delete bufs_out.at(it);
  }
}

