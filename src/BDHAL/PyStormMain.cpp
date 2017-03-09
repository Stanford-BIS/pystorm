#include <PyStormMain.h>

namespace PyStorm
{
//////////////////////////////////////////////////////////////////////////////
//
// Program functionality
//
//////////////////////////////////////////////////////////////////////////////
PyStorm::NetModels::Network* CreateNetwork(
    std::string name)
{
    assert(!name.empty());
    return new PyStorm::NetModels::Network(name);
}

PyStorm::Netlist* CreateNetlist()
{
    return new PyStorm::Netlist();
}

PyStorm::MappedNetwork* CreateMappedNetwork(
    PyStorm::NetModels::Network* newNetwork)
{
    return new PyStorm::MappedNetwork(newNetwork);
}

void Load(PyStorm::MappedNetwork* mappedNet, 
    PyStorm::LoadBehavior loadBehavior)
{

}

//////////////////////////////////////////////////////////////////////////////
//
// Control functionality
//
//////////////////////////////////////////////////////////////////////////////

void ResetBrainstorm()
{
}

void StartBrainstorm()
{
}

void stopBrainstorm()
{
}

void startBrainstormCore(uint16_t coreId)
{
}

void stopBrainstormCore(uint16_t coreId)
{
}

PyStorm::BrainstormHWDesc* getHardwareDescription()
{
    return g_hardwareDesc;
}

//////////////////////////////////////////////////////////////////////////////
//
// Data Flow functionality
//
//////////////////////////////////////////////////////////////////////////////

PyStorm::StreamHandle* createStream(
    std::vector<PyStorm::NetModels::Pool> vecOfPools,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new PyStorm::StreamHandle();
}

PyStorm::StreamHandle* createStream(
    std::vector<PyStorm::NetModels::Pool> vecOfPools)
{
    return new PyStorm::StreamHandle();
}

PyStorm::StreamHandle* createStream(
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new StreamHandle();
}

PyStorm::FileStreamHandle* createFileStream(std::string fileName,
    std::vector<PyStorm::NetModels::Pool> vecOfPools,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new PyStorm::FileStreamHandle(fileName);
}

PyStorm::FileStreamHandle* createFileStream(std::string fileName,
    std::vector<PyStorm::NetModels::Pool> vecOfPools)
{
    return new PyStorm::FileStreamHandle(fileName);
}

PyStorm::FileStreamHandle* createFileStream(std::string fileName,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new PyStorm::FileStreamHandle(fileName);
}

PyStorm::SHMemStreamHandle* createSHMStream(std::string streamName,
    std::vector<PyStorm::NetModels::Pool> vecOfPools,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new PyStorm::SHMemStreamHandle(streamName);
}

PyStorm::SHMemStreamHandle* createSHMStream(std::string streamName,
    std::vector<PyStorm::NetModels::Pool> vecOfPools)
{
    return new PyStorm::SHMemStreamHandle(streamName);
}

PyStorm::SHMemStreamHandle* createSHMStream(std::string streamName,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new PyStorm::SHMemStreamHandle(streamName);
}

void startStreams()
{

}

void stopStreams()
{

}

bool getStreamingStatus()
{
    bool status;

    return status;
}

void runStreams(uint32_t timeInMilliseconds)
{

}
}
