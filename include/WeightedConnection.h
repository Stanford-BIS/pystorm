#ifndef WEIGHTEDCONNECTION_H
#define WEIGHTEDCONNECTION_H

#include <iostream>
#include <stdint.h>
#include <ConnectableObject.h>
#include <Transform.h>

namespace PyStorm
{
namespace NetModels{
/// A projection from one ConnectableObject (i.e. Pool or StateSpace) to
/// another where the outputs of the first object can be linearly combined
/// prior to becoming inputs to the second object. This linear combination
/// is acheived through the use of a Transform object.
///
class WeightedConnection
{
public:
    /// \brief Default constructor
    ///        This class assumes ownership of all pointers (should use 
    ///        smart pointer objects then? yes; need to see how this plays
    ///         with boost python)
    ///
    /// \param src The source of the connection where data flow out of.
    /// \param dest The destination of the connection where data flows into.
    /// \param transform_matrix The transformation matrix. 
    ///
    WeightedConnection(std::string name, ConnectableObject* src, 
        ConnectableObject* dest, Transform<uint32_t>* transform_matrix) :
        m_name(name),
        m_src(src),
        m_dest(dest),
        m_transform(transform_matrix)
    {
        assert(nullptr != m_src);
        assert(nullptr != m_dest);
        assert(nullptr != m_transform);

        // maybe we should construct a name
        if (m_name.empty())
        {
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
    ~WeightedConnection()
    {
        if (nullptr != m_src)
        {
            delete m_src;
            m_src = 0;
        }

        if (nullptr != m_dest)
        {
            delete m_dest;
            m_dest = 0;
        }

        if (nullptr != m_transform)
        {
            delete m_transform;
            m_transform = 0;
        }
    }

    /// \brief Return the name assigned to this connection
    ///
    /// \return The name assigned to this connection. If no name is assigned,
    /// an empty string will be returned.
    std::string getName()
    {
        return m_name;
    }

    /// \brief Return the source object to this connection
    ///
    /// The source object to this connection.
    ///
    ConnectableObject* getSrc()
    {
        return m_src;
    }

    /// \brief Return the destination object from this connection
    ///
    /// The destination object from this connection.
    ///
    ConnectableObject* getDest()
    {
        return m_dest;
    }

    /// \brief Return the transform matrix associated with this connection.
    ///
    /// The transform matrix.
    ///
    Transform<uint32_t>* getTransformMatrix()
    {
        return m_transform;
    }

private:
    /// Name assigned to the object
    std::string m_name;

    /// Source of the connection
    ConnectableObject* m_src;

    /// Destination of the connection
    ConnectableObject* m_dest;

    /// Transform matrix
    Transform<uint32_t>* m_transform;
};

} // namespace NetModels
} // namespace PyStorm

#endif // ifndef WEIGHTEDCONNECTION_H
