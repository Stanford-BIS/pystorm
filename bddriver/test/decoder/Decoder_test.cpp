#include "Decoder.h"

#include <chrono>
#include <cstdint>
#include <random>
#include <vector>
#include <iostream>
#include <thread>

#include "MutexBuffer.h"
#include "binary_util.h"
#include "BDPars.h"
#include "gtest/gtest.h"
#include "test_util/Producer_Consumer.h"

using std::cout;
using std::endl;
using namespace pystorm;
using namespace bddriver;
using namespace bdpars;

#define BYTES_PER_WORD 5

std::pair<std::vector<DecInput>, std::vector<std::vector<DecOutput> > > MakeDecInputAndOutputs(unsigned int N, const BDPars * pars, unsigned int max_leaf_idx_to_use) {

  // return values
  std::vector<DecInput> dec_inputs;
  std::vector<std::vector<DecOutput> > dec_outputs;
  for (unsigned int i = 0; i < static_cast<unsigned int>(LastFunnelLeafId+1); i++) {
    dec_outputs.push_back({});
  }

  // rng
  assert(max_leaf_idx_to_use <= LastFunnelLeafId);
  std::default_random_engine generator(0);
  std::uniform_int_distribution<unsigned int> leaf_idx_distribution(0, max_leaf_idx_to_use);
  std::uniform_int_distribution<uint32_t> payload_distribution(0, UINT32_MAX);

  for (unsigned int i = 0; i < N; i++) {
    // randomly determine which funnel leaf to make data from, determine payload bits
    unsigned int leaf_idx = leaf_idx_distribution(generator);
    uint32_t payload_val  = payload_distribution(generator);

    // look up route
    uint32_t route_val;
    unsigned int route_len;
    std::tie(route_val, route_len) = pars->FunnelRoute(leaf_idx); 
    
    // look up data width and output width
    unsigned int BD_output_width = pars->Width(BD_output);
    unsigned int payload_width = pars->Width(static_cast<FunnelLeafId>(leaf_idx));

    // mask payload value
    payload_val = payload_val % (1 << payload_width);

    // pack dec input word
    // msb <-- lsb
    // [route | X | payload]
    uint64_t dec_input_packed = PackV64(
        {static_cast<uint64_t>(payload_val), 0, static_cast<uint64_t>(route_val)},
        {payload_width, BD_output_width - payload_width - route_len, route_len}
    );

    //cout << "route(" << route_len << "): " << route_val << endl;
    //cout << UintAsString(dec_input, 34) << endl;
    
    // now unpack input into uint8_ts
    std::vector<unsigned int> byte_widths (BYTES_PER_WORD, 8);
    std::vector<uint64_t> dec_input = UnpackV64(dec_input_packed, byte_widths);
    for (auto& it : dec_input) {
      dec_inputs.push_back(static_cast<uint8_t>(it));
    }

    // push dec output word to appropriate queue
    // XXX ignore time epoch for now
    DecOutput dec_output = {payload_val, 0, 0};
    dec_outputs[leaf_idx].push_back(dec_output);
  }

  return make_pair(dec_inputs, dec_outputs);
}

class DecoderFixture : public testing::TestWithParam<unsigned int>
{
  public:
    void SetUp() 
    {

      pars = new BDPars(); 

      buf_in = new MutexBuffer<DecInput>(buf_depth);
      for (unsigned int i = 0; i < pars->FunnelRoutes()->size(); i++) {
        MutexBuffer<DecOutput> * buf_ptr = new MutexBuffer<DecOutput>(buf_depth);
        bufs_out.push_back(buf_ptr);
      }

      std::tie(dec_inputs, dec_outputs) = MakeDecInputAndOutputs(N, pars, GetParam());

      for (unsigned int i = 0; i < bufs_out.size(); i++) {
        consumed_data.push_back({});
      }

    }

    void TearDown() 
    {
      delete pars;
      delete buf_in;
      for (auto& buf_out : bufs_out) {
        delete buf_out;
      }
    }

    const unsigned int input_width = 34;
    unsigned int N = 10e6;
    unsigned int M = 10000;
    unsigned int buf_depth = 100000;

    // XXX set this to something meaningful later, compiler flags?
    const double fastEnough = .2;

    MutexBuffer<DecInput> * buf_in;
    std::vector<MutexBuffer<DecOutput> *> bufs_out;

    BDPars * pars;

    std::vector<DecInput> dec_inputs;
    std::vector<std::vector<DecOutput> > dec_outputs;

