#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H

#include "MutexBuffer.h"
#include "gtest/gtest.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>
#include <memory>

#include <iostream>

using namespace pystorm;
using namespace bddriver;
using namespace std;

template <class T>
void Produce(bddriver::MutexBuffer<T>* buf, const std::vector<std::vector<T>> &vals)
{
  unsigned int i = 0;
  for (auto& v : vals) {
    //cout << "prod " << i << endl;
    // copy each vector and push
    std::unique_ptr<std::vector<T>> un_copy = std::make_unique<std::vector<T>>(v);
    assert(un_copy.get() != nullptr);
    buf->Push(std::move(un_copy));
    i++;
  }
}

template <class T>
void Consume(bddriver::MutexBuffer<T>* buf, std::vector<std::vector<T>> * vals, unsigned int N, unsigned int try_for_us) {
  vals->clear();
  unsigned int N_consumed = 0;
  while (N_consumed < N) {
    //cout << "before pop" << N_consumed << endl;
    std::unique_ptr<std::vector<T>> popped = buf->Pop(try_for_us);
    //cout << "after pop" << N_consumed << endl;
    if (popped->size() > 0) {
      // copy popped data
      vals->push_back({});
      for (auto& it : *popped) {
        vals->back().push_back(it);
      }
      N_consumed++;
    }
    //cout << "after pop" << N_consumed << endl;
  }
}

template <class T>
void ConsumeAndCheck(bddriver::MutexBuffer<T>* buf, const std::vector<std::vector<T>> &check_vals, unsigned int try_for_us) {
  std::vector<std::vector<T>> to_fill;
  Consume(buf, &to_fill, check_vals.size(), try_for_us);
  ASSERT_EQ(to_fill.size(), check_vals.size());
  for (unsigned int i = 0; i < check_vals.size(); i++) {
    ASSERT_EQ(to_fill.at(i), check_vals.at(i));
  }
}

#endif
