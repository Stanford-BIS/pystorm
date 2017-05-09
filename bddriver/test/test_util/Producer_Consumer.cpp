#include "MutexBuffer.h"
#include "gtest/gtest.h"

#include <chrono>
#include <vector>
#include <cstdint>
#include <string>
#include <thread>

#include <iostream>

using namespace pystorm;
using namespace bddriver;
using namespace std;

template <class T>
void ProduceN(bddriver::MutexBuffer<T> * buf, T * vals, unsigned int N, unsigned int M, unsigned int try_for_us)
// push M elements N times
{
  unsigned int num_pushed = 0;
  while (num_pushed < N) {

    unsigned int num_to_push;
    if (N - num_pushed > M) {
      num_to_push = M;
    } else {
      num_to_push = N - num_pushed;
    }

    //cout << "pushing: ";
    //for (unsigned int j = 0; j < num_to_push; j++) {
    ////  cout << data[j] << ", ";
    //}
    //cout << ". " << num_pushed + num_to_push << " pushed" << endl;

    bool success = false;
    while (!success) {
      success = buf->Push(&vals[num_pushed], num_to_push, try_for_us);
    }

    num_pushed += num_to_push;

  }
}

template <class T>
void ConsumeN(bddriver::MutexBuffer<T> * buf, T * vals, unsigned int N, unsigned int M, unsigned int try_for_us)
{
  unsigned int N_consumed = 0;
  while (N_consumed != N) {
    unsigned int num_popped = buf->Pop(&vals[N_consumed], M, try_for_us);

    //cout << "popping: ";
    //for (unsigned int i = 0; i < num_popped; i++) {
    //  //cout << data[i] << ", ";
    //}
    //cout << ". " << N_consumed + num_popped << " popped" << endl;

    N_consumed += num_popped;
  }
}

template <class T>
void ConsumeVectNGiveUpAfter(bddriver::MutexBuffer<T> * buf, vector<T> * vals, unsigned int N, unsigned int M, unsigned int try_for_us, unsigned int give_up_after_us)
{
  T data[M];

  bool give_up = false;
  auto t0 = std::chrono::high_resolution_clock::now();

  unsigned int N_consumed = 0;
  while (N_consumed != N && give_up == false) {
    //cout << "hi" << endl;
    unsigned int num_popped = buf->Pop(data, M, try_for_us);

    //cout << "popping: ";
    //for (unsigned int i = 0; i < num_popped; i++) {
    //  //cout << data[i] << ", ";
    //}
    //cout << ". " << N_consumed + num_popped << " popped" << endl;
    
    for (unsigned int j = 0; j < num_popped; j++) {
      vals->push_back(data[j]);
    }

    N_consumed += num_popped;

    auto tnow = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(tnow - t0).count();
    if (give_up_after_us != 0 && diff > give_up_after_us) {
      give_up = true;
    }
  }
}

template <class T>
void ConsumeVectN(bddriver::MutexBuffer<T> * buf, vector<T> * vals, unsigned int N, unsigned int M, unsigned int try_for_us)
{
  ConsumeVectNGiveUpAfter(buf, vals, N, M, try_for_us, 0);
}


template <class T>
void ConsumeVectReadThenPopN(bddriver::MutexBuffer<T> * buf, vector<T> * vals, unsigned int N, unsigned int M, unsigned int try_for_us)
{
  unsigned int N_consumed = 0;
  while (N_consumed != N) {
    //cout << "hi" << endl;
    const T * read_ptr;
    unsigned int num_to_read;
    std::tie(read_ptr, num_to_read) = buf->Read(M, try_for_us);

    //cout << "popping: ";
    //for (unsigned int i = 0; i < num_popped; i++) {
    //  //cout << data[i] << ", ";
    //}
    //cout << ". " << N_consumed + num_popped << " popped" << endl;
    
    for (unsigned int j = 0; j < num_to_read; j++) {
      vals->push_back(read_ptr[j]);
    }

    buf->PopAfterRead();

    N_consumed += num_to_read;
  }
}

template <class T>
void ConsumeAndCheckN(bddriver::MutexBuffer<T> * buf, T * check_vals, unsigned int N, unsigned int M, unsigned int try_for_us)
{
  T data[M];

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

