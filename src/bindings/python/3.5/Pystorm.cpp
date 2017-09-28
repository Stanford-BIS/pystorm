#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>

#include <Pystorm.h>

#include <iostream>

PYBIND11_MAKE_OPAQUE(pystorm::bdhal::VecOfPools);
PYBIND11_MAKE_OPAQUE(pystorm::bdhal::VecOfBuckets);
PYBIND11_MAKE_OPAQUE(pystorm::bdhal::VecOfConnections);
PYBIND11_MAKE_OPAQUE(pystorm::bdhal::CorePars);
PYBIND11_MAKE_OPAQUE(pystorm::bdhal::VecOfInputs);
PYBIND11_MAKE_OPAQUE(pystorm::bdhal::VecOfOutputs);

namespace pystorm
{

namespace HAL = pystorm::bdhal;
namespace PYTHON = pybind11;


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
HAL::Weights<T>* makeWeights(PYTHON::array_t<T, PYTHON::array::c_style | PYTHON::array::forcecast> weights) {
    auto weights_buffer = weights.request();

    const int num_dims_allowed = 2;

    if (num_dims_allowed != weights_buffer.ndim){
        throw std::logic_error("Matrix must have 2 dimensions");
    }

    unsigned int num_rows = weights_buffer.shape[0]; unsigned int num_columns = weights_buffer.shape[1]; // copy the weight matrix
    HAL::Weights<T>* weightMatrix = new HAL::Weights<T>(static_cast<T*>(weights_buffer.ptr), num_rows,
        num_columns);

    return weightMatrix;
}

HAL::Connection* 
    (HAL::Network::*CreateConnection_1) (
    std::string name, HAL::ConnectableInput* in_object, 
    HAL::ConnectableOutput* out_object) = 
        &HAL::Network::CreateConnection;

template<typename T>
HAL::Connection* makeConnectionWithWeights (HAL::Network& net, std::string name,
    HAL::ConnectableInput* in_object, HAL::ConnectableOutput* out_object,
    PYTHON::array_t<T>& weights) {


    HAL::Weights<T>* weightMatrix = makeWeights<T>(weights);

    HAL::Connection* newConnection = net.CreateConnection(name, in_object, 
        out_object, weightMatrix);

    return newConnection;
} 


PYBIND11_MODULE(_PyStorm, m)
{

    //////////////////////////////////////////////////////////////////////
    //
    // Common network objects
    //
    // Objects that Networks are composed of
    //
    //////////////////////////////////////////////////////////////////////

    typedef HAL::Weights<uint32_t> Weights_32;

    PYTHON::bind_vector<HAL::VecOfPools>(m, "VecOfPools");
    PYTHON::bind_vector<HAL::VecOfBuckets>(m, "VecOfBuckets");

    PYTHON::bind_vector<HAL::VecOfConnections>(m, "VecOfConnections");

    PYTHON::enum_<HAL::CoreParsIndex>(m, "CoreParsIndex")
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

    PYTHON::bind_map<HAL::CorePars>(m, "CorePars");

    PYTHON::bind_vector<HAL::VecOfInputs>(m, "VecOfInputs");

    PYTHON::bind_vector<HAL::VecOfOutputs>(m, "VecOfOutputs");

    PYTHON::class_<HAL::Connectable>(m, "Connectable");

    PYTHON::class_<HAL::ConnectableOutput>(m, "ConnectableOutput");

    PYTHON::class_<HAL::ConnectableInput>(m, "ConnectableInput");

    PYTHON::class_<HAL::Pool, HAL::ConnectableInput, HAL::ConnectableOutput>(m, "Pool")
        .def(PYTHON::init<std::string, uint32_t, uint32_t, uint32_t, uint32_t>())
        .def(PYTHON::init<std::string,uint32_t,uint32_t>())
        .def("get_label", &HAL::Pool::GetLabel)
        .def("get_num_neurons", &HAL::Pool::GetNumNeurons)
        .def("get_num_dimensions", &HAL::Pool::GetNumDimensions)
        .def("get_width", &HAL::Pool::GetWidth)
        .def("get_height", &HAL::Pool::GetHeight)
        .def("set_size", &HAL::Pool::SetSize)
    ;

    PYTHON::class_<HAL::Bucket, HAL::ConnectableInput, HAL::ConnectableOutput>(m, "Bucket")
        .def(PYTHON::init<std::string, uint32_t>())
        .def("get_label",&HAL::Bucket::GetLabel, "Returns the Bucket label")
        .def("get_num_dimensions",&HAL::Bucket::GetNumDimensions, "Returns the number of dimensions")
    ;


    PYTHON::class_<HAL::Input, HAL::ConnectableInput>(m, "Input")
        .def(PYTHON::init<std::string, uint32_t>())
        .def("get_label",&HAL::Input::GetLabel, "Returns the Input label")
        .def("get_num_dimensions",&HAL::Input::GetNumDimensions, "Returns the number of dimensions")
    ;

    PYTHON::class_<HAL::Output, HAL::ConnectableOutput>(m, "Output")
        .def(PYTHON::init<std::string, uint32_t>())
        .def("get_label",&HAL::Output::GetLabel, "Returns the Output label")
        .def("get_num_dimensions",&HAL::Output::GetNumDimensions, "Returns the number of dimensions")
    ;

    PYTHON::class_<Weights_32>(m, "WeightsClass") // no init
        .def("get_num_rows",&Weights_32::GetNumRows)
        .def("get_num_columns",&Weights_32::GetNumColumns)
        .def("get_element",&Weights_32::GetElement)
        .def("set_element",&Weights_32::SetElement)
    ;
    m.def("Weights", [](PYTHON::array_t<uint32_t> weights) { return pystorm::makeWeights<uint32_t>(weights); }, PYTHON::return_value_policy::reference_internal);

    PYTHON::class_<HAL::Connection>
        (m, "Connection")
        .def(PYTHON::init<std::string, 
            HAL::ConnectableInput*, 
            HAL::ConnectableOutput*,
            Weights_32* >())
        .def(PYTHON::init<std::string, HAL::ConnectableInput*, 
            HAL::ConnectableOutput*>())
        .def("get_label",&HAL::Connection::GetLabel)
        .def("get_source",&HAL::Connection::GetSrc, PYTHON::return_value_policy::reference_internal)
        .def("get_dest",&HAL::Connection::GetDest, PYTHON::return_value_policy::reference_internal)
        .def("get_weights",&HAL::Connection::GetWeights, PYTHON::return_value_policy::reference_internal)
        .def("set_weights",&HAL::Connection::SetWeights, PYTHON::return_value_policy::reference_internal)
    ;

    PYTHON::class_<HAL::Network>(m, "Network")
        .def(PYTHON::init<std::string>())
        .def("get_name",&HAL::Network::GetName)
        .def("get_pools",&HAL::Network::GetPools, PYTHON::return_value_policy::automatic)
        .def("get_buckets",&HAL::Network::GetBuckets, PYTHON::return_value_policy::automatic)
        .def("get_connections",&HAL::Network::GetConnections, PYTHON::return_value_policy::automatic)
        .def("get_inputs",&HAL::Network::GetInputs, PYTHON::return_value_policy::automatic)
        .def("get_outputs",&HAL::Network::GetOutputs, PYTHON::return_value_policy::automatic)
        .def("create_pool", CreatePool_1, PYTHON::return_value_policy::reference_internal)
        .def("create_pool", CreatePool_2, PYTHON::return_value_policy::reference_internal)
        .def("create_bucket",&HAL::Network::CreateBucket, PYTHON::return_value_policy::reference_internal)
        .def("create_input",&HAL::Network::CreateInput, PYTHON::return_value_policy::reference_internal)
        .def("create_output",&HAL::Network::CreateOutput, PYTHON::return_value_policy::reference_internal)
        .def("create_connection", CreateConnection_1, PYTHON::return_value_policy::reference_internal,
            PYTHON::arg("self"), PYTHON::arg("in_obj"), PYTHON::arg("out_obj"))
        .def("create_connection", pystorm::makeConnectionWithWeights<uint32_t>, PYTHON::return_value_policy::reference_internal)
    ;

//////////////////////////////////////////////////////////////////////////////  
//                                                                              
// NetworkCreation Control functionality                                        
//                                                                              
// The following functions allow pystorm users to create networks that can         
// be mapped and loaded onto Braindrop                                          
//                                                                              
////////////////////////////////////////////////////////////////////////////// 

    m.def("create_network",
        HAL::Hal::CreateNetwork,
        PYTHON::return_value_policy::take_ownership);

//////////////////////////////////////////////////////////////////////////////
//
// NetworkMapping Control functionality
//
//////////////////////////////////////////////////////////////////////////////

    m.def("get_core_pars",
        HAL::Hal::GetCorePars,
        PYTHON::return_value_policy::take_ownership);

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
