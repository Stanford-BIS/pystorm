#include "MutexBuffer.h"
#include "gtest/gtest.h"

#include <cstdint>
#include <string>
#include <thread>
#include <vector>
#include <memory>

#include <iostream>

#include "test_util/Producer_Consumer.h"

using namespace pystorm;
using namespace bddriver;
using namespace std;

class MutexBufferFixture : public testing::Test {
 public:
  void SetUp() {
    buf = new bddriver::MutexBuffer<unsigned int>();

    for (unsigned int i = 0; i < N; i++) {
      vals0.push_back({});
      for (unsigned int j = 0; j < M; j++) {
        vals0.back().push_back(i * M + j);
      }
    }
    
    for (unsigned int i = 0; i < N; i++) {
      vals1.push_back({});
      for (unsigned int j = 0; j < M; j++) {
        vals1.back().push_back(i * M + j + N*M);
      }
    }
  }

  unsigned int N = 1000; // number of messages
  unsigned int M = 1000; // message chunk size

  bddriver::MutexBuffer<unsigned int>* buf;

  std::vector<std::vector<unsigned int>> vals0;
  std::vector<std::vector<unsigned int>> vals1;

  std::thread producer0;
  std::thread producer1;
  std::thread consumer0;
  std::thread consumer1;
};

TEST_F(MutexBufferFixture, Test1to1) {
  producer0 = std::thread(Produce<unsigned int>, buf, vals0);
  consumer0 = std::thread(ConsumeAndCheck<unsigned int>, buf, vals0, 0);

  producer0.join();
  consumer0.join();
}

TEST_F(MutexBufferFixture, Test1to1WithTimeout) {
  vector<unsigned int> consumed;

  producer0 = std::thread(Produce<unsigned int>, buf, vals0);
  consumer0 = std::thread(ConsumeAndCheck<unsigned int>, buf, vals0, 1000);

  producer0.join();
  consumer0.join();
}


TEST_F(MutexBufferFixture, Test2to1) {
  std::vector<vector<unsigned int>> consumed;
  // probably closest to the actual use case
  producer0 = std::thread(Produce<unsigned int>, buf, vals0);
  producer1 = std::thread(Produce<unsigned int>, buf, vals1);

  consumer0 = std::thread(Consume<unsigned int>, buf, &consumed, 2 * N, 0);

  producer0.join();
  producer1.join();

  consumer0.join();

  // because of nondeterminism, can't test for much more than the following
  // XXX FIXME
  //unsigned int* counts = new unsigned int[2 * N*M];
  //for (unsigned int i = 0; i < 2 * N*M; i++) {
  //  counts[i] = 0;
  //}

  //ASSERT_EQ(consumed.size(), 2 * N);
  //for (unsigned int i = 0; i < consumed.size(); i++) {
  //  ASSERT_LE(consumed[i], 2 * N);
  //  counts[consumed[i]]++;
  //}

  //for (unsigned int i = 0; i < 2 * N; i++) {
  //  // cout << counts[i] << endl;
  //  ASSERT_EQ(counts[i], static_cast<unsigned int>(1));
  //}
}

TEST_F(MutexBufferFixture, Test2to2) {
  std::vector<vector<unsigned int>> consumed0;
  std::vector<vector<unsigned int>> consumed1;

  producer0 = std::thread(Produce<unsigned int>, buf, vals0);
  producer1 = std::thread(Produce<unsigned int>, buf, vals1);

  consumer0 = std::thread(Consume<unsigned int>, buf, &consumed0, 2 * N, 0);
  consumer1 = std::thread(Consume<unsigned int>, buf, &consumed1, 2 * N, 0);

  producer0.join();
  // cout << "p 0 joined" << endl;
  producer1.join();
  // cout << "p 1 joined" << endl;

  consumer0.join();
  // cout << "c 0 joined" << endl;
  consumer1.join();
  // cout << "c 1 joined" << endl;

  //XXX FIXME
  //unsigned int* counts = new unsigned int[2 * N];
  //for (unsigned int i = 0; i < 2 * N; i++) {
  //  counts[i] = 0;
  //}

  //EXPECT_EQ(consumed0.size() + consumed1.size(), 2 * N);
  //for (unsigned int i = 0; i < consumed0.size(); i++) {
  //  ASSERT_LE(consumed0[i], 2 * N);
  //  counts[consumed0[i]]++;
  //}
  //for (unsigned int i = 0; i < consumed1.size(); i++) {
  //  ASSERT_LE(consumed1[i], 2 * N);
  //  counts[consumed1[i]]++;
  //}

  //for (unsigned int i = 0; i < 2 * N; i++) {
  //  ASSERT_EQ(counts[i], static_cast<unsigned int>(1));
  //}
}

