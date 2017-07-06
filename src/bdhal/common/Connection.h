#ifndef CONNECTION_H
#define CONNECTION_H

#include <iostream>
#include <stdint.h>
#include <common/Connectable.h>
#include <common/Weights.h>

namespace pystorm {
namespace bdhal {
///
/// A projection from one ConnectableInput object to a ConnectableOutput
/// object where the outputs of the first object can be linearly combined
/// prior to becoming inputs to the second object. This linear combination
/// is acheived through the use of a Weights object.
///
class Connection {
public:
    ///
    /// Default constructor
    ///
    /// This class assumes ownership of all pointers (should use 
    /// smart pointer objects then? yes; need to see how this plays
    /// with boost python)
    ///
    /// \param name Label given to the connection object
    /// \param src The source of the connection where data flow out of.
    /// \param dest The destination of the connection where data flows into.
    /// \param transform_matrix The transformation matrix. 
    ///
    Connection(std::string name, ConnectableInput* src, 
        ConnectableOutput* dest, Weights<uint32_t>* transform_matrix) :
        m_name(name),
        m_src(src),
        m_dest(dest),
        m_transform(transform_matrix) {
        assert(nullptr != m_src);
        assert(nullptr != m_dest);
        assert(nullptr != m_transform);

        // maybe we should construct a name
        if (m_name.empty()) {
            m_name = "";
        }

        // check that the dimensions of the transform matrix match the dimensions
        // of the src and destination
    }

    ///
    /// Default constructor
    ///
    /// This class assumes ownership of all pointers (should use 
    /// smart pointer objects then? yes; need to see how this plays
    /// with boost python)
    ///
    /// \param name Label given to the connection object
    /// \param src The source of the connection where data flow out of.
    /// \param dest The destination of the connection where data flows into.
    /// \param transform_matrix The transformation matrix. 
    ///
    Connection(std::string name, ConnectableInput* src, 
        ConnectableOutput* dest) :
        m_name(name),
        m_src(src),
        m_dest(dest),
        m_transform(nullptr) {
        assert(nullptr != m_src);
        assert(nullptr != m_dest);

        // maybe we should construct a name
        if (m_name.empty()) {
            m_name = "";
        }

        // check that the dimensions of the transform matrix match the dimensions
        // of the src and destination
    }

    /// \brief Destructor
    ///
    /// NOTE: Since this class takes ownership of the transform, it will 
    /// delete memeory allocated for the transform matrix. Ensure that no
    /// other code are using the transform memory memory owned by this 
    /// class after deleting this (I will add smart pointers soon).
    ~Connection() {
        if (nullptr != m_transform)
        {
            delete m_transform;
            m_transform = 0;
        }
    }

    Connection(const Connection &) = delete;
    Connection(Connection&&) = delete;
    Connection& operator=(const Connection &) = delete;
    Connection& operator=(Connection&&) = delete;

    /// \brief Return the name assigned to this connection
    ///
    /// \return The name assigned to this connection. If no name is assigned,
    /// an empty string will be returned.
    std::string getName() {
        return m_name;
    }

    /// \brief Return the source object to this connection
    ///
    /// The source object to this connection.
    ///
    ConnectableInput* getSrc() {
        return m_src;
    }

    /// \brief Return the destination object from this connection
    ///
    /// The destination object from this connection.
    ///
    ConnectableOutput* getDest() {
        return m_dest;
    }

    /// \brief Return the transform matrix associated with this connection.
    ///
    /// The transform matrix.
    ///
    Weights<uint32_t>* getWeights() {
        return m_transform;
    }

private:
    /// Name assigned to the object
    std::string m_name;

    /// Source of the connection
    ConnectableInput* m_src;

    /// Destination of the connection
    ConnectableOutput* m_dest;

    /// Weights matrix
    Weights<uint32_t>* m_transform;
};

} // namespace bdhal
} // namespace pystorm

#endif // ifndef CONNECTION_H
