#include <gtest/gtest.h>
#include <ctime>

#include "util/performancetimer.h"

class PerformanceTimerTest : public testing::Test {
  protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

// This test was added because of an signed/unsigned underflow bug that
// affected Windows and (presumably) Symbian.
// See https://bugs.launchpad.net/mixxx/+bug/1300664
TEST_F(PerformanceTimerTest, DifferenceCanBeNegative) {
    PerformanceTimer early;
    early.start();

    std::time_t start = time(0);
    while (1) {
        // use the standard clock to make sure we don't run forever
        double seconds = difftime(time(0), start);

        PerformanceTimer late;
        late.start();
        mixxx::Duration difference = early.difference(late);

        // If the high-res clock hasn't ticked yet, the difference should be 0.
        // If it has ticked, then the difference better be negative.
        ASSERT_LE(difference.toIntegerNanos(), 0);

        if (difference < mixxx::Duration::fromNanos(0)) {
            break; // test passed
        }

        if (seconds > 0.9) {
            // The standard clock ticked, but difference is still zero.
            // Assume that there is no high-resolution clock on this system.
            ASSERT_EQ(0, difference.toIntegerNanos());

            // It would be nice if gtest printed this, but it currently doesn't.
            // https://code.google.com/p/googletest/wiki/AdvancedGuide
            SUCCEED() << "inconclusive: no high-resolution timer on this system?";
            break;
        }
    }
}
