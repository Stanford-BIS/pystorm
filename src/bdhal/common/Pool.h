#ifndef POOL_H
#define POOL_H

#include <iostream>

#include <stdint.h>
#include <assert.h>

#include <common/Connectable.h>
#include <common/HelperFuncs.h>

namespace pystorm {
namespace bdhal {
///
/// A Pool of neurons
///
class Pool : public ConnectableInput, public ConnectableOutput {
public:
    ///
    /// Default constructor 
    ///
    /// \param label Label assigned to Pool
    /// \param num_neurons Number of neurons assigned to Pool
    /// \param num_dims Number of dimensions assigned to Pool
    /// \param width width of the Pool
    /// \param height height of the Pool
    ///
    Pool(std::string label, 
        uint32_t num_neurons,
        uint32_t num_dims,
        uint32_t width,
        uint32_t height);

    ///
    /// Default constructor 
    ///
    /// This constructor sets the width and height to invalid values. If the 
    /// user tries to retrieve the width or height before setting them 
    /// (e.g. by calling the GetWidth or GetHeight methods), an exception
    /// will be thrown.
    ///
    /// \param label Label assigned to Pool
    /// \param num_neurons Number of neurons assigned to Pool
    /// \param num_dims Number of dimensions assigned to Pool
    ///
    Pool(std::string label, 
        uint32_t num_neurons,
        uint32_t num_dims);

    ~Pool(); 

    Pool(const Pool&) = delete;
    Pool(Pool&&) = delete;
    Pool& operator=(const Pool&) = delete;
    Pool& operator=(Pool&&) = delete;

    ///
    /// Returns the label assigned to Pool 
    ///
    std::string GetLabel() {
        return m_label;
    }

    ///
    /// Returns the number of neurons assigned to the Pool
    ///
    uint32_t GetNumNeurons() {
        return m_num_neurons;
    }

    ///
    /// Returns the number of dimensions assigned to the Pool
    ///
    uint32_t GetNumDimensions() {
        return m_num_dims;
    }

    ///
    /// Returns the Pool width
    ///
    /// Returns std::logic_error if width is not (e.g. by calling SetSize or
    /// by calling the correct constructor).
    ///
    uint32_t GetWidth();

    ///
    /// Returns the Pool height
    ///
    /// Returns std::logic_error if height is not (e.g. by calling SetSize or
    /// by calling the correct constructor).
    ///
    uint32_t GetHeight();

    ///
    /// Sets the Pool size
    ///
    /// Returns std::logic_error if the width times the height is not equal to
    /// the number of neurons in the Pool.
    ///
    void SetSize(uint32_t width, uint32_t height);

    /// Min/Max values
    static const uint32_t MIN_NEURONS = 4;
    static const uint32_t MAX_NEURONS = 4096;
    static const uint32_t MIN_NEURON_WIDTH_HEIGHT = 2;
    static const uint32_t MAX_NEURON_WIDTH_HEIGHT = 64;
    static const uint32_t MIN_DIMS    = 1;
    static const uint32_t MAX_DIMS    = 1024;

private:

    /// Pool label
    std::string m_label;

    /// Number of neurons 
    uint32_t m_num_neurons;

    /// Number of dimensions 
    uint32_t m_num_dims;

    /// Size of Pool
    /// First value of tuple is width
    /// Second value of tuple is height
    std::pair<uint32_t, uint32_t> m_poolSize;
};

} // namespace bdhal
} // namespace pystorm

#endif // ifndef POOL_H
