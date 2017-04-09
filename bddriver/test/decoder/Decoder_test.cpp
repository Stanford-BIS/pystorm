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
    unsigned int N = 100000;
    unsigned int M = 1000;
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

class MyStupidClass {
  public:
    MyStupidClass(uint64_t x) { the_int = {x}; }
    uint64_t StupidMemberFn() { return the_int[0] + 1; }

  private:
    vector<uint64_t> the_int;
};

void MyStupidBinaryFunction(unsigned int * in, Binary * out, unsigned int M)
{
  for (unsigned int i = 0; i < M; i++) {
    out[i] = Binary(i + 1, 32);
  }
}

void MyStupidFunction(unsigned int * in, uint64_t * out, unsigned int M)
{
  for (unsigned int i = 0; i < M; i++) {
    MyStupidClass foo(in[i]);
    out[i] = foo.StupidMemberFn();
  }
}

//TEST_F(DecoderFixture, TestMaxDecodeSpeed)
//{
//  Decoder dec(pars, buf_in, buf_out, M);
//
//  // XXX there's probably a bug in how I initialize Xcoder's std::thread
//  dec.Start();
//  dec.Stop();
//
//  // no producer/consumer in this one, just want to see the max single-threaded speed of the Decode() function
//  
//  std::vector<DecOutput> output_vals;
//  for (unsigned int i = 0; i < N; i++) {
//    output_vals.push_back(std::make_pair(HWLoc(0, "foo"), Binary(0,1)));
//  }
//
//  unsigned int in[N];
//  uint64_t out[N];
//  for (unsigned int i = 0; i < N; i++) {
//    in[i] = i;
//  }
//
//  Binary out_bin[N];
//
//  auto t0 = std::chrono::high_resolution_clock::now();
//  //for (unsigned int i = 0; i < N/M; i++) {
//  //  dec.Decode(&input_vals[i*M], M, &output_vals[i*M]);
//  //}
//  
//  //for (unsigned int i = 0; i < N/M; i++) {
//  //  MyStupidFunction(&in[i*M], &out[i*M], M;
//  //}
//
//  for (unsigned int i = 0; i < N/M; i++) {
//    MyStupidBinaryFunction(&in[i*M], &out_bin[i*M], M);
//  }
//  auto tend = std::chrono::high_resolution_clock::now();
//  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(tend - t0).count();
//  double throughput = static_cast<double>(N) / diff; // in million entries/sec
//  cout << "throughput: " << throughput << " Mwords/s" << endl;
//}

TEST_F(DecoderFixture, TestMaxDecodeSpeed)
{
  Decoder dec(pars, buf_in, buf_out, M);

  // XXX there's probably a bug in how I initialize Xcoder's std::thread
  dec.Start();
  dec.Stop();

  // no producer/consumer in this one, just want to see the max single-threaded speed of the Decode() function
  
  std::vector<DecOutput> output_vals;
  for (unsigned int i = 0; i < N; i++) {
    output_vals.push_back(std::make_pair(HWLoc(0, "foo"), Binary(0,1)));
  }

  unsigned int in[N];
  uint64_t out[N];
  for (unsigned int i = 0; i < N; i++) {
    in[i] = i;
  }

  Binary out_bin[N];

  auto t0 = std::chrono::high_resolution_clock::now();
  //for (unsigned int i = 0; i < N/M; i++) {
  //  dec.Decode(&input_vals[i*M], M, &output_vals[i*M]);
  //}
  
  //for (unsigned int i = 0; i < N/M; i++) {
  //  MyStupidFunction(&in[i*M], &out[i*M], M;
  //}

  for (unsigned int i = 0; i < N/M; i++) {
    MyStupidBinaryFunction(&in[i*M], &out_bin[i*M], M);
  }
  auto tend = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(tend - t0).count();
  double throughput = static_cast<double>(N) / diff; // in million entries/sec
  cout << "throughput: " << throughput << " Mwords/s" << endl;
}


TEST_F(DecoderFixture, Test1xDecoder)
{
  Decoder dec(pars, buf_in, buf_out, M);

  // start producer/consumer threads
  std::vector<DecOutput> consumed;
  consumed.reserve(N);
  producer = std::thread(ProduceN<DecInput>, buf_in, &input_vals[0], N, M, 0);
  consumer = std::thread(ConsumeVectN<DecOutput>, buf_out, &consumed, N, M, 0);

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
  Decoder dec0(pars, buf_in, buf_out, M);
  Decoder dec1(pars, buf_in, buf_out, M);

  // start producer/consumer threads
  std::vector<DecOutput> consumed;
  producer = std::thread(ProduceN<DecInput>, buf_in, &input_vals[0], N, M, 0);
  consumer = std::thread(ConsumeVectN<DecOutput>, buf_out, &consumed, N, M, 0);

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
