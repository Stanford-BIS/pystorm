#include <boost/python.hpp>
#include <Pool.h>
#include <StateSpace.h>
#include <WeightedConnection.h>
#include <ConnectableObject.h>
#include <Network.h>
#include <pystormMain.h>

namespace pystorm
{

boost::python::object _CreateStreamWithOneList(
    const std::vector<pystorm::NetModels::Pool>& vecOfPools)
{

    CreateStream(vecOfPools);
    return boost::python::object();
}

boost::python::object _CreateStreamWithOneList(
    const std::vector<pystorm::NetModels::StateSpace> & vecOfStateSpaces)
{

    CreateStream(vecOfStateSpaces);
    return boost::python::object();
}

boost::python::object CreateStreamWithOneList( 
    const boost::python::list& listOfObjects)
{
    std::vector<pystorm::NetModels::Pool> vecOfPools;
    // add logic to first determine what type of objects were passed
    // should be list of StateSpaces or list of Pools
    // then copy the objects to a vecto and call one of the overloaded functions
    return _CreateStreamWithOneList(vecOfPools);
}

boost::python::object CreateStreamWithTwoLists( 
    const boost::python::list& listOfObjects1,
    const boost::python::list& listOfObjects2)
{
    std::vector<pystorm::NetModels::Pool> vecOfPools;
    std::vector<pystorm::NetModels::StateSpace> vecOfStateSpaces;

    // add logic to first determine what type of objects were passed
    // in the first parameter and the second parameter
    // then copy the objects to two vectors and call 
    // the method
    CreateStream(vecOfPools, vecOfStateSpaces);
    return boost::python::object();
}

boost::python::object _CreateFileStreamWithOneList(const std::string& fileName, 
    const std::vector<pystorm::NetModels::Pool>& vecOfPools)
{

    CreateFileStream(fileName, vecOfPools);
    return boost::python::object();
}

boost::python::object _CreateFileStreamWithOneList(const std::string& fileName, 
    const std::vector<pystorm::NetModels::StateSpace> & vecOfStateSpaces)
{

    CreateFileStream(fileName, vecOfStateSpaces);
    return boost::python::object();
}

boost::python::object CreateFileStreamWithOneList(const std::string& fileName, 
    const boost::python::list& listOfObjects)
{
    std::vector<pystorm::NetModels::Pool> vecOfPools;
    // add logic to first determine what type of objects were passed
    // should be list of StateSpaces or list of Pools
    // then copy the objects to a vecto and call one of the overloaded functions
    return _CreateFileStreamWithOneList(fileName, vecOfPools);
}

boost::python::object CreateFileStreamWithTwoLists(const std::string& fileName, 
    const boost::python::list& listOfObjects1,
    const boost::python::list& listOfObjects2)
{
    std::vector<pystorm::NetModels::Pool> vecOfPools;
    std::vector<pystorm::NetModels::StateSpace> vecOfStateSpaces;

    // add logic to first determine what type of objects were passed
    // in the first parameter and the second parameter
    // then copy the objects to two vectors and call 
    // the method
    CreateFileStream(fileName, vecOfPools, vecOfStateSpaces);
    return boost::python::object();
}

boost::python::object _CreateSHMStreamWithOneList(const std::string& streamName, 
    const std::vector<pystorm::NetModels::Pool>& vecOfPools)
{

    CreateSHMStream(streamName, vecOfPools);
    return boost::python::object();
}

boost::python::object _CreateSHMStreamWithOneList(const std::string& streamName, 
    const std::vector<pystorm::NetModels::StateSpace> & vecOfStateSpaces)
{

    CreateSHMStream(streamName, vecOfStateSpaces);
    return boost::python::object();
}

boost::python::object CreateSHMStreamWithOneList(const std::string& streamName, 
    const boost::python::list& listOfObjects)
{
    std::vector<pystorm::NetModels::Pool> vecOfPools;
    // add logic to first determine what type of objects were passed
    // should be list of StateSpaces or list of Pools
    // then copy the objects to a vecto and call one of the overloaded functions
    return _CreateSHMStreamWithOneList(streamName, vecOfPools);
}

boost::python::object CreateSHMStreamWithTwoLists(const std::string& streamName, 
    const boost::python::list& listOfObjects1,
    const boost::python::list& listOfObjects2)
{
    std::vector<pystorm::NetModels::Pool> vecOfPools;
    std::vector<pystorm::NetModels::StateSpace> vecOfStateSpaces;

    // add logic to first determine what type of objects were passed
    // in the first parameter and the second parameter
    // then copy the objects to two vectors and call 
    // the method
    CreateSHMStream(streamName, vecOfPools, vecOfStateSpaces);
    return boost::python::object();
}

void StartStreams()
{
    StartStreams();
}

void StopStreams()
{
    StopStreams();
}

void RunStreams(uint32_t timeInMilliseconds)
{
    RunStreams(timeInMilliseconds);
}

bool GetStreamingStatus()
{
    return GetStreamingStatus();
}

pystorm::NetModels::WeightedConnection* 
    (pystorm::NetModels::Network::*createWeightedConnection_1) (
    std::string name, pystorm::NetModels::ConnectableObject* in_object, 
    pystorm::NetModels::ConnectableObject* out_object) = 
        &pystorm::NetModels::Network::createWeightedConnection;

pystorm::NetModels::WeightedConnection* 
    (pystorm::NetModels::Network::*createWeightedConnection_2) (
    std::string name, pystorm::NetModels::ConnectableObject* in_object, 
    pystorm::NetModels::ConnectableObject* out_object, 
    pystorm::NetModels::Transform<uint32_t>* transformMatrix) = 
        &pystorm::NetModels::Network::createWeightedConnection;

BOOST_PYTHON_MODULE(pystorm)
{
    boost::python::def("CreateStream",CreateStreamWithOneList);
    boost::python::def("CreateStream",CreateStreamWithTwoLists);

    boost::python::def("CreateFileStream",CreateFileStreamWithOneList);
    boost::python::def("CreateFileStream",CreateFileStreamWithTwoLists);

    boost::python::def("CreateSHMStream",CreateSHMStreamWithOneList);
    boost::python::def("CreateSHMStream",CreateSHMStreamWithTwoLists);

    boost::python::def("StartStreams",StartStreams);
    boost::python::def("StopStreams",StopStreams);
    boost::python::def("RunStreams",RunStreams);
    boost::python::def("GetStreamingStatus",GetStreamingStatus);

    boost::python::class_<pystorm::NetModels::Pool>("Pool",
        boost::python::init<std::string, uint32_t>())
        .def("GetName",&pystorm::NetModels::Pool::getName)
        .def("GetNumNeurons",&pystorm::NetModels::Pool::getNumNeurons)
    ;

    boost::python::class_<pystorm::NetModels::StateSpace>("StateSpace",
        boost::python::init<std::string, uint32_t>())
        .def("GetName",&pystorm::NetModels::StateSpace::getName)
        .def("GetNumDims",&pystorm::NetModels::StateSpace::getNumDims)
    ;

    boost::python::class_<pystorm::NetModels::WeightedConnection>
        ("WeightedConnection",boost::python::init<std::string, 
        pystorm::NetModels::ConnectableObject*, 
        pystorm::NetModels::ConnectableObject*,
        pystorm::NetModels::Transform<uint32_t>* >())
        .def("GetName",&pystorm::NetModels::StateSpace::getName)
    ;

    boost::python::class_<pystorm::NetModels::Network>("Network",
        boost::python::init<std::string>())
        .def("GetName",&pystorm::NetModels::Network::getName)
        .def("CreatePool",&pystorm::NetModels::Network::createPool, 
            boost::python::return_internal_reference<>())
        .def("CreateStateSpace",&pystorm::NetModels::Network::createStateSpace, 
            boost::python::return_internal_reference<>())
        .def("CreateWeightedConnection", createWeightedConnection_1,
            boost::python::return_internal_reference<>())
        .def("CreateWeightedConnection", createWeightedConnection_2,
            boost::python::return_internal_reference<>())
    ;

    boost::python::class_<pystorm::StreamHandle>("StreamHandle",
        boost::python::init<>())
        .def("Close", &pystorm::StreamHandle::close)
        .def("GetHandleId", &pystorm::StreamHandle::getHandleId, 
            boost::python::return_internal_reference<>())
        .def("CloseAll", &pystorm::StreamHandle::closeAll)
    ;

    boost::python::class_<pystorm::FileStreamHandle>("FileStreamHandle",
        boost::python::init<std::string>())
        .def("Flush", &pystorm::FileStreamHandle::flush)
        .def("Close", &pystorm::FileStreamHandle::close)
        .def("GetHandleId", &pystorm::FileStreamHandle::getHandleId, 
            boost::python::return_internal_reference<>())
        .def("CloseAll", &pystorm::FileStreamHandle::closeAll)
    ;

    boost::python::class_<pystorm::SHMemStreamHandle>("SHMemStreamHandle",
        boost::python::init<std::string>())
        .def("Close", &pystorm::SHMemStreamHandle::close)
        .def("GetHandleId", &pystorm::SHMemStreamHandle::getHandleId, 
            boost::python::return_internal_reference<>())
        .def("CloseAll", &pystorm::SHMemStreamHandle::closeAll)
    ;
}
}
