#include "MutexBuffer.h"
#include "gtest/gtest.h"
#include "Binary.h"

#include <vector>
#include <cstdint>
#include <string>
#include <thread>

#include <iostream>

#include "test_util/Producer_Consumer.cpp"

using namespace pystorm;
using namespace bddriver;
using namespace std;


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

    unsigned int buf_depth = 10000;
    unsigned int N = 100000; // number of messages
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

  producer0 = std::thread(ProduceN<unsigned int>, buf, vals0, N, M, 0);
  consumer0 = std::thread(ConsumeVectN<unsigned int>, buf, &consumed, N, M, 0);

  producer0.join();
  consumer0.join();

  for(unsigned int i = 0; i < N; i++) {
    ASSERT_EQ(consumed[i], vals0[i]);
  }
}

TEST_F(MutexBufferFixture, Test1to1WithTimeout)
{
  vector<unsigned int> consumed;

  producer0 = std::thread(ProduceN<unsigned int>, buf, vals0, N, M, 1000);
  consumer0 = std::thread(ConsumeVectN<unsigned int>, buf, &consumed, N, M, 1000);

  producer0.join();
  consumer0.join();

  for(unsigned int i = 0; i < N; i++) {
    ASSERT_EQ(consumed[i], vals0[i]);
  }
}

TEST_F(MutexBufferFixture, Test1to1OddSizes)
{
  vector<unsigned int> consumed;

  unsigned int MProd = 3;
  unsigned int MCons = 5;

  unsigned int vals_prod[N];
  for (unsigned int i = 0; i < N; i++) {
    vals_prod[i] = i;
  }

  producer0 = std::thread(ProduceN<unsigned int>, buf, &vals_prod[0], N, MProd, 0);
  consumer0 = std::thread(ConsumeAndCheckN<unsigned int>, buf, &vals_prod[0], N, MCons, 0);

  producer0.join();
  consumer0.join();
}

TEST_F(MutexBufferFixture, Test1to1OddSizesTimeout)
{
  vector<unsigned int> consumed;

  unsigned int MProd = 3;
  unsigned int MCons = 5;

  unsigned int vals_prod[N];
  for (unsigned int i = 0; i < N; i++) {
    vals_prod[i] = i;
  }

  producer0 = std::thread(ProduceN<unsigned int>, buf, &vals_prod[0], N, MProd, 1000);
  consumer0 = std::thread(ConsumeAndCheckN<unsigned int>, buf, &vals_prod[0], N, MCons, 1000);

  producer0.join();
  consumer0.join();
}

TEST_F(MutexBufferFixture, Test1to1OddSizesCheckAfter)
{
  vector<unsigned int> consumed;

  unsigned int MProd = 3;
  unsigned int MCons = 5;

  unsigned int vals_prod[N];
  for (unsigned int i = 0; i < N; i++) {
    vals_prod[i] = i;
  }

  producer0 = std::thread(ProduceN<unsigned int>, buf, &vals_prod[0], N, MProd, 0);
  consumer0 = std::thread(ConsumeVectN<unsigned int>, buf, &consumed, N, MCons, 0);

  producer0.join();
  consumer0.join();

  ASSERT_EQ(consumed.size(), N);
  for(unsigned int i = 0; i < N; i++) {
    ASSERT_EQ(consumed[i], vals_prod[i]);
  }
}

