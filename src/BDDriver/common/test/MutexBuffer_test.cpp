#include <iostream>
#include <thread>

#include "BDDriver/common/MutexBuffer.h"

using std::cout;
using std::endl;
using namespace pystorm;

void ProduceNxM(bddriver::MutexBuffer<unsigned int> * buf, unsigned int * vals, unsigned int N, unsigned int M)
// push M elements N times
{
  unsigned int data[M];

  for (unsigned int i = 0; i < N; i++) {

    // push a bunch of copies of i 
    for (unsigned int j = 0; j < M; j++) {
      data[j] = vals[i];
    }

    buf->Push(data, M);
    cout << "pushed " << i << endl;
  }
}

void ConsumeNxM(bddriver::MutexBuffer<unsigned int> * buf, unsigned int N, unsigned int M)
{
  unsigned int data[M];

  unsigned int num_of_M = 0;
  for (unsigned int i = 0; i < N; i++) {

    cout << "recv: ";
    while (num_of_M != M) {
      unsigned int num_needed = M - num_of_M;
      unsigned int num_popped = buf->PopN(data, num_needed);
      num_of_M += num_popped;
      cout << num_popped << ":";
    }
    num_of_M = 0;

    for (unsigned int j = 0; j < M; j++) {
      cout << data[j] << ",";
    }
    cout << endl;
  }
}

void Foo() {
  cout << "hi" << endl;
}

int main () {

  bddriver::MutexBuffer<unsigned int> buf(32);

  unsigned int N = 100;
  unsigned int M = 4;

  unsigned int vals0[N];
  for (unsigned int i = 0; i < N; i++) {
    vals0[i] = i;
  }
  unsigned int vals1[N];
  for (unsigned int i = 0; i < N; i++) {
    vals1[i] = i + N;
  }

  std::thread producer0(ProduceNxM, &buf, &vals0[0], N, M);
  std::thread producer1(ProduceNxM, &buf, &vals1[0], N, M);
  std::thread consumer0(ConsumeNxM, &buf, N, M);
  std::thread consumer1(ConsumeNxM, &buf, N, M);
  // this was for debugging
  //std::thread producer(Foo);
  //std::thread consumer(Foo);

  producer0.join();
  producer1.join();
  consumer0.join();
  consumer1.join();

  return 0;

}
