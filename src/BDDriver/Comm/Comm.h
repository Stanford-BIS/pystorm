#ifndef COMM_H
#define COMM_H

#include <vector>
#include <deque>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>

#include "common/TSQueue.h"

namespace PyStorm
{
namespace BDDriver
{
namespace BDComm
{

enum class CommStreamState { STARTED=0, STOPPED=1 };

template <typename Enumeration>
auto as_integer(Enumeration const value)
    -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

typedef unsigned char COMMWord;
typedef std::vector<COMMWord> COMMWordStream;
typedef std::deque<std::unique_ptr<COMMWordStream> > COMMWordStreamQueue;

// Comm Interface
class Comm
{
public:
    virtual void StartStreaming() = 0;
    virtual void StopStreaming() = 0;
    CommStreamState GetStreamState()
    {
        return m_state;
    };

    void SetStreamState(CommStreamState new_state)
    {
        m_state = new_state;
    };

    virtual void Write(std::unique_ptr<COMMWordStream> wordStream) = 0;
    virtual std::unique_ptr<COMMWordStream> Read() = 0;

protected:

    std::atomic<CommStreamState> m_state;
    std::recursive_mutex m_state_mutex;

};

} // BDComm namespace
} // BDDriver namespace
} // PyStorm namespace
#endif