    std::vector<std::vector<DecOutput> > consumed_data;
};


TEST_P(DecoderFixture, Test1xDecoder)
{
  Decoder dec(pars, buf_in, bufs_out, M);

  // start producer/consumer threads
  std::thread producer = std::thread(ProduceN<DecInput>, buf_in, &dec_inputs[0], N*BYTES_PER_WORD, M, 0);

  // XXX this isn't quite the use-case, but it fits well with the testing structures
  // in reality, there isn't necessarily one thread per bufs_out queue
  std::vector<std::thread> consumers;

  //cout << "dec outputs size: " << dec_outputs.size() << endl;
  //cout << "bufs out size: " << bufs_out.size() << endl;
  //cout << "outputs per leaf" << endl;
  //for (unsigned int i = 0; i < bufs_out.size(); i++) {
  //  cout << dec_outputs[i].size() << endl;
  //}

  for (unsigned int i = 0; i < bufs_out.size(); i++) {
    // give up after 10s
    consumers.push_back(
        std::thread(ConsumeVectNGiveUpAfter<DecOutput>, bufs_out[i], &consumed_data[i], dec_outputs[i].size(), M, 1000, 10e6)
    );
    
    //// expect exact number
    //consumers.push_back(
    //    std::thread(ConsumeVectN<DecOutput>, bufs_out[i], &consumed_data[i], dec_outputs[i].size(), M, 1000)
    //);
  }

  // start decoder, sources from producer through buf_in, sinks to consumer through buf_out
  auto t0 = std::chrono::high_resolution_clock::now();
  dec.Start();

  producer.join();
  //cout << "producer joined" << endl;

  for (unsigned int i = 0; i < consumers.size(); i++) {
    consumers[i].join();
    //cout << "consumer[" << i << "] joined" << endl;
  }

  //cout << "consumed data sizes" << endl;
  //for (unsigned int i = 0; i < consumed_data.size(); i++) {
  //  cout << consumed_data[i].size() << endl;
  //}

  auto tend = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(tend - t0).count();
  double throughput = static_cast<double>(N) / diff; // in million entries/sec
  cout << "throughput: " << throughput << " Mwords/s" << endl;

  // eventually, decoder will time out and thread will join
  dec.Stop();

  EXPECT_GT(throughput, fastEnough);

  for (unsigned int i = 0; i < consumed_data.size(); i++) {
    ASSERT_EQ(consumed_data[i].size(), dec_outputs[i].size());
    for (unsigned int j = 0; j < consumed_data[i].size(); j++) {
      ASSERT_EQ(consumed_data[i][j].payload, dec_outputs[i][j].payload);
      ASSERT_EQ(consumed_data[i][j].core_id, dec_outputs[i][j].core_id);
    }
  }

}

INSTANTIATE_TEST_CASE_P(
    TestDecoderDifferentNumLeaves,
    DecoderFixture,
    ::testing::Values(LastFunnelLeafId, 1, 0)
);

//TEST_F(DecoderFixture, Test2xDecoder)
//{
//  // two identical Decoders in this test
//  Decoder dec0(pars, buf_in, bufs_out, M);
//  Decoder dec1(pars, buf_in, bufs_out, M);
//
//  // start producer/consumer threads
//  std::vector<DecOutput> consumed;
//  producer = std::thread(ProduceN<DecInput>, buf_in, &input_vals[0], N, M, 0);
//  consumer = std::thread(ConsumeVectN<DecOutput>, bufs_out[pars->FunnelIdx(NRNI)], &consumed, N, M, 0);
//
//  // start decoder, sources from producer through buf_in, sinks to consumer through buf_out
//  auto t0 = std::chrono::high_resolution_clock::now();
//  dec0.Start();
//  dec1.Start();
//  
//  producer.join();
//  //cout << "producer joined" << endl;
//  consumer.join();
//  //cout << "consumer joined" << endl;
//
//  auto tend = std::chrono::high_resolution_clock::now();
//  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(tend - t0).count();
//  double throughput = static_cast<double>(N) / diff; // in million entries/sec
//  cout << "throughput: " << throughput << " Mwords/s" << endl;
//
//  // eventually, decoder will time out and thread will join
//  dec0.Stop();
//  dec1.Stop();
//
//  // for now, testing speedup
//  EXPECT_GT(throughput, 2*fastEnough);
//
//  // XXX should check output is correct
//}
