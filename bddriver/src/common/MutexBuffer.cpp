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
  delete[] vals_;
}


template<class T>
bool MutexBuffer<T>::WaitForHasAtLeast(std::unique_lock<std::mutex> * lock, unsigned int try_for_us, unsigned int multiple)
{
  // wait for the queue to have something to output
  // if try_for_us == 0, wait until notified
  if (!HasAtLeast(multiple)) { 
    if (try_for_us == 0) { 
      just_pushed_.wait(*lock, [this, multiple]{ return HasAtLeast(multiple); });
    // else, time out after try_for_us microseconds
    } else {
      bool success = just_popped_.wait_for(
          *lock, 
          std::chrono::duration<unsigned int, std::micro>(try_for_us),
          [this, multiple]{ return HasAtLeast(multiple); }
      );

      if (!success) {
        return false;
      }
    }
  }
  return true;
}


template<class T>
bool MutexBuffer<T>::WaitForHasRoomFor(std::unique_lock<std::mutex> * lock, unsigned int input_len, unsigned int try_for_us)
{
  // wait for the queue to have enough room for the input
  // if <try_for_us> == 0, then wait until notified without timeout
  if (!HasRoomFor(input_len)) { 
    if (try_for_us == 0) {
      just_popped_.wait(
          *lock, 
          [this, input_len]{ return HasRoomFor(input_len); }
      );
    // else, time out after try_for_us microseconds
    } else {
      bool success = just_popped_.wait_for(
          *lock, 
          std::chrono::duration<unsigned int, std::micro>(try_for_us),
          [this, input_len]{ return HasRoomFor(input_len); }
      );

      if (!success) {
        return false;
      }
    }
  }
  return true;
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

  std::unique_lock<std::mutex> back_lock(back_lock_); // have to wait if someone else is doing BackFront()
  std::unique_lock<std::mutex> lock(main_lock_);

  // we currently have the lock, but if there isn't room to push, we need to wait
  bool success = WaitForHasRoomFor(&lock, input_len, try_for_us);
  if (!success) {
    return false;
  }

  // copy the data, update queue state
  PushData(input, input_len);

  // let the consumer know it's time to wake up
  just_pushed_.notify_all();

  return true;
}


template<class T>
bool MutexBuffer<T>::Push(const std::vector<T>& input, unsigned int try_for_us)
{
  return Push(&input[0], input.size(), try_for_us);
}


