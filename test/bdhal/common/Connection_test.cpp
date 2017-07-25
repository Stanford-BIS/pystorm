#include "Connection.h"
#include "Pool.h"
#include "Bucket.h"

#include <iostream>

#include "gtest/gtest.h"

namespace pystorm {
namespace bdhal {

TEST(TESTConnection, testConstructionValidParams) {
    std::string conn_label = "Connection1";
    std::string pool_label = "Pool";
    std::string bucket_label = "Bucket";
    std::string bucket2_label = "Bucket2";

    uint32_t pool_dims = 3;
    uint32_t num_neurons = 100;
    uint32_t bucket_dims = 2;
    uint32_t bucket2_dims = 3;
    Pool* _pool = new Pool(pool_label, num_neurons, pool_dims);
    Bucket* _bucket = new Bucket(bucket_label, bucket_dims);

    Bucket* _bucket2 = new Bucket(bucket2_label, bucket2_dims);

    uint32_t* m = (uint32_t*)
        std::calloc((_pool->GetNumDimensions()*_bucket->GetNumDimensions()),         
        sizeof(uint32_t));  

    Weights<uint32_t>* wMatrix =
        new Weights<uint32_t>(m, _bucket->GetNumDimensions(),
        _pool->GetNumDimensions());

    Connection* _conn1 = nullptr;
    Connection* _conn2 = nullptr;

    EXPECT_NO_THROW(_conn1 = new Connection(conn_label, _pool, _bucket, 
        wMatrix));

    EXPECT_NO_THROW(_conn2 = new Connection(conn_label, _pool, _bucket2));

    delete _conn1;
    delete _conn2;
}

TEST(TESTConnection, testConstructionInvalidParams) {
    std::string conn_label = "Connection1";
    std::string pool_label = "Pool";
    std::string pool2_label = "Pool2";
    std::string bucket_label = "Bucket";
    std::string bucket2_label = "Bucket2";

    uint32_t pool_dims = 3;
    uint32_t pool2_dims = 2;
    uint32_t num_neurons = 100;
    uint32_t bucket_dims = 2;
    uint32_t bucket2_dims = 3;
    Pool* _pool = new Pool(pool_label, num_neurons, pool_dims);
    Pool* _pool2 = new Pool(pool2_label, num_neurons, pool2_dims);
    Bucket* _bucket = new Bucket(bucket_label, bucket_dims);

    Bucket* _bucket2 = new Bucket(bucket2_label, bucket2_dims);

    uint32_t* m = (uint32_t*)
        std::calloc((_pool->GetNumDimensions()*_bucket->GetNumDimensions()),         
        sizeof(uint32_t));  

    Weights<uint32_t>* wMatrix =
        new Weights<uint32_t>(m, _bucket->GetNumDimensions(),
        _pool->GetNumDimensions());

    Connection* _conn1 = nullptr;

    EXPECT_THROW(_conn1 = new Connection(conn_label, nullptr, _bucket, 
        wMatrix), std::logic_error);

    delete _conn1;

    EXPECT_THROW(_conn1 = new Connection(conn_label, _pool, nullptr, 
        wMatrix), std::logic_error);

    delete _conn1;

    EXPECT_THROW(_conn1 = new Connection(conn_label, _pool, _bucket, 
        nullptr), std::logic_error);

    delete _conn1;

    EXPECT_THROW(_conn1 = new Connection(conn_label, _pool2, _bucket,
        wMatrix), std::logic_error);

    delete _conn1;

    EXPECT_THROW(_conn1 = new Connection(conn_label, _pool, _bucket2,
        wMatrix), std::logic_error);

    delete _conn1;

    EXPECT_THROW(_conn1 = new Connection(conn_label, nullptr, _bucket),
        std::logic_error);

    delete _conn1;

    EXPECT_THROW(_conn1 = new Connection(conn_label, _pool, nullptr),
        std::logic_error);

    delete _conn1;

    EXPECT_THROW(_conn1 = new Connection(conn_label, _pool, _bucket),
        std::logic_error);

    delete _conn1;
}

TEST(TESTConnection, testGetLabel) {
    std::string conn_label = "Connection1";
    std::string pool_label = "Pool";
    std::string bucket_label = "Bucket";

    uint32_t pool_dims = 3;
    uint32_t num_neurons = 100;
    uint32_t bucket_dims = 2;

    Pool* _pool = new Pool(pool_label, num_neurons, pool_dims);
    Bucket* _bucket = new Bucket(bucket_label, bucket_dims);

    uint32_t* m = (uint32_t*)
        std::calloc((_pool->GetNumDimensions()*_bucket->GetNumDimensions()),         
        sizeof(uint32_t));  

    Weights<uint32_t>* wMatrix =
        new Weights<uint32_t>(m, _bucket->GetNumDimensions(),
        _pool->GetNumDimensions());

    Connection* _conn1 = new Connection(conn_label, _pool, _bucket, 
        wMatrix);

    EXPECT_EQ(_conn1->GetLabel(), conn_label);

    delete _conn1;
}

TEST(TESTConnection, testGetSrc) {
    std::string conn_label = "Connection1";
    std::string pool_label = "Pool";
    std::string bucket_label = "Bucket";

    uint32_t pool_dims = 3;
    uint32_t num_neurons = 100;
    uint32_t bucket_dims = 2;

    Pool* _pool = new Pool(pool_label, num_neurons, pool_dims);
    Bucket* _bucket = new Bucket(bucket_label, bucket_dims);

    uint32_t* m = (uint32_t*)
        std::calloc((_pool->GetNumDimensions()*_bucket->GetNumDimensions()),         
        sizeof(uint32_t));  

    Weights<uint32_t>* wMatrix =
        new Weights<uint32_t>(m, _bucket->GetNumDimensions(),
        _pool->GetNumDimensions());

    Connection* _conn1 = new Connection(conn_label, _pool, _bucket, 
        wMatrix);

    EXPECT_EQ(_conn1->GetSrc(), _pool);

    delete _conn1;
}

TEST(TESTConnection, testGetDest) {
    std::string conn_label = "Connection1";
    std::string pool_label = "Pool";
    std::string bucket_label = "Bucket";

    uint32_t pool_dims = 3;
    uint32_t num_neurons = 100;
    uint32_t bucket_dims = 2;

    Pool* _pool = new Pool(pool_label, num_neurons, pool_dims);
    Bucket* _bucket = new Bucket(bucket_label, bucket_dims);

    uint32_t* m = (uint32_t*)
        std::calloc((_pool->GetNumDimensions()*_bucket->GetNumDimensions()),         
        sizeof(uint32_t));  

    Weights<uint32_t>* wMatrix =
        new Weights<uint32_t>(m, _bucket->GetNumDimensions(),
        _pool->GetNumDimensions());

    Connection* _conn1 = new Connection(conn_label, _pool, _bucket, 
        wMatrix);

    EXPECT_EQ(_conn1->GetDest(), _bucket);

    delete _conn1;
}


} // namespace bdhal
} // namespace pystorm
