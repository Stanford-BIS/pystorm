#ifndef CONNECTION_H
#define CONNECTION_H

#include <iostream>
#include <stdint.h>
#include <common/Connectable.h>
#include <common/Weights.h>

namespace pystorm {
namespace bdhal {
///
/// A projection from a ConnectableInput object to a ConnectableOutput
/// object where the values of first object can be weighted, using
/// a Weight object, prior to being received by the second object.
///
class Connection {
public:
    ///
    /// Default constructor
    ///
    /// \param label Label given to the connection object
    /// \param src The source of the connection where data flow out of.
    /// \param dest The destination of the connection where data flows into.
    /// \param weight_matrix The weight matrix. 
    ///
    Connection(std::string label, ConnectableInput* src, 
        ConnectableOutput* dest, Weights<uint32_t>* weight_matrix) :
        m_label(label),
        m_src(src),
        m_dest(dest),
        m_weights(weight_matrix) {
        assert(nullptr != m_src);
        assert(nullptr != m_dest);
        assert(nullptr != m_weights);

        if (m_label.empty()) {
            m_label = "";
        }

        assert(m_src->GetNumDimensions() == m_weights->GetNumColumns());
        assert(m_dest->GetNumDimensions() == m_weights->GetNumRows());
    }

    ///
    /// Default constructor
    ///
    /// \param label Label given to the connection object
    /// \param src The source of the connection where data flow out of.
    /// \param dest The destination of the connection where data flows into.
    /// \param weight_matrix The weight matrix. 
    ///
    Connection(std::string label, ConnectableInput* src, 
        ConnectableOutput* dest) :
        m_label(label),
        m_src(src),
        m_dest(dest),
        m_weights(nullptr) {
        assert(nullptr != m_src);
        assert(nullptr != m_dest);

        if (m_label.empty()) {
            m_label = "";
        }

        assert(m_src->GetNumDimensions() == m_dest->GetNumDimensions());
    }

    ///
    /// Destructor
    ///
    /// NOTE: Since this class takes ownership of the weights, it will 
    /// delete memory allocated for the weight matrix. Ensure that no
    /// other code are using the weight memory owned by this 
    /// class after deleting this (consider smart pointers).
    ///
    ~Connection() {
        if (nullptr != m_weights)
        {
            delete m_weights;
            m_weights = nullptr;
        }
    }

    Connection(const Connection &) = delete;
    Connection(Connection&&) = delete;
    Connection& operator=(const Connection &) = delete;
    Connection& operator=(Connection&&) = delete;

    ///
    /// Return the label assigned to this connection
    ///
    /// \return The label assigned to this connection. If no label is assigned,
    /// an empty string will be returned.
    ///
    std::string GetName() {
        return m_label;
    }

    ///
    /// Return the source object to this connection
    ///
    /// The source object to this connection.
    ///
    ConnectableInput* GetSrc() {
        return m_src;
    }

    ///
    /// Return the destination object from this connection
    ///
    /// The destination object from this connection.
    ///
    ConnectableOutput* GetDest() {
        return m_dest;
    }

    ///
    /// Return the weight matrix associated with this connection.
    ///
    /// The weight matrix.
    ///
    Weights<uint32_t>* GetWeights() {
        return m_weights;
    }

private:
    /// Name assigned to the object
    std::string m_label;

    /// Source of the connection
    ConnectableInput* m_src;

    /// Destination of the connection
    ConnectableOutput* m_dest;

    /// Weights matrix
    Weights<uint32_t>* m_weights;
};

} // namespace bdhal
} // namespace pystorm

#endif // ifndef CONNECTION_H
