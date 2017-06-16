#ifndef PYSTORM_H
#define PYSTORM_H

#include <iostream>
#include <vector>

#include <stdint.h>

#include <common/Pool.h>
#include <common/StateSpace.h>
#include <common/MappedNetwork.h>
#include <common/Network.h>

/// \file pystorm.h

namespace pystorm {
namespace bdhal {
// Enum types
enum class LoadBehavior { POOLS_ONLY, ALL_OBJECTS };
enum class StreamingStatus {STREAMING, NOT_STREAMING};
enum class ProgramStatus {PROGRAMMED, NOT_PROGRAMMED};

// Global variables

class Hal {
public:
    Hal();
    ~Hal();

    Hal(const Hal&) = delete;
    Hal(Hal&&) = delete;
    Hal& operator=(const Hal&) = delete;
    Hal& operator=(Hal&&) = delete;

//////////////////////////////////////////////////////////////////////////////
//
// NetworkCreation Control functionality
//
// The following functions allow pystorm users to create networks that can
// be mapped and loaded onto Braindrop
//
//////////////////////////////////////////////////////////////////////////////

///
/// Create an instance of a Network
///
/// \param name Name assigned to network
///
/// \return A pointer to an instance of Network
/// 
/// NOTE: A Pystorm module user can create many instances of a Network,
/// however, only one Network instance can be loaded onto Braindrop.
/// This allows a user to create and map several Networks and switch
/// between mapped networks by simply loading them.
/// 
    static pystorm::bdhal::Network* CreateNetwork(std::string name);

//////////////////////////////////////////////////////////////////////////////
//
//  NetworkMapping Control functionality
//
// The following functions allow pystorm users to map Networks to hardware.
// A MappedNetwork can be loaded onto hardware using the Load method.
//
//////////////////////////////////////////////////////////////////////////////

///
/// Create an instance of MappedNetwork
///
    static pystorm::bdhal::MappedNetwork* CreateMappedNetwork(
        pystorm::bdhal::Network* newNetwork);


//////////////////////////////////////////////////////////////////////////////
//
// Platform Control functionality
//
// The following functions allow pystorm users to control Braindrops
// behavior. For example, the user can reset Braindrop as well as 
// indicate to Braindrop to start producing spikes and decoded values
// on specific cores or all cores.
//
//////////////////////////////////////////////////////////////////////////////

///
/// Load a MappedNetwork
///
/// \param mappedNet A network that has been synthesized, placed and routed
/// \param loadBehavior Indicator of whether all objects or only Pools should
///                     be loaded onto Braindrop
///
/// Loading will first reset pystorm and Braindrop (i.e. call 
/// resetBrainstomr), set the proper data structures in pystorm and 
/// program Braindrop. After programming Braindrop, the chip will not
/// produce spikes or decoded values until the startBraindrop method is
/// called. This allows the user to setup streams and start pystorms
/// streaming functionality before allowing Braindrop to produce upstream
/// data.
///
    static void Load(pystorm::bdhal::MappedNetwork* mappedNet, 
        pystorm::bdhal::LoadBehavior loadBehavior);

///
/// Reset Braindrop
///
/// This method will stop the chip from producing data (i.e. spikes and 
/// decoded values), wipe out all data mapped onto the chip and clear any 
/// data structures stored in pystorm.
/// The effect is similar to turning off Braindrop and shutting down
/// the process that has loaded pystorm.
///
    static void ResetBraindrop();

///
/// Start all Braindrop cores 
///
/// This method will cause Braindrop to start producing spike and 
/// decoded values.
///
    static void StartBraindrop();

///
/// Stop all Braindrop cores 
///
/// This method will cause Braindrop to stop producing spike and 
/// decoded values.
///
    static void StopBraindrop();

///
/// Start one Braindrop core 
///
/// \param coreId An identifier assigned to a Braindrop core
///
/// The id's are zero-based. If an id is given that does not exist on the
/// hardware, the method will throw an exception.
/// The method is blocking and will not return until the core is started.
///
    static void StartBraindropCore(uint16_t coreId);

///
/// Stop one Braindrop core 
///
/// \param coreId An identifier assigned to a Braindrop core
///
/// The id's are zero-based. If an id is given that does not exist on the
/// hardware, the method will throw an exception.
/// The method is blocking and will not return until the core is started.
///
    static void StopBraindropCore(uint16_t coreId);

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

protected:
    StreamingStatus*  g_streamingStatus;
    ProgramStatus*    g_programStatus;
};

} // namespace bdhal
} // namespace pystorm

#endif // ifndef PYSTORMMAIN_H
