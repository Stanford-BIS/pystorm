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
    void Push(const T * input, unsigned int input_len);
    unsigned int Pop(T * copy_to, unsigned int max_to_pop);

    // vector interface, extra allocation
    void Push(const std::vector<T> & input);
    std::vector<T> PopVect(unsigned int max_to_pop);

  private:
    T * vals_;
    unsigned int capacity_;
    unsigned int front_;
    unsigned int back_;
    unsigned int count_;

    // XXX originally I designed this with two locks to allow concurrent push/pop
    // (which should be possible--a completely lockless circular buffer is supposedly even possible!)
    // the way I implemented it originally (probably) had a race conditions when
    // the front/back catches the back/front. I reverted to a single mutex because I wasn't confident.
    // if performance is an issue, we can explore having multiple locks again.
    std::mutex mutex_;
    std::condition_variable just_popped_;
    std::condition_variable just_pushed_;

    bool IsEmpty(); 
    bool HasRoomFor(unsigned int size);

};

} // bddriver
} // pystorm

// we have defined a template class. We must include the implementation
#include "bddriver/common/MutexBuffer.cpp"

#endif
