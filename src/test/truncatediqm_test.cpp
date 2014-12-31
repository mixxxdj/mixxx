#include <gtest/gtest.h>
#include "truncatediqm.h"

#include <QDebug>
namespace {

// The fixture for testing class Foo.
class TruncatedIQMTest : public ::testing::Test {
 protected:
  TruncatedIQMTest(): maxAcceptedError(0.5E-9) {
      // This is the current lowest error that lets the class pass
      // test doubles7.
  }

  virtual ~TruncatedIQMTest() {
  }

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

  const double maxAcceptedError;
};

TEST_F(TruncatedIQMTest, zeros1) {
    TruncatedIQM iqm(1);
    double mean;
    for (int i=1; i<=50; ++i) {
        mean = iqm.insert(0);
        EXPECT_DOUBLE_EQ(0, mean);
    }
    iqm.insert(-4);
    mean = iqm.mean();
    EXPECT_DOUBLE_EQ(-4, mean);
}

TEST_F(TruncatedIQMTest, zeros2) {
    TruncatedIQM iqm(2);
    double mean = iqm.insert(0);
    EXPECT_DOUBLE_EQ(0, mean);
    for (int i=1; i<=50; ++i) {
        mean = iqm.insert(0);
        EXPECT_DOUBLE_EQ(0, mean);
    }
    iqm.insert(-4);
    mean = iqm.mean();
    EXPECT_DOUBLE_EQ(-2, mean);
}

TEST_F(TruncatedIQMTest, integers1) {
    TruncatedIQM iqm(1);
    double mean;
    for (int i=1; i<=50; ++i) {
        mean = iqm.insert(i);
        EXPECT_DOUBLE_EQ(i, mean);
    }
    iqm.insert(-4);
    mean = iqm.mean();
    EXPECT_DOUBLE_EQ(-4, mean);
}

TEST_F(TruncatedIQMTest, integers2) {
    TruncatedIQM iqm(2);
    double mean = iqm.insert(0);
    EXPECT_DOUBLE_EQ(0, mean);
    for (int i=1; i<=50; ++i) {
        mean = iqm.insert(i);
        EXPECT_DOUBLE_EQ((2*i-1)/2.0, mean);
    }
    iqm.insert(-4);
    mean = iqm.mean();
    EXPECT_DOUBLE_EQ(23, mean);
}

TEST_F(TruncatedIQMTest, doubles7) {
    // TODO(Ferran Pujol): Correctly rewrite all means[i]
    // with the same number of significant figures.
    TruncatedIQM iqm(7);
    double input[9] = { 16.345, 2.129674, 77, -40.23, 12071, -12071,
                        -15, 0, 1};
    double means[9] = { 16.345, 9.237337, 31.8248913333, 13.8111685,
                        30.2769022, 12.286558, 3.6191925714,
                        -1.0508074285, -1.3735714285};
    for (int i=0; i<9; ++i) {
        qDebug() << "Testing i =" << i << ":";
        double mean = iqm.insert(input[i]);
        //TODO(Ferran Pujol): Why does EXPECT_DOUBLE_EQ fail here?
        EXPECT_NEAR(means[i], mean, maxAcceptedError);
    }
}

}  // namespace
