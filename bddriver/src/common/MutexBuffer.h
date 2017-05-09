#ifndef MUTEXBUFFER_H
#define MUTEXBUFFER_H

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

    // fast array interface
    bool Push(const T * input, unsigned int input_len, unsigned int try_for_us=0);
    unsigned int Pop(T * copy_to, unsigned int max_to_pop, unsigned int try_for_us=0, unsigned int multiple=1);

    // two part read-then-pop call. Saves some copying
    // returns head pointer, num that may be read
    std::pair<const T *, unsigned int> Read(unsigned int max_to_pop, unsigned int try_for_us=0, unsigned int multiple=1);
    void PopAfterRead();

    // vector interface, extra allocation
    bool Push(const std::vector<T> & input, unsigned int try_for_us=0);
    std::vector<T> PopVect(unsigned int max_to_pop, unsigned int try_for_us=0, unsigned int multiple=1);

  private:
    T * vals_;
    unsigned int capacity_;
    unsigned int front_;
    unsigned int back_;
    unsigned int count_;

    unsigned int num_to_read_; // used in Read/PopAfterRead

    // a few notes:
    // on init, front_ == back_ == 0
    // when full, back == front_ - 1
    // "back_ is next place to push to"
    // "front_ is next place to pop from"

    // XXX originally I designed this with two locks to allow concurrent push/pop
    // (which should be possible--a completely lockless circular buffer is supposedly even possible!)
    // the way I implemented it originally (probably) had a race conditions when
    // the front/back catches the back/front. I reverted to a single mutex because I wasn't confident.
    // if performance is an issue, we can explore different locking schemes again
    std::mutex mutex_;
    std::condition_variable just_popped_;
    std::condition_variable just_pushed_;

    std::mutex read_in_progress_;

    bool HasAtLeast(unsigned int num);
    bool HasRoomFor(unsigned int size);
    void PushData(const T * input, unsigned int input_len);
    unsigned int PopData(T * copy_to, unsigned int max_to_pop, unsigned int multiple);

};

} // bddriver
} // pystorm

// we have defined a template class. We must include the implementation
#include "MutexBuffer.cpp"

#endif
