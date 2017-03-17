#ifndef SHMEMSTREAMHANDLE_H
#define SHMEMSTREAMHANDLE_H

#include <iostream>

namespace pystorm
{
/// Handle to a shared memory location where spikes and decoded values are 
/// streamed to
class SHMemStreamHandle 
{
public:
    /// \brief Constructor using assigned shared mem name
    ///
    /// the name will be the handle id
    SHMemStreamHandle(std::string shared_mem_name) :
        m_handleId(shared_mem_name)
    {
    }

    /// \brief Destructor
    ///
    ~SHMemStreamHandle()
    {

    }

    /// \brief Close the shared memory stream
    ///
    void close()
    {
    }

    /// \brief return the handle id
    ///
    /// \return Handle id
    ////
    const std::string& getHandleId()
    {
        return m_handleId;
    }

    /// \brief Close all shared memory streams
    ///
    static void closeAll()
    {
    }

private:
    /// Handle id
    std::string m_handleId;

    static std::vector<SHMemStreamHandle*> g_shMemStreamHandles;
};

} // namespace pystorm

#endif // ifndef SHMEMSTREAMHANDLE_H
