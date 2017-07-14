#include "Connectable.h"

#include <iostream>

#include "gtest/gtest.h"

namespace pystorm {
namespace bdhal {

TEST(TESTConnectable, testConstructionValidParams) {
    Connectable * conn = nullptr;

    EXPECT_NO_THROW(conn = new Connectable());
    delete conn;
}

TEST(TESTConnectable, testCallGetNumDimensions) {
    Connectable * conn = new Connectable();

    EXPECT_THROW(conn->GetNumDimensions(),std::logic_error);
    delete conn;
}

TEST(TESTConnectableInput, testConstructionValidParams) {
    ConnectableInput * conn = nullptr;

    EXPECT_NO_THROW(conn = new ConnectableInput());
    delete conn;
}

TEST(TESTConnectableInput, testCallGetNumDimensions) {
    ConnectableInput * conn = new ConnectableInput();

    EXPECT_THROW(conn->GetNumDimensions(),std::logic_error);
    delete conn;
}

TEST(TESTConnectableOutput, testConstructionValidParams) {
    ConnectableOutput * conn = nullptr;

    EXPECT_NO_THROW(conn = new ConnectableOutput());
    delete conn;
}

TEST(TESTConnectableOutput, testCallGetNumDimensions) {
    ConnectableOutput * conn = new ConnectableOutput();

    EXPECT_THROW(conn->GetNumDimensions(),std::logic_error);
    delete conn;
}

} // namespace bdhal
} // namespace pystorm
