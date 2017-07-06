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

ConnectableObjectInput::ConnectableObjectInput() {
}

ConnectableObjectInput::~ConnectableObjectInput() {
}

uint32_t ConnectableObjectInput::GetNumDimensions() {
    throw std::logic_error("Cannot call ConnectableObjectInput::GetNumDimensions");
}

ConnectableObjectOutput::ConnectableObjectOutput() {
}

ConnectableObjectOutput::~ConnectableObjectOutput() {
}

uint32_t ConnectableObjectOutput::GetNumDimensions() {
    throw std::logic_error("Cannot call ConnectableObjectOutput::GetNumDimensions");
}

} // namespace bdhal
} // namespace pystorm
