#include <PyStormMain.h>

namespace PyStorm
{

PyStormMain::PyStormMain()
{

}

PyStormMain::~PyStormMain()
{

}

// Program functionality
PyStorm::NetModels::Network* PyStormMain::createNetwork(
    std::string name)
{
    assert(!name.empty());
    return new PyStorm::NetModels::Network(name);
}

PyStorm::Netlist* PyStormMain::createNetlist()
{
    return new PyStorm::Netlist();
}

PyStorm::MappedNetwork* PyStormMain::createMappedNetwork(
    PyStorm::NetModels::Network* newNetwork)
{
    return new PyStorm::MappedNetwork(newNetwork);
}

void PyStormMain::load(PyStorm::MappedNetwork* mappedNet, 
    PyStorm::LoadBehavior loadBehavior)
{

}

// Control functionality

void PyStormMain::resetBrainstorm()
{
}

void PyStormMain::startBrainstorm()
{
}

void PyStormMain::stopBrainstorm()
{
}

void PyStormMain::startBrainstormCore(uint16_t coreId)
{
}

void PyStormMain::stopBrainstormCore(uint16_t coreId)
{
}

PyStorm::BrainstormHWDesc* PyStormMain::getHardwareDescription()
{
    return m_hardwareDesc;
}

// Data Flow functionality

PyStorm::StreamHandle* PyStormMain::createStream(
    std::vector<PyStorm::NetModels::Pool> vecOfPools,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new PyStorm::StreamHandle();
}

PyStorm::StreamHandle* PyStormMain::createStream(
    std::vector<PyStorm::NetModels::Pool> vecOfPools)
{
    return new PyStorm::StreamHandle();
}

PyStorm::StreamHandle* PyStormMain::createStream(
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new StreamHandle();
}

PyStorm::FileStreamHandle* PyStormMain::createFileStream(std::string fileName,
    std::vector<PyStorm::NetModels::Pool> vecOfPools,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new PyStorm::FileStreamHandle(fileName);
}

PyStorm::FileStreamHandle* PyStormMain::createFileStream(std::string fileName,
    std::vector<PyStorm::NetModels::Pool> vecOfPools)
{
    return new PyStorm::FileStreamHandle(fileName);
}

PyStorm::FileStreamHandle* PyStormMain::createFileStream(std::string fileName,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new PyStorm::FileStreamHandle(fileName);
}

PyStorm::SHMemStreamHandle* PyStormMain::createSHMStream(std::string streamName,
    std::vector<PyStorm::NetModels::Pool> vecOfPools,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new PyStorm::SHMemStreamHandle(streamName);
}

PyStorm::SHMemStreamHandle* PyStormMain::createSHMStream(std::string streamName,
    std::vector<PyStorm::NetModels::Pool> vecOfPools)
{
    return new PyStorm::SHMemStreamHandle(streamName);
}

PyStorm::SHMemStreamHandle* PyStormMain::createSHMStream(std::string streamName,
    std::vector<PyStorm::NetModels::StateSpace> vecOfStateSpaces)
{
    return new PyStorm::SHMemStreamHandle(streamName);
}

void PyStormMain::startStreams()
{

}

void PyStormMain::stopStreams()
{

}

bool PyStormMain::getStreamingStatus()
{
    bool status;

    return status;
}

void PyStormMain::runStreams(uint32_t timeInMilliseconds)
{

}
}
