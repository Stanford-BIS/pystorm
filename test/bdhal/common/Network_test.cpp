#include "Network.h"
#include "Pool.h"
#include "Bucket.h"
#include "Weights.h"

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
    std::string net_name = "Network1";
    Network* _net = new Network(net_name);

    std::string conn_label = "Connection1";
    std::string conn2_label = "Connection2";

    Pool* _pool = new Pool("Pool1", 100,3);
    Bucket * _bucket = new Bucket("Bucket1",2);

    Connection* _conn = nullptr;
    Connection* _conn2 = nullptr;
    Weights<uint32_t>* wMatrix = nullptr;

    EXPECT_NO_THROW(_conn = _net->CreateConnection(conn_label, _pool, _bucket));

    wMatrix = _net->GetConnections().at(0)->GetWeights();

    EXPECT_EQ(_net->GetConnections().at(0), _conn);
    EXPECT_EQ(_net->GetConnections().at(0)->GetLabel(), conn_label);
    EXPECT_EQ(_net->GetConnections().at(0)->GetSrc(), _pool);
    EXPECT_EQ(_net->GetConnections().at(0)->GetDest(), _bucket);

    EXPECT_EQ(wMatrix->GetNumRows(),_bucket->GetNumDimensions());
    EXPECT_EQ(wMatrix->GetNumColumns(),_pool->GetNumDimensions());

    uint32_t* m2 = (uint32_t*)
        std::calloc((_pool->GetNumDimensions()*_bucket->GetNumDimensions()),         
        sizeof(uint32_t));  

    Weights<uint32_t>* wMatrix2 =
        new Weights<uint32_t>(m2, _bucket->GetNumDimensions(),
        _pool->GetNumDimensions());

    EXPECT_NO_THROW(_conn2 = _net->CreateConnection(conn2_label, _pool, _bucket,
        wMatrix2));

    wMatrix = _net->GetConnections().at(1)->GetWeights();

    EXPECT_EQ(_net->GetConnections().at(1), _conn2);
    EXPECT_EQ(_net->GetConnections().at(1)->GetLabel(), conn2_label);
    EXPECT_EQ(_net->GetConnections().at(1)->GetSrc(), _pool);
    EXPECT_EQ(_net->GetConnections().at(1)->GetDest(), _bucket);

    EXPECT_EQ(wMatrix->GetNumRows(),_bucket->GetNumDimensions());
    EXPECT_EQ(wMatrix->GetNumColumns(),_pool->GetNumDimensions());
   
    delete _conn2;
    delete _conn;
    delete _net;
}

} // namespace bdhal
} // namespace pystorm
