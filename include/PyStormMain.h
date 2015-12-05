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

namespace PyStorm
{

enum class LoadBehavior { POOLS_ONLY, ALL_OBJECTS };

class PyStormMain
{
public:
    PyStormMain();

    ~PyStormMain();

    // Program functionality
    // The following methods allow PyStorm users to program Brainstorm

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
    PyStorm::NetModels::Network* createNetwork(std::string name);

    /// \brief Create an instance of Netlist
    ///
    PyStorm::Netlist* createNetlist();

    /// \brief Create an instance of MappedNetwork
    ///
    PyStorm::MappedNetwork* createMappedNetwork(
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
    void load(PyStorm::MappedNetwork* mappedNet, LoadBehavior loadBehavior);

    // Control functionality
    // The following methods allow PyStorm users to control Brainstorms
    // behavior. For example, the user can reset Brainstorm as well as 
    // indicate to Brainstorm to start producing spikes and decoded values
    // on specific cores or all cores.

    /// \brief Reset Brainstorm
    ///
    /// This method will stop the chip from producing data (i.e. spikes and 
    /// decoded values), wipe out all data mapped onto the chip and clear any 
    /// data structures stored in PyStorm.
    /// The effect is similar to turning off Brainstorm and shutting down
    /// the process that has loaded PyStorm.
    ///
    void resetBrainstorm();

    /// \brief Start all Brainstorm cores 
    ///
    /// This method will cause Brainstorm to start producing spike and 
    /// decoded values.
    ///
    void startBrainstorm();

    /// \brief Stop all Brainstorm cores 
    ///
    /// This method will cause Brainstorm to stop producing spike and 
    /// decoded values.
    ///
    void stopBrainstorm();

    /// \brief Start one Brainstorm core 
    ///
    /// \param coreId An identifier assigned to a Brainstorm core
    ///
    /// The id's are zero-based. If an id is given that does not exist on the
    /// hardware, the method will throw an exception.
    /// The method is blocking and will not return until the core is started.
    ///
    void startBrainstormCore(uint16_t coreId);

    /// \brief Stop one Brainstorm core 
    ///
    /// \param coreId An identifier assigned to a Brainstorm core
    ///
    /// The id's are zero-based. If an id is given that does not exist on the
    /// hardware, the method will throw an exception.
    /// The method is blocking and will not return until the core is started.
    ///
    void stopBrainstormCore(uint16_t coreId);

    /// \brief Return an instance of BranstormHWDesc which describes properties
    /// of the attached hardware.
    ///
    /// \return An instance of BrainstormHWDesc
    ///
    PyStorm::BrainstormHWDesc* getHardwareDescription();

    // Data Flow functionality

    PyStorm::StreamHandle* createStream(
        std::vector<PyStorm::NetModels::Pool> vecOfPools,
        std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces);

    PyStorm::StreamHandle* createStream(
        std::vector<PyStorm::NetModels::Pool> vecOfPools);

    PyStorm::StreamHandle* createStream(
        std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces);

    PyStorm::FileStreamHandle* createFileStream(std::string fileName,
        std::vector<PyStorm::NetModels::Pool> vecOfPools,
        std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces);

    PyStorm::FileStreamHandle* createFileStream(std::string fileName,
        std::vector<PyStorm::NetModels::Pool> vecOfPools);

    PyStorm::FileStreamHandle* createFileStream(std::string fileName,
        std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces);

    PyStorm::SHMemStreamHandle* createSHMStream(std::string streamName,
        std::vector<PyStorm::NetModels::Pool> vecOfPools,
        std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces);

    PyStorm::SHMemStreamHandle* createSHMStream(std::string streamName,
        std::vector<PyStorm::NetModels::Pool> vecOfPools);

    PyStorm::SHMemStreamHandle* createSHMStream(std::string streamName,
        std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces);

    void startStreams();

    void stopStreams();

    bool getStreamingStatus();

    void runStreams(uint32_t timeInMilliseconds);

private:
    BrainstormHWDesc* m_hardwareDesc;

};

} // namespace PyStorm

#endif // ifndef PYSTORMMAIN_H
