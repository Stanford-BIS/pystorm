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
  for (auto& v : vals) {
    std::unique_ptr<std::vector<T>> un_copy(new std::vector<T>(v));
    buf->Push(std::move(un_copy));
  }
}

template <class T>
void Consume(bddriver::MutexBuffer<T>* buf, std::vector<std::vector<T>> * vals, unsigned int N, unsigned int try_for_us) {
  vals->clear();
  unsigned int N_consumed = 0;
  while (N_consumed != N) {
    std::unique_ptr<std::vector<T>> popped = buf->Pop(try_for_us);
    if (popped->size() > 0) {
      vals->push_back({});
      for (auto& it : *popped) {
        vals->back().push_back(it);
      }
      N_consumed++;
    }
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
