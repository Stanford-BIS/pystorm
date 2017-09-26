#include "Input.h"

#include <iostream>

#include "gtest/gtest.h"

namespace pystorm {
namespace bdhal {

TEST(TESTInput, testConstructionValidParams) {
    std::string label = "InputN";
    uint32_t dims = 3;
    Input * _in = nullptr;

    EXPECT_NO_THROW(_in = new Input(label, dims));
    delete _in;
}

TEST(TESTInput, testConstructionInvalidParams) {
    std::string badlabel = "";
    std::string label = "InputN";
    uint32_t baddims = 0;
    uint32_t dims = 3;
    Input * _in = nullptr;

    EXPECT_THROW(_in = new Input(badlabel, dims),std::logic_error);
    delete _in;

    EXPECT_THROW(_in = new Input(label, baddims),std::out_of_range);
    delete _in;
}

TEST(TESTInput, testGetLabel) {
    std::string label = "InputN";
    uint32_t dims = 3;
    Input * _in = new Input(label, dims);


    EXPECT_EQ(_in->GetLabel(),label);

    delete _in;
}

TEST(TESTInput, testGetNumDims) {
    std::string label = "InputN";
    uint32_t dims = 3;
    Input * _in = new Input(label, dims);


    EXPECT_EQ(_in->GetNumDimensions(),dims);

    delete _in;
}

TEST(TESTInput, testGetConnectionSize) {
    std::string label = "InputN";
    uint32_t dims = 3;
    Input * _in = new Input(label, dims);


    EXPECT_EQ(_in->GetConnectionSize(),dims);

    delete _in;
}


} // namespace bdhal
} // namespace pystorm
