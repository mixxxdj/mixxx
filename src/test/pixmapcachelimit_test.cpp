#include "util/pixmapcachelimit.h"

#include <gtest/gtest.h>

#include <limits>

TEST(PixmapCacheLimitTest, ScalesWithSquareOfDevicePixelRatio) {
    struct TestCase {
        double devicePixelRatio;
        int expectedLimitKiB;
    };
    const TestCase testCases[] = {
            {1.0, 32 * 1024},
            {1.25, 50 * 1024},
            {1.5, 72 * 1024},
            {1.75, 98 * 1024},
            {2.0, 128 * 1024},
    };

    for (const auto& testCase : testCases) {
        EXPECT_EQ(
                mixxx::pixmapcache::cacheLimitKiB(testCase.devicePixelRatio),
                testCase.expectedLimitKiB)
                << "devicePixelRatio=" << testCase.devicePixelRatio;
    }
}

TEST(PixmapCacheLimitTest, RepairsInvalidDevicePixelRatio) {
    EXPECT_EQ(32 * 1024, mixxx::pixmapcache::cacheLimitKiB(0.0));
    EXPECT_EQ(32 * 1024, mixxx::pixmapcache::cacheLimitKiB(-1.0));
    EXPECT_EQ(32 * 1024,
            mixxx::pixmapcache::cacheLimitKiB(
                    std::numeric_limits<double>::quiet_NaN()));
}

TEST(PixmapCacheLimitTest, ClampsOverflow) {
    EXPECT_EQ(std::numeric_limits<int>::max(),
            mixxx::pixmapcache::cacheLimitKiB(
                    std::numeric_limits<double>::max()));
}
