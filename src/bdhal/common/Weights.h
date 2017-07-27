#ifndef WEIGHTS_H
#define WEIGHTS_H

#include <iostream>
#include <stdint.h>
#include <assert.h>

namespace pystorm {
namespace bdhal {
///
/// A matrix representing the weights between two connected objects.
///
template<typename T>
class Weights {
public:
    ///
    /// Default constructor
    /// 
    /// \param matrix A matrix representing the connection weights.
    /// \param num_rows The number of rows in the weight matrix.
    /// \param num_columns The number of columns in the weight matrix.
    ///
    Weights(T* matrix, uint32_t num_rows, uint32_t num_columns) :
        m_matrix(matrix),
        m_num_rows(num_rows),
        m_num_columns(num_columns) {
        if (m_matrix == nullptr) {
            throw std::logic_error("Cannot construct Weights with null matrix");
        }
        if ((m_num_rows <= 0) || (m_num_columns <= 0)) {
            throw std::out_of_range("Number of rows and columns must be greater than zero.");
        }
    }

    ~Weights() {
        delete m_matrix;
        m_matrix = nullptr;
    }

    Weights(const Weights&) = delete;
    Weights(Weights&&) = delete;
    Weights& operator=(const Weights&) = delete;
    Weights& operator=(Weights&&) = delete;

    ///
    /// Returns the represented matrix
    ///
    /// \return Const pointer to matrix
    ///
    const T* GetMatrix() {
        return m_matrix;
    }

    ///
    /// Returns the number of rows in the matrix
    ///
    /// \return Number of rows in the matrix
    /// 
    uint32_t GetNumRows() {
        return m_num_rows;
    }

    ///
    /// Returns the number of columns in the matrix
    ///
    /// \return Number of columns in the matrix
    ///
    uint32_t GetNumColumns() {
        return m_num_columns;
    }

    ///
    /// Returns the element at (row,column) of the matrix
    ///        The matrix is zero-based to the last element is at
    ///        position (num_rows-1, num_cols - 1)
    ///
    /// \return The element at (row,column).
    T GetElement(uint32_t row, uint32_t column) {
        if ((row < 0) || (row >= m_num_rows)) {
            throw std::out_of_range("row parameter out of bounds");
        }

        if ((column < 0) || (column >= m_num_columns)) {
            throw std::out_of_range("row parameter out of bounds");
        }
    
        return m_matrix[(row*m_num_columns) + column];
    }

    ///
    /// Returns the element at (row,column) of the matrix
    ///        The matrix is zero-based to the last element is at
    ///        position (num_rows-1, num_cols - 1)
    ///
    /// \return The element at (row,column).
    void SetElement(uint32_t row, uint32_t column, T new_value) {
        if ((row < 0) || (row >= m_num_rows)) {
            throw std::out_of_range("row parameter out of bounds");
        }

        if ((column < 0) || (column >= m_num_columns)) {
            throw std::out_of_range("row parameter out of bounds");
        }

        m_matrix[(row*m_num_columns) + column] = new_value;
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

#endif // ifndef WEIGHTS_H
