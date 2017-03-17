#include <iostream>

#include <Network.h>

namespace pystorm
{
/// A MappedNetwork owns references to a pystorm network as well as other
/// objects created during Neuromorphs placing and routing. These objects
/// combined are used by pystorm to program Brainstorm.
class MappedNetwork
{
public:
    /// \brief Default constructor
    ///
    MappedNetwork(pystorm::NetModels::Network* network) :
        m_network(network)
    {
    }

    /// \brief Network that is to be mapped.
    /// 
    /// \return Network object that is to be mapped.
    const pystorm::NetModels::Network* getNetwork()
    {
        return m_network;
    }

private:
    /// A network representing a Nengo network. 
    pystorm::NetModels::Network* m_network;

    /// what else is represented and owned by this object?
};

} // namespace pystorm
