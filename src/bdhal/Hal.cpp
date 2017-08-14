#include <Hal.h>

namespace pystorm {
namespace bdhal {

CorePars* Hal::g_corePars = nullptr;

Hal::Hal() {
}

Hal::~Hal() {
}
//////////////////////////////////////////////////////////////////////////////
//
// NetworkCreation Control functionality
//
//////////////////////////////////////////////////////////////////////////////

pystorm::bdhal::Network* Hal::CreateNetwork(std::string name) {
    assert(!name.empty());
    return new pystorm::bdhal::Network(name);
}

//////////////////////////////////////////////////////////////////////////////
//
// NetworkMapping Control functionality
//
//////////////////////////////////////////////////////////////////////////////

pystorm::bdhal::CorePars* Hal::GetCorePars() {
    if (nullptr == g_corePars) {
        g_corePars = new CorePars();

        // Need to change this to use BDPars
        // This was added now to get the glue created between HAL and Neuromorph
        g_corePars->insert(std::make_pair(CoreParsIndex::MM_height,8));
        g_corePars->insert(std::make_pair(CoreParsIndex::MM_width,8));
        g_corePars->insert(std::make_pair(CoreParsIndex::AM_size,16));
        g_corePars->insert(std::make_pair(CoreParsIndex::TAT_size,32));
        g_corePars->insert(std::make_pair(CoreParsIndex::NeuronArray_height,
            8));
        g_corePars->insert(std::make_pair(CoreParsIndex::NeuronArray_width, 
            8));
        g_corePars->insert(std::make_pair(CoreParsIndex::NeuronArray_pool_size,
            4));
        g_corePars->insert(std::make_pair(CoreParsIndex::num_threshold_levels,
            8));
        g_corePars->insert(std::make_pair(CoreParsIndex::min_threshold_value,
            64));
        g_corePars->insert(std::make_pair(CoreParsIndex::max_weight_value, 
            127));
        g_corePars->insert(
            std::make_pair(CoreParsIndex::NeuronArray_neurons_per_tap,4));
    }

    return g_corePars;
}

//////////////////////////////////////////////////////////////////////////////
//
// Platform Control functionality
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
// Experiment Control functionality
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Data Flow Control functionality
//
//////////////////////////////////////////////////////////////////////////////

} // namespace bdhal
} // namespace pystorm

