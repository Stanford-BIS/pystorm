#ifndef NETWORK_H
#define NETWORK_H

#include <iostream>
#include <common/ConnectableObject.h>
#include <common/Pool.h>
#include <common/StateSpace.h>
#include <common/WeightedConnection.h>

#include <stdint.h>
#include <vector>
#include <map>

namespace pystorm {
namespace bdhal {
///
/// A Network composed of WeightedConnections and ConnectableObjects
///
/// A network that represents a Nengo network. The network will be passed
/// to Neuromorph to be placed and routed and will be passed to a MappedNetwork
/// object which will track this object as well as other objects that
/// allow pystorm to program Brainstorm.
/// 
/// NOTE: As much as possible we want objects to be immutable. Create an 
/// object and don't change it. One exception will be the transform matrix
/// associated with WeightedConnections. We don't want to create a new
/// Network if a WeightedConnections transform matrix changes so we make an
/// exception there.
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
    std::string getName();

    ///
    /// Create a Pool object for this network
    ///
    /// \param name Name assigned to the Pool
    /// \param n_neurons Number of neurons assigned to the Pool
    ///
    /// \return a new Pool object
    ///
    Pool* createPool(std::string name, uint32_t n_neurons);

    ///
    /// Create a Statespace object for this network
    ///
    /// \param name Names assigned to the Statespace
    /// \param n_dims Number of dimensions the Statespace represents
    ///
    /// \return a new Statespace object
    ///
    StateSpace* createStateSpace(std::string name, uint32_t n_dims);

    ///
    /// Create a WeightedConnection object for this network
    ///
    /// \param name Name assigned to the connection
    /// \param src Connections source object
    /// \param dest Connections destination object
    ///
    /// \return a new WeightedConnection object
    ///
    WeightedConnection* createWeightedConnection( std::string name, 
        ConnectableObject* src, ConnectableObject* dest);

    ///
    /// Create a WeightedConnection object for this network
    ///
    /// \param name Name assigned to the connection
    /// \param src Connections source object
    /// \param dest Connections destination object
    /// \param transformMatrix Transform matrix assigned to the connection
    ///
    /// \return a new WeightedConnection object
    ///
    WeightedConnection* createWeightedConnection(std::string name, 
        ConnectableObject* src, ConnectableObject* dest, 
        Transform<uint32_t>* transformMatrix);

private:
    /// Name assigned to Network
    std::string m_name;

    /// Pools created for Network
    std::vector<Pool*> m_pools;

    /// Statespaces created for Network
    std::vector<StateSpace*> m_statespaces;

    /// WeightedConnections created for Network
    std::vector<WeightedConnection*> m_weightedConnections;

    //Chances are the Network will need more structures for bookkeeping
    // including a map from WeightedConnection to a tuple (pair) consisting
    // of src and dest objects (both ConnectableObject instances)
    // Let's consider how this is useful with some use cases
    std::map<WeightedConnection*, 
        std::pair<ConnectableObject*, ConnectableObject*> > m_connectionMap;

    /// A map from a ConnectableObject (i.e. Pool or StateSpace) to its input
    /// connections.
    std::map<ConnectableObject*, std::vector<WeightedConnection*> > 
        m_inConnections;

    /// A map from a ConnectableObject (i.e. Pool or StateSpace) to its output
    /// connections.
    std::map<ConnectableObject*, std::vector<WeightedConnection*> > 
        m_outConnections;
};

} // namespace bdhal
} // namespace pystorm

#endif // ifndef NETWORK_H
