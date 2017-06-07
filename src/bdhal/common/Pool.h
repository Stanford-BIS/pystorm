#ifndef POOL_H
#define POOL_H

#include <iostream>

#include <stdint.h>
#include <assert.h>

#include <common/ConnectableObject.h>

namespace pystorm {
namespace bdhal {
///
/// A Pool of neurons
///
class Pool : public ConnectableObject {
public:
    ///
    /// Default constructor 
    ///
    /// \param name Name assigned to Pool
    /// \param n_neurons Number of neurons assigned to Pool
    ///
    Pool(std::string name, uint32_t n_neurons) : 
        m_name(name), 
        m_num_neurons(n_neurons) {
        assert(m_num_neurons > 0);
        assert(!m_name.empty());
    }

    ~Pool() {
    }

    Pool(const Pool&) = delete;
    Pool(Pool&&) = delete;
    Pool& operator=(const Pool&) = delete;
    Pool& operator=(Pool&&) = delete;

    /// \brief Returns name assigned to Pool 
    ///
    /// \return Name assigned to Pool
    ///
    std::string getName() {
        return m_name;
    }

    /// \brief Returns number of neurons assigned to Pool 
    ///
    /// \return Numberof neurons assigned to Pool
    ///
    uint32_t getNumNeurons() {
        return m_num_neurons;
    }

private:
    /// Pool name
    std::string m_name;

    /// Number of neurons 
    uint32_t m_num_neurons;
};

} // namespace bdhal
} // namespace pystorm

#endif // ifndef POOL_H
