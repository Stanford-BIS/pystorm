#include "Pool.h"

#include <iostream>

#include "gtest/gtest.h"

namespace pystorm {
namespace bdhal {

TEST(TESTPool, testConstructionValidParams) {
    std::string label = "PoolN";
    uint32_t    numNeurons = 100;
    uint32_t    width      = 10;
    uint32_t    height     = 10;
    uint32_t    dims       = 1;
    Pool* _pool = nullptr;

    EXPECT_NO_THROW(_pool = new Pool(label, numNeurons, dims, width, height));
    delete _pool;

    EXPECT_NO_THROW(_pool = new Pool(label, numNeurons, dims));
    
    delete _pool;
}

TEST(TESTPool, testConstructionInvalidParams) {
    std::string label = "PoolN";
    uint32_t    numNeurons = 100;
    uint32_t    width      = 10;
    uint32_t    height     = 10;
    uint32_t    dims       = 1;
    Pool* _pool = nullptr;

    EXPECT_THROW(_pool = new Pool(std::string(""), numNeurons, dims, width, 
        height), std::logic_error);

    delete _pool;

    EXPECT_THROW(_pool = new Pool(label, (Pool::MIN_NEURONS - 1), dims, width, 
        height), std::logic_error);

    delete _pool;

    EXPECT_THROW(_pool = new Pool(label, (Pool::MAX_NEURONS + 1), dims, width, 
        height), std::logic_error);

    delete _pool;

    EXPECT_THROW(_pool = new Pool(label, numNeurons, (Pool::MIN_DIMS - 1), 
        width, height), std::logic_error);

    delete _pool;
}

TEST(TESTPool, testGetSetSizeValid) {
    std::string label = "PoolN";
    uint32_t    numNeurons = 100;
    uint32_t    width      = 10;
    uint32_t    height     = 10;
    uint32_t    dims       = 1;
    Pool* _pool = new Pool(label,numNeurons,dims);

    EXPECT_NO_THROW(_pool->SetSize(width,height));

    delete _pool;
}

TEST(TESTPool, testGetSetSizeInValidParams) {
    std::string label = "PoolN";
    uint32_t    numNeurons = 100;
    uint32_t    width      = 10;
    uint32_t    height     = 11;
    uint32_t    dims       = 1;
    Pool* _pool = new Pool(label,numNeurons,dims);

    EXPECT_THROW(_pool->SetSize(width,height),std::logic_error);

    delete _pool;
}

TEST(TESTPool, testGetLabelsNeuronsDims) {
    std::string label = "PoolN";
    uint32_t    numNeurons = 100;
    uint32_t    width      = 10;
    uint32_t    height     = 10;
    uint32_t    dims       = 1;
    Pool* _pool = nullptr;

    EXPECT_NO_THROW(_pool = new Pool(label,numNeurons,dims, width, height));

    EXPECT_EQ(_pool->GetLabel(),label);
    EXPECT_EQ(_pool->GetNumNeurons(),numNeurons);
    EXPECT_EQ(_pool->GetWidth(),width);
    EXPECT_EQ(_pool->GetHeight(),height);
    
    delete _pool;
}

} // namespace bdhal
} // namespace pystorm
