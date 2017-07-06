#include <boost/python.hpp>
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/register_ptr_to_python.hpp>
#include <boost/shared_ptr.hpp>

#include <PyStorm.h>

namespace pystorm
{

namespace HAL = pystorm::bdhal;
namespace PYTHON = boost::python;

HAL::WeightedConnection* 
    (HAL::Network::*CreateWeightedConnection_1) (
    std::string name, HAL::ConnectableObject* in_object, 
    HAL::ConnectableObject* out_object) = 
        &HAL::Network::CreateWeightedConnection;

HAL::WeightedConnection* 
    (HAL::Network::*CreateWeightedConnection_2) (
    std::string name, HAL::ConnectableObject* in_object, 
    HAL::ConnectableObject* out_object, 
    HAL::Transform<uint32_t>* transformMatrix) = 
        &HAL::Network::CreateWeightedConnection;

BOOST_PYTHON_MODULE(PyStorm)
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

    PYTHON::class_<HAL::VecOfStateSpaces>("VecOfStateSpaces")
        .def(PYTHON::vector_indexing_suite<HAL::VecOfStateSpaces,true>() )
    ;

    PYTHON::class_<HAL::VecOfWeightedConnections>("VecOfWeightedConnections")
        .def(PYTHON::vector_indexing_suite<HAL::VecOfWeightedConnections,true>()
         )
    ;

    PYTHON::class_<HAL::ConnectableObject, 
        boost::noncopyable>("ConnectableObject",
        PYTHON::no_init)
    ;

    PYTHON::class_<HAL::Pool, HAL::Pool*, boost::noncopyable, 
        PYTHON::bases<HAL::ConnectableObject> >("Pool",
        PYTHON::init<std::string, uint32_t, uint32_t, uint32_t, uint32_t>())
        .def("GetLabel",&HAL::Pool::GetLabel)
        .def("GetNumNeurons",&HAL::Pool::GetNumNeurons)
        .def("GetNumDimensions",&HAL::Pool::GetNumDimensions)
        .def("GetWidth",&HAL::Pool::GetWidth)
        .def("GetHeight",&HAL::Pool::GetHeight)
    ;

    PYTHON::class_<HAL::StateSpace, HAL::StateSpace*, boost::noncopyable,
        PYTHON::bases<HAL::ConnectableObject> >("StateSpace",
        PYTHON::init<std::string, uint32_t>())
        .def("GetLabel",&HAL::StateSpace::GetLabel)
        .def("GetNumDimensions",&HAL::StateSpace::GetNumDimensions)
    ;

    PYTHON::class_<HAL::WeightedConnection, HAL::WeightedConnection*,
        boost::noncopyable>
        ("WeightedConnection",PYTHON::init<std::string, 
        HAL::ConnectableObject*, 
        HAL::ConnectableObject*,
        HAL::Transform<uint32_t>* >())
        .def("GetLabel",&HAL::StateSpace::GetLabel)
    ;

    PYTHON::class_<HAL::Network, boost::noncopyable>("Network",
        PYTHON::init<std::string>())
        .def("GetName",&HAL::Network::GetName)
        .def("GetPools",&HAL::Network::GetPools,
            PYTHON::return_value_policy<PYTHON::reference_existing_object>())
        .def("GetStateSpaces",&HAL::Network::GetStateSpaces,
            PYTHON::return_value_policy<PYTHON::reference_existing_object>())
        .def("GetWeightedConnections",&HAL::Network::GetWeightedConnections,
            PYTHON::return_value_policy<PYTHON::reference_existing_object>())
        .def("CreatePool",&HAL::Network::CreatePool, 
            PYTHON::return_internal_reference<>())
        .def("CreateStateSpace",&HAL::Network::CreateStateSpace, 
            PYTHON::return_internal_reference<>())
        .def("CreateWeightedConnection", CreateWeightedConnection_1,
            PYTHON::return_internal_reference<>(),
            PYTHON::args("self","in_obj","out_obj"))
        .def("CreateWeightedConnection", CreateWeightedConnection_2,
            PYTHON::return_internal_reference<>(),
            PYTHON::args("self","in_obj","out_obj","trans_matrix"))
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
//  NetworkMapping Control functionality                                        
//                                                                              
// The following functions allow pystorm users to map Networks to hardware.        
// A MappedNetwork can be loaded onto hardware using the Load method.           
//                                                                              
//////////////////////////////////////////////////////////////////////////////  


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
