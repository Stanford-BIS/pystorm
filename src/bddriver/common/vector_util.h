#ifndef VECTOR_UTIL_H
#define VECTOR_UTIL_H

#include <cassert>
#include <vector>
#include <utility>

namespace pystorm {
namespace bddriver {

/// JoinedVector is useful when you have two vectors (like a remainder_ and new inputs)
/// that you want to treat like one concatenated vector.
/// This is better than inserting the second vector at the end of the first
/// Supports at(), size()
template <class T>
class JoinedVector {
 private:
  // we don't own any pointers, JoinedVector
  // breaks if either lo_ or hi_ goes away
  const std::vector<T> * lo_;
  const std::vector<T> * hi_;

 public:
  JoinedVector(const std::vector<T> * lo, const std::vector<T> * hi)
    : lo_(lo), hi_(hi) {};

  inline size_t size() const { return lo_->size() + hi_->size(); }

  const T& at(size_t idx) const { 
    if(idx < lo_->size()) {
      return lo_->at(idx);
    } else {
      return hi_->at(idx + lo_->size());
    }
  }
};

// VectorDeserializer helps work with vectors for 
// consumers that need D*x inputs
// it's a bit brittle: you have to call NewInput(),
// followed by calls to GetOneOutput() until false is returned
template <class T>
class VectorDeserializer {
 private:
  const unsigned int D;

  std::vector<T> remainder_;
  JoinedVector<T> joined_vect_;
  unsigned int read_word_idx_ = 0;

  // intelligible brittleness
  bool call_NewInput_next_ = true;

 public:
  VectorDeserializer(unsigned int D) : D(D), joined_vect_(nullptr, nullptr) {};

  void NewInput(const std::vector<T> * input) {
    assert(call_NewInput_next_);
    joined_vect_ = JoinedVector<T>(&remainder_, input);
    read_word_idx_ = 0;
    call_NewInput_next_ = false;
  }

  void GetOneOutput(std::vector<T> * write_into) {
    assert(!call_NewInput_next_);

    write_into->clear();

    // there's at least D inputs left
    if ((read_word_idx_ + 1) * D <= joined_vect_.size()) {

      for (unsigned int i = 0 ; i < D; i++) {

        unsigned int idx = D * read_word_idx_ + i;
        write_into->push_back(joined_vect_.at(idx));
      }

    } else {
      // we're at the end, set up remainder for next time

      remainder_.clear();
      unsigned int num_el_left = joined_vect_.size() - read_word_idx_ * D;
      for (unsigned int i = 0; i < num_el_left; i++) {
        unsigned int idx = D * read_word_idx_ + i;
        remainder_.push_back(joined_vect_.at(idx));
      }

      call_NewInput_next_ = true;
    }

    read_word_idx_++;
  }

};

} // bddriver
} // pystorm

#endif
