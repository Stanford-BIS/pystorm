#include "Network.h"
#include "Pool.h"

#include <iostream>

#include "gtest/gtest.h"

namespace pystorm {
namespace bdhal {

TEST(TESTNetwork, testConstructionValidParams) {
    std::string name = "NetworkN";
    Network* _net = nullptr;

    EXPECT_NO_THROW(_net = new Network(name));

    delete _net;
}

TEST(TESTNetwork, testConstructionInvalidParams) {
    std::string name = "";
    Network* _net = nullptr;

    EXPECT_THROW(_net = new Network(name),std::logic_error);

    delete _net;
}

TEST(TESTNetwork, testCreatePool) {
    std::string name = "Network1";
    Network* _net = new Network(name);

    std::string pool1Label = "Pool1";
    uint32_t poolNeurons = 100;
    uint32_t poolDims = 3;
    uint32_t poolWidth = 10;
    uint32_t poolHeight = 10;

    std::string pool2Label = "Pool2";

    Pool* _pool1 = nullptr;
    Pool* _pool2 = nullptr;

    EXPECT_NO_THROW(_pool1 = _net->CreatePool(pool1Label, poolNeurons,
        poolDims, poolWidth, poolHeight));

    EXPECT_NO_THROW(_pool2 = _net->CreatePool(pool2Label, poolNeurons,
        poolDims));

    VecOfPools& _vecOfPools = _net->GetPools();

    EXPECT_EQ(_pool1, _vecOfPools.at(0));
    EXPECT_EQ(_pool2, _vecOfPools.at(1));

    delete _net;
}

TEST(TESTNetwork, testCreateBucket) {
    std::string name = "Network1";
    Network* _net = new Network(name);

    std::string bucket1Label = "Bucket1";
    uint32_t bucketDims = 3;

    std::string bucket2Label = "Bucket2";

    Bucket* _bucket1 = nullptr;
    Bucket* _bucket2 = nullptr;

    EXPECT_NO_THROW(_bucket1 = _net->CreateBucket(bucket1Label, bucketDims));
    EXPECT_NO_THROW(_bucket2 = _net->CreateBucket(bucket2Label, bucketDims));

    VecOfBuckets& _vecOfBuckets = _net->GetBuckets();

    EXPECT_EQ(_bucket1, _vecOfBuckets.at(0));
    EXPECT_EQ(_bucket2, _vecOfBuckets.at(1));

    delete _net;
}

TEST(TESTNetwork, testCreateConnection) {
// Determine what combinations need to be tested.
}

} // namespace bdhal
} // namespace pystorm
