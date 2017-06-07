#include <common/Network.h>

namespace pystorm {
namespace bdhal {
Network::Network(std::string name) : m_name(name) {
}

Network::~Network() {
}

std::string Network::getName() {
    return m_name;
}

Pool* Network::createPool(std::string name, uint32_t n_neurons) {
    Pool* newPool = new Pool(name, n_neurons);
    m_pools.push_back(newPool);
    return newPool;
}

StateSpace* Network::createStateSpace(std::string name, uint32_t n_dims) {
    StateSpace* newStatespace = new StateSpace(name,n_dims);
    m_statespaces.push_back(newStatespace);
    return newStatespace;
}

WeightedConnection* Network::createWeightedConnection( std::string name, 
    ConnectableObject* src, ConnectableObject* dest) {
    
    WeightedConnection* newConnection = new WeightedConnection(name, src, 
        dest, nullptr);
    m_weightedConnections.push_back(newConnection);
    return newConnection;
}

WeightedConnection* Network::createWeightedConnection(std::string name, 
    ConnectableObject* src, ConnectableObject* dest, 
    Transform<uint32_t>* transformMatrix) {
    WeightedConnection* newConnection = new WeightedConnection(name, 
        src, dest, transformMatrix);
    m_weightedConnections.push_back(newConnection);
    return newConnection;
}

} // bdhal namespace
} // pystorm namespace
