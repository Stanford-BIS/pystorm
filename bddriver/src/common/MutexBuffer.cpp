#include <assert.h>

#include <vector>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <chrono> // duration, for wait_for

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
void MutexBuffer<T>::PushData(const T * input, unsigned int input_len)
// the base of the Push and TryPush calls
{
  // copy the data
  for (unsigned int i = 0; i < input_len; i++) {
    vals_[(back_ + i) % capacity_] = input[i];
  }

  // update queue state
  back_ = (back_ + input_len) % capacity_;
  count_ += input_len;
  
  //cout << "pushed data starting with " << vals_[back_ - input_len];
  //cout << "just pushed, size = " << count_ << endl;

}

template<class T>
bool MutexBuffer<T>::Push(const T * input, unsigned int input_len, unsigned int try_for_us)
// block until <input_len> elements at <input> are successfully pushed or <try_for_us> us have expired
// returns true if push suceeded, false if push failed
{
  assert(input_len < capacity_ && "trying to insert a vector longer than queue capacity"); // _capacity is TS

  std::unique_lock<std::mutex> lock(mutex_);

  // wait for the queue to have enough room for the input
  // if <try_for_us> == 0, then wait until notified without timeout
  if (try_for_us == 0) {
    just_popped_.wait(
        lock, 
        [this, input_len]{ return HasRoomFor(input_len); }
    );
  // else, time out after try_for_us microseconds
  } else {
    bool success = just_popped_.wait_for(
        lock, 
        std::chrono::duration<unsigned int, std::micro>(try_for_us),
        [this, input_len]{ return HasRoomFor(input_len); }
    );

    if (!success) {
      return false;
    }
  }

  // copy the data, update queue state
  PushData(input, input_len);

  // let the consumer know it's time to wake up
  lock.unlock();
  just_pushed_.notify_all();

  return true;
}

template<class T>
bool MutexBuffer<T>::Push(const std::vector<T>& input, unsigned int try_for_us)
{
  return Push(&input[0], input.size(), try_for_us);
}

template<class T>
unsigned int MutexBuffer<T>::PopData(T * copy_to, unsigned int max_to_pop) 
{
  // figure out how many elements are going to be in output
  unsigned int num_popped;
  if (max_to_pop < count_) {
    num_popped = max_to_pop;
  } else {
    num_popped = count_;
  }

  // do copy
  for (unsigned int i = 0; i < num_popped; i++) {
    copy_to[i] = vals_[(front_ + i) % capacity_];
  }

  // update queue state
  front_ = (front_ + num_popped) % capacity_;
  count_ -= num_popped;

  //cout << "popped data starting with " << vals_[front_ - num_popped];
  //cout << "just popped, size = " << count_ << endl;

  return num_popped;
}

template<class T>
unsigned int MutexBuffer<T>::Pop(T * copy_to, unsigned int max_to_pop, unsigned int try_for_us)
// copies data from front_ into copy_to. Returns number read (because maybe num_to_read > count_)
{
  std::unique_lock<std::mutex> lock(mutex_);

  // wait for the queue to have something to output
  // if try_for_us == 0, wait until notified
  if (try_for_us == 0) {
    just_pushed_.wait(lock, [this]{ return !IsEmpty(); });
  // else, time out after try_for_us microseconds
  } else {
    bool success = just_popped_.wait_for(
        lock, 
        std::chrono::duration<unsigned int, std::micro>(try_for_us),
        [this]{ return !IsEmpty(); }
    );

    if (!success) {
      return 0;
    }
  }

  // copy the data, update queue state
  unsigned int num_popped = PopData(copy_to, max_to_pop);

  // notify the producer that they may wake up
  lock.unlock();
  just_popped_.notify_all();

  return num_popped;
}


template<class T>
std::vector<T> MutexBuffer<T>::PopVect(unsigned int max_to_pop, unsigned int try_for_us)
{
  std::vector<T> output;
  output.reserve(max_to_pop);

  unsigned int num_popped = Pop(&output[0], try_for_us);
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
