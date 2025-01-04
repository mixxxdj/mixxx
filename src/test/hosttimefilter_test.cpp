#include "util/hosttimefilter.h"

#include <gtest/gtest.h>

#include <chrono>

using namespace std::chrono_literals;

namespace mixxx {

class HostTimeFilterTest : public ::testing::Test {
  protected:
    HostTimeFilterTest()
            : m_filter(5) { // Initialize with 5 points for testing
    }

    HostTimeFilter m_filter;
};

TEST_F(HostTimeFilterTest, InitialState) {
    EXPECT_EQ(m_filter.calcFilteredHostTime(0.0, 0us), 0us);
}

TEST_F(HostTimeFilterTest, AddSinglePoint) {
    EXPECT_NEAR(m_filter.calcFilteredHostTime(1.0, 1050us).count(), 1050, 1);
}

TEST_F(HostTimeFilterTest, EqualFreqNoJitter) {
    // Wo perfectly synced clocks, the filter should return the same host time as the auxiliary time
    EXPECT_NEAR(m_filter.calcFilteredHostTime(1000.0, 1000us).count(), 1000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(2000.0, 2000us).count(), 2000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(3000.0, 3000us).count(), 3000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(4000.0, 4000us).count(), 4000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(5000.0, 5000us).count(), 5000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(6000.0, 6000us).count(), 6000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(7000.0, 7000us).count(), 7000, 1);
}

TEST_F(HostTimeFilterTest, FasterFreqNoJitter) {
    // Use 1024 sample buffer interval, instead of auxiliarry clock in time units
    EXPECT_NEAR(m_filter.calcFilteredHostTime(1024.0, 1000us).count(), 1000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(2048.0, 2000us).count(), 2000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(3072.0, 3000us).count(), 3000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(4096.0, 4000us).count(), 4000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(5120.0, 5000us).count(), 5000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(6144.0, 6000us).count(), 6000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(7168.0, 7000us).count(), 7000, 1);
}

TEST_F(HostTimeFilterTest, FasterFreqWithJitter) {
    // Use 1024 sample buffer interval, with 100us host time jitter
    EXPECT_NEAR(m_filter.calcFilteredHostTime(1024.0, 1000us).count(), 1000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(2048.0, 2100us).count(), 2100, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(3072.0, 3000us).count(), 3033, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(4096.0, 3900us).count(), 3940, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(5120.0, 5000us).count(), 4960, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(6144.0, 6000us).count(), 5960, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(7168.0, 7000us).count(), 7000, 1);
}

TEST_F(HostTimeFilterTest, FasterFreqSkippedPoints) {
    // Use 1024 sample buffer interval, instead of auxiliarry clock in time units
    EXPECT_NEAR(m_filter.calcFilteredHostTime(1024.0, 1000us).count(), 1000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(2048.0, 2000us).count(), 2000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(4096.0, 4000us).count(), 4000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(5120.0, 5000us).count(), 5000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(8192.0, 8000us).count(), 8000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(9216.0, 9000us).count(), 9000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(11264.0, 11000us).count(), 11000, 1);
}

TEST_F(HostTimeFilterTest, Reset) {
    m_filter.calcFilteredHostTime(1.0, 1050us);
    m_filter.calcFilteredHostTime(2.0, 1950us);
    m_filter.reset();
    EXPECT_NEAR(m_filter.calcFilteredHostTime(4.0, 7777us).count(), 7777, 1);
}

TEST_F(HostTimeFilterTest, DenominatorZero) {
    // Add two identical points to ensure the denominator becomes zero
    EXPECT_NEAR(m_filter.calcFilteredHostTime(1.0, 1000us).count(), 1000, 1);
    EXPECT_NEAR(m_filter.calcFilteredHostTime(1.0, 1000us).count(), 1000, 1);
}

} // namespace mixxx
