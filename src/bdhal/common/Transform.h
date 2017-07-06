#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <iostream>
#include <stdint.h>

namespace pystorm {
namespace bdhal {
///
/// A matrix representing the transform used in a Connection.
///
template<typename T>
class Transform {
public:
    ///
    /// Default constructor
    /// 
    /// \param matrix The transform matrix.
    /// \param num_rows The number of rows in the transform matrix.
    /// \param num_columns The number of columns in the transform matrix.
    ///
    Transform(T* matrix, uint32_t num_rows, uint32_t num_columns) :
        m_matrix(matrix),
        m_num_rows(num_rows),
        m_num_columns(num_columns) {
        assert(m_matrix != nullptr);
        // need to check that the size of m_matrix is sizeof(uint32_t) * m_num_rows * m_num_cols
        assert(m_num_rows > 0);
        assert(m_num_columns > 0);
    }

    ~Transform() {
    }

    Transform(const Transform&) = delete;
    Transform(Transform&&) = delete;
    Transform& operator=(const Transform&) = delete;
    Transform& operator=(Transform&&) = delete;

    /// \brief Returns the represented matrix
    ///
    /// \return Const pointer to transform matrix
    ///
    const T* getMatrix() {
        return m_matrix;
    }

    /// \brief Returns the number of rows in the matrix
    ///
    /// \return Number of rows in the matrix
    /// 
    uint32_t getNumRows() {
        return m_num_rows;
    }

    /// \brief Returns the number of columns in the matrix
    ///
    /// \return Number of columns in the matrix
    ///
    uint32_t getNumColumns() {
        return m_num_columns;
    }

    /// \brief Returns the element at (row,column) of the matrix
    ///        The matrix is zero-based to the last element is at
    ///        position (num_rows-1, num_cols - 1)
    ///
    /// \return The element at (row,column).
    T getElement(uint32_t row, uint32_t column) {
        return *m_matrix((row*m_num_columns) + column);
    }

private:
    /// The matrix
    T* m_matrix;

    /// Number of rows in the matrix
    uint32_t m_num_rows;

    /// Number of columns in the matrix
    uint32_t m_num_columns;
};

} // namespace bdhal
} // namespace pystorm

#endif // ifndef TRANSFORM_H
