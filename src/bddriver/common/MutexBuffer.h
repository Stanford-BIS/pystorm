#ifndef MUTEXBUFFER_H
#define MUTEXBUFFER_H

#include <condition_variable>
#include <chrono>  // duration, for wait_for
#include <mutex>
#include <deque>
#include <vector>
#include <memory>

namespace pystorm {
namespace bddriver {

// thread-safe deque of vectors
template <class T>
class MutexBuffer {
 private:
  std::deque<std::unique_ptr<std::vector<T>>> vals_;

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
    std::unique_lock<std::mutex>(lock_);

    // push (move) vector pointer to back of deque
    vals_.push_back(std::move(input));

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
    if (vals_.size() == 0) {
      if (try_for_us == 0) { // sleep indefinitely
        just_pushed_.wait(ulock, [this] { return vals_.size() > 0; });
      } else { // else, time out after try_for_us microseconds
        auto timeout = std::chrono::duration<unsigned int, std::micro>(try_for_us);
        bool success = just_pushed_.wait_for(ulock, timeout, [this] { return vals_.size() > 0; });
        if (!success) {
          // return empty vector pointer if we timed out
          return std::unique_ptr<std::vector<T>>(new std::vector<T>());
        }
      }
    }

    // return front vector pointer
    std::unique_ptr<std::vector<T>> front_vect = std::move(vals_.front());
    vals_.pop_front();

    return front_vect;
  }

};


}  // bddriver
}  // pystorm

#endif
