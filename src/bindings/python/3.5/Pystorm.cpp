#include <boost/python.hpp>
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/register_ptr_to_python.hpp>
#include <boost/shared_ptr.hpp>

#include <Pystorm.h>

namespace pystorm
{

namespace HAL = pystorm::bdhal;
namespace PYTHON = boost::python;

HAL::Connection* 
    (HAL::Network::*CreateConnection_1) (
    std::string name, HAL::ConnectableInput* in_object, 
    HAL::ConnectableOutput* out_object) = 
        &HAL::Network::CreateConnection;

HAL::Connection* 
    (HAL::Network::*CreateConnection_2) (
    std::string name, HAL::ConnectableInput* in_object, 
    HAL::ConnectableOutput* out_object, 
    HAL::Weights<uint32_t>* weightMatrix) = 
        &HAL::Network::CreateConnection;

HAL::Pool* 
    (HAL::Network::*CreatePool_1) (
    std::string label, uint32_t n_neurons, uint32_t n_dims, uint32_t width,
    uint32_t height) =
        &HAL::Network::CreatePool;

HAL::Pool* 
    (HAL::Network::*CreatePool_2) (
    std::string label, uint32_t n_neurons, uint32_t n_dims) = 
        &HAL::Network::CreatePool;

BOOST_PYTHON_MODULE(Pystorm)
{
    //////////////////////////////////////////////////////////////////////
    //
    // Common network objects
    //
    // Objects that Networks are composed of
    //
    //////////////////////////////////////////////////////////////////////

    PYTHON::class_<HAL::VecOfPools>("VecOfPools")
        .def(PYTHON::vector_indexing_suite<HAL::VecOfPools,true>() )
    ;

    PYTHON::class_<HAL::VecOfBuckets>("VecOf Buckets")
        .def(PYTHON::vector_indexing_suite<HAL::VecOfBuckets,true>() )
    ;

    PYTHON::class_<HAL::VecOfConnections>("VecOfConnections")
        .def(PYTHON::vector_indexing_suite<HAL::VecOfConnections,true>()
         )
    ;

    PYTHON::class_<HAL::ConnectableOutput, 
        boost::noncopyable>("ConnectableOutput",
        PYTHON::no_init)
    ;

    PYTHON::class_<HAL::ConnectableInput, 
        boost::noncopyable>("ConnectableInput",
        PYTHON::no_init)
    ;

    PYTHON::class_<HAL::Pool, HAL::Pool*, boost::noncopyable, 
        PYTHON::bases<HAL::ConnectableInput, HAL::ConnectableOutput> >("Pool",
        PYTHON::init<std::string, uint32_t, uint32_t, uint32_t, uint32_t>())
        .def(PYTHON::init<std::string,uint32_t,uint32_t>())
        .def("GetLabel", &HAL::Pool::GetLabel)
        .def("GetNumNeurons", &HAL::Pool::GetNumNeurons)
        .def("GetNumDimensions", &HAL::Pool::GetNumDimensions)
        .def("GetWidth", &HAL::Pool::GetWidth)
        .def("GetHeight", &HAL::Pool::GetHeight)
        .def("SetSize", &HAL::Pool::SetSize)
    ;

    PYTHON::class_<HAL::Bucket, HAL::Bucket*, boost::noncopyable,
        PYTHON::bases<HAL::ConnectableInput, HAL::ConnectableOutput> >("Bucket",
        PYTHON::init<std::string, uint32_t>())
        .def("GetLabel",&HAL::Bucket::GetLabel)
        .def("GetNumDimensions",&HAL::Bucket::GetNumDimensions)
    ;

    PYTHON::class_<HAL::Connection, HAL::Connection*,
        boost::noncopyable>
        ("Connection",PYTHON::init<std::string, 
        HAL::ConnectableInput*, 
        HAL::ConnectableOutput*,
        HAL::Weights<uint32_t>* >())
        .def("GetLabel",&HAL::StateSpace::GetLabel)
    ;

    PYTHON::class_<HAL::Network, boost::noncopyable>("Network",
        PYTHON::init<std::string>())
        .def("GetName",&HAL::Network::GetName)
        .def("GetPools",&HAL::Network::GetPools,
            PYTHON::return_value_policy<PYTHON::reference_existing_object>())
        .def("GetBuckets",&HAL::Network::GetBuckets,
            PYTHON::return_value_policy<PYTHON::reference_existing_object>())
        .def("GetConnections",&HAL::Network::GetConnections,
            PYTHON::return_value_policy<PYTHON::reference_existing_object>())
        .def("CreatePool", CreatePool_1, 
            PYTHON::return_internal_reference<>())
        .def("CreatePool", CreatePool_2, 
            PYTHON::return_internal_reference<>())
        .def("CreateBucket",&HAL::Network::CreateBucket, 
            PYTHON::return_internal_reference<>())
        .def("CreateConnection", CreateConnection_1,
            PYTHON::return_internal_reference<>(),
            PYTHON::args("self","in_obj","out_obj"))
        .def("CreateConnection", CreateConnection_2,
            PYTHON::return_internal_reference<>(),
            PYTHON::args("self","in_obj","out_obj","weight_matrix"))
    ;

    PYTHON::register_ptr_to_python< boost::shared_ptr<HAL::Pool> >();

//////////////////////////////////////////////////////////////////////////////  
//                                                                              
// NetworkCreation Control functionality                                        
//                                                                              
// The following functions allow pystorm users to create networks that can         
// be mapped and loaded onto Braindrop                                          
//                                                                              
////////////////////////////////////////////////////////////////////////////// 

    def("CreateNetwork",
        HAL::Hal::CreateNetwork,
        PYTHON::return_value_policy<PYTHON::reference_existing_object>());


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
}
}
