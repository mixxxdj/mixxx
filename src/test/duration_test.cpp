#include <gtest/gtest.h>

#include "test/mixxxtest.h"
#include "util/duration.h"

namespace mixxx {

TEST(DurationTest, Nanos) {
    Duration d = Duration::fromNanos(255);
    EXPECT_EQ(255, d.toNanos());
    EXPECT_EQ(0, d.toMicros());
    EXPECT_EQ(0, d.toMillis());
    EXPECT_EQ(0, d.toSeconds());

    d = Duration::fromNanos(1e9);
    EXPECT_EQ(1e9, d.toNanos());
    EXPECT_EQ(1e6, d.toMicros());
    EXPECT_EQ(1e3, d.toMillis());
    EXPECT_EQ(1, d.toSeconds());
}

TEST(DurationTest, Micros) {
    Duration d = Duration::fromMicros(255);
    EXPECT_EQ(255000, d.toNanos());
    EXPECT_EQ(255, d.toMicros());
    EXPECT_EQ(0, d.toMillis());
    EXPECT_EQ(0, d.toSeconds());

    d = Duration::fromMicros(1e9);
    EXPECT_EQ(1e12, d.toNanos());
    EXPECT_EQ(1e9, d.toMicros());
    EXPECT_EQ(1e6, d.toMillis());
    EXPECT_EQ(1e3, d.toSeconds());
}

TEST(DurationTest, Millis) {
    Duration d = Duration::fromMillis(255);
    EXPECT_EQ(255000000, d.toNanos());
    EXPECT_EQ(255000, d.toMicros());
    EXPECT_EQ(255, d.toMillis());
    EXPECT_EQ(0, d.toSeconds());

    d = Duration::fromMillis(1e9);
    EXPECT_EQ(1e15, d.toNanos());
    EXPECT_EQ(1e12, d.toMicros());
    EXPECT_EQ(1e9, d.toMillis());
    EXPECT_EQ(1e6, d.toSeconds());
}

TEST(DurationTest, Seconds) {
    Duration d = Duration::fromSeconds(255);
    EXPECT_EQ(255000000000, d.toNanos());
    EXPECT_EQ(255000000, d.toMicros());
    EXPECT_EQ(255000, d.toMillis());
    EXPECT_EQ(255, d.toSeconds());

    d = Duration::fromSeconds(1e9);
    EXPECT_EQ(1e18, d.toNanos());
    EXPECT_EQ(1e15, d.toMicros());
    EXPECT_EQ(1e12, d.toMillis());
    EXPECT_EQ(1e9, d.toSeconds());
}

TEST(DurationTest, Add) {
    Duration d = Duration::fromSeconds(5);
    Duration d2 = Duration::fromSeconds(2);
    Duration d3 = d + d2;

    EXPECT_EQ(5, d.toSeconds());
    EXPECT_EQ(2, d2.toSeconds());
    EXPECT_EQ(7, d3.toSeconds());
}

TEST(DurationTest, AssignAdd) {
    Duration d = Duration::fromSeconds(5);
    Duration d2 = Duration::fromSeconds(2);
    d += d2;

    EXPECT_EQ(7, d.toSeconds());
    EXPECT_EQ(2, d2.toSeconds());
}

TEST(DurationTest, Subtract) {
    Duration d = Duration::fromSeconds(5);
    Duration d2 = Duration::fromSeconds(2);
    Duration d3 = d - d2;

    EXPECT_EQ(5, d.toSeconds());
    EXPECT_EQ(2, d2.toSeconds());
    EXPECT_EQ(3, d3.toSeconds());
}

TEST(DurationTest, AssignSubtract) {
    Duration d = Duration::fromSeconds(5);
    Duration d2 = Duration::fromSeconds(10);
    d -= d2;

    EXPECT_EQ(-5, d.toSeconds());
    EXPECT_EQ(10, d2.toSeconds());
}

TEST(DurationTest, Equals) {
    Duration d = Duration::fromSeconds(5);
    Duration d2 = Duration::fromSeconds(2);
    Duration d3 = Duration::fromSeconds(5);

    EXPECT_EQ(d, d);
    EXPECT_EQ(d, d3);
    EXPECT_NE(d, d2);
}

TEST(DurationTest, Format) {
    Duration d = Duration::fromNanos(255);
    EXPECT_QSTRING_EQ("0x00000000000000ff", d.formatHex());
    EXPECT_QSTRING_EQ("255ns", d.formatNanosWithUnit());

    d = Duration::fromNanos(-255);
    // Formatted as -255 in two's-complement.
    EXPECT_QSTRING_EQ("0xffffffffffffff01", d.formatHex());
    EXPECT_QSTRING_EQ("-255ns", d.formatNanosWithUnit());

    d = Duration::fromNanos(1e9);
    EXPECT_QSTRING_EQ("0x000000003b9aca00", d.formatHex());
    EXPECT_QSTRING_EQ("1000000000ns", d.formatNanosWithUnit());
}

}  // namespace mixxx