template<class T>
unsigned int MutexBuffer<T>::PopData(T * copy_to, unsigned int max_to_pop, unsigned int multiple) 
{
  // figure out how many elements are going to be in output
  unsigned int num_popped;
  if (max_to_pop < count_) {
    num_popped = max_to_pop;
  } else {
    num_popped = count_;
  }

  // num_popped must be a multiple
  num_popped -= num_popped % multiple;
  assert(num_popped > 0);

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
unsigned int MutexBuffer<T>::Pop(T * copy_to, unsigned int max_to_pop, unsigned int try_for_us, unsigned int multiple)
// copies data from front_ into copy_to. Returns number read (because maybe num_to_read > count_)
{
  std::unique_lock<std::mutex> front_lock(front_lock_); // have to wait if someone else is doing LockFront()
  std::unique_lock<std::mutex> lock(main_lock_);

  // we currently have the lock, but if the buffer is empty, we need to wait
  bool success = WaitForHasAtLeast(&lock, try_for_us, multiple);
  if (!success) { // timed out before there was enough data
    return 0;
  }

  // copy the data, update queue state
  unsigned int num_popped = PopData(copy_to, max_to_pop, multiple);

  // notify the producer that they may wake up
  just_popped_.notify_all();

  return num_popped;
}


template<class T>
std::vector<T> MutexBuffer<T>::PopVect(unsigned int max_to_pop, unsigned int try_for_us, unsigned int multiple)
{
  std::vector<T> output;
  output.reserve(max_to_pop);

  unsigned int num_popped = Pop(&output[0], max_to_pop, try_for_us, multiple);
  output.resize(num_popped);

  return output;
}


template<class T>
bool MutexBuffer<T>::HasAtLeast(unsigned int num)
{
  return count_ >= num;
}


template<class T>
bool MutexBuffer<T>::HasRoomFor(unsigned int size)
{
  bool has_room = size <= capacity_ - count_;
  //cout << "seeing if there's room: " << has_room << endl;
  return has_room;
}


template<class T>
std::pair<const T *, unsigned int> MutexBuffer<T>::LockFront(unsigned int max_to_pop, unsigned int try_for_us, unsigned int multiple)
{
  // only returns a pointer to the front_ and a max number of entries
  // that may be read, starting from there, does not update queue state
  
  // can only have one thread doing this type of read at a time
  // otherwise much harder to keep track of the front_
  front_lock_.lock();

  std::unique_lock<std::mutex> lock(main_lock_);
  
  // we currently have the lock, but if the buffer is empty, we need to wait
  bool success = WaitForHasAtLeast(&lock, try_for_us, multiple);
  if (!success) { // timed out before there was enough data
    return std::make_pair(nullptr, 0);
  }

  // figure out how many entries can be read sequentially (handle wrap-around)
  unsigned int num_to_read;
  if (front_ + max_to_pop > capacity_) { // wrap-around, read only up to end
    num_to_read = capacity_ - front_;
  } else {
    num_to_read = max_to_pop;
  }
  // num_to_read must be a multiple
  num_to_read -= (num_to_read) % multiple;
  assert(num_to_read > 0);

  // keep track of how many we're allowing the (single) consumer to read
  num_to_read_ = num_to_read;

  // front_lock_ stays locked (so no one else can issue Read())

  return std::make_pair(&vals_[front_], num_to_read);

}


template<class T>
void MutexBuffer<T>::UnlockFront()
{
  // tells the MB that the client is done reading from the memory associated with LockFront()
  
  std::unique_lock<std::mutex> lock(main_lock_);
 
  // update front_ and count_;
  front_ = (front_ + num_to_read_) % capacity_;
  count_ -= num_to_read_;

  // only now have we actually made space
  // notify the producers that they may wake up
  just_popped_.notify_all();
  
  // allow someone else to issue LockFront()
  front_lock_.unlock();
}


template<class T>
T * MutexBuffer<T>::LockBack(unsigned int input_len, unsigned int try_for_us)
{
  back_lock_.lock(); // prevent any other push-like operations until this one is over

  std::unique_lock<std::mutex> lock(main_lock_);

  // we currently have the global lock, but if the buffer is nearly full, we have to wait
  bool success = WaitForHasRoomFor(&lock, input_len, try_for_us);
  if (!success) { // signal timeout by returning a null ptr
    return nullptr;
  }

  num_to_write_ = input_len;

  // if returning back_ would cause wraparound, return &scratchpad_[0] instead
  // user will write to scratchpad_ and we will copy it back during UnlockBack()
  //
  // This represents some performance overhead vs. normal operation, but keeps
  // the interface clean (otherwise, the user would have to deal with what would
  // happen if not all the data could be pushed). If capacity_ is a multiple of <input_len>,
  // then the scratchpad will never be used.
  
  if (back_ + input_len > capacity_) { 
    scratchpad_.resize(input_len);
    return &scratchpad_[0];
  } else {
    scratchpad_.resize(0); // does not free memory
    return &vals_[back_];
  }

}


template<class T>
void MutexBuffer<T>::UnlockBack()
{
  // tells the MB that the client is done writing to the memory associated with LockBack()

  std::unique_lock<std::mutex> lock(main_lock_);

  // have to do a copy if scratchpad_ was used
  if (scratchpad_.size() > 0) {
    for (unsigned int i = 0; i < scratchpad_.size(); i++) {
      vals_[(back_ + i) % capacity_] = scratchpad_[i];
    }
  }

  // update queue state
  back_ = (back_ + num_to_write_) % capacity_;
  count_ += num_to_write_;

  // only now have we actually written the data in
  // notify the consumers that they may wake up
  just_pushed_.notify_all();

  // allow someone else to issue LockBack()
  back_lock_.unlock();
}


} // bddriver
} // pystorm
