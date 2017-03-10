#ifndef DRIVER_H
#define DRIVER_H   

#include <memory>

namespace PyStorm
{
namespace BDDriver
{
/**
 * \class Driver is a singleton; There should only be one instance of this.
 *
 */
class Driver
{
public:
    // Return a global instance of BDDriver
    static Driver& getInstance();

    // Just a test call
    void testcall(const std::string& msg);

protected:
    Driver();

    ~Driver();
};

} // BDDriver namespace
} // PyStorm namespace

#endif
