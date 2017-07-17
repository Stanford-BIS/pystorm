#include <cstdlib>

#include <common/Network.h>

namespace pystorm {
namespace bdhal {
Network::Network(std::string name) : m_name(name) {
    if (name.size() == 0) {
        throw std::logic_error("Network name cannot be empty string");
    }
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

Pool* Network::CreatePool(std::string label, 
        uint32_t n_neurons, 
        uint32_t n_dims) {
    Pool* newPool = new Pool(label, n_neurons, n_dims);
    m_pools.push_back(newPool);
    return newPool;
}

Bucket* Network::CreateBucket(std::string name, uint32_t n_dims) {
    Bucket* newBucket = new Bucket(name,n_dims);
    m_buckets.push_back(newBucket);
    return newBucket;
}

Connection* Network::CreateConnection( std::string name, 
    ConnectableInput* src, ConnectableOutput* dest) {

    uint32_t* m = (uint32_t*) 
        std::calloc((src->GetNumDimensions()*dest->GetNumDimensions()), 
        sizeof(uint32_t));

    Weights<uint32_t>* transformMatrix = 
        new pystorm::bdhal::Weights<uint32_t>(m, src->GetNumDimensions(), 
            dest->GetNumDimensions());
    
    Connection* newConnection = new Connection(name, src, 
        dest, transformMatrix);
    m_connections.push_back(newConnection);
    return newConnection;
}

Connection* Network::CreateConnection(std::string name, 
    ConnectableInput* src, ConnectableOutput* dest, 
    Weights<uint32_t>* transformMatrix) {
    Connection* newConnection = new Connection(name, 
        src, dest, transformMatrix);
    m_connections.push_back(newConnection);
    return newConnection;
}

} // bdhal namespace
} // pystorm namespace
