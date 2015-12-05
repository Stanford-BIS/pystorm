#ifndef WEIGHTEDCONNECTION_H
#define WEIGHTEDCONNECTION_H

#include <iostream>
#include <stdint.h>
#include <ConnectableObject.h>

namespace PyStorm
{
namespace NetModels{
class WeightedConnection
{
public:
    WeightedConnection(std::string name, ConnectableObject* in_object, 
        ConnectableObject* out_object) :
        m_name(name),
        m_in_object(in_object),
        m_out_object(out_object)
    {
        assert(m_in_object != nullptr);
        assert(m_out_object != nullptr);
        assert(!m_name.empty());
    }

    std::string getName()
    {
        return m_name;
    }
protected:
    std::string m_name;
    ConnectableObject* m_in_object;
    ConnectableObject* m_out_object;
};

} // namespace NetModels
} // namespace PyStorm

#endif // ifndef WEIGHTEDCONNECTION_H
