#include <assert.h>

#include <vector>
#include <mutex>
#include <condition_variable>

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

template<class T>
MutexBuffer<T>::MutexBuffer(unsigned int capacity)
{
  vals_ = new T[capacity];
  capacity_ = capacity;
  front_ = 0;
  back_ = 0;
  count_ = 0;
}

template<class T>
MutexBuffer<T>::~MutexBuffer()
{
  delete vals_;
}

template<class T>
void MutexBuffer<T>::Push(const T * input, unsigned int input_len)
{
  assert(input_len < capacity_ && "trying to insert a vector longer than queue capacity"); // _capacity is TS

  std::unique_lock<std::mutex> lock(mutex_);

  // wait for the queue to have enough room for the input
  //cout << "about to try the lock" << endl;
  just_popped_.wait(lock, [this, input_len]{ return HasRoomFor(input_len); });
  //cout << "got the lock" << endl;

  // copy the data
  for (unsigned int i = 0; i < input_len; i++) {
    vals_[back_ + i] = input[i];
  }

  // update queue state
  back_ = (back_ + input_len) % capacity_;
  count_ += input_len;

  // let the consumer know it's time to wake up
  mutex_.unlock();
  just_pushed_.notify_all();
}

template<class T>
void MutexBuffer<T>::Push(const std::vector<T>& input)
{
  Push(&input[0], input.size());
}

template<class T>
unsigned int MutexBuffer<T>::Pop(T * copy_to, unsigned int max_to_pop)
// copies data from front_ into copy_to. Returns number read (because maybe num_to_read > count_)
{
  std::unique_lock<std::mutex> lock(mutex_);

  // wait for the queue to have something to output
  just_pushed_.wait(lock, [this]{ return !IsEmpty(); });

  // figure out how many elements are going to be in output
  unsigned int num_popped;
  if (max_to_pop < count_) {
    num_popped = max_to_pop;
  } else {
    num_popped = count_;
  }

  // do copy
  for (unsigned int i = 0; i < max_to_pop; i++) {
    copy_to[i] = vals_[(front_ + i) % capacity_];
  }

  // update queue state
  front_ = (front_ + num_popped) % capacity_;
  count_ -= num_popped;

  // notify the producer that they may wake up
  mutex_.unlock();
  just_popped_.notify_all();

  return num_popped;
}


template<class T>
std::vector<T> MutexBuffer<T>::PopVect(unsigned int max_to_pop)
{
  std::vector<T> output;
  output.reserve(max_to_pop);

  unsigned int num_popped = Pop(&output[0]);
  output.resize(num_popped);

  return output;
}

template<class T>
bool MutexBuffer<T>::IsEmpty()
{
  return count_ == 0;
}

template<class T>
bool MutexBuffer<T>::HasRoomFor(unsigned int size)
{
  bool has_room = size <= capacity_ - count_;
  //cout << "seeing if there's room: " << has_room << endl;
  return has_room;
}

} // bddriver
} // pystorm
