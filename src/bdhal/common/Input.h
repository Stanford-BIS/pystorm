#ifndef INPUT_H
#define INPUT_H

#include <iostream>
#include <stdint.h>

#include <common/Connectable.h>

namespace pystorm {
namespace bdhal {
///
/// A set of numerical values (dimensions) which are weighted projections from 
/// another Pool or Input. A Input can be connected to a Pool using the
/// Connection class.
///
class Input : public ConnectableInput {
public:
    ///
    /// Default constructor
    /// 
    /// \param name Name assigned to the Input
    /// \param n_dims Number of dimensions the Input represents
    ///
    Input(std::string name, uint32_t n_dims);

    ~Input();

    Input(const Input&) = delete;
    Input(Input&&) = delete;
    Input& operator=(const Input&) = delete;
    Input& operator=(Input&&) = delete;

    ///
    /// Returns name assigned to Input
    ///
    std::string GetLabel() {
        return m_label;
    }

    ///
    /// Returns name assigned to Input
    ///
    /// \return Number of dimensions assigned to Input
    ///
    uint32_t GetNumDimensions() {
        return m_dims;
    }
private:
    /// Input name
    std::string m_label;

    /// Number of dimensions assigned to Input
    uint32_t    m_dims;
};

} // namespace bdhal
} // namespace pystorm

#endif // ifndef INPUT_H
