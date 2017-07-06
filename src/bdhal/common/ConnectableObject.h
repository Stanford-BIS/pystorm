#ifndef CONNECTABLE_OBJECT_H
#define CONNECTABLE_OBJECT_H

#include <iostream>

#include <stdint.h>

namespace pystorm {
namespace bdhal {

///
/// Base class for objects that can be connected.
///
class ConnectableObject {
public:
    ConnectableObject();
    ~ConnectableObject();

    ConnectableObject(const ConnectableObject&) = delete;
    ConnectableObject(ConnectableObject&&) = delete;
    ConnectableObject& operator=(const ConnectableObject&) = delete;
    ConnectableObject& operator=(ConnectableObject&&) = delete;

    virtual uint32_t GetNumDimensions();

protected:
};

///
/// Base class for objects that can be inputs to a connection.
///
class ConnectableObjectInput {
public:
    ConnectableObjectInput();
    ~ConnectableObjectInput();

    ConnectableObjectInput(const ConnectableObjectInput&) = delete;
    ConnectableObjectInput(ConnectableObjectInput&&) = delete;
    ConnectableObjectInput& operator=(const ConnectableObjectInput&) = delete;
    ConnectableObjectInput& operator=(ConnectableObjectInput&&) = delete;

    virtual uint32_t GetNumDimensions();

protected:
};

///
/// Base class for objects that can be outputs of a connection.
///
class ConnectableObjectOutput {
public:
    ConnectableObjectOutput();
    ~ConnectableObjectOutput();

    ConnectableObjectOutput(const ConnectableObjectOutput&) = delete;
    ConnectableObjectOutput(ConnectableObjectOutput&&) = delete;
    ConnectableObjectOutput& operator=(const ConnectableObjectOutput&) = delete;
    ConnectableObjectOutput& operator=(ConnectableObjectOutput&&) = delete;

    virtual uint32_t GetNumDimensions();

protected:
};

} // namespace bdhal
} // namespace pystorm

#endif // ifndef CONNECTABLE_OBJECT_H
