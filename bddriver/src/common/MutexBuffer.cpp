#include <assert.h>

#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono> // duration, for wait_for


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
bool MutexBuffer<T>::HasAtLeast(unsigned int num)
{
  return count_ >= num;
}

template<class T>
bool MutexBuffer<T>::HasRoomFor(unsigned int size)
{
  bool has_room = size <= capacity_ - count_;
  return has_room;
}


template<class T>
bool MutexBuffer<T>::WaitForHasAtLeast(std::unique_lock<std::mutex> * lock, unsigned int try_for_us, unsigned int multiple)
{
  assert(multiple != 0);
  // wait for the queue to have something to output
  // if try_for_us == 0, wait until notified
  if (!HasAtLeast(multiple)) { 
    if (try_for_us == 0) { 
      just_pushed_.wait(*lock, [this, multiple]{ return HasAtLeast(multiple); });
    // else, time out after try_for_us microseconds
    } else {
      bool success = just_pushed_.wait_for(
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
  assert(input_len <= capacity_ && "trying to insert a vector longer than queue capacity"); // _capacity is TS

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
void MutexBuffer<T>::UpdateStateForPush(unsigned int num_pushed)
{
  back_ = (back_ + num_pushed) % capacity_;
  count_ += num_pushed;
}

template<class T>
void MutexBuffer<T>::UpdateStateForPop(unsigned int num_popped)
{
  front_ = (front_ + num_popped) % capacity_;
  count_ -= num_popped;
}

template<class T>
unsigned int MutexBuffer<T>::DetermineNumToPop(unsigned int max_to_pop, unsigned int multiple) 
{
  // sample count_ once for the entire function
  // if you used count_ in place of maybe_stale_count in the if statement below, you could run into problems:
  // e.g. the first time you accessed count_, in the if(), count_ could be small
  // then someone else pushes, and if you go into the else, count_ could suddenly be larger.
  // it could even be bigger than max_to_pop (which could cause big problems for the user).
  unsigned int maybe_stale_ct = count_; 

  // figure out how many elements are going to be in output
  unsigned int num_to_pop;
  if (max_to_pop < maybe_stale_ct) {
    num_to_pop = max_to_pop;
  } else {
    num_to_pop = maybe_stale_ct;
  }

  // num_to_pop must be a multiple
  num_to_pop -= num_to_pop % multiple;

  assert(num_to_pop <= max_to_pop);
  assert(num_to_pop <= maybe_stale_ct);

  return num_to_pop;
}


template<class T>
bool MutexBuffer<T>::Push(const T * input, unsigned int input_len, unsigned int try_for_us)
// block until <input_len> elements at <input> are successfully pushed or <try_for_us> us have expired
// returns true if push suceeded, false if push failed
{
  std::unique_lock<std::mutex> back_lock(back_lock_); // have to wait if someone else is doing BackFront()

  // we currently have the lock, but if there isn't room to push, we need to wait
  bool success = WaitForHasRoomFor(&back_lock, input_len, try_for_us);
  if (!success) {
    return false;
  }

  // copy the data
  for (unsigned int i = 0; i < input_len; i++) {
    vals_[(back_ + i) % capacity_] = input[i];
  }

  // update buffer state
  UpdateStateForPush(input_len);

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
unsigned int MutexBuffer<T>::Pop(T * copy_to, unsigned int max_to_pop, unsigned int try_for_us, unsigned int multiple)
// copies data from front_ into copy_to. Returns number read (because maybe num_to_read > count_)
{
  std::unique_lock<std::mutex> front_lock(front_lock_); // have to wait if someone else is doing LockFront()

  // we currently have the lock, but if the buffer is empty, we need to wait
  bool success = WaitForHasAtLeast(&front_lock, try_for_us, multiple);
  if (!success) { // timed out before there was enough data
    return 0;
  }

  // copy the data
  unsigned int num_to_pop = DetermineNumToPop(max_to_pop, multiple);

  // do copy
  for (unsigned int i = 0; i < num_to_pop; i++) {
    copy_to[i] = vals_[(front_ + i) % capacity_];
  }

  // update buffer state
  UpdateStateForPop(num_to_pop);

  // notify the producer that they may wake up
  just_popped_.notify_all();

  return num_to_pop;
}


template<class T>
unsigned int Pop(std::vector<T> * push_to, unsigned int max_to_pop, unsigned int try_for_us=0, unsigned int multiple=1)
{
  unsigned int orig_size = push_to->size();
  push_to->resize(orig_size + max_to_pop); // have to make it big enough to fit the max, we'll size down later

  unsigned int num_popped = Pop(&push_to[orig_size], max_to_pop, try_for_us, multiple);
  push_to.resize(orig_size + num_popped);

  return num_popped;
}


template<class T>
std::vector<T> MutexBuffer<T>::PopVect(unsigned int max_to_pop, unsigned int try_for_us, unsigned int multiple)
{
  std::vector<T> output;
  output.resize(max_to_pop); // can't just reserve, vector is lazy, size down after though

  unsigned int num_popped = Pop(&output[0], max_to_pop, try_for_us, multiple);
  output.resize(num_popped);

  return output;
}


template<class T>
std::pair<const T *, unsigned int> MutexBuffer<T>::LockFront(unsigned int max_to_pop, unsigned int try_for_us, unsigned int multiple)
{
  // only returns a pointer to the front_ and a max number of entries
  // that may be read, starting from there, does not update queue state
  
  // can only have one thread doing this type of read at a time
  // otherwise much harder to keep track of the front_
  front_ulock_ = new std::unique_lock<std::mutex>(front_lock_);

  // we currently have the lock, but if the buffer is empty, we need to wait
  bool success = WaitForHasAtLeast(front_ulock_, try_for_us, multiple);
  if (!success) { // timed out before there was enough data
    num_to_read_ = 0;
    return std::make_pair(nullptr, 0);
  }

  // figure out how many entries can be read sequentially (handle wrap-around)
  unsigned int max_num_to_read;
  if (front_ + max_to_pop > capacity_) { // wrap-around, read only up to end
    max_num_to_read = capacity_ - front_;
  } else {
    max_num_to_read = max_to_pop;
  }

  // pass the new upper bound to the normal function
  unsigned int num_to_read = DetermineNumToPop(max_num_to_read, multiple);
  assert(front_ + num_to_read <= capacity_);

  // keep track of how many we're allowing the (single) consumer to read
  num_to_read_ = num_to_read;

  // front_ulock_ stays locked (so no one else can issue Read())

  return std::make_pair(&vals_[front_], num_to_read);

}


template<class T>
void MutexBuffer<T>::UnlockFront()
{
  // tells the MB that the client is done reading from the memory associated with LockFront()
  
  assert(num_to_read_ <= count_);
 
  // update front_ and count_;
  UpdateStateForPop(num_to_read_);

  // allow someone else to issue LockFront()
  front_lock_.unlock();

  // only now have we actually made space
  // notify the producers that they may wake up
  just_popped_.notify_all();
  
  // allow someone else to issue LockFront()
  delete front_ulock_;
}


template<class T>
T * MutexBuffer<T>::LockBack(unsigned int input_len, unsigned int try_for_us)
{
  back_ulock_ = new std::unique_lock<std::mutex>(back_lock_);

  // we currently have the global lock, but if the buffer is nearly full, we have to wait
  bool success = WaitForHasRoomFor(back_ulock_, input_len, try_for_us);
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

  // have to do a copy if scratchpad_ was used
  if (scratchpad_.size() > 0) {
    for (unsigned int i = 0; i < scratchpad_.size(); i++) {
      vals_[(back_ + i) % capacity_] = scratchpad_[i];
    }
  }

  // update queue state
  UpdateStateForPush(num_to_write_);

  // allow someone else to issue LockBack()
  back_lock_.unlock();

  // only now have we actually written the data in
  // notify the consumers that they may wake up
  just_pushed_.notify_all();

  // allow someone else to issue LockBack()
  delete back_ulock_;
}


} // bddriver
} // pystorm
