#include <gtest/gtest.h>

#include <QtDebug>
#include <cstring>

#include "util/denormalsarezero.h"
#include "util/fpclassify.h"
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
    EXPECT_TRUE(util_isinf(util_float_infinity()));

    // Test doubles can be recognized as infinity.
    EXPECT_FALSE(util_isinf(0.0f));
    EXPECT_TRUE(util_isinf(util_double_infinity()));
}

TEST_F(MathUtilTest, Denormal) {
#ifdef __SSE__

    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_OFF);
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);

    // Note: The volatile keyword makes sure that the division is executed on the target
    // and not by the pre-processor. In case of clang >= 15 the pre-processor flushes to
    // zero with -ffast-math enabled.
    volatile float fDenormal = std::numeric_limits<float>::min();
    fDenormal = fDenormal / 2.0f;
    EXPECT_NE(0.0f, fDenormal);

    volatile double dDenormal = std::numeric_limits<double>::min();
    dDenormal = dDenormal / 2.0;
    EXPECT_NE(0.0, dDenormal);

    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

    fDenormal = std::numeric_limits<float>::min();
    fDenormal = fDenormal / 2.0f;
    EXPECT_EQ(0.0f, fDenormal);

    dDenormal = std::numeric_limits<double>::min();
    dDenormal = dDenormal / 2.0;
    EXPECT_EQ(0.0, dDenormal);

#endif
}

TEST_F(MathUtilTest, DoubleValues) {
    // This verifies that the infinity value can be copied into -ffastmath code

    // All supported targets are using IEC559 floats
    static_assert(std::numeric_limits<double>::is_iec559);
    long long int_value;
    double double_value = util_double_infinity();
    std::memcpy(&int_value, &double_value, sizeof(double_value));
    long long int_diff = int_value - 0x7FF0000000000000; // IEC 559 (IEEE 754) Infinity
    EXPECT_EQ(int_diff, 0);
    // Comparing directly does not work because the compiler (clang >= 19) may
    // optimize the long long roundtrip away, compares as double and discards the
    // comparison because of -ffastmath (clang >= 19)
    // EXPECT_EQ(int_value, 0x7FF0000000000000);
}

}  // namespace
