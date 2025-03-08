#include "util/rotary.h"

#include <gtest/gtest.h>

TEST(Rotary, noOverFlow) {
    Rotary rot(3);
    EXPECT_EQ(rot.filter(3), 1);
    EXPECT_EQ(rot.filter(3), 2);
    EXPECT_EQ(rot.filter(3), 3);
}

TEST(Rotary, basicOverflow) {
    Rotary rot(2);
    EXPECT_EQ(rot.filter(2), 1);
    EXPECT_EQ(rot.filter(2), 2);
    EXPECT_EQ(rot.filter(-2), 0);
    EXPECT_EQ(rot.filter(-2), -2);
}

TEST(Rotary, noQuantization) {
    Rotary rot(2);
    EXPECT_DOUBLE_EQ(rot.filter(0.25), 0.125);
    EXPECT_DOUBLE_EQ(rot.filter(0.0), 0.125);
    EXPECT_DOUBLE_EQ(rot.filter(0.0), 0.0);
}

TEST(Rotary, hugeFilterSequential) {
    static constexpr int size = 200;
    Rotary rot(size);
    double sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += i;
        EXPECT_DOUBLE_EQ(rot.filter(i), sum / size) << "i: " << i << " sum: " << sum;
    }
}
