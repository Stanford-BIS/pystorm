#ifndef POOL_H
#define POOL_H

#include <iostream>

#include <stdint.h>

namespace PyStorm
{
namespace NetModels{
class Pool : public ConnectableObject
{
public:
    Pool(std::string name, uint32_t n_neurons) : m_name(name), 
        m_num_neurons(n_neurons)
    {
        assert(m_num_neurons > 0);
        assert(!m_name.empty());
    }

    std::string getName()
    {
        return m_name;
    }
protected:
    std::string m_name;
    uint32_t m_num_neurons;
};

} // namespace NetModels
} // namespace PyStorm

#endif // ifndef POOL_H
