#ifndef CONNECTION_H
#define CONNECTION_H

#include <iostream>
#include <stdint.h>
#include <typeinfo>

#include <common/Connectable.h>
#include <common/Weights.h>
#include <common/Pool.h>
#include <common/Input.h>
#include <common/Output.h>

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
        if (nullptr == m_src) {
            throw std::logic_error("Connection input cannot be null pointer");
        }

        if (nullptr == m_dest) {
            throw std::logic_error("Connection output cannot be null pointer");
        }

        if (nullptr == m_weights) {
            throw std::logic_error("Connection weights point to null pointer");
        }

        if (nullptr == m_weights) {
            throw std::logic_error("Connection weights point to null pointer");
        }

        // This check fails for connections out of Pools, since they
        //  should be sized based on the number of neurons
        // We should eventually fix this, but for now let's just remove
        //  the check
        //if(m_src->GetNumDimensions() != m_weights->GetNumColumns()) {
        //    throw std::logic_error("Connection input dimensions != Weight columns");
        //}

        if (m_dest->GetNumDimensions() != m_weights->GetNumRows()) {
            throw std::logic_error("Connection output dimensions != Weight rows");
        }

        if ((typeid(*src) == typeid(Pool)) && (typeid(*dest) == typeid(Pool))) {
            std::logic_error("Pools cannot be connected directly to Pools");
        }

        if ((typeid(*src) == typeid(Input)) 
            && (typeid(*dest) == typeid(Output))) {
            std::logic_error("Inputs cannot be connected directly to Outputs");
        }
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
        if (nullptr == m_src) {
            throw std::logic_error("Connection input cannot be null pointer");
        }

        if (nullptr == m_dest) {
            throw std::logic_error("Connection output cannot be null pointer");
        }

        if ((typeid(*src) == typeid(Pool)) && (typeid(*dest) == typeid(Pool))) {
            std::logic_error("Pools cannot be connected directly to Pools");
        }

        if ((typeid(*src) == typeid(Input)) 
            && (typeid(*dest) == typeid(Output))) {
            std::logic_error("Inputs cannot be connected directly to Outputs");
        }
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
    std::string GetLabel() {
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

    ///
    /// Set the weight matrix associated with this connection.
    ///
    void SetWeights(Weights<uint32_t>* newWeights) {
        m_weights = newWeights;
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
