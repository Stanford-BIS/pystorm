#ifndef PYSTORMMAIN_H
#define PYSTORMMAIN_H

#include <iostream>
#include <vector>

#include <stdint.h>

#include <StreamHandle.h>
#include <FileStreamHandle.h>
#include <SHMemStreamHandle.h>
#include <Pool.h>
#include <StateSpace.h>
#include <MappedNetwork.h>
#include <Network.h>
#include <BrainstormHWDesc.h>
#include <Netlist.h>

/// \file PyStormMain.h

namespace PyStorm
{
// Enum types
enum class LoadBehavior { POOLS_ONLY, ALL_OBJECTS };
enum class StreamingStatus {STREAMING, NOT_STREAMING};
enum class ProgramStatus {PROGRAMMED, NOT_PROGRAMMED};

// Global variables

extern BrainstormHWDesc* g_hardwareDesc;
extern StreamingStatus*  g_streamingStatus;
extern ProgramStatus*    g_programStatus;


//////////////////////////////////////////////////////////////////////////////
//
// Program functionality
//
// The following functions allow PyStorm users to program Brainstorm
//
//////////////////////////////////////////////////////////////////////////////


/// \brief Create an instance of Network
///
/// \param name Name assigned to network
///
/// \return A pointer to an instance of Network
/// 
/// NOTE: PyStorm does not hold a reference to this instance until it 
/// is passed to PyStorm via the load method. The load method actually
/// passed an instance of MappedNetwork which holds a reference to 
/// a Network object.
PyStorm::NetModels::Network* CreateNetwork(std::string name);

/// \brief Create an instance of Netlist
///
PyStorm::Netlist* CreateNetlist();

/// \brief Create an instance of MappedNetwork
///
PyStorm::MappedNetwork* CreateMappedNetwork(
    PyStorm::NetModels::Network* newNetwork);

/// \brief Load a MappedNetwork
///
/// \param mappedNet A network that has been synthesized, placed and routed
/// \param loadBehavior Indicator of whether all objects or only Pools should
///                     be loaded onto Brainstorm
///
/// Loading will first reset PyStorm and Brainstorm (i.e. call 
/// resetBrainstomr), set the proper data structures in PyStorm and 
/// program Brainstorm. After programming Brainstorm, the chip will not
/// produce spikes or decoded values until the startBrainstorm method is
/// called. This allows the user to setup streams and start PyStorms
/// streaming functionality before allowing Brainstorm to produce upstream
/// data.
void Load(PyStorm::MappedNetwork* mappedNet, PyStorm::LoadBehavior loadBehavior);

//////////////////////////////////////////////////////////////////////////////
//
// Control functionality
//
// The following functions allow PyStorm users to control Brainstorms
// behavior. For example, the user can reset Brainstorm as well as 
// indicate to Brainstorm to start producing spikes and decoded values
// on specific cores or all cores.
//
//////////////////////////////////////////////////////////////////////////////

/// \brief Reset Brainstorm
///
/// This method will stop the chip from producing data (i.e. spikes and 
/// decoded values), wipe out all data mapped onto the chip and clear any 
/// data structures stored in PyStorm.
/// The effect is similar to turning off Brainstorm and shutting down
/// the process that has loaded PyStorm.
///
void ResetBrainstorm();

/// \brief Start all Brainstorm cores 
///
/// This method will cause Brainstorm to start producing spike and 
/// decoded values.
///
void StartBrainstorm();

/// \brief Stop all Brainstorm cores 
///
/// This method will cause Brainstorm to stop producing spike and 
/// decoded values.
///
void StopBrainstorm();

/// \brief Start one Brainstorm core 
///
/// \param coreId An identifier assigned to a Brainstorm core
///
/// The id's are zero-based. If an id is given that does not exist on the
/// hardware, the method will throw an exception.
/// The method is blocking and will not return until the core is started.
///
void StartBrainstormCore(uint16_t coreId);

/// \brief Stop one Brainstorm core 
///
/// \param coreId An identifier assigned to a Brainstorm core
///
/// The id's are zero-based. If an id is given that does not exist on the
/// hardware, the method will throw an exception.
/// The method is blocking and will not return until the core is started.
///
void StopBrainstormCore(uint16_t coreId);

/// \brief Return an instance of BranstormHWDesc which describes properties
/// of the attached hardware.
///
/// \return An instance of BrainstormHWDesc
///
PyStorm::BrainstormHWDesc* GetHardwareDescription();

//////////////////////////////////////////////////////////////////////////////
//
// Data Flow functionality
//
//////////////////////////////////////////////////////////////////////////////

/// \brief Create a stream for a specific set of Pools and StateSpaces.
///
/// \param vecOfPools A vector of Pools
/// \param vecOfStateSpaces A vector of StateSpaces
///
/// \return A StreamHandle to use when retreiving data
///
PyStorm::StreamHandle* CreateStream(
    std::vector<PyStorm::NetModels::Pool> vecOfPools,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces);

/// \brief Create a stream for a specific set of Pools.
///
/// \param vecOfPools A vector of Pools
///
/// \return A StreamHandle to use when retreiving data
///
PyStorm::StreamHandle* CreateStream(
    std::vector<PyStorm::NetModels::Pool> vecOfPools);

/// \brief Create a stream for a specific set of StateSpaces.
///
/// \param vecOfStateSpaces A vector of StateSpaces
///
/// \return A StreamHandle to use when retreiving data
///
PyStorm::StreamHandle* CreateStream(
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces);

/// \brief Create a file stream for a specific set of Pools and StateSpaces.
///
/// \param fileName Filename that data will be sent to
/// \param vecOfPools A vector of Pools
/// \param vecOfStateSpaces A vector of StateSpaces
///
/// \return A FileStreamHandle to use when retreiving data
///
/// When constructing an instancce, the return value will be null if the 
/// the constructor cannot open a file handle with the given filename.
///
PyStorm::FileStreamHandle* CreateFileStream(std::string fileName,
    std::vector<PyStorm::NetModels::Pool> vecOfPools,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces);

/// \brief Create a file stream for a specific set of Pools and StateSpaces.
///
/// \param fileName Filename that data will be sent to
/// \param vecOfPools A vector of Pools
///
/// \return A FileStreamHandle to use when retreiving data
///
/// When constructing an instancce, the return value will be null if the 
/// the constructor cannot open a file handle with the given filename.
///
PyStorm::FileStreamHandle* CreateFileStream(std::string fileName,
    std::vector<PyStorm::NetModels::Pool> vecOfPools);

/// \brief Create a file stream for a specific set of Pools and StateSpaces.
///
/// \param fileName Filename that data will be sent to
/// \param vecOfStateSpaces A vector of StateSpaces
///
/// \return A FileStreamHandle to use when retreiving data
/// \param vecOfStateSpaces A vector of StateSpaces
///
/// When constructing an instancce, the return value will be null if the 
/// the constructor cannot open a file handle with the given filename.
///
PyStorm::FileStreamHandle* CreateFileStream(std::string fileName,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces);

/// \brief Create a shared memory stream for a specific set of Pools and StateSpaces.
///
/// \param streamName The name assigned to the stream
/// \param vecOfPools A vector of Pools
/// \param vecOfStateSpaces A vector of StateSpaces
///
/// \return A SHMStreamHandle to use when retreiving data
///
/// When constructing an instancce, the return value will be null if the 
/// the constructor cannot open a shared memory segment with the given name.
///
PyStorm::SHMemStreamHandle* CreateSHMStream(std::string streamName,
    std::vector<PyStorm::NetModels::Pool> vecOfPools,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces);

/// \brief Create a shared memory stream for a specific set of Pools and StateSpaces.
///
/// \param streamName The name assigned to the stream
/// \param vecOfPools A vector of Pools
///
/// \return A SHMStreamHandle to use when retreiving data
///
/// When constructing an instancce, the return value will be null if the 
/// the constructor cannot open a shared memory segment with the given name.
///
PyStorm::SHMemStreamHandle* CreateSHMStream(std::string streamName,
    std::vector<PyStorm::NetModels::Pool> vecOfPools);

/// \brief Create a shared memory stream for a specific set of Pools and StateSpaces.
///
/// \param streamName The name assigned to the stream
/// \param vecOfStateSpaces A vector of StateSpaces
///
/// \return A SHMStreamHandle to use when retreiving data
/// \param vecOfStateSpaces A vector of StateSpaces
///
/// When constructing an instancce, the return value will be null if the 
/// the constructor cannot open a shared memory segment with the given name.
///
PyStorm::SHMemStreamHandle* CreateSHMStream(std::string streamName,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces);

/// \brief Start streaming data from Brainstorm
///
void StartStreams();

/// \brief Stop streaming data from Brainstorm
///
void StopStreams();

/// \brief Get Brainstorms streaming status
///
bool GetStreamingStatus();

/// \brief Start streaming and stop streaming after a given time
///
/// \param timeInMilliseconds The amount of time to stream
///
void RunStreams(uint32_t timeInMilliseconds);


} // namespace PyStorm

#endif // ifndef PYSTORMMAIN_H
