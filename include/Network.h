#ifndef NETWORK_H
#define NETWORK_H

#include <iostream>
#include <ConnectableObject.h>
#include <Pool.h>
#include <StateSpace.h>
#include <WeightedConnection.h>
#include <stdint.h>

namespace PyStorm
{
namespace NetModels{

class Network
{
public:
    Network(std::string name)
    {
        m_name = name;
    }

    std::string getName()
    {
        return m_name;
    }

    PyStorm::NetModels::Pool* createPool(std::string name, uint32_t n_neurons)
    {
        return new PyStorm::NetModels::Pool(name, n_neurons);
    }

    PyStorm::NetModels::StateSpace* createStateSpace(std::string name, 
        uint32_t n_dims)
    {
        return new PyStorm::NetModels::StateSpace(name,n_dims);
    }

    PyStorm::NetModels::WeightedConnection* createWeightedConnection(
        std::string name, ConnectableObject* in_object, 
        ConnectableObject* out_object)
    {
        return new PyStorm::NetModels::WeightedConnection(name, in_object, 
            out_object);
    }
protected:
    std::string m_name;

};

} // namespace NetModels
} // namespace PyStorm

#endif // ifndef NETWORK_H
