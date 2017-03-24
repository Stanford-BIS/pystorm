#include <unistd.h> // usleep
#include <chrono>

#include <iostream>
#include <thread>

#include "common/MutexBuffer.h"
#include "encoder/Encoder.h"

using std::cout;
using std::endl;
using namespace pystorm;
using namespace bddriver;

void ProduceNxM(MutexBuffer<EncInput> * buf, EncInput * vals, unsigned int N, unsigned int M)
// push M elements N times
{
  EncInput data[M];

  for (unsigned int i = 0; i < N; i++) {

    // push a bunch of copies of i 
    for (unsigned int j = 0; j < M; j++) {
      data[j] = vals[i];
    }

    buf->Push(data, M);
    //cout << "pushed " << data[0].second.AsUint() << endl;
  }
}

void ConsumeNxM(MutexBuffer<EncOutput> * buf, unsigned int N, unsigned int M)
{
  EncOutput data[M];

  unsigned int num_of_M = 0;
  for (unsigned int i = 0; i < N; i++) {

    //cout << "recv: ";
    while (num_of_M != M) {
      unsigned int num_needed = M - num_of_M;
      unsigned int num_popped = buf->Pop(data, num_needed);
      num_of_M += num_popped;
      //cout << num_popped << ":";
    }
    num_of_M = 0;

    for (unsigned int j = 0; j < M; j++) {
      //cout << data[j].AsUint() << ",";
    }
    //cout << endl;
  }
}

void Foo() {
  cout << "hi" << endl;
}

std::vector<EncInput> MakeEncInput(unsigned int N) {
  std::vector<EncInput> vals;

  for (unsigned int i = 0; i < N; i++) {
    // chip id 0, leaf "softleaf", payload N
    vals.push_back(std::make_pair(HWLoc(0, "softleaf"), Binary(i, 32)));
    std::vector<uint64_t> foo = {1};
    std::vector<uint8_t> bar = {2};
    Binary someBin = Binary(foo, bar);
  }

  return vals;
}

int main () {

  unsigned int N = 100000;
  unsigned int M = 100;

  MutexBuffer<EncInput> buf_in(10000);
  MutexBuffer<EncOutput> buf_out(10000);

  BDPars pars("foo.yaml");
  Encoder enc(&pars, &buf_in, &buf_out, M);


  std::vector<EncInput> input_vals = MakeEncInput(N);

  auto t0 = std::chrono::high_resolution_clock::now();

  std::thread producer(ProduceNxM, &buf_in, &input_vals[0], N, M);
  std::thread consumer(ConsumeNxM, &buf_out, N, M);
  enc.Start();
  // this was for debugging
  //std::thread producer(Foo);
  //std::thread consumer(Foo);
  
  
  producer.join();
  cout << "producer joined" << endl;
  consumer.join();
  cout << "consumer joined" << endl;

  auto tend = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(tend - t0).count();
  cout << N*M << " entries took " << diff << "us" << endl;
  cout << static_cast<double>(N*M) / diff * 1000000.0 << " entries/s" << endl;

  enc.Stop();
  // XXX this won't happen, the encoder is hung waiting
  cout << "encoder joined" << endl;

  return 0;

}
