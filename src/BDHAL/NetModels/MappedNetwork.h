#include <iostream>

#include <Network.h>

namespace PyStorm
{
/// A MappedNetwork owns references to a PyStorm network as well as other
/// objects created during Neuromorphs placing and routing. These objects
/// combined are used by PyStorm to program Brainstorm.
class MappedNetwork
{
public:
    /// \brief Default constructor
    ///
    MappedNetwork(PyStorm::NetModels::Network* network) :
        m_network(network)
    {
    }

    /// \brief Network that is to be mapped.
    /// 
    /// \return Network object that is to be mapped.
    const PyStorm::NetModels::Network* getNetwork()
    {
        return m_network;
    }

private:
    /// A network representing a Nengo network. 
    PyStorm::NetModels::Network* m_network;

    /// what else is represented and owned by this object?
};

} // namespace PyStorm
