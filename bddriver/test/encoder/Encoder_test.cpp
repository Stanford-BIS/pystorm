#include "MutexBuffer.h"
#include "Encoder.h"
#include "gtest/gtest.h"

#include <unistd.h> // usleep
#include <chrono>

#include <iostream>
#include <thread>

#include "test_util/Producer_Consumer.cpp"

using std::cout;
using std::endl;
using namespace pystorm;
using namespace bddriver;

std::vector<EncInput> MakeEncInput(unsigned int N) {
  std::vector<EncInput> vals;

  for (unsigned int i = 0; i < N; i++) {
    // chip id 0, leaf "softleaf", payload N
    vals.push_back(std::make_pair(HWLoc(0, "softleaf"), Binary(i, 32)));
  }

  return vals;
}

class EncoderFixture : public testing::Test
{
  public:
    void SetUp() 
    {
      buf_in = new MutexBuffer<EncInput>(buf_depth);
      buf_out = new MutexBuffer<EncOutput>(buf_depth);

      pars = new BDPars("foo.yaml"); // filename unused for now

      input_vals = MakeEncInput(N);
    }

    unsigned int N = 100000;
    unsigned int M = 100;
    unsigned int buf_depth = 10000;

    // XXX set this to something meaningful later, compiler flags?
    const double fastEnough = .2;

    MutexBuffer<EncInput> * buf_in;
    MutexBuffer<EncOutput> * buf_out;

    BDPars * pars;

    std::vector<EncInput> input_vals;

    std::thread producer;
    std::thread consumer;
};


TEST_F(EncoderFixture, Test1xEncoder)
{
  Encoder enc(pars, buf_in, buf_out, M);

  // start producer/consumer threads
  std::vector<EncOutput> consumed;
  producer = std::thread(ProduceN<EncInput>, buf_in, &input_vals[0], N, M, 0);
  consumer = std::thread(ConsumeVectN<EncOutput>, buf_out, &consumed, N, M, 0);

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

  // XXX should check output is correct

}

TEST_F(EncoderFixture, Test2xEncoder)
{
  // two identical Encoders in this test
  Encoder enc0(pars, buf_in, buf_out, M);
  Encoder enc1(pars, buf_in, buf_out, M);

  // start producer/consumer threads
  std::vector<EncOutput> consumed;
  producer = std::thread(ProduceN<EncInput>, buf_in, &input_vals[0], N, M, 0);
  consumer = std::thread(ConsumeVectN<EncOutput>, buf_out, &consumed, N, M, 0);

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

  // for now, testing speedup
  EXPECT_GT(throughput, 2*fastEnough);

  // XXX should check output is correct
}

