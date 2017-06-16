#ifndef CONNECTABLE_OBJECT_H
#define CONNECTABLE_OBJECT_H

#include <iostream>

#include <stdint.h>

namespace pystorm {
namespace bdhal {

///
/// Base class for objects that can be connected via WeightedConnections.
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

} // namespace bdhal
} // namespace pystorm

#endif // ifndef CONNECTABLE_OBJECT_H
