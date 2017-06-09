#include <iostream>

#include <common/Network.h>

namespace pystorm {
namespace bdhal {
///
/// Network object and it's mapped resources
///
/// A MappedNetwork owns references to a pystorm network as well as other
/// objects created during Neuromorphs placing and routing. These objects
/// combined are used by pystorm to program Brainstorm.
///
class MappedNetwork {
public:
    MappedNetwork(pystorm::bdhal::Network* network) :
        m_network(network) {
    }

    ~MappedNetwork() {
    }

    MappedNetwork(const MappedNetwork &) = delete;
    MappedNetwork(MappedNetwork &&) = delete;
    MappedNetwork& operator=(const MappedNetwork &) = delete;
    MappedNetwork& operator=(MappedNetwork&&) = delete;

    ///
    /// Network that is to be mapped.
    /// 
    /// \return Network object that is to be mapped.
    ///
    const pystorm::bdhal::Network* getNetwork() {
        return m_network;
    }

private:
    /// A network representing a Nengo network. 
    pystorm::bdhal::Network* m_network;

};

} // namespace bdhal
} // namespace pystorm
