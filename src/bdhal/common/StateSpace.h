#ifndef STATESPACE_H
#define STATESPACE_H

#include <iostream>
#include <stdint.h>

namespace pystorm {
namespace bdhal {
///
/// A set of numerical values (dimensions) which are weighted projections from 
/// another Pool or StateSpace. A StateSpace can be connected to another
/// Pool or StateSpace by using the WeightedConnection class.
///
class StateSpace : ConnectableObject {
public:
    ///
    /// Default constructor
    /// 
    /// \param name Name assigned to the StateSpace
    /// \param n_dims Number of dimensions the StateSpace represents
    ///
    StateSpace(std::string name, uint32_t n_dims) : 
        m_name(name),
        m_dims(n_dims) {
        assert(m_dims > 0);
        assert(!m_name.empty());
    }

    ~StateSpace() {
    }

    StateSpace(const StateSpace&) = delete;
    StateSpace(StateSpace&&) = delete;
    StateSpace& operator=(const StateSpace&) = delete;
    StateSpace& operator=(StateSpace&&) = delete;

    ///
    /// Returns name assigned to StateSpace
    ///
    /// \return Name assigned to StateSpace
    ///
    std::string getName() {
        return m_name;
    }

    ///
    /// Returns name assigned to StateSpace
    ///
    /// \return Number of dimensions assigned to StateSpace
    ///
    uint32_t getNumDims() {
        return m_dims;
    }
private:
    /// StateSpace name
    std::string m_name;

    /// Number of dimensions assigned to StateSpace
    uint32_t    m_dims;
};

} // namespace bdhal
} // namespace pystorm

#endif // ifndef STATESPACE_H
