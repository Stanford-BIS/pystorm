#include <pystormMain.h>

namespace pystorm
{
//////////////////////////////////////////////////////////////////////////////
//
// Program functionality
//
//////////////////////////////////////////////////////////////////////////////
pystorm::NetModels::Network* CreateNetwork(
    std::string name)
{
    assert(!name.empty());
    return new pystorm::NetModels::Network(name);
}

pystorm::Netlist* CreateNetlist()
{
    return new pystorm::Netlist();
}

pystorm::MappedNetwork* CreateMappedNetwork(
    pystorm::NetModels::Network* newNetwork)
{
    return new pystorm::MappedNetwork(newNetwork);
}

void Load(pystorm::MappedNetwork* mappedNet, 
    pystorm::LoadBehavior loadBehavior)
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

pystorm::BrainstormHWDesc* getHardwareDescription()
{
    return g_hardwareDesc;
}

//////////////////////////////////////////////////////////////////////////////
//
// Data Flow functionality
//
//////////////////////////////////////////////////////////////////////////////

pystorm::StreamHandle* createStream(
    std::vector<pystorm::NetModels::Pool> vecOfPools,
    std::vector<pystorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new pystorm::StreamHandle();
}

pystorm::StreamHandle* createStream(
    std::vector<pystorm::NetModels::Pool> vecOfPools)
{
    return new pystorm::StreamHandle();
}

pystorm::StreamHandle* createStream(
    std::vector<pystorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new StreamHandle();
}

pystorm::FileStreamHandle* createFileStream(std::string fileName,
    std::vector<pystorm::NetModels::Pool> vecOfPools,
    std::vector<pystorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new pystorm::FileStreamHandle(fileName);
}

pystorm::FileStreamHandle* createFileStream(std::string fileName,
    std::vector<pystorm::NetModels::Pool> vecOfPools)
{
    return new pystorm::FileStreamHandle(fileName);
}

pystorm::FileStreamHandle* createFileStream(std::string fileName,
    std::vector<pystorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new pystorm::FileStreamHandle(fileName);
}

pystorm::SHMemStreamHandle* createSHMStream(std::string streamName,
    std::vector<pystorm::NetModels::Pool> vecOfPools,
    std::vector<pystorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new pystorm::SHMemStreamHandle(streamName);
}

pystorm::SHMemStreamHandle* createSHMStream(std::string streamName,
    std::vector<pystorm::NetModels::Pool> vecOfPools)
{
    return new pystorm::SHMemStreamHandle(streamName);
}

pystorm::SHMemStreamHandle* createSHMStream(std::string streamName,
    std::vector<pystorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new pystorm::SHMemStreamHandle(streamName);
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
