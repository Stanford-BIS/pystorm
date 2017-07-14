#include <common/Pool.h>
#include <algorithm>
#include <cmath>

namespace pystorm {
namespace bdhal {

const uint32_t Pool::MIN_NEURONS;
const uint32_t Pool::MAX_NEURONS;
const uint32_t Pool::MIN_NEURON_WIDTH_HEIGHT;
const uint32_t Pool::MAX_NEURON_WIDTH_HEIGHT;
const uint32_t Pool::MIN_DIMS;
const uint32_t Pool::MAX_DIMS;

Pool::Pool(std::string label, 
        uint32_t num_neurons,
        uint32_t num_dims,
        uint32_t width,
        uint32_t height) : 
        m_label(label), 
        m_num_neurons(num_neurons),
        m_num_dims(num_dims) {
    if (m_label.size() == 0) {
        throw std::logic_error("Label cannot be an empty string");
    }
    ValueIsInRange(m_num_neurons, MIN_NEURONS, MAX_NEURONS, "Neurons out of range");
    uint32_t maxDimensions = std::min(MAX_DIMS, static_cast<uint32_t>(floor(m_num_neurons / 4)));
    ValueIsInRange(m_num_dims, MIN_DIMS, maxDimensions, "Dimensions out of range");
    ValueIsInRange(width, MIN_NEURON_WIDTH_HEIGHT, MAX_NEURON_WIDTH_HEIGHT, "Width is out of range");
    ValueIsInRange(height, MIN_NEURON_WIDTH_HEIGHT, MAX_NEURON_WIDTH_HEIGHT, "Height is out of range");
    if ((width * height) != m_num_neurons) {
        throw std::logic_error("width * Height must equal number of neurons");
    }
    m_poolSize.first = width;
    m_poolSize.second = height;
}

Pool::Pool(std::string label, 
        uint32_t num_neurons,
        uint32_t num_dims) :
        m_label(label), 
        m_num_neurons(num_neurons),
        m_num_dims(num_dims) {
    if (m_label.size() == 0) {
        throw std::logic_error("Label cannot be an empty string");
    }
    ValueIsInRange(m_num_neurons, MIN_NEURONS, MAX_NEURONS, "Neurons out of range");
    uint32_t maxDimensions = std::min(MAX_DIMS, static_cast<uint32_t>(floor(m_num_neurons / 4)));
    ValueIsInRange(m_num_dims, MIN_DIMS, maxDimensions, "Dimensions out of range");

    m_poolSize.first = 0;
    m_poolSize.second = 0;
}

Pool::~Pool() {
}

uint32_t Pool::GetWidth() {
    if (m_poolSize.first == 0) {
        throw std::logic_error("Pool::GetWidth called before Pool width was set.");
    }
    return m_poolSize.first;
}

uint32_t Pool::GetHeight() {
    if (m_poolSize.second == 0) {
        throw std::logic_error("Pool::GetHeight called before Pool height was set.");
    }
    return m_poolSize.second;
}

void Pool::SetSize(uint32_t width, uint32_t height) {
    if ((width*height) != m_num_neurons) {
        throw std::logic_error("width * Height must equal number of neurons");
    }

    m_poolSize.first = width;
    m_poolSize.second = height;
}

}
}
