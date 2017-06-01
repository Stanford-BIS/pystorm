#include <iostream>
#include <boost/python.hpp>


#include "Driver.h"
#include "DriverPy.h"

namespace pystorm
{
namespace bddriver
{
namespace driverifc
{

void testcall(const std::string& msg)
{
    // XXX this is just a stopgap while we fix the singleton business
    Driver * driver = new Driver();
    driver->testcall(msg);
}

BOOST_PYTHON_MODULE(DriverPy)
{
    boost::python::docstring_options doc_options(true,false,false);                            
    boost::python::def("testcall",testcall);
}

} // driverifc namespace
} // bddriver namespace
} // pystorm namespace
