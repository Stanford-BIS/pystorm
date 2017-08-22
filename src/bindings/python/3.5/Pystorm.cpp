#include <boost/python.hpp>
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/register_ptr_to_python.hpp>
#include <boost/python/enum.hpp>
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

    int num_dims_allowed = 2;

    if (false == PyArray_CheckExact(weights.ptr())) {
        std::logic_error("Weights must be a numpy array");
    }

    PyArrayObject *arrayobj_ptr = (PyArrayObject*) PyArray_FROM_O(weights.ptr());

    if (num_dims_allowed != PyArray_NDIM(arrayobj_ptr)){
        throw std::logic_error("Matrix must have 2 dimensions");
    }

    npy_intp* dims = PyArray_DIMS(arrayobj_ptr);

    npy_intp num_rows = dims[0]; npy_intp num_columns = dims[1]; // copy the weight matrix
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

template<typename T>
HAL::Weights<T>* makeNullWeights(int in_dims, int out_dims) {

    T* weights_ptr = (T*) std::calloc((in_dims*out_dims),sizeof(T));

    HAL::Weights<T>* weightMatrix = new HAL::Weights<T>(weights_ptr, out_dims,
        in_dims);

    for (unsigned int row = 0; row < weightMatrix->GetNumRows(); row++)
    {
        for (unsigned int col = 0; col < weightMatrix->GetNumColumns(); col++)
        {
            weightMatrix->SetElement(row, col, 0);
        }
    }

    return weightMatrix;
}

HAL::Connection* 
    (HAL::Network::*CreateConnection_1) (
    std::string name, HAL::ConnectableInput* in_object, 
    HAL::ConnectableOutput* out_object) = 
        &HAL::Network::CreateConnection;

template<typename T>
HAL::Connection* makeConnectionWithoutWeights (HAL::Network& net, std::string name,
    HAL::ConnectableInput* in_object, HAL::ConnectableOutput* out_object) {

    HAL::Weights<T>* weightMatrix = makeNullWeights<T>(in_object->GetNumDimensions(), 
        out_object->GetNumDimensions());

    HAL::Connection* newConnection = net.CreateConnection(name, in_object, 
        out_object, weightMatrix);

    return newConnection;
} 

template<typename T>
HAL::Connection* makeConnectionWithWeights (HAL::Network& net, std::string name,
    HAL::ConnectableInput* in_object, HAL::ConnectableOutput* out_object,
    PYTHON::object& weights) {


    HAL::Weights<T>* weightMatrix = makeWeights<T>(weights);

    HAL::Connection* newConnection = net.CreateConnection(name, in_object, 
        out_object, weightMatrix);

    return newConnection;
} 

#if PY_MAJOR_VERSION >= 3
    int
#else
    void
#endif
init_numpy() {
    import_array();
#if PY_MAJOR_VERSION >= 3
    return 0;
#endif
}

