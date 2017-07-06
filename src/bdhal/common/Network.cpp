#include <cstdlib>

#include <common/Network.h>

namespace pystorm {
namespace bdhal {
Network::Network(std::string name) : m_name(name) {
}

Network::~Network() {
}

std::string Network::GetName() {
    return m_name;
}

Pool* Network::CreatePool(std::string label, 
        uint32_t n_neurons, 
        uint32_t n_dims,
        uint32_t width,
        uint32_t height) {
    Pool* newPool = new Pool(label, n_neurons, n_dims, width, height);
    m_pools.push_back(newPool);
    return newPool;
}

StateSpace* Network::CreateStateSpace(std::string name, uint32_t n_dims) {
    StateSpace* newStatespace = new StateSpace(name,n_dims);
    m_statespaces.push_back(newStatespace);
    return newStatespace;
}

Connection* Network::CreateConnection( std::string name, 
    ConnectableObject* src, ConnectableObject* dest) {

    uint32_t* m = (uint32_t*) 
        std::calloc((src->GetNumDimensions()*dest->GetNumDimensions()), 
        sizeof(uint32_t));

    Transform<uint32_t>* transformMatrix = 
        new pystorm::bdhal::Transform<uint32_t>(m, src->GetNumDimensions(), 
            dest->GetNumDimensions());
    
    Connection* newConnection = new Connection(name, src, 
        dest, transformMatrix);
    m_weightedConnections.push_back(newConnection);
    return newConnection;
}

Connection* Network::CreateConnection(std::string name, 
    ConnectableObject* src, ConnectableObject* dest, 
    Transform<uint32_t>* transformMatrix) {
    Connection* newConnection = new Connection(name, 
        src, dest, transformMatrix);
    m_weightedConnections.push_back(newConnection);
    return newConnection;
}

} // bdhal namespace
} // pystorm namespace
