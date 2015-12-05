#include <boost/python.hpp>
#include <Network.h>

BOOST_PYTHON_MODULE(pystorm)
{
    boost::python::class_<PyStorm::NetModels::Network>("Network",boost::python::init<std::string>())
        .def("GetName",&PyStorm::NetModels::Network::getName)
    ;
}
