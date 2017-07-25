#include <boost/python.hpp>
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/register_ptr_to_python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python/numeric.hpp>
#include <boost/python/tuple.hpp>
#include <numpy/arrayobject.h>

#include <Pystorm.h>

#include <iostream>

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


template<typename T>
HAL::Weights<T>* makeWeights(PYTHON::object& weights) {

    static int num_dims_allowed = 2;

    if (false == PyArray_CheckExact(weights.ptr())) {
        std::logic_error("Weights must be a numpy array");
    }

    PyArrayObject *arrayobj_ptr = (PyArrayObject*) PyArray_FROM_O(weights.ptr());

    if (num_dims_allowed != PyArray_NDIM(arrayobj_ptr)){
        throw std::logic_error("Matrix must have 2 dimensions");
    }

    npy_intp* dims = PyArray_DIMS(arrayobj_ptr);

    npy_intp num_rows = dims[0];
    npy_intp num_columns = dims[1];

    // copy the weight matrix
    T* weights_ptr = (T*) std::calloc((num_rows*num_columns),sizeof(T));

    HAL::Weights<T>* weightMatrix = new HAL::Weights<T>(weights_ptr, num_rows,
        num_columns);

    for (unsigned int row = 0; row < weightMatrix->GetNumRows(); row++)
    {
        for (unsigned int col = 0; col < weightMatrix->GetNumColumns(); col++)
        {
            npy_intp npy_row = static_cast<npy_intp>(row);
            npy_intp npy_col = static_cast<npy_intp>(col);
            T new_value = *((T*) PyArray_GETPTR2(arrayobj_ptr, npy_row, npy_col));
            weightMatrix->SetElement(row, col, new_value);
        }
    }

    return weightMatrix;
}

#if PY_MAJOR_VERSION >= 3
    int
#else
    void
#endif
init_numpy() {
    import_array();
}

BOOST_PYTHON_MODULE(Pystorm)
{

    init_numpy();

    PYTHON::numeric::array::set_module_and_type("ndtype","ndarray");
    //////////////////////////////////////////////////////////////////////
    //
    // Common network objects
    //
    // Objects that Networks are composed of
    //
    //////////////////////////////////////////////////////////////////////

    typedef HAL::Weights<uint32_t> Weights_32;

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

    PYTHON::class_<HAL::Connectable, boost::noncopyable>("Connectable",
        PYTHON::no_init)
    ;
    PYTHON::class_<HAL::ConnectableOutput, boost::noncopyable,
        PYTHON::bases<HAL::Connectable> >("ConnectableOutput",
        PYTHON::no_init)
    ;

    PYTHON::class_<HAL::ConnectableInput, boost::noncopyable,
        PYTHON::bases<HAL::Connectable> >("ConnectableInput",
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
        .def("GetLabel",&HAL::Bucket::GetLabel
            , "Returns the Bucket label"
            , PYTHON::args("self"))
        .def("GetNumDimensions",&HAL::Bucket::GetNumDimensions
            , "Returns the number of dimensions"
            , PYTHON::args("self"))
    ;

    PYTHON::class_<HAL::Input, HAL::Input*, boost::noncopyable,
        PYTHON::bases<HAL::ConnectableInput> >("Input",
        PYTHON::init<std::string, uint32_t>())
        .def("GetLabel",&HAL::Input::GetLabel
            , "Returns the Input label"
            , PYTHON::args("self"))
        .def("GetNumDimensions",&HAL::Input::GetNumDimensions
            , "Returns the number of dimensions"
            , PYTHON::args("self"))
    ;

    PYTHON::class_<HAL::Output, HAL::Output*, boost::noncopyable,
        PYTHON::bases<HAL::ConnectableOutput> >("Output",
        PYTHON::init<std::string, uint32_t>())
        .def("GetLabel",&HAL::Output::GetLabel
            , "Returns the Output label"
            , PYTHON::args("self"))
        .def("GetNumDimensions",&HAL::Output::GetNumDimensions
            , "Returns the number of dimensions"
            , PYTHON::args("self"))
    ;

    PYTHON::class_<Weights_32, Weights_32*, boost::noncopyable>
        ("Weights", PYTHON::no_init)
        .def("__init__",
            PYTHON::make_constructor(pystorm::makeWeights<uint32_t>))
        .def("GetNumRows",&Weights_32::GetNumRows)
        .def("GetNumColumns",&Weights_32::GetNumColumns)
        .def("GetElement",&Weights_32::GetElement)
        .def("SetElement",&Weights_32::SetElement)
    ;

    PYTHON::class_<HAL::Connection, HAL::Connection*,
        boost::noncopyable>
        ("Connection",PYTHON::init<std::string, 
        HAL::ConnectableInput*, 
        HAL::ConnectableOutput*,
        HAL::Weights<uint32_t>* >())
        .def(PYTHON::init<std::string, HAL::ConnectableInput*, 
            HAL::ConnectableOutput*>())
        .def("GetLabel",&HAL::Connection::GetLabel)
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
