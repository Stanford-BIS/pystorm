#ifndef COMMSOFT_H
#define COMMSOFT_H

#include <vector>
#include <deque>
#include <iostream>
#include <thread>
#include <mutex>
#include <memory>

#include <cstring>

#include "common/TSQueue.h"
#include "Comm.h"
#include "Emulator.h"

namespace pystorm
{
namespace bddriver
{
namespace bdcomm
{

class CommSoft : public Comm, EmulatorClientIfc
{
public:
    CommSoft(std::string& in_file_name, std::string& out_file_name);
    ~CommSoft();
    CommSoft(const CommSoft&) = delete;

    // Comm interface

    /* StartStreaming
     *
     * Tells Comm's software emulator to start streaming packets to Comm
     *
     */ 
    virtual void StartStreaming();

    /* StopStreaming
     *
     * Tells Comm's software emulator to stop streaming packets to Comm
     *
     */ 
    virtual void StopStreaming();

    /* Write
     * 
     * Moves a unique ptr to a CommWordStream to the Comm class
     * This is a non-blocking call, therefore, the stream is not
     * guaranteed to have been sent to the software simulator when
     * the method call returns.
     */
    virtual void Write(std::unique_ptr<COMMWordStream> wordStream);

    /* Read
     * 
     * Moves a unique ptr to a CommWordStream or returns nullptr
     * This is a non-blocking call, therefore, the stream is not
     * guaranteed to have been sent to the software simulator when
     * the method call returns.
     */
    virtual std::unique_ptr<COMMWordStream> Read();

    // EmulatorCallbackIfc interface

    virtual void ReadCallback(std::unique_ptr<EmulatorCallbackData> cb);
    virtual void WriteCallback(std::unique_ptr<EmulatorCallbackData> cb);

protected:
    void CommSoftController();

    // Read packets in from file and place them on read queue
    void ReadPacketsIn();

    // Read packets from write queue and place them into file
    void WritePacketsOut();

    TSQueue<std::unique_ptr<COMMWordStream> > m_read_queue;
    TSQueue<std::unique_ptr<COMMWordStream> > m_write_queue;

    std::thread m_control_thread;

    std::unique_ptr<Emulator> m_emulator;

};

} // bdcomm namespace
} // bddriver namespace
} // pystorm namespace
#endif