BOOST_PYTHON_MODULE(Pystorm)
{

    init_numpy();

    PYTHON::numeric::array::set_module_and_type("ndtype","ndarray");

    PYTHON::implicitly_convertible<HAL::Input*, HAL::ConnectableInput*>();
    PYTHON::implicitly_convertible<HAL::Output*, HAL::ConnectableOutput*>();
    PYTHON::implicitly_convertible<HAL::Pool*, HAL::ConnectableInput*>();
    PYTHON::implicitly_convertible<HAL::Pool*, HAL::ConnectableOutput*>();
    PYTHON::implicitly_convertible<HAL::Bucket*, HAL::ConnectableInput*>();
    PYTHON::implicitly_convertible<HAL::Bucket*, HAL::ConnectableOutput*>();

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

    PYTHON::class_<HAL::VecOfBuckets>("VecOfBuckets")
        .def(PYTHON::vector_indexing_suite<HAL::VecOfBuckets,true>() )
    ;

    PYTHON::class_<HAL::VecOfConnections>("VecOfConnections")
        .def(PYTHON::vector_indexing_suite<HAL::VecOfConnections,true>()
         )
    ;

    PYTHON::enum_<HAL::CoreParsIndex>("CoreParsIndex")
        .value("MM_height", HAL::CoreParsIndex::MM_height)
        .value("MM_width", HAL::CoreParsIndex::MM_width)
        .value("AM_size", HAL::CoreParsIndex::AM_size)
        .value("TAT_size", HAL::CoreParsIndex::TAT_size)
        .value("NeuronArray_height", HAL::CoreParsIndex::NeuronArray_height)
        .value("NeuronArray_width", HAL::CoreParsIndex::NeuronArray_width)
        .value("NeuronArray_pool_size", 
            HAL::CoreParsIndex::NeuronArray_pool_size)
        .value("num_threshold_levels", HAL::CoreParsIndex::num_threshold_levels)
        .value("min_threshold_value", HAL::CoreParsIndex::min_threshold_value)
        .value("max_weight_value", HAL::CoreParsIndex::max_weight_value)
        .value("NeuronArray_neurons_per_tap", 
            HAL::CoreParsIndex::NeuronArray_neurons_per_tap)
    ;

    PYTHON::class_<HAL::CorePars>("CorePars")
        .def(PYTHON::map_indexing_suite<HAL::CorePars,true>()
         )
    ;

    PYTHON::class_<HAL::VecOfInputs>("VecOfInputs")
        .def(PYTHON::vector_indexing_suite<HAL::VecOfInputs,true>()
         )
    ;

    PYTHON::class_<HAL::VecOfOutputs>("VecOfOutputs")
        .def(PYTHON::vector_indexing_suite<HAL::VecOfOutputs,true>()
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
        .def("get_label", &HAL::Pool::GetLabel)
        .def("get_num_neurons", &HAL::Pool::GetNumNeurons)
        .def("get_num_dimensions", &HAL::Pool::GetNumDimensions)
        .def("get_width", &HAL::Pool::GetWidth)
        .def("get_height", &HAL::Pool::GetHeight)
        .def("set_size", &HAL::Pool::SetSize)
    ;

    PYTHON::class_<HAL::Bucket, HAL::Bucket*, boost::noncopyable,
        PYTHON::bases<HAL::ConnectableInput, HAL::ConnectableOutput> >("Bucket",
        PYTHON::init<std::string, uint32_t>())
        .def("get_label",&HAL::Bucket::GetLabel
            , "Returns the Bucket label"
            , PYTHON::args("self"))
        .def("get_num_dimensions",&HAL::Bucket::GetNumDimensions
            , "Returns the number of dimensions"
            , PYTHON::args("self"))
    ;

    PYTHON::class_<HAL::Input, HAL::Input*, boost::noncopyable,
        PYTHON::bases<HAL::ConnectableInput> >("Input",
        PYTHON::init<std::string, uint32_t>())
        .def("get_label",&HAL::Input::GetLabel
            , "Returns the Input label"
            , PYTHON::args("self"))
        .def("get_num_dimensions",&HAL::Input::GetNumDimensions
            , "Returns the number of dimensions"
            , PYTHON::args("self"))
    ;

    PYTHON::class_<HAL::Output, HAL::Output*, boost::noncopyable,
        PYTHON::bases<HAL::ConnectableOutput> >("Output",
        PYTHON::init<std::string, uint32_t>())
        .def("get_label",&HAL::Output::GetLabel
            , "Returns the Output label"
            , PYTHON::args("self"))
        .def("get_num_dimensions",&HAL::Output::GetNumDimensions
            , "Returns the number of dimensions"
            , PYTHON::args("self"))
    ;

    PYTHON::class_<Weights_32, Weights_32*, boost::noncopyable>
        ("Weights", PYTHON::no_init)
        .def("__init__",
            PYTHON::make_constructor(pystorm::makeWeights<uint32_t>))
        .def("get_num_rows",&Weights_32::GetNumRows)
        .def("get_num_columns",&Weights_32::GetNumColumns)
        .def("get_element",&Weights_32::GetElement)
        .def("set_element",&Weights_32::SetElement)
    ;

    PYTHON::class_<HAL::Connection, HAL::Connection*,
        boost::noncopyable>
        ("Connection",PYTHON::init<std::string, 
        HAL::ConnectableInput*, 
        HAL::ConnectableOutput*,
        HAL::Weights<uint32_t>* >())
        .def(PYTHON::init<std::string, HAL::ConnectableInput*, 
            HAL::ConnectableOutput*>())
        .def("get_label",&HAL::Connection::GetLabel)
        .def("get_source",&HAL::Connection::GetSrc,PYTHON::return_internal_reference<>())
        .def("get_dest",&HAL::Connection::GetDest,PYTHON::return_internal_reference<>())
        .def("get_weights",&HAL::Connection::GetWeights,PYTHON::return_internal_reference<>())
        .def("set_weights",&HAL::Connection::SetWeights,PYTHON::return_internal_reference<>())
    ;

    PYTHON::class_<HAL::Network, boost::noncopyable>("Network",
        PYTHON::init<std::string>())
        .def("get_name",&HAL::Network::GetName)
        .def("get_pools",&HAL::Network::GetPools,
            PYTHON::return_value_policy<PYTHON::reference_existing_object>())
        .def("get_buckets",&HAL::Network::GetBuckets,
            PYTHON::return_value_policy<PYTHON::reference_existing_object>())
        .def("get_connections",&HAL::Network::GetConnections,
            PYTHON::return_value_policy<PYTHON::reference_existing_object>())
        .def("get_inputs",&HAL::Network::GetInputs,
            PYTHON::return_value_policy<PYTHON::reference_existing_object>())
        .def("get_outputs",&HAL::Network::GetOutputs,
            PYTHON::return_value_policy<PYTHON::reference_existing_object>())
        .def("create_pool", CreatePool_1, 
            PYTHON::return_internal_reference<>())
        .def("create_pool", CreatePool_2, 
            PYTHON::return_internal_reference<>())
        .def("create_bucket",&HAL::Network::CreateBucket, 
            PYTHON::return_internal_reference<>())
        .def("create_input",&HAL::Network::CreateInput, 
            PYTHON::return_internal_reference<>())
        .def("create_output",&HAL::Network::CreateOutput, 
            PYTHON::return_internal_reference<>())
        .def("create_connection", pystorm::makeConnectionWithoutWeights<uint32_t>,
            PYTHON::return_internal_reference<>(),
            PYTHON::args("self","in_obj","out_obj"))
        .def("create_connection", pystorm::makeConnectionWithWeights<uint32_t>,
            PYTHON::return_internal_reference<>())
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

    def("create_network",
        HAL::Hal::CreateNetwork,
        PYTHON::return_value_policy<PYTHON::reference_existing_object>());

//////////////////////////////////////////////////////////////////////////////
//
// NetworkMapping Control functionality
//
//////////////////////////////////////////////////////////////////////////////

    def("get_core_pars",
        HAL::Hal::GetCorePars,
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
