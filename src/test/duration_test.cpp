#include <gtest/gtest.h>

#include "test/mixxxtest.h"
#include "util/duration.h"

namespace mixxx {

TEST(DurationTest, Nanos) {
    Duration d = Duration::fromNanos(255);
    EXPECT_EQ(255, d.toIntegerNanos());
    EXPECT_EQ(0, d.toIntegerMicros());
    EXPECT_EQ(0, d.toIntegerMillis());
    EXPECT_EQ(0, d.toIntegerSeconds());

    d = Duration::fromNanos(1e9);
    EXPECT_EQ(1e9, d.toIntegerNanos());
    EXPECT_EQ(1e6, d.toIntegerMicros());
    EXPECT_EQ(1e3, d.toIntegerMillis());
    EXPECT_EQ(1, d.toIntegerSeconds());
}

TEST(DurationTest, Micros) {
    Duration d = Duration::fromMicros(255);
    EXPECT_EQ(255000, d.toIntegerNanos());
    EXPECT_EQ(255, d.toIntegerMicros());
    EXPECT_EQ(0, d.toIntegerMillis());
    EXPECT_EQ(0, d.toIntegerSeconds());

    d = Duration::fromMicros(1e9);
    EXPECT_EQ(1e12, d.toIntegerNanos());
    EXPECT_EQ(1e9, d.toIntegerMicros());
    EXPECT_EQ(1e6, d.toIntegerMillis());
    EXPECT_EQ(1e3, d.toIntegerSeconds());
}

TEST(DurationTest, Millis) {
    Duration d = Duration::fromMillis(255);
    EXPECT_EQ(255000000, d.toIntegerNanos());
    EXPECT_EQ(255000, d.toIntegerMicros());
    EXPECT_EQ(255, d.toIntegerMillis());
    EXPECT_EQ(0, d.toIntegerSeconds());

    d = Duration::fromMillis(1e9);
    EXPECT_EQ(1e15, d.toIntegerNanos());
    EXPECT_EQ(1e12, d.toIntegerMicros());
    EXPECT_EQ(1e9, d.toIntegerMillis());
    EXPECT_EQ(1e6, d.toIntegerSeconds());
}

TEST(DurationTest, Seconds) {
    Duration d = Duration::fromSeconds(255);
    EXPECT_EQ(255000000000, d.toIntegerNanos());
    EXPECT_EQ(255000000, d.toIntegerMicros());
    EXPECT_EQ(255000, d.toIntegerMillis());
    EXPECT_EQ(255, d.toIntegerSeconds());

    d = Duration::fromSeconds(1e9);
    EXPECT_EQ(1e18, d.toIntegerNanos());
    EXPECT_EQ(1e15, d.toIntegerMicros());
    EXPECT_EQ(1e12, d.toIntegerMillis());
    EXPECT_EQ(1e9, d.toIntegerSeconds());
}

TEST(DurationTest, Add) {
    Duration d = Duration::fromSeconds(5);
    Duration d2 = Duration::fromSeconds(2);
    Duration d3 = d + d2;

    EXPECT_EQ(5, d.toIntegerSeconds());
    EXPECT_EQ(2, d2.toIntegerSeconds());
    EXPECT_EQ(7, d3.toIntegerSeconds());
}

TEST(DurationTest, AssignAdd) {
    Duration d = Duration::fromSeconds(5);
    Duration d2 = Duration::fromSeconds(2);
    d += d2;

    EXPECT_EQ(7, d.toIntegerSeconds());
    EXPECT_EQ(2, d2.toIntegerSeconds());
}

TEST(DurationTest, Subtract) {
    Duration d = Duration::fromSeconds(5);
    Duration d2 = Duration::fromSeconds(2);
    Duration d3 = d - d2;

    EXPECT_EQ(5, d.toIntegerSeconds());
    EXPECT_EQ(2, d2.toIntegerSeconds());
    EXPECT_EQ(3, d3.toIntegerSeconds());
}

TEST(DurationTest, AssignSubtract) {
    Duration d = Duration::fromSeconds(5);
    Duration d2 = Duration::fromSeconds(10);
    d -= d2;

    EXPECT_EQ(-5, d.toIntegerSeconds());
    EXPECT_EQ(10, d2.toIntegerSeconds());
}

TEST(DurationTest, ScalarMultiply) {
    Duration d = Duration::fromNanos(0);
    Duration d2 = Duration::fromNanos(-2);
    Duration d3 = Duration::fromNanos(2);

    EXPECT_EQ(d, d2 * 0);
    EXPECT_EQ(d3, d2 * -1);
    EXPECT_EQ(d2, d3 * -1);
    EXPECT_EQ(d3, -1 * d2);
}

TEST(DurationTest, AssignScalarMultiply) {
    Duration d2 = Duration::fromNanos(-2);
    Duration d3 = Duration::fromNanos(2);

    d2 *= 2;
    d3 *= -2;
    EXPECT_EQ(d2, d3);
}

TEST(DurationTest, Equals) {
    Duration d = Duration::fromSeconds(5);
    Duration d2 = Duration::fromSeconds(2);
    Duration d3 = Duration::fromSeconds(5);

    EXPECT_EQ(d, d);
    EXPECT_EQ(d, d3);
    EXPECT_NE(d, d2);
}

TEST(DurationTest, LessThan) {
    Duration d = Duration::fromNanos(0);
    Duration d2 = Duration::fromNanos(1);

    EXPECT_LE(d, d);
    EXPECT_FALSE(d < d);

    EXPECT_LT(d, d2);
    EXPECT_LE(d, d2);
    EXPECT_FALSE(d2 < d);
    EXPECT_FALSE(d2 <= d);
}

TEST(DurationTest, GreaterThan) {
    Duration d = Duration::fromNanos(1);
    Duration d2 = Duration::fromNanos(0);

    EXPECT_GE(d, d);
    EXPECT_FALSE(d > d);

    EXPECT_GT(d, d2);
    EXPECT_GE(d, d2);
    EXPECT_FALSE(d2 > d);
    EXPECT_FALSE(d2 >= d);
}

TEST(DurationTest, Format) {
    Duration d = Duration::fromNanos(255);
    EXPECT_QSTRING_EQ("0x00000000000000ff", d.formatHex());
    EXPECT_QSTRING_EQ("255 ns", d.formatNanosWithUnit());

    d = Duration::fromNanos(-255);
    // Formatted as -255 in two's-complement.
    EXPECT_QSTRING_EQ("0xffffffffffffff01", d.formatHex());
    EXPECT_QSTRING_EQ("-255 ns", d.formatNanosWithUnit());

    d = Duration::fromNanos(1e9);
    EXPECT_QSTRING_EQ("0x000000003b9aca00", d.formatHex());
    EXPECT_QSTRING_EQ("1000000000 ns", d.formatNanosWithUnit());
}

}  // namespace mixxx
