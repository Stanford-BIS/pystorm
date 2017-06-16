#include <common/ConnectableObject.h>

namespace pystorm {
namespace bdhal {

ConnectableObject::ConnectableObject() {
}
ConnectableObject::~ConnectableObject() {
}

uint32_t ConnectableObject::GetNumDimensions() {
    throw std::logic_error("Cannot call ConnectableObject::GetNumDimensions");
}

} // namespace bdhal
} // namespace pystorm
