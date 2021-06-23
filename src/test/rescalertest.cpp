#include <gtest/gtest.h>
#include <QDebug>

#include "util/math.h"
#include "util/rescaler.h"

class RescalerUtilsTest : public testing::Test {
};

TEST_F(RescalerUtilsTest, Test) {
    double result;

    // test upper border
    result = RescalerUtils::linearToOneByX(100, 0, 100, 1000);
    EXPECT_DOUBLE_EQ(result, 1000);

    result = RescalerUtils::linearToOneByX(100, 50, 100, 1000);
    EXPECT_DOUBLE_EQ(result, 1000);

    result = RescalerUtils::linearToOneByX(50, 40, 50, 1000);
    EXPECT_DOUBLE_EQ(result, 1000);

    // test lower border
    result = RescalerUtils::linearToOneByX(0, 0, 100, 1000);
    EXPECT_DOUBLE_EQ(result, 1);

    result = RescalerUtils::linearToOneByX(50, 50, 100, 1000);
    EXPECT_DOUBLE_EQ(result, 1);

    result = RescalerUtils::linearToOneByX(40, 40, 50, 1000);
    EXPECT_DOUBLE_EQ(result, 1);

    // test upper border inverse
    result = RescalerUtils::oneByXToLinear(100, 100, 0, 1000);
    EXPECT_DOUBLE_EQ(result, 1000);

    result = RescalerUtils::oneByXToLinear(100, 100, 50, 1000);
    EXPECT_DOUBLE_EQ(result, 1000);

    result = RescalerUtils::oneByXToLinear(50, 50, 50, 1000);
    EXPECT_DOUBLE_EQ(result, 1000);

    // test lower border inverse
    result = RescalerUtils::oneByXToLinear(1, 100, 0, 1000);
    EXPECT_DOUBLE_EQ(result, 0);

    result = RescalerUtils::oneByXToLinear(1, 50, 100, 1000);
    EXPECT_TRUE(fabs(result - 100) < 0.0000000001);

    result = RescalerUtils::oneByXToLinear(1, 40, 50, 1000);
    EXPECT_TRUE(fabs(result - 50) < 0.0000000001);

    // Test Roundtrip
    // test upper border
    result = RescalerUtils::linearToOneByX(50, 0, 100, 1000);
    result = RescalerUtils::oneByXToLinear(result, 1000, 0, 100);
    EXPECT_TRUE(fabs(result - 50) < 0.0000000001);
}
