#ifndef STATESPACE_H
#define STATESPACE_H

#include <iostream>
#include <stdint.h>

namespace pystorm
{
namespace NetModels
{
/// A set of numerical values (dimensions) which are weighted projections from 
/// another Pool or StateSpace. A StateSpace can be connected to another
/// Pool or StateSpace by using the WeightedConnection class.
///
class StateSpace : ConnectableObject
{
public:
    /// \brief Default constructor
    /// 
    /// \param name Name assigned to the StateSpace
    /// \param n_dims Number of dimensions the StateSpace represents
    StateSpace(std::string name, uint32_t n_dims) : 
        m_name(name),
        m_dims(n_dims)
    {
        assert(m_dims > 0);
        assert(!m_name.empty());
    }

    /// \brief Destructor
    ///
    ~StateSpace()
    {
    }

    /// \brief Returns name assigned to StateSpace
    ///
    /// \return Name assigned to StateSpace
    ///
    std::string getName()
    {
        return m_name;
    }

    /// \brief Returns name assigned to StateSpace
    ///
    /// \return Number of dimensions assigned to StateSpace
    ///
    uint32_t getNumDims()
    {
        return m_dims;
    }
private:
    /// StateSpace name
    std::string m_name;

    /// Number of dimensions assigned to StateSpace
    uint32_t    m_dims;
};

} // namespace NetModels
} // namespace pystorm

#endif // ifndef STATESPACE_H
