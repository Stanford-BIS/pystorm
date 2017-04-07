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

void ProduceN(bddriver::MutexBuffer<unsigned int> * buf, unsigned int * vals, unsigned int N, unsigned int M, unsigned int try_for_us)
// push M elements N times
{
  unsigned int data[M];

  unsigned int num_pushed = 0;
  while (num_pushed < N) {

    unsigned int num_to_push;
    if (N - num_pushed > M) {
      num_to_push = M;
    } else {
      num_to_push = N - num_pushed;
    }

    for (unsigned int j = 0; j < num_to_push; j++) {
      data[j] = vals[num_pushed + j];
    }

    //cout << "pushing: ";
    //for (unsigned int j = 0; j < num_to_push; j++) {
    //  cout << data[j] << ", ";
    //}
    //cout << endl;

    bool success = false;
    while (!success) {
      success = buf->Push(data, num_to_push, try_for_us);
    }

    num_pushed += num_to_push;

  }
}

void ProduceNxM(bddriver::MutexBuffer<unsigned int> * buf, unsigned int * vals, unsigned int N, unsigned int M, unsigned int try_for_us)
// push M elements N times
{
  unsigned int data[M];

  for (unsigned int i = 0; i < N; i++) {

    // push a bunch of copies of i 
    for (unsigned int j = 0; j < M; j++) {
      data[j] = vals[i];
    }

    bool success = false;
    while (!success) {
      success = buf->Push(data, M, try_for_us);
    }
  }
}

void ConsumeNxM(bddriver::MutexBuffer<unsigned int> * buf, vector<unsigned int> * vals, unsigned int N, unsigned int M, unsigned int try_for_us)
{
  unsigned int data[M];

  unsigned int N_consumed = 0;
  while (N_consumed != N*M) {
    unsigned int num_popped = buf->Pop(data, M, try_for_us);

    //cout << "popping: ";
    //for (unsigned int i = 0; i < num_popped; i++) {
    //  cout << data[i] << ", ";
    //}
    //cout << ". " << N_consumed + num_popped << " popped" << endl;
    //
    for (unsigned int j = 0; j < num_popped; j++) {
      vals->push_back(data[j]);
    }

    N_consumed += num_popped;
  }
}

void ConsumeAndCheckN(bddriver::MutexBuffer<unsigned int> * buf, unsigned int * check_vals, unsigned int N, unsigned int M, unsigned int try_for_us)
{
  unsigned int data[M];

  unsigned int N_consumed = 0;
  while (N_consumed != N) {
    unsigned int num_popped = buf->Pop(data, M, try_for_us);

    //cout << "popping: ";
    //for (unsigned int i = 0; i < num_popped; i++) {
    //  cout << data[i] << ", ";
    //}
    //cout << ". " << N_consumed + num_popped << " popped" << endl;

    for (unsigned int i = 0; i < num_popped; i++) {
      ASSERT_EQ(check_vals[N_consumed + i], data[i]);
    }

    N_consumed += num_popped;
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

    unsigned int buf_depth = 100000;
    unsigned int N = 10000; // number of messages
    unsigned int M = 1000; // message chunk size

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

  producer0 = std::thread(ProduceNxM, buf, vals0, N, M, 0);
  consumer0 = std::thread(ConsumeNxM, buf, &consumed, N, M, 0);

  producer0.join();
  consumer0.join();

  for(unsigned int i = 0; i < N; i++) {
    for(unsigned int j = 0; j < M; j++) {
      ASSERT_EQ(consumed[i*M + j], vals0[i]);
    }
  }
}

TEST_F(MutexBufferFixture, Test1to1WithTimeout)
{
  vector<unsigned int> consumed;

  producer0 = std::thread(ProduceNxM, buf, vals0, N, M, 1000);
  consumer0 = std::thread(ConsumeNxM, buf, &consumed, N, M, 1000);

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
  unsigned int NTot = N*MProd*MCons;

  unsigned int vals_prod[NTot];
  for (unsigned int i = 0; i < NTot; i++) {
    vals_prod[i] = i;
  }

  producer0 = std::thread(ProduceN, buf, &vals_prod[0], NTot, MProd, 0);
  consumer0 = std::thread(ConsumeAndCheckN, buf, &vals_prod[0], NTot, MCons, 0);

  producer0.join();
  consumer0.join();
}

TEST_F(MutexBufferFixture, Test1to1OddSizesTimeout)
{
  vector<unsigned int> consumed;

  unsigned int MProd = 3;
  unsigned int MCons = 5;
  unsigned int NTot = N*MProd*MCons;

  unsigned int vals_prod[NTot];
  for (unsigned int i = 0; i < NTot; i++) {
    vals_prod[i] = i;
  }

  producer0 = std::thread(ProduceN, buf, &vals_prod[0], NTot, MProd, 1000);
  consumer0 = std::thread(ConsumeAndCheckN, buf, &vals_prod[0], NTot, MCons, 1000);

  producer0.join();
  consumer0.join();
}

TEST_F(MutexBufferFixture, Test1to1OddSizesCheckAfter)
{
  vector<unsigned int> consumed;

  unsigned int MProd = 3;
  unsigned int MCons = 5;
  unsigned int NTot = N*MProd*MCons;

  unsigned int vals_prod[NTot];
  for (unsigned int i = 0; i < NTot; i++) {
    vals_prod[i] = i;
  }

  producer0 = std::thread(ProduceN, buf, &vals_prod[0], NTot, MProd, 0);
  consumer0 = std::thread(ConsumeNxM, buf, &consumed, NTot / MCons, MCons, 0);

  producer0.join();
  consumer0.join();

  ASSERT_EQ(consumed.size(), NTot);
  for(unsigned int i = 0; i < NTot; i++) {
    ASSERT_EQ(consumed[i], vals_prod[i]);
  }
}

TEST_F(MutexBufferFixture, Test2to1)
{
  vector<unsigned int> consumed;

  // probably closest to the actual use case
  producer0 = std::thread(ProduceNxM, buf, vals0, N, M, 0);
  producer1 = std::thread(ProduceNxM, buf, vals1, N, M, 0);

  consumer0 = std::thread(ConsumeNxM, buf, &consumed, 2*N, M, 0);

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

  producer0 = std::thread(ProduceNxM, buf, vals0, N, M, 0);
  producer1 = std::thread(ProduceNxM, buf, vals1, N, M, 0);

  consumer0 = std::thread(ConsumeNxM, buf, &consumed0, N, M, 0);
  consumer1 = std::thread(ConsumeNxM, buf, &consumed1, N, M, 0);

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

