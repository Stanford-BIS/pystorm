#ifndef NETWORK_H
#define NETWORK_H

#include <iostream>
#include <map>
#include <memory>
#include <stdint.h>
#include <vector>

#include <common/Connectable.h>
#include <common/Input.h>
#include <common/Output.h>
#include <common/Pool.h>
#include <common/Bucket.h>
#include <common/Connection.h>
#include <common/Weights.h>

namespace pystorm {
namespace bdhal {

typedef std::vector<Pool*> VecOfPools;
typedef std::vector<Bucket*> VecOfBuckets;
typedef std::vector<Input*> VecOfInputs;
typedef std::vector<Output*> VecOfOutputs;
typedef std::vector<Connection*> VecOfConnections;

///
/// A Network composed of Connection and Connectable objects
///
/// A network that represents a Nengo network. The network will be passed
/// to Neuromorph to be placed and routed.
/// 
/// NOTE: Connectable objects (Buckets and Pools) are immutable but Weights are not.
///
class Network {
public:
    /// Default constructor
    ///
    /// \param name Name assigned to network
    ///
    Network(std::string name);

    ~Network();

    Network(const Network &) = delete;
    Network(Network&&) = delete;
    Network& operator=(const Network &) = delete;
    Network& operator=(Network &&) = delete;

    ///
    /// Name assigned to Network
    ///
    std::string GetName();

    ///
    /// Create a Pool object for this network
    ///
    /// \param label Label assigned to the Pool
    /// \param n_neurons Number of neurons assigned to the Pool
    /// \param n_dims Number of dimensions assigned to the Pool
    /// \param width Width of Pool
    /// \param height Height of Pool
    ///
    /// \return a new Pool object
    ///
    Pool* CreatePool(std::string label, 
        uint32_t n_neurons,
        uint32_t n_dims,
        uint32_t width,
        uint32_t height);

    ///
    /// Create a Statespace object for this network
    ///
    /// \param name Names assigned to the Statespace
    /// \param n_dims Number of dimensions the Statespace represents
    ///
    /// \return a new Statespace object
    ///
    Bucket* CreateBucket(std::string name, uint32_t n_dims);

    ///
    /// Create a Connection object for this network
    ///
    /// \param name Name assigned to the connection
    /// \param src Connections source object
    /// \param dest Connections destination object
    ///
    /// \return a new Connection object
    ///
    Connection* CreateConnection( std::string name, 
        ConnectableInput* src, ConnectableOutput* dest);

    ///
    /// Create a Connection object for this network
    ///
    /// \param name Name assigned to the connection
    /// \param src Connections source object
    /// \param dest Connections destination object
    /// \param transformMatrix Weights matrix assigned to the connection
    ///
    /// \return a new Connection object
    ///
    Connection* CreateConnection(std::string name, 
        ConnectableInput* src, ConnectableOutput* dest, 
        Weights<uint32_t>* transformMatrix);

    VecOfPools& GetPools() {
        return m_pools;
    }

    VecOfBuckets& GetBuckets() {
        return m_buckets;
    }

    VecOfConnections& GetConnections() {
        return m_connections;
    }

private:
    /// Name assigned to Network
    std::string m_name;

    /// Pools created for Network
    VecOfPools m_pools;

    /// Statespaces created for Network
    std::vector<Bucket*> m_buckets;

    /// Connection created for Network
    std::vector<Connection*> m_connections;

    //Chances are the Network will need more structures for bookkeeping
    // including a map from Connection to a tuple (pair) consisting
    // of src and dest objects (both Connectable instances)
    // Let's consider how this is useful with some use cases
    std::map<Connection*, 
        std::pair<ConnectableInput*, ConnectableOutput*> > m_connectionMap;

    /// A map from a Connectable object to its input connections.
    std::map<ConnectableInput*, std::vector<Connection*> > 
        m_inConnections;

    /// A map from a Connectable object to its output connections.
    std::map<ConnectableOutput*, std::vector<Connection*> > 
        m_outConnections;
};

} // namespace bdhal
} // namespace pystorm

#endif // ifndef NETWORK_H
