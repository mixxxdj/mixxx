#include <gtest/gtest.h>

#include <QDebug>

#include "track/bpm.h"

class BpmTest : public testing::Test {
};

TEST_F(BpmTest, TestBpmEquality) {
    EXPECT_EQ(mixxx::Bpm(120), mixxx::Bpm(120));
    EXPECT_EQ(mixxx::Bpm(120), mixxx::Bpm(60) * 2);
    EXPECT_EQ(mixxx::Bpm(120), mixxx::Bpm(240) / 2);

    // Verify that invalid values are equal to each other, regardless of their
    // actual value.
    EXPECT_EQ(mixxx::Bpm(mixxx::Bpm::kValueUndefined), mixxx::Bpm());
    EXPECT_EQ(mixxx::Bpm(0.0), mixxx::Bpm());
    EXPECT_EQ(mixxx::Bpm(-120.0), mixxx::Bpm());
    EXPECT_EQ(mixxx::Bpm(-120.0), mixxx::Bpm(0.0));
    EXPECT_EQ(mixxx::Bpm(-120.0), mixxx::Bpm(-100.0));

    // Verify that valid and invalid values are not equal
    EXPECT_NE(mixxx::Bpm(120.0), mixxx::Bpm());
    EXPECT_NE(mixxx::Bpm(120.0), mixxx::Bpm(-120.0));
}
