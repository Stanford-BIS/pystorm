#ifndef BUCKET_H
#define BUCKET_H

#include <iostream>
#include <stdint.h>

#include <common/Connectable.h>

namespace pystorm {
namespace bdhal {

class Bucket : public ConnectableInput, public ConnectableOutput {
public:
    ///
    /// Default constructor
    /// 
    /// \param name Name assigned to the Bucket
    /// \param n_dims Number of dimensions the Bucket represents
    ///
    Bucket(std::string label, uint32_t n_dims);

    virtual ~Bucket();

    Bucket(const Bucket&) = delete;
    Bucket(Bucket&&) = delete;
    Bucket& operator=(const Bucket&) = delete;
    Bucket& operator=(Bucket&&) = delete;

    ///
    /// Returns name assigned to Bucket
    ///
    std::string GetLabel() {
        return m_label;
    }

    ///
    /// Returns name assigned to Bucket
    ///
    /// \return Number of dimensions assigned to Bucket
    ///
    uint32_t GetNumDimensions() {
        return m_dims;
    }

    ///
    /// Returns size when used with a Connection
    ///
    /// \return Size used with a Connection
    ///
    uint32_t GetConnectionSize() {
        return m_dims;
    }
private:
    /// Bucket name
    std::string m_label;

    /// Number of dimensions assigned to Bucket
    uint32_t    m_dims;
};

} // namespace bdhal
} // namespace pystorm

#endif // ifndef BUCKET_H
