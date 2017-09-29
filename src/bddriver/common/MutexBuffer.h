#ifndef MUTEXBUFFER_H
#define MUTEXBUFFER_H

#include <condition_variable>
#include <chrono>  // duration, for wait_for
#include <mutex>
#include <queue>
#include <vector>
#include <memory>
#include <cassert>

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

// thread-safe deque of vectors
template <class T>
class MutexBuffer {
 private:
  std::queue<std::unique_ptr<std::vector<T>>> vals_;

  // signal sleeping consumer threads to wake up
  std::condition_variable just_pushed_;

  // single lock to modify vals_
  // note that the producers and consumers can simultaneously read/write the
  // CONTENTS of the vectors Pop()ed/Push()ed
  std::mutex lock_;

 public:

  MutexBuffer() {};
  ~MutexBuffer() {};

  // simple vector interface, the slowest option

  /// Push() pushes the elements in <input> to the back of the buffer.
  /// Blocks until it can gain the lock (shouldn't take long)
  void Push(std::unique_ptr<std::vector<T>> input) {
    // gain lock, release it when we fall out of scope
    std::unique_lock<std::mutex> ulock(lock_);

    assert(input.get() != nullptr);

    // push (move) vector pointer to back of queue
    vals_.emplace(std::move(input));

    // let the sleeping threads know they can wake up
    just_pushed_.notify_all();
  }

  /// Pop() gets a vector of elements from the front of the buffer
  /// Blocks until the lock can be gained and there is somethign to pop
  /// Optionally can time out, returns empty vector in that case
  std::unique_ptr<std::vector<T>> Pop(unsigned int try_for_us=0) {
    // gain lock, release it when we fall out of scope or go to sleep
    std::unique_lock<std::mutex> ulock(lock_);

    // if empty, go to sleep until notified
    if (vals_.empty()) {
      if (try_for_us == 0) { // sleep indefinitely
        just_pushed_.wait(ulock, [this] { return !vals_.empty(); });
      } else { // else, time out after try_for_us microseconds
        auto timeout = std::chrono::duration<unsigned int, std::micro>(try_for_us);
        bool success = just_pushed_.wait_for(ulock, timeout, [this] { return !vals_.empty(); });
        if (!success) {
          // return empty vector pointer if we timed out
          return std::make_unique<std::vector<T>>();
        }
      }
    }

    // return front vector pointer
    std::unique_ptr<std::vector<T>> front_vect = std::move(vals_.front());
    vals_.pop();

    return front_vect;
  }

  /// PopAll() works similar to Pop(), except it pops everything available
  std::vector<std::unique_ptr<std::vector<T>>> PopAll(unsigned int try_for_us=1) {
    // gain lock, release it when we fall out of scope or go to sleep
    std::unique_lock<std::mutex> ulock(lock_);

    // Returned data
    std::vector<std::unique_ptr<std::vector<T>>> buf_out;

    // if empty, go to sleep until notified
    if (vals_.empty()) {
      if (try_for_us == 0) { // sleep indefinitely
        just_pushed_.wait(ulock, [this] { return !vals_.empty(); });
      } else { // else, time out after try_for_us microseconds
        auto timeout = std::chrono::duration<unsigned int, std::micro>(try_for_us);
        bool success = just_pushed_.wait_for(ulock, timeout, [this] { return !vals_.empty(); });
        if (!success) {
          // return empty container if we timed out
          return buf_out;
        }
      }
    }

    while (!vals_.empty())
    {
        buf_out.emplace_back(std::move(vals_.front()));
        vals_.pop();
    }

    return buf_out;
  }

};


}  // bddriver
}  // pystorm

#endif
