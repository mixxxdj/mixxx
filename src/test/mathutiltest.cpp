#include <gtest/gtest.h>

#include <QtDebug>
#include <limits>

#include "util/denormalsarezero.h"
#include "util/math.h"

namespace {

class MathUtilTest : public testing::Test {
  protected:
    static const int MIN;
    static const int MAX;

    static const int VALUE_MIN;
    static const int VALUE_MAX;
};

const int MathUtilTest::MIN = -10;
const int MathUtilTest::MAX = 10;

const int MathUtilTest::VALUE_MIN = 2 * MathUtilTest::MIN;
const int MathUtilTest::VALUE_MAX = 2 * MathUtilTest::MAX;

TEST_F(MathUtilTest, MathClampUnsafe) {
    for (int i = VALUE_MIN; i <= VALUE_MAX; ++i) {
        EXPECT_LE(MIN, math_clamp(i, MIN, MAX));
        EXPECT_GE(MAX, math_clamp(i, MIN, MAX));
        EXPECT_EQ(MIN, math_clamp(i, MIN, MIN));
        EXPECT_EQ(MAX, math_clamp(i, MAX, MAX));
        if (MIN >= i) {
            EXPECT_EQ(MIN, math_clamp(i, MIN, MAX));
        }
        if (MAX <= i) {
            EXPECT_EQ(MAX, math_clamp(i, MIN, MAX));
        }
    }
}

TEST_F(MathUtilTest, IsNaN) {
    // Test floats can be recognized as nan.
    EXPECT_FALSE(util_isnan(0.0f));
    EXPECT_TRUE(util_isnan(std::numeric_limits<float>::quiet_NaN()));

    // Test doubles can be recognized as nan.
    EXPECT_FALSE(util_isnan(0.0));
    EXPECT_TRUE(util_isnan(std::numeric_limits<double>::quiet_NaN()));
}

TEST_F(MathUtilTest, IsInf) {
    // Test floats can be recognized as infinity.
    EXPECT_FALSE(util_isinf(0.0f));
    EXPECT_TRUE(util_isinf(std::numeric_limits<float>::infinity()));

    // Test doubles can be recognized as infinity.
    EXPECT_FALSE(util_isinf(0.0f));
    EXPECT_TRUE(util_isinf(std::numeric_limits<double>::infinity()));
}

TEST_F(MathUtilTest, Denormal) {
#ifdef __SSE__

    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_OFF);
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);

    volatile float fDenormal = std::numeric_limits<float>::min() / 2.0f;
    EXPECT_NE(0.0f, fDenormal);

    volatile double dDenormal = std::numeric_limits<double>::min() / 2.0;
    EXPECT_NE(0.0, dDenormal);

    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

    fDenormal = std::numeric_limits<float>::min() / 2.0f;
    EXPECT_EQ(0.0f, fDenormal);

    dDenormal = std::numeric_limits<double>::min() / 2.0;
    EXPECT_EQ(0.0, dDenormal);

#endif
}


}  // namespace
