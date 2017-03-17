#include <iostream>

#include "Driver.h"

namespace pystorm
{
namespace BDDriver
{

Driver& Driver::getInstance()
{
    // In C++11, if control from two threads occurs concurrently, execution
    // shall wait during static variable initialization, therefore, this is 
    // thread safe
    static Driver m_instance;
    return m_instance;
}

Driver::Driver()
{
}

Driver::~Driver()
{
}

void Driver::testcall(const std::string& msg)
{
    std::cout << msg << std::endl;
}

} // BDDriver namespace
} // pystorm namespace
