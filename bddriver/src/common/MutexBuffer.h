#ifndef MUTEXBUFFER_H
#define MUTEXBUFFER_H

#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace pystorm {
namespace bddriver {
 
template<class T>
class MutexBuffer {

  public:
    MutexBuffer(unsigned int capacity);
    ~MutexBuffer();

    // simple vector interface, the slowest option
    bool Push(const std::vector<T> & input, unsigned int try_for_us=0);
    std::vector<T> PopVect(unsigned int max_to_pop, unsigned int try_for_us=0, unsigned int multiple=1);

    // fast(er) array interface
    bool Push(const T * input, unsigned int input_len, unsigned int try_for_us=0);
    unsigned int Pop(T * copy_to, unsigned int max_to_pop, unsigned int try_for_us=0, unsigned int multiple=1);

    // Two-part calls
    // These are more complicated, but using them can avoid unecessary copying by the client.
    // Using these also allows for concurrent reads and writes!
    
    // Lock/UnlockBack function like a two-part Push() call
    T * LockBack(unsigned int input_len, unsigned int try_for_us=0);
    void UnlockBack();
    
    // Lock/UnlockFront function like a two-part Pop() call
    // LockFront returns front_ pointer, num that it is safe to access
    std::pair<const T *, unsigned int> LockFront(unsigned int max_to_pop, unsigned int try_for_us=0, unsigned int multiple=1);
    void UnlockFront();

  private:
    T * vals_;
    unsigned int capacity_;
    unsigned int front_;
    unsigned int back_;
    std::atomic<unsigned int> count_; // guarantees that concurrent access will serialize in a reasonable fashion (no crazy values will get read)

    // a few notes:
    // on init, front_ == back_ == 0
    // when full, back == front_ - 1
    // "back_ is next place to push to"
    // "front_ is next place to pop from"

    unsigned int num_to_read_; // used in LockFront/UnlockFront
    unsigned int num_to_write_; // used in LockBack/UnlockBack

    std::vector<T> scratchpad_; // used in LockBack/UnlockBack in case LockBack() is called when back_ is close to end of memory

    std::condition_variable just_popped_;
    std::condition_variable just_pushed_;

    // these locks are only for preventing multiple concurrent two-part calls of the same type
    std::mutex front_lock_;
    std::mutex back_lock_;
    std::unique_lock<std::mutex> * front_ulock_;
    std::unique_lock<std::mutex> * back_ulock_;

    bool HasAtLeast(unsigned int num);
    bool HasRoomFor(unsigned int size);
    void PushData(const T * input, unsigned int input_len);
    unsigned int PopData(T * copy_to, unsigned int max_to_pop, unsigned int multiple);

    bool WaitForHasAtLeast(std::unique_lock<std::mutex> * lock, unsigned int try_for_us, unsigned int multiple);
    bool WaitForHasRoomFor(std::unique_lock<std::mutex> * lock, unsigned int input_len, unsigned int try_for_us);

};

} // bddriver
} // pystorm

// we have defined a template class. We must include the implementation
#include "MutexBuffer.cpp"

#endif
