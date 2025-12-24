#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <vector>

using namespace std::chrono_literals;

namespace {

static constexpr auto kMinResolution = 1ms;
static constexpr auto kNumIterations = 1000;

} // namespace

TEST(ChronoClockResolutionTest, SteadyClockTicksAreSmallerThan1Ms) {
    std::vector<std::chrono::nanoseconds> ticks;
    std::generate_n(std::back_inserter(ticks), kNumIterations, []() {
        const auto start = std::chrono::steady_clock::now();
        const auto end = std::chrono::steady_clock::now();
        return end - start;
    });
    // Make sure there is no aliasing going on.
    ASSERT_TRUE(std::all_of(ticks.begin(), ticks.end(), [](const auto& tick) {
        return tick >= 0ns;
    }));
    // make sure that at least one duration is smaller than the minimum resolution
    ASSERT_LT(*std::min_element(ticks.begin(), ticks.end()), kMinResolution);
};
