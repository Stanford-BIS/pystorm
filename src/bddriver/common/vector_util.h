#ifndef VECTOR_UTIL_H
#define VECTOR_UTIL_H

#include <cassert>
#include <vector>
#include <utility>

namespace pystorm {
namespace bddriver {

/// VectorQueue virtually presents a monolithic queue of the elements of the vectors
/// that are actually enqueued
/// returns these single elements by value, so there's a potential performance bottleneck
template <class T>
class VectorQueue {
 private:
  std::queue<std::unique_ptr::<std::vector<T>>> queue_;

  // XXX should probably use iterators
  unsigned int curr_idx_front_; // we can virtually pop elements from the back() vector

 public:
  VectorQueue() : curr_idx_back_(0) {};

  inline void PushVect(std::unique_ptr<std::vector<T>> to_push) {
    queue_.push_back(std::move(to_push));
  }

  inline bool HasN(unsigned int N) {
    return (queue_.size() > 0) && (queue_.front()->size() > N);
  }

  inline T PopEl() {
    T to_return = queue_.front()->at(curr_idx_front++);
    if (curr_idx_front >= queue_.front()->size()) {
      // done with a vector
      queue_.pop_front();
      curr_idx_front = 0;
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
  VectorDeserializer(unsigned int D) : D(D) {};

  inline void NewInput(std::unique_ptr<std::vector<T>> input) {
    base_.PushVect(std::move(input));
  }

  // XXX this does copies right now
  void GetOneOutput(std::vector<T> * write_into) {
    if (base_.HasN(D)) {
      for (unsigned int i = 0; i < D; i++) {
        write_into->push_back(base_.PopEl());
      }
    }
  }
}

} // bddriver
} // pystorm

#endif
