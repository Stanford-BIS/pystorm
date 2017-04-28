#include "Decoder.h"

#include <unistd.h> // usleep
#include <chrono>

#include <vector>
#include <iostream>
#include <thread>

#include "MutexBuffer.h"
#include "binary_util.h"
#include "BDPars.h"
#include "gtest/gtest.h"
#include "test_util/Producer_Consumer.cpp"

using std::cout;
using std::endl;
using namespace pystorm;
using namespace bddriver;


std::vector<DecInput> MakeDecInput(unsigned int N, const BDPars * pars) {
  std::vector<DecInput> vals;

  //cout << "route " << pars->FunnelRoute("NRNI").first << endl;
  for (unsigned int i = 0; i < N; i++) {
    unsigned int input_width = pars->Width(BD_output);
    FHRoute route = pars->FunnelRoute(NRNI); 
    uint32_t route_val = route.first;
    unsigned int route_len = route.second;

    unsigned int payload_width = input_width - route_len;

    uint64_t payload_route = PackV64({static_cast<uint64_t>(i), static_cast<uint64_t>(route_val)}, {payload_width, route_len});

    vals.push_back(payload_route);
  }

  return vals;
}

class DecoderFixture : public testing::Test
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

      input_vals = MakeDecInput(N, pars);
    }

    const unsigned int input_width = 34;
    unsigned int N = 10e6;
    unsigned int M = 1000;
    unsigned int buf_depth = 10000;

    // XXX set this to something meaningful later, compiler flags?
    const double fastEnough = .2;

    MutexBuffer<DecInput> * buf_in;
    std::vector<MutexBuffer<DecOutput> *> bufs_out;

    BDPars * pars;

    std::vector<DecInput> input_vals;

    std::thread producer;
    std::thread consumer;
};


TEST_F(DecoderFixture, Test1xDecoder)
{
  Decoder dec(pars, buf_in, bufs_out, M);

  // start producer/consumer threads
  std::vector<DecOutput> consumed;
  consumed.reserve(N);
  producer = std::thread(ProduceN<DecInput>, buf_in, &input_vals[0], N, M, 0);
  //cout << "NRNI HAS IDX: " << pars->FunnelIdx("NRNI") << endl;
  consumer = std::thread(ConsumeVectN<DecOutput>, bufs_out[pars->FunnelIdx(NRNI)], &consumed, N, M, 0);

  // start decoder, sources from producer through buf_in, sinks to consumer through buf_out
  auto t0 = std::chrono::high_resolution_clock::now();
  dec.Start();
  
  producer.join();
  //cout << "producer joined" << endl;
  consumer.join();
  //cout << "consumer joined" << endl;

  auto tend = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(tend - t0).count();
  double throughput = static_cast<double>(N) / diff; // in million entries/sec
  cout << "throughput: " << throughput << " Mwords/s" << endl;

  // eventually, decoder will time out and thread will join
  dec.Stop();

  EXPECT_GT(throughput, fastEnough);

  // XXX should check output is correct

}

TEST_F(DecoderFixture, Test2xDecoder)
{
  // two identical Decoders in this test
  Decoder dec0(pars, buf_in, bufs_out, M);
  Decoder dec1(pars, buf_in, bufs_out, M);

  // start producer/consumer threads
  std::vector<DecOutput> consumed;
  producer = std::thread(ProduceN<DecInput>, buf_in, &input_vals[0], N, M, 0);
  consumer = std::thread(ConsumeVectN<DecOutput>, bufs_out[pars->FunnelIdx(NRNI)], &consumed, N, M, 0);

  // start decoder, sources from producer through buf_in, sinks to consumer through buf_out
  auto t0 = std::chrono::high_resolution_clock::now();
  dec0.Start();
  dec1.Start();
  
  producer.join();
  //cout << "producer joined" << endl;
  consumer.join();
  //cout << "consumer joined" << endl;

  auto tend = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(tend - t0).count();
  double throughput = static_cast<double>(N) / diff; // in million entries/sec
  cout << "throughput: " << throughput << " Mwords/s" << endl;

  // eventually, decoder will time out and thread will join
  dec0.Stop();
  dec1.Stop();

  // for now, testing speedup
  EXPECT_GT(throughput, 2*fastEnough);

  // XXX should check output is correct
}