TEST_F(MutexBufferFixture, Test2to1)
{
  vector<unsigned int> consumed;

  // probably closest to the actual use case
  producer0 = std::thread(ProduceN<unsigned int>, buf, vals0, N, M, 0);
  producer1 = std::thread(ProduceN<unsigned int>, buf, vals1, N, M, 0);

  consumer0 = std::thread(ConsumeVectN<unsigned int>, buf, &consumed, 2*N, M, 0);

  producer0.join();
  producer1.join();

  consumer0.join();

  // because of nondeterminism, can't test for much more than the following
  unsigned int counts[2*N];
  for (unsigned int i = 0; i < 2*N; i++) {
    counts[i] = 0;
  }

  ASSERT_EQ(consumed.size(), 2*N);
  for (unsigned int i = 0; i < consumed.size(); i++) {
    ASSERT_LE(consumed[i], 2*N);
    counts[consumed[i]]++;
  }

  for (unsigned int i = 0; i < 2*N; i++) {
    //cout << counts[i] << endl;
    EXPECT_EQ(counts[i], static_cast<unsigned int>(1)); 
  }
}

TEST_F(MutexBufferFixture, Test2to2)
{
  vector<unsigned int> consumed0;
  vector<unsigned int> consumed1;

  producer0 = std::thread(ProduceN<unsigned int>, buf, vals0, N, M, 0);
  producer1 = std::thread(ProduceN<unsigned int>, buf, vals1, N, M, 0);

  consumer0 = std::thread(ConsumeVectN<unsigned int>, buf, &consumed0, N, M, 0);
  consumer1 = std::thread(ConsumeVectN<unsigned int>, buf, &consumed1, N, M, 0);

  producer0.join();
  producer1.join();

  consumer0.join();
  consumer1.join();

  unsigned int counts[2*N];
  for (unsigned int i = 0; i < 2*N; i++) {
    counts[i] = 0;
  }

  ASSERT_EQ(consumed0.size() + consumed1.size(), 2*N);
  for (unsigned int i = 0; i < consumed0.size(); i++) {
    ASSERT_LE(consumed0[i], 2*N);
    counts[consumed0[i]]++;
  }
  for (unsigned int i = 0; i < consumed1.size(); i++) {
    ASSERT_LE(consumed1[i], 2*N);
    counts[consumed1[i]]++;
  }

  for (unsigned int i = 0; i < 2*N; i++) {
    EXPECT_EQ(counts[i], static_cast<unsigned int>(1)); 
  }
}

TEST(MutexBufferPerformance, UintThroughput)
{
  const unsigned int N = 1000000;
  const unsigned int M = 1000;
  const unsigned int buf_depth = 100000;
  
  MutexBuffer<unsigned int> buf(buf_depth);

  unsigned int vals[N];
  for (unsigned int i = 0; i < N; i++) {
    vals[i] = i;
  }

  unsigned int consumed[N];

  auto t0 = std::chrono::high_resolution_clock::now();

  std::thread producer(ProduceN<unsigned int>, &buf, &vals[0], N, M, 0);
  std::thread consumer(ConsumeN<unsigned int>, &buf, &consumed[0], N, M, 0);

  producer.join();
  consumer.join();

  auto tend = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(tend - t0).count();
  double throughput = static_cast<double>(N) / diff; // in million entries/sec
  cout << "throughput: " << throughput << " Mwords/s" << endl;
}

//TEST(MutexBufferPerformance, BinaryThroughput)
//{
//  const unsigned int N = 1000000;
//  const unsigned int M = 1000;
//  const unsigned int buf_depth = 100000;
//  
//  MutexBuffer<Binary> buf(buf_depth);
//
//  Binary vals[N];
//  for (unsigned int i = 0; i < N; i++) {
//    vals[i] = Binary(i, 32);
//  }
//
//  Binary consumed[N];
//
//  auto t0 = std::chrono::high_resolution_clock::now();
//
//  std::thread producer(ProduceN<Binary>, &buf, &vals[0], N, M, 0);
//  std::thread consumer(ConsumeN<Binary>, &buf, &consumed[0], N, M, 0);
//
//  producer.join();
//  consumer.join();
//
//  auto tend = std::chrono::high_resolution_clock::now();
//  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(tend - t0).count();
//  double throughput = static_cast<double>(N) / diff; // in million entries/sec
//  cout << "throughput: " << throughput << " Mwords/s" << endl;
//}

