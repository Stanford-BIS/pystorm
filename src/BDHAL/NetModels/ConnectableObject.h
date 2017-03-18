#ifndef CONNECTABLE_OBJECT_H
#define CONNECTABLE_OBJECT_H

#include <iostream>

#include <stdint.h>

namespace pystorm
{
namespace netmodels{
/// Base class for objects that can be connected via WeightedConnections.
///
class ConnectableObject
{
public:
    /// \brief Default constructor
    ///
    ConnectableObject()
    {

    }

    /// \brief Destructor
    ///
    ~ConnectableObject()
    {

    }
protected:
};

} // namespace netmodels
} // namespace pystorm

#endif // ifndef CONNECTABLE_OBJECT_H
