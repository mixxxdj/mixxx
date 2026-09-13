#include "util/hosttimefilter.h"

#include <gtest/gtest.h>

#include <chrono>

using namespace std::chrono_literals;

class HostTimeFilterTest : public ::testing::Test {
  protected:
    HostTimeFilterTest()
            : m_filter(5) { // Initialize with 5 points for testing
    }

    HostTimeFilter m_filter;
};

TEST_F(HostTimeFilterTest, InitialState) {
    EXPECT_EQ(m_filter.calcHostTime(0.0), HostTimeFilter::kInvalidHostTime);
}

TEST_F(HostTimeFilterTest, AddSinglePoint) {
    m_filter.insertTimePoint(1.0, 1050us);
    EXPECT_EQ(m_filter.calcHostTime(1.0), HostTimeFilter::kInvalidHostTime);
}

TEST_F(HostTimeFilterTest, EqualFreqNoJitter) {
    // Perfectly synced clocks, the filter should return hosttime == auxiliary time
    m_filter.insertTimePoint(1000.0, 1000us);
    EXPECT_EQ(m_filter.calcHostTime(1000.0), HostTimeFilter::kInvalidHostTime);
    m_filter.insertTimePoint(2000.0, 2000us);
    EXPECT_EQ(m_filter.calcHostTime(2000.0), 2000us);
    m_filter.insertTimePoint(3000.0, 3000us);
    EXPECT_EQ(m_filter.calcHostTime(3000.0), 3000us);
    m_filter.insertTimePoint(4000.0, 4000us);
    EXPECT_EQ(m_filter.calcHostTime(4000.0), 4000us);
    m_filter.insertTimePoint(5000.0, 5000us);
    EXPECT_EQ(m_filter.calcHostTime(5000.0), 5000us);
    m_filter.insertTimePoint(6000.0, 6000us);
    EXPECT_EQ(m_filter.calcHostTime(6000.0), 6000us);
    m_filter.insertTimePoint(7000.0, 7000us);
    EXPECT_EQ(m_filter.calcHostTime(7000.0), 7000us);
}

TEST_F(HostTimeFilterTest, FasterFreqNoJitter) {
    // Use 1024 sample buffer interval, instead of auxiliarry clock in time units
    m_filter.insertTimePoint(1024.0, 1000us);
    EXPECT_EQ(m_filter.calcHostTime(1024.0), HostTimeFilter::kInvalidHostTime);
    m_filter.insertTimePoint(2048.0, 2000us);
    EXPECT_EQ(m_filter.calcHostTime(2048.0), 2000us);
    m_filter.insertTimePoint(3072.0, 3000us);
    EXPECT_EQ(m_filter.calcHostTime(3072.0), 3000us);
    m_filter.insertTimePoint(4096.0, 4000us);
    EXPECT_EQ(m_filter.calcHostTime(4096.0), 4000us);
    m_filter.insertTimePoint(5120.0, 5000us);
    EXPECT_EQ(m_filter.calcHostTime(5120.0), 5000us);
    m_filter.insertTimePoint(6144.0, 6000us);
    EXPECT_EQ(m_filter.calcHostTime(6144.0), 6000us);
    m_filter.insertTimePoint(7168.0, 7000us);
    EXPECT_EQ(m_filter.calcHostTime(7168.0), 7000us);
}

TEST_F(HostTimeFilterTest, FasterFreqWithJitter) {
    // Use 1024 sample buffer interval, with 100us host time jitter
    m_filter.insertTimePoint(1024.0, 1000us);
    EXPECT_EQ(m_filter.calcHostTime(1024.0), HostTimeFilter::kInvalidHostTime);
    m_filter.insertTimePoint(2048.0, 2100us);
    EXPECT_EQ(m_filter.calcHostTime(2048.0), 2100us);
    m_filter.insertTimePoint(3072.0, 3000us);
    EXPECT_EQ(m_filter.calcHostTime(3072.0), 3033us);
    m_filter.insertTimePoint(4096.0, 3900us);
    EXPECT_EQ(m_filter.calcHostTime(4096.0), 3940us);
    m_filter.insertTimePoint(5120.0, 5000us);
    EXPECT_EQ(m_filter.calcHostTime(5120.0), 4960us);
    m_filter.insertTimePoint(6144.0, 6000us);
    EXPECT_EQ(m_filter.calcHostTime(6144.0), 5960us);
    m_filter.insertTimePoint(7168.0, 7000us);
    EXPECT_EQ(m_filter.calcHostTime(7168.0), 7000us);
}

TEST_F(HostTimeFilterTest, FasterFreqSkippedPoints) {
    // Use 1024 sample buffer interval, instead of auxiliarry clock in time units
    m_filter.insertTimePoint(1024.0, 1000us);
    EXPECT_EQ(m_filter.calcHostTime(1024.0), HostTimeFilter::kInvalidHostTime);
    m_filter.insertTimePoint(2048.0, 2000us);
    EXPECT_EQ(m_filter.calcHostTime(2048.0), 2000us);
    m_filter.insertTimePoint(4096.0, 4000us);
    EXPECT_EQ(m_filter.calcHostTime(4096.0), 4000us);
    m_filter.insertTimePoint(5120.0, 5000us);
    EXPECT_EQ(m_filter.calcHostTime(5120.0), 5000us);
    m_filter.insertTimePoint(8192.0, 8000us);
    EXPECT_EQ(m_filter.calcHostTime(8192.0), 8000us);
    m_filter.insertTimePoint(9216.0, 9000us);
    EXPECT_EQ(m_filter.calcHostTime(9216.0), 9000us);
    m_filter.insertTimePoint(11264.0, 11000us);
    EXPECT_EQ(m_filter.calcHostTime(11264.0), 11000us);
}

TEST_F(HostTimeFilterTest, Reset) {
    m_filter.insertTimePoint(1.0, 1050us);
    m_filter.insertTimePoint(2.0, 1950us);
    m_filter.clear();
    EXPECT_EQ(m_filter.calcHostTime(4.0), HostTimeFilter::kInvalidHostTime);
}

TEST_F(HostTimeFilterTest, DenominatorZero) {
    // Add two identical points to ensure the denominator becomes zero
    m_filter.insertTimePoint(1.0, 1000us);
    EXPECT_EQ(m_filter.calcHostTime(1.0), HostTimeFilter::kInvalidHostTime);
    m_filter.insertTimePoint(1.0, 1000us);
    EXPECT_EQ(m_filter.calcHostTime(1.0), HostTimeFilter::kInvalidHostTime);
}
