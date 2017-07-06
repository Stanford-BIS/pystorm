#include <common/Connectable.h>

namespace pystorm {
namespace bdhal {

Connectable::Connectable() {
}

Connectable::~Connectable() {
}

uint32_t Connectable::GetNumDimensions() {
    throw std::logic_error("Cannot call Connectable::GetNumDimensions");
}

ConnectableInput::ConnectableInput() {
}

ConnectableInput::~ConnectableInput() {
}

uint32_t ConnectableInput::GetNumDimensions() {
    throw std::logic_error("Cannot call ConnectableInput::GetNumDimensions");
}

ConnectableOutput::ConnectableOutput() {
}

ConnectableOutput::~ConnectableOutput() {
}

uint32_t ConnectableOutput::GetNumDimensions() {
    throw std::logic_error("Cannot call ConnectableOutput::GetNumDimensions");
}

} // namespace bdhal
} // namespace pystorm
