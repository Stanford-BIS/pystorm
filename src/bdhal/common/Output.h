#ifndef OUTPUT_H
#define OUTPUT_H

#include <iostream>
#include <stdint.h>

#include <common/Connectable.h>

namespace pystorm {
namespace bdhal {
///
/// A set of numerical values (dimensions) which are projections from 
/// a Pool or Bucket. An Output can be connected to a Pool using the
/// Connection class.
///
class Output : public ConnectableOutput {
public:
    ///
    /// Default constructor
    /// 
    /// \param name Name assigned to the Output
    /// \param n_dims Number of dimensions the Output represents
    ///
    Output(std::string name, uint32_t n_dims);

    ~Output();

    Output(const Output&) = delete;
    Output(Output&&) = delete;
    Output& operator=(const Output&) = delete;
    Output& operator=(Output&&) = delete;

    ///
    /// Returns name assigned to Output
    ///
    std::string GetLabel() {
        return m_label;
    }

    ///
    /// Returns name assigned to Output
    ///
    /// \return Number of dimensions assigned to Output
    ///
    uint32_t GetNumDimensions() {
        return m_dims;
    }
private:
    /// Output name
    std::string m_label;

    /// Number of dimensions assigned to Output
    uint32_t    m_dims;
};

} // namespace bdhal
} // namespace pystorm

#endif // ifndef OUTPUT_H
