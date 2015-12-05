#ifndef STATESPACE_H
#define STATESPACE_H

#include <iostream>
#include <stdint.h>

namespace PyStorm
{
namespace NetModels{
class StateSpace : ConnectableObject
{
public:
    StateSpace(std::string name, uint32_t n_dims) : m_name(name),
        m_dims(n_dims)
    {
        assert(m_dims > 0);
        assert(!m_name.empty());
    }

    std::string getName()
    {
        return m_name;
    }
protected:
    std::string m_name;
    uint32_t    m_dims;
};

} // namespace NetModels
} // namespace PyStorm

#endif // ifndef STATESPACE_H
