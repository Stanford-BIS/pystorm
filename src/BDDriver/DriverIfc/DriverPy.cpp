#include <iostream>
#include <boost/python.hpp>


#include "DriverPy.h"

namespace pystorm
{
namespace bddriver
{
namespace DriverIfc
{

void testcall(const std::string& msg)
{
    Driver::getInstance().testcall(msg);
}

BOOST_PYTHON_MODULE(DriverPy)
{
    boost::python::docstring_options doc_options(true,false,false);                            
    boost::python::def("testcall",testcall);
}

} // DriverIfc namespace
} // bddriver namespace
} // pystorm namespace
