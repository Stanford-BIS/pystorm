#include "MutexBuffer.h"
#include "gtest/gtest.h"

#include <vector>
#include <cstdint>
#include <string>
#include <thread>

#include <iostream>

using namespace pystorm;
using namespace bddriver;
using namespace std;

void ProduceN(bddriver::MutexBuffer<unsigned int> * buf, unsigned int * vals, unsigned int N, unsigned int M)
// push M elements N times
{
  unsigned int data[M];

  for (unsigned int i = 0; i < N; i++) {

    // push a bunch of copies of i 
    for (unsigned int j = 0; j < M; j++) {
      data[j] = vals[i*M + j];
    }

    buf->Push(data, M);
  }
}

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
  }
}

void ConsumeNxM(bddriver::MutexBuffer<unsigned int> * buf, vector<unsigned int> * vals, unsigned int N, unsigned int M)
{
  unsigned int data[M];

  unsigned int num_of_M = 0;
  for (unsigned int i = 0; i < N; i++) {

    while (num_of_M != M) {
      unsigned int num_needed = M - num_of_M;
      unsigned int num_popped = buf->Pop(data, num_needed);
      num_of_M += num_popped;
    }
    num_of_M = 0;

    for (unsigned int j = 0; j < M; j++) {
      vals->push_back(data[j]);
    }
  }
}

class MutexBufferFixture : public testing::Test
{
  public:
    void SetUp() {

      buf = new bddriver::MutexBuffer<unsigned int>(buf_depth);

      vals0 = new unsigned int[N];
      for (unsigned int i = 0; i < N; i++) {
        vals0[i] = i;
      }

      vals1 = new unsigned int[N];
      for (unsigned int i = 0; i < N; i++) {
        vals1[i] = i + N;
      }

    }

    unsigned int buf_depth = 32;
    unsigned int N = 100; // number of messages
    unsigned int M = 4; // message chunk size

    bddriver::MutexBuffer<unsigned int> * buf;

    unsigned int * vals0;
    unsigned int * vals1;

    std::thread producer0;
    std::thread producer1;
    std::thread consumer0;
    std::thread consumer1;

};

TEST_F(MutexBufferFixture, Test1to1)
{
  vector<unsigned int> consumed;

  producer0 = std::thread(ProduceNxM, buf, vals0, N, M);
  consumer0 = std::thread(ConsumeNxM, buf, &consumed, N, M);

  producer0.join();
  consumer0.join();

  for(unsigned int i = 0; i < N; i++) {
    for(unsigned int j = 0; j < M; j++) {
      ASSERT_EQ(consumed[i*M + j], vals0[i]);
    }
  }
}

TEST_F(MutexBufferFixture, Test1to1OddSizes)
{
  vector<unsigned int> consumed;

  unsigned int MProd = 3;
  unsigned int MCons = 5;
  unsigned int vals_prod[N*MCons*MProd];
  for (unsigned int i = 0; i < N*MCons*MProd; i++) {
    vals_prod[i] = 0;
  }

  producer0 = std::thread(ProduceN, buf, &vals_prod[0], N*MCons, MProd);
  consumer0 = std::thread(ConsumeNxM, buf, &consumed, N*MProd, MCons);

  producer0.join();
  consumer0.join();

  for(unsigned int i = 0; i < N; i++) {
    for(unsigned int j = 0; j < M; j++) {
      ASSERT_EQ(consumed[i*M + j], vals_prod[i]);
    }
  }
}

TEST_F(MutexBufferFixture, Test2to1)
{
  vector<unsigned int> consumed;

  // probably closest to the actual use case
  producer0 = std::thread(ProduceNxM, buf, vals0, N, M);
  producer1 = std::thread(ProduceNxM, buf, vals1, N, M);

  consumer0 = std::thread(ConsumeNxM, buf, &consumed, 2*N, M);

  producer0.join();
  producer1.join();

  consumer0.join();

  // because of nondeterminism, can't test for much more than the following
  unsigned int counts[2*N];
  for (unsigned int i = 0; i < 2*N; i++) {
    counts[i] = 0;
  }

  ASSERT_EQ(consumed.size(), 2*N*M);
  for (unsigned int i = 0; i < 2*N*M; i++) {
    ASSERT_LE(consumed[i], 2*N);
    counts[consumed[i]]++;
  }

  for (unsigned int i = 0; i < 2*N; i++) {
    //cout << counts[i] << endl;
    EXPECT_EQ(counts[i], M); 
  }
}

TEST_F(MutexBufferFixture, Test2to2)
{
  vector<unsigned int> consumed0;
  vector<unsigned int> consumed1;

  producer0 = std::thread(ProduceNxM, buf, vals0, N, M);
  producer1 = std::thread(ProduceNxM, buf, vals1, N, M);

  consumer0 = std::thread(ConsumeNxM, buf, &consumed0, N, M);
  consumer1 = std::thread(ConsumeNxM, buf, &consumed1, N, M);

  producer0.join();
  producer1.join();

  consumer0.join();
  consumer1.join();

  unsigned int counts[2*N];
  for (unsigned int i = 0; i < 2*N; i++) {
    counts[i] = 0;
  }

  ASSERT_EQ(consumed0.size() + consumed1.size(), 2*N*M);
  for (unsigned int i = 0; i < consumed0.size(); i++) {
    ASSERT_LE(consumed0[i], 2*N);
    counts[consumed0[i]]++;
  }
  for (unsigned int i = 0; i < consumed1.size(); i++) {
    ASSERT_LE(consumed1[i], 2*N);
    counts[consumed1[i]]++;
  }

  for (unsigned int i = 0; i < 2*N; i++) {
    EXPECT_EQ(counts[i], M); 
  }
}

