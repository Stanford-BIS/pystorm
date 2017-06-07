#include <Hal.h>

namespace pystorm {
namespace bdhal {
//////////////////////////////////////////////////////////////////////////////
//
// Program functionality
//
//////////////////////////////////////////////////////////////////////////////
pystorm::bdhal::Network* Hal::CreateNetwork(std::string name) {
    assert(!name.empty());
    return new pystorm::bdhal::Network(name);
}

pystorm::bdhal::MappedNetwork* Hal::CreateMappedNetwork(
    pystorm::bdhal::Network* newNetwork) {
    return new pystorm::bdhal::MappedNetwork(newNetwork);
}

void Hal::Load(pystorm::bdhal::MappedNetwork* mappedNet, 
    pystorm::bdhal::LoadBehavior loadBehavior) {

}

//////////////////////////////////////////////////////////////////////////////
//
// Control functionality
//
//////////////////////////////////////////////////////////////////////////////

void Hal::ResetBraindrop() {
}

void Hal::StartBraindrop() {
}

void Hal::StopBraindrop() {
}

void Hal::StartBraindropCore(uint16_t coreId) {
}

void Hal::StopBraindropCore(uint16_t coreId) {
}

//////////////////////////////////////////////////////////////////////////////
//
// Data Flow functionality
//
//////////////////////////////////////////////////////////////////////////////

} // namespace bdhal
} // namespace pystorm

