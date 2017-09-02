#ifndef MUTEXBUFFER_H
#define MUTEXBUFFER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace pystorm {
namespace bddriver {

/// Multiple-producer, multiple-consumer (MPMC) thread-safe circular buffer.
///
/// There are two kinds of public calls: one-part, like Push(); and two-part, like
/// LockBack() and UnlockBack(). The one-part calls are simpler (and come in vector
/// and non-vector versions), but are likely to be slower because they require
/// more copying.
///
/// Many functions have a <try_for_us> parameter, which controls how long to wait in the event that
/// the buffer is either full or empty (which makes it impossible to push or pop, respectively).
/// if try_for_us > 0, then a push or pop call that is made when the buffer is in a
/// full or empty state will return with a failure if, after try_for_us, the buffer is still
/// in the full or empty state.
/// If try_for_us = 0, then the call will block until success is possible.
///
/// The pop and LockFront/UnlockFront calls also have a <multiple> parameter,
/// which instructs the call to only yield an integer multiple of <multiple> elements.
/// This is useful if a consumer can only do something useful with N*<multiple> elements,
/// preventing the consumer from having to keep track of fragments between calls.
///
/// In implementation, this is closest to a single-producer, single consumer (SPSC)
/// lockless circular buffer (using atomic variables),
/// but with an additional front_lock_ and back_lock_ to arbitrate concurrent access
/// from between multiple producers or multiple consumers.
template <class T>
class MutexBuffer {
 public:
  MutexBuffer(unsigned int capacity);
  ~MutexBuffer();

  // simple vector interface, the slowest option

  /// Push() pushes the elements in <input> to the back of the buffer.
  ///
  /// Returns false if not successful in <try_for_us> us (e.g. if the buffer was full),
  /// else returns true. <try_for_us>=0 will cause the call to block until success.
  bool Push(const std::vector<T> &input, unsigned int try_for_us = 0);

  /// Pop() pushes up to <max_to_pop> elements from the front of the buffer onto <push_to>.
  ///
  /// The return value is the number of elements which were copied.
  /// May copy fewer elements than <max_to_pop> if the buffer contains only a small number of elements
  /// May copy zero elements if the buffer does not contain any contents before <try_for_us> us.
  /// Setting <multiple> > 1 will only return N*<multiple> elements, leaving any remainder in the buffer.
  unsigned int Pop(std::vector<T> *push_to, unsigned int max_to_pop, unsigned int try_for_us = 0,
                   unsigned int multiple = 1);

  /// PopVect() returns a vector containing up to <max_to_pop> elements from the front of the buffer.
  ///
  /// May return fewer elements than <max_to_pop> if the buffer contains only a small number of elements
  /// May return zero elements if the buffer does not contain any contents before <try_for_us> us.
  /// Setting <multiple> > 1 will only return N*<multiple> elements, leaving any remainder in the buffer.
  std::vector<T> PopVect(unsigned int max_to_pop, unsigned int try_for_us = 0, unsigned int multiple = 1);

  // fast(er) array interface

  /// Push() pushes <input_len> elements pointed to by <input> to the back of the buffer.
  ///
  /// Returns false if not successful in <try_for_us> us (e.g. if the buffer was full),
  /// else returns true. <try_for_us>=0 will cause the call to block until success.
  bool Push(const T *input, unsigned int input_len, unsigned int try_for_us = 0);

  /// Pop() copies up to <max_to_pop> elements from the front of the buffer to <copy_to>.
  ///
  /// The return value is the number of elements which were copyied.
  /// May copy fewer elements than <max_to_pop> if the buffer contains only a small number of elements
  /// May copy zero elements if the buffer does not contain any contents before <try_for_us> us.
  /// Setting <multiple> > 1 will only return N*<multiple> elements, leaving any remainder in the buffer.
  unsigned int Pop(T *copy_to, unsigned int max_to_pop, unsigned int try_for_us = 0, unsigned int multiple = 1);

  // Two-part calls
  // These are more complicated, but using them can avoid unecessary copying by the client.
  // Using these also allows for concurrent reads and writes!

  /// LockBack() returns a pointer that the user may write <input_len> elements to.
  ///
  /// When done writing to the memory pointed to by this pointer, the user must call
  /// UnlockBack() to finish  it into the buffer.
  /// Returns nullptr if not successful in <try_for_us> us (e.g. if the buffer was full),
  /// else returns a non-null pointer. <try_for_us>=0 will cause the call to block until success.
  /// LockBack()/UnlockBack() function like a two-part Push() call.
  T *LockBack(unsigned int input_len, unsigned int try_for_us = 0);

  /// UnlockBack() is called after the user is done writing to the memory pointed
  /// to by LockBack(), to finish pushing it into the buffer.
  void UnlockBack();

  /// LockFront() returns a pair<A,B> where A is a pointer that the user may read B elements from.
  ///
  /// B will not be larger than <max_to_pop>.
  /// When done writing to the memory pointed to by this pointer, the user must call
  /// UnlockFront() to finish it into the buffer.
  /// Returns <nullptr,0> if not successful in <try_for_us> us (e.g. if the buffer was full),
  /// else returns a non-null pointer. <try_for_us>=0 will cause the call to block until success.
  /// Setting <multiple> > 1 will force B = N*<multiple> elements, leaving any remainder in the buffer.
  /// LockBack()/UnlockBack() function like a two-part Push() call.
  std::pair<const T *, unsigned int> LockFront(unsigned int max_to_pop, unsigned int try_for_us = 0,
                                               unsigned int multiple = 1);

  /// UnlockFront() is called after the user is done reading from the memory pointed
  /// to by LockFront(), to finish popping it from the buffer.
  void UnlockFront();

 private:
  T *vals_;
  unsigned int capacity_;
  unsigned int front_;
  unsigned int back_;
  std::atomic<unsigned int> count_;  // guarantees that concurrent access will serialize in a reasonable fashion (no
                                     // crazy values will get read)

  // a few notes:
  // on init, front_ == back_ == 0
  // when full, back == front_ - 1
  // "back_ is next place to push to"
  // "front_ is next place to pop from"

  unsigned int num_to_read_;   // used in LockFront/UnlockFront
  unsigned int num_to_write_;  // used in LockBack/UnlockBack

  std::vector<T>
      scratchpad_;  // used in LockBack/UnlockBack in case LockBack() is called when back_ is close to end of memory

  std::condition_variable just_popped_;
  std::condition_variable just_pushed_;

  // these locks are only for preventing multiple concurrent two-part calls of the same type
  std::mutex front_lock_;
  std::mutex back_lock_;
  std::mutex count_lock_;
  std::unique_lock<std::mutex> *front_ulock_;
  std::unique_lock<std::mutex> *back_ulock_;

  bool HasAtLeast(unsigned int num);
  bool HasRoomFor(unsigned int size);
  void UpdateStateForPush(unsigned int num_pushed);
  void UpdateStateForPop(unsigned int num_popped);
  unsigned int DetermineNumToPop(unsigned int max_to_pop, unsigned int multiple);

  bool WaitForHasAtLeast(std::unique_lock<std::mutex> *lock, unsigned int try_for_us, unsigned int multiple);
  bool WaitForHasRoomFor(std::unique_lock<std::mutex> *lock, unsigned int input_len, unsigned int try_for_us);
};

}  // bddriver
}  // pystorm

// we have defined a template class. We must include the implementation
#include "MutexBuffer.cpp"

#endif
