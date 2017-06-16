#include <common/Pool.h>

namespace pystorm {
namespace bdhal {

Pool::Pool(std::string label, 
        uint32_t num_neurons,
        uint32_t num_dims,
        uint32_t width,
        uint32_t height) : 
        m_label(label), 
        m_num_neurons(num_neurons),
        m_num_dims(num_dims),
        m_width(width),
        m_height(height) {
    if (m_label.size() == 0) {
        throw std::logic_error("Label cannot be an empty string");
    }
    ValueIsInRange(m_num_neurons, MIN_NEURONS, MAX_NEURONS, "Neurons out of range");
    ValueIsInRange(m_num_dims, MIN_DIMS, MAX_DIMS, "Dimensions out of range");
    ValueIsInRange(m_width, MIN_NEURON_WIDTH_HEIGHT, MAX_NEURON_WIDTH_HEIGHT, "Width is out of range");
    ValueIsInRange(m_height, MIN_NEURON_WIDTH_HEIGHT, MAX_NEURON_WIDTH_HEIGHT, "Height is out of range");
    if ((m_width*height) != m_num_neurons) {
        throw std::logic_error("width * Height must equal number of neurons");
    }
}

Pool::~Pool() {
}

}
}
