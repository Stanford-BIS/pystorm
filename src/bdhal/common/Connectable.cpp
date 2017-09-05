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

uint32_t Connectable::GetConnectionSize() {
    throw std::logic_error("Cannot call Connectable::GetConnectionSize");
}

ConnectableInput::ConnectableInput() {
}

ConnectableInput::~ConnectableInput() {
}

uint32_t ConnectableInput::GetNumDimensions() {
    throw std::logic_error("Cannot call ConnectableInput::GetNumDimensions");
}

uint32_t ConnectableInput::GetConnectionSize() {
    throw std::logic_error("Cannot call ConnectableInput::GetConnectionSize");
}

ConnectableOutput::ConnectableOutput() {
}

ConnectableOutput::~ConnectableOutput() {
}

uint32_t ConnectableOutput::GetNumDimensions() {
    throw std::logic_error("Cannot call ConnectableOutput::GetNumDimensions");
}

uint32_t ConnectableOutput::GetConnectionSize() {
    throw std::logic_error("Cannot call ConnectableOutput::GetConnectionSize");
}

} // namespace bdhal
} // namespace pystorm
