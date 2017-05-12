#include "BDPars.h"

#include <iostream>
using std::cout;
using std::endl;

#include "gtest/gtest.h"

using namespace pystorm;
using namespace bddriver;

class BDParsFixture : public testing::Test
{
  public:
    void SetUp() {
      pars = new BDPars();
    }
    void TearDown() {
      delete pars;
    }

    BDPars * pars;
};

TEST_F(BDParsFixture, TestSetUpTearDown) {}
