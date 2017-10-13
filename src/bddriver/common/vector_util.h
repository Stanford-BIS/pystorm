#ifndef VECTOR_UTIL_H
#define VECTOR_UTIL_H

#include <cassert>
#include <vector>
#include <queue>
#include <memory>
#include <utility>

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

/// VectorQueue virtually presents a monolithic queue of the elements of the vectors
/// that are actually enqueued
/// returns these single elements by value, so there's a potential performance bottleneck
template <class T>
class VectorQueue {
 private:
  std::queue<std::unique_ptr<std::vector<T>>> queue_;

  // XXX should probably use iterators
  unsigned int curr_idx_front_; // we can virtually pop elements from the back() vector

 public:
  // we contain unique_ptrs, can't copy
  explicit VectorQueue() : curr_idx_front_(0) {};
  VectorQueue(const VectorQueue&) = delete;
  VectorQueue& operator=(const VectorQueue&) = delete;

  inline void PushVect(std::unique_ptr<std::vector<T>> to_push) {
    //cout << "pushing" << endl;
    queue_.push(std::move(to_push));
    //cout << "size " << queue_.size() << endl;
  }

  inline bool HasN(unsigned int N) {
    return (queue_.size() > 0) && (queue_.front()->size() > N);
  }

  inline T PopEl() {
    //cout << "popping " << curr_idx_front_ << endl;
    T to_return = queue_.front()->at(curr_idx_front_++);
    if (curr_idx_front_ >= queue_.front()->size()) {
      //cout << "done with vect" << endl;
      // done with a vector
      queue_.pop();
      curr_idx_front_ = 0;
      //cout << "size " << queue_.size() << endl;
    }
    return to_return;
  }
};

template <class T>
class VectorDeserializer {
 private:
  const unsigned int D;
  VectorQueue<T> base_;
 public:
  // we contain unique_ptrs, can't copy
  explicit VectorDeserializer(unsigned int D) : D(D) {};
  VectorDeserializer(const VectorDeserializer&) = delete;
  VectorDeserializer& operator=(const VectorDeserializer&) = delete;

  inline void NewInput(std::unique_ptr<std::vector<T>> input) {
    base_.PushVect(std::move(input));
  }

  // XXX this does copies right now
  void GetOneOutput(std::vector<T> * write_into) {
    write_into->clear();
    if (base_.HasN(D)) {
      for (unsigned int i = 0; i < D; i++) {
        write_into->push_back(base_.PopEl());
      }
    }
  }
};

} // bddriver
} // pystorm

#endif
