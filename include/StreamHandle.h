#ifndef STREAMHANDLE_H
#define STREAMHANDLE_H

#include <iostream>

namespace PyStorm
{
/// Handle used to retrieve a stream of spikes and decoded values from the 
/// DataFlowController (first mention of this; need to flessh this out in
/// design)
class StreamHandle 
{
public:
    /// \brief Defeault constructor
    ///
    StreamHandle()
    {
    }

    /// \brief Destructor
    ~StreamHandle()
    {

    }

    /// \brief Close the handle?
    void close()
    {
    }

    /// \brief return the handle id
    std::string& getHandleId()
    {
        return m_handleId;
    }

    /// \brief Close all input streams?
    static void closeAll()
    {
    }

private:
    /// handle id
    std::string m_handleId;

    // All stream handles
    static std::vector<StreamHandle*> g_streamHandles;
};

} // namespace PyStorm

#endif // ifndef STREAMHANDLE_H
