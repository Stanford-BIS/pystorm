#include "Hal.h"

#include <iostream>

#include "gtest/gtest.h"


namespace pystorm {
namespace bdhal {

class HalFixture : public testing::Test {
  public:
    HalFixture() {
      //driver = Driver::GetInstance();
      m_hal = new Hal();
    }

    void SetUp() {
    }

    void TearDown() {

    }

    ~HalFixture() {
      delete m_hal;
    }

    Hal * m_hal;

};

TEST_F(HalFixture, Construct) {}


} // namespace bdhal
} // namespace pystorm
