#include <stdexcept>

#include <common/Output.h>

namespace pystorm {
namespace bdhal {

Output::Output(std::string label, uint32_t n_dims) : 
        m_label(label),
        m_dims(n_dims) {
    if (m_label.size() == 0) {
        throw std::logic_error("Label size must be greater than 0");
    }

    if (m_dims <= 0) {
        throw std::out_of_range("Dimensions must be greater than 0");
    }
}

Output::~Output() {
}

}
}
