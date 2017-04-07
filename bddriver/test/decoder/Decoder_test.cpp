#include "MutexBuffer.h"
#include "Decoder.h"

#include <unistd.h> // usleep
#include <chrono>

#include <iostream>
#include <thread>

#include "gtest/gtest.h"
#include "test_util/Producer_Consumer.cpp"

using std::cout;
using std::endl;
using namespace pystorm;
using namespace bddriver;


std::vector<DecInput> MakeDecInput(unsigned int N, const BDPars * pars) {
  std::vector<DecInput> vals;

  for (unsigned int i = 0; i < N; i++) {
    unsigned int input_width = pars->Width("BD_output");
    const Binary * route = pars->FunnelRoute("softleaf"); 
    Binary payload(i, input_width - route->Bitwidth());

    Binary payload_route({payload, *route});

    vals.push_back(payload_route);
  }

  return vals;
}

class DecoderFixture : public testing::Test
{
  public:
    void SetUp() 
    {
      buf_in = new MutexBuffer<DecInput>(buf_depth);
      buf_out = new MutexBuffer<DecOutput>(buf_depth);

      pars = new BDPars("foo.yaml"); // filename unused for now

      input_vals = MakeDecInput(N, pars);
    }

    const unsigned int input_width = 34;
    unsigned int N = 500000;
    unsigned int M = 100;
    unsigned int buf_depth = 10000;

    // XXX set this to something meaningful later, compiler flags?
    const double fastEnough = .2;

    MutexBuffer<DecInput> * buf_in;
    MutexBuffer<DecOutput> * buf_out;

    BDPars * pars;

    std::vector<DecInput> input_vals;

    std::thread producer;
    std::thread consumer;
};


TEST_F(DecoderFixture, Test1xDecoder)
{
  Decoder enc(pars, buf_in, buf_out, M);

  // start producer/consumer threads
  std::vector<DecOutput> consumed;
  producer = std::thread(ProduceN<DecInput>, buf_in, &input_vals[0], N, M, 0);
  consumer = std::thread(ConsumeN<DecOutput>, buf_out, &consumed, N, M, 0);

  // start encoder, sources from producer through buf_in, sinks to consumer through buf_out
  auto t0 = std::chrono::high_resolution_clock::now();
  enc.Start();
  
  producer.join();
  cout << "producer joined" << endl;
  consumer.join();
  cout << "consumer joined" << endl;

  auto tend = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(tend - t0).count();
  double throughput = static_cast<double>(N*M) / diff; // in million entries/sec
  cout << "throughput: " << throughput << " Mwords/s" << endl;

  // eventually, encoder will time out and thread will join
  enc.Stop();

  EXPECT_GT(throughput, fastEnough);

  // XXX should check output is correct

}

//TEST_F(DecoderFixture, Test2xDecoder)
//{
//  // two identical Decoders in this test
//  Decoder enc0(pars, buf_in, buf_out, M);
//  Decoder enc1(pars, buf_in, buf_out, M);
//
//  // start producer/consumer threads
//  std::vector<DecOutput> consumed;
//  producer = std::thread(ProduceNxM, buf_in, &input_vals[0], N, M, 0);
//  consumer = std::thread(ConsumeNxM, buf_out, &consumed, N, M, 0);
//
//  // start encoder, sources from producer through buf_in, sinks to consumer through buf_out
//  auto t0 = std::chrono::high_resolution_clock::now();
//  enc0.Start();
//  enc1.Start();
//  
//  producer.join();
//  cout << "producer joined" << endl;
//  consumer.join();
//  cout << "consumer joined" << endl;
//
//  auto tend = std::chrono::high_resolution_clock::now();
//  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(tend - t0).count();
//  double throughput = static_cast<double>(N*M) / diff; // in million entries/sec
//  cout << "throughput: " << throughput << " Mwords/s" << endl;
//
//  // eventually, encoder will time out and thread will join
//  enc0.Stop();
//  enc1.Stop();
//
//  // for now, testing speedup
//  EXPECT_GT(throughput, 2*fastEnough);
//
//  // XXX should check output is correct
//}
