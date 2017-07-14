#include "Bucket.h"

#include <iostream>

#include "gtest/gtest.h"

namespace pystorm {
namespace bdhal {

TEST(TESTBucket, testConstructionValidParams) {
    std::string bucketLabel = "BucketLabel";
    uint32_t bucketDims = 1;
    Bucket * testBucket = nullptr;

    EXPECT_NO_THROW(testBucket = new Bucket(bucketLabel, bucketDims));

    EXPECT_EQ(testBucket->GetLabel(), bucketLabel);
    EXPECT_EQ(testBucket->GetNumDimensions(),bucketDims);
    delete testBucket;
}

TEST(TESTBucket, testConstructionInvalidLabel) {
    std::string bucketLabel = "";
    uint32_t bucketDims = 1;
    Bucket * testBucket = nullptr;

    EXPECT_THROW(testBucket = new Bucket(bucketLabel, bucketDims),std::logic_error);
    delete testBucket;
}

TEST(TESTBucket, testConstructionInvalidDimension) {
    std::string bucketLabel = "BucketLabel";
    uint32_t invalidBucketDims = 0;
    Bucket * testBucket = nullptr;

    EXPECT_THROW(testBucket = new Bucket(bucketLabel, invalidBucketDims),std::out_of_range);
    delete testBucket;
}

} // namespace bdhal
} // namespace pystorm
