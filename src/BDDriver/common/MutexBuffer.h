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

    // XXX dual mutexes didn't really help because we still need both locks
    // to check the empty/almost full state
    std::mutex push_mutex_;
    std::mutex pop_mutex_;
    std::condition_variable just_popped_;
    std::condition_variable just_pushed_;

    bool IsEmpty(); 
    bool HasRoomFor(unsigned int size);

};

} // bddriver
} // pystorm

// we have defined a template class. We must include the implementation
#include "BDDriver/common/MutexBuffer.cpp"

#endif
