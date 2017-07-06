#ifndef CONNECTABLE_H
#define CONNECTABLE_H

#include <iostream>

#include <stdint.h>

namespace pystorm {
namespace bdhal {

///
/// Base class for objects that can be connected.
///
class Connectable {
public:
    Connectable();
    ~Connectable();
Connectable(const Connectable&) = delete;
    Connectable(Connectable&&) = delete;
    Connectable& operator=(const Connectable&) = delete;
    Connectable& operator=(Connectable&&) = delete;

    virtual uint32_t GetNumDimensions();

protected:
};

///
/// Base class for objects that can be inputs to a connection.
///
class ConnectableInput {
public:
    ConnectableInput();
    ~ConnectableInput();

    ConnectableInput(const ConnectableInput&) = delete;
    ConnectableInput(ConnectableInput&&) = delete;
    ConnectableInput& operator=(const ConnectableInput&) = delete;
    ConnectableInput& operator=(ConnectableInput&&) = delete;

    virtual uint32_t GetNumDimensions();

protected:
};

///
/// Base class for objects that can be outputs of a connection.
///
class ConnectableOutput {
public:
    ConnectableOutput();
    ~ConnectableOutput();

    ConnectableOutput(const ConnectableOutput&) = delete;
    ConnectableOutput(ConnectableOutput&&) = delete;
    ConnectableOutput& operator=(const ConnectableOutput&) = delete;
    ConnectableOutput& operator=(ConnectableOutput&&) = delete;

    virtual uint32_t GetNumDimensions();

protected:
};

} // namespace bdhal
} // namespace pystorm

#endif // ifndef CONNECTABLE_H
