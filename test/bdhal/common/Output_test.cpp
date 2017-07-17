#include "Output.h"

#include <iostream>

#include "gtest/gtest.h"

namespace pystorm {
namespace bdhal {

TEST(TESTOutput, testConstructionValidParams) {
    std::string label = "OutputN";
    uint32_t dims = 3;
    Output * _out = nullptr;

    EXPECT_NO_THROW(_out = new Output(label, dims));
    delete _out;
}

TEST(TESTOutput, testConstructionInvalidParams) {
    std::string badlabel = "";
    std::string label = "OutputN";
    uint32_t baddims = 0;
    uint32_t dims = 3;
    Output * _out = nullptr;

    EXPECT_THROW(_out = new Output(badlabel, dims),std::logic_error);
    delete _out;

    EXPECT_THROW(_out = new Output(label, baddims),std::out_of_range);
    delete _out;
}

TEST(TESTOutput, testCallGetLabel) {
    std::string label = "OutputN";
    uint32_t dims = 3;
    Output * _out = new Output(label, dims);


    EXPECT_EQ(_out->GetLabel(),label);

    delete _out;
}

TEST(TESTOutput, testCallGetNumDims) {
    std::string label = "OutputN";
    uint32_t dims = 3;
    Output * _out = new Output(label, dims);


    EXPECT_EQ(_out->GetNumDimensions(),dims);

    delete _out;
}


} // namespace bdhal
} // namespace pystorm
