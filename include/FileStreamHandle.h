#ifndef FILESTREAMHANDLE_H
#define FILESTREAMHANDLE_H

#include <iostream>

namespace PyStorm
{
/// Handle to a file where spikes or decdoed values are streamed to
///
class FileStreamHandle 
{
public:
    /// \brief Default constructor
    ///
    /// \param fileName Name of the file to open
    ///
    /// Throws an exception if the file cannot be opened for streaming
    ///
    FileStreamHandle(std::string fileName)
    {
        // how do we assign handle id ?
    }

    /// \brief Destructor
    ///
    ~FileStreamHandle()
    {

    }

    /// \brief Flush the file streams buffer contents
    ///
    /// Should be called before closing the stream
    /// Throws an exception if the file stream is already closed
    ///
    void flush()
    {
    }

    /// \brief Close the file stream
    ///
    /// Consider calling flush before close to ensure all data is written to
    /// file
    /// Throws an exception if the file stream is already closed
    ///
    void close()
    {
    }

    /// \brief return the handle id
    ///
    /// \return Handle id
    ///
    const std::string& getHandleId()
    {
        return m_handleId;
    }

    /// \brief Flush all filestreams
    ///
    static void flushAll()
    {
    }

    /// \brief Close all open file streams
    ///
    static void closeAll()
    {
    }
private:
    /// Handle id
    std::string m_handleId;

    static std::vector<FileStreamHandle*> g_fileStreamHandles;
};

} // namespace PyStorm

#endif // ifndef FILESTREAMHANDLE_H
