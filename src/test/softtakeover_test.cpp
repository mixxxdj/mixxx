#include "controllers/softtakeover.h"

#include <gtest/gtest.h>

#include <gsl/pointers>

#include "control/controlpotmeter.h"
#include "test/mixxxtest.h"
#include "util/time.h"

namespace {

using namespace std::chrono_literals;

class SoftTakeoverTest : public MixxxTest {
  protected:
    void SetUp() override {
        mixxx::Time::setTestMode(true);
        mixxx::Time::addTestTime(10ms);
    }

    void TearDown() override {
        mixxx::Time::setTestMode(false);
    }
};

TEST_F(SoftTakeoverTest, DoesntIgnoreDisabledControl) {
    // Range -1.0 to 1.0
    ControlPotmeter co(ConfigKey("[Channel1]", "test_pot"), -1.0, 1.0);

    SoftTakeoverCtrl st_control;
    EXPECT_FALSE(st_control.ignore(&co, co.get()));
}

TEST_F(SoftTakeoverTest, IgnoresFirstValue) {
    // Range -1.0 to 1.0
    ControlPotmeter co(ConfigKey("[Channel1]", "test_pot"), -1.0, 1.0);

    SoftTakeoverCtrl st_control;
    st_control.enable(gsl::make_not_null(&co));
    EXPECT_TRUE(st_control.ignore(&co, 5));
}

TEST_F(SoftTakeoverTest, DoesntIgnoreSameValue) {
    // Range -1.0 to 1.0
    ControlPotmeter co(ConfigKey("[Channel1]", "test_pot"), -1.0, 1.0);

    co.set(0.6);
    SoftTakeoverCtrl st_control;
    st_control.enable(gsl::make_not_null(&co));
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(0.6)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(0.6)));
}

// These are corner cases that allow for quickly flicking/whipping controls
//  from a standstill when the previous knob value matches the current CO value
TEST_F(SoftTakeoverTest, SuperFastPrevEqCurrent) {
    ControlPotmeter co(ConfigKey("[Channel1]", "test_pot"), -250, 250);

    // From the bottom
    co.set(-250);
    SoftTakeoverCtrl st_control;
    st_control.enable(gsl::make_not_null(&co));

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(-250)));
    // This can happen any time afterwards, so we test 10 seconds
    mixxx::Time::addTestTime(10s);
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(200)));

    // From the top
    co.set(250);
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(250)));
    // This can happen any time afterwards, so we test 10 seconds
    mixxx::Time::addTestTime(10s);
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(-200)));
}

// But when they don't match, this type of thing should be ignored!
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTest, DISABLED_SuperFastNotSame) {
    ControlPotmeter co(ConfigKey("[Channel1]", "test_pot"), -250, 250);

    co.set(250);
    SoftTakeoverCtrl st_control;
    st_control.enable(gsl::make_not_null(&co));

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(249)));
    // This can happen any time afterwards, so we test 10 seconds
    mixxx::Time::addTestTime(10s);
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(-200)));
}

/* The meat of the tests
 * (See description in SoftTakeover::ignore() )
 *
 * The test matrix is given in the accompanying CSV file.
 * There are 32 possible cases due to five binary possibilities:
 *  - Previous value distance from current (near/far)
 *  - Previous value side of current (less/greater)
 *  - New value distance
 *  - New value side
 *  - New value arrival time (below or above threshold)
 */

class SoftTakeoverTestWithValue : public SoftTakeoverTest {
  protected:
    ControlPotmeter co{ConfigKey("[Channel1]", "test_pot"), -250, 250};
    SoftTakeoverCtrl st_control{};

    void SetUp() override {
        SoftTakeoverTest::SetUp();
        co.set(50);
        st_control.enable(gsl::make_not_null(&co));
    }
};

// ---- Previous Near & less than current
TEST_F(SoftTakeoverTestWithValue, PrevNearLess_NewNearLess_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(40)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(45)));
}

TEST_F(SoftTakeoverTestWithValue, PrevNearLess_NewNearMore_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(40)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(60)));
}

TEST_F(SoftTakeoverTestWithValue, PrevNearLess_NewFarLess_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(40)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(1)));
}

TEST_F(SoftTakeoverTestWithValue, PrevNearLess_NewFarMore_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(40)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(100)));
}

TEST_F(SoftTakeoverTestWithValue, PrevNearLess_NewNearLess_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(40)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(45)));
}

TEST_F(SoftTakeoverTestWithValue, PrevNearLess_NewNearMore_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(40)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(60)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     close           far             later               TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTestWithValue, DISABLED_PrevNearLess_NewFarLess_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(40)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(1)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  opposite close           far             later               TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTestWithValue, DISABLED_PrevNearLess_NewFarMore_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(40)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(100)));
}

// ---- Previous Near & greater than current

TEST_F(SoftTakeoverTestWithValue, PrevNearMore_NewNearLess_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(55)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(45)));
}

TEST_F(SoftTakeoverTestWithValue, PrevNearMore_NewNearMore_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(55)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(60)));
}

TEST_F(SoftTakeoverTestWithValue, PrevNearMore_NewFarLess_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(55)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(1)));
}

TEST_F(SoftTakeoverTestWithValue, PrevNearMore_NewFarMore_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(55)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(100)));
}

TEST_F(SoftTakeoverTestWithValue, PrevNearMore_NewNearLess_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(55)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(45)));
}

TEST_F(SoftTakeoverTestWithValue, PrevNearMore_NewNearMore_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(55)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(60)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  opposite close           far             later               TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTestWithValue, DISABLED_PrevNearMore_NewFarLess_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(55)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(1)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     close           far             later               TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTestWithValue, DISABLED_PrevNearMore_NewFarMore_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(55)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(100)));
}

// ---- Previous Far & less than current

TEST_F(SoftTakeoverTestWithValue, PrevFarLess_NewNearLess_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(-50)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(45)));
}

TEST_F(SoftTakeoverTestWithValue, PrevFarLess_NewNearMore_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(-50)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(60)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     far             far             soon                TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTestWithValue, DISABLED_PrevFarLess_NewFarLess_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(-50)));
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(1)));
}

TEST_F(SoftTakeoverTestWithValue, PrevFarLess_NewFarMore_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(-50)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(100)));
}

TEST_F(SoftTakeoverTestWithValue, PrevFarLess_NewNearLess_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(-50)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(45)));
}

TEST_F(SoftTakeoverTestWithValue, PrevFarLess_NewNearMore_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(-50)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(60)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     far             far             later               TRUE
TEST_F(SoftTakeoverTestWithValue, PrevFarLess_NewFarLess_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(-50)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(1)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  opposite far             far             later               TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTestWithValue, DISABLED_PrevFarLess_NewFarMore_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(-50)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(100)));
}

// ---- Previous Far & greater than current

TEST_F(SoftTakeoverTestWithValue, PrevFarMore_NewNearLess_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(120)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(45)));
}

TEST_F(SoftTakeoverTestWithValue, PrevFarMore_NewNearMore_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(120)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(60)));
}

TEST_F(SoftTakeoverTestWithValue, PrevFarMore_NewFarLess_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(120)));
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(1)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     far             far             soon                TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTestWithValue, DISABLED_PrevFarMore_NewFarMore_Soon) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(120)));
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(100)));
}

TEST_F(SoftTakeoverTestWithValue, PrevFarMore_NewNearLess_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(120)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(45)));
}

TEST_F(SoftTakeoverTestWithValue, PrevFarMore_NewNearMore_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(120)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(60)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  opposite far             far             later               TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTestWithValue, DISABLED_PrevFarMore_NewFarLess_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(120)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(1)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     far             far             later               TRUE
TEST_F(SoftTakeoverTestWithValue, PrevFarMore_NewFarMore_Late) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(120)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(100)));
}

TEST_F(SoftTakeoverTest, CatchOutOfBounds) {
    ControlPotmeter co(ConfigKey("[Channel1]", "test_pot"), -250, 250, true);

    co.set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(&co);

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(45)));
    // Cross the original value to take over
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(55)));

    // Set value to an out of bounds value
    co.set(300);
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    // Actions in the same direction shall be ignored
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(60)));
    // actions in the other edirection shall be ignored
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(40)));
    // reaching the lower border should be ignored
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(-250)));
    // reaching the upper border near the out of bounds value should not be ignored.
    EXPECT_FALSE(st_control.ignore(&co, co.getParameterForValue(250)));
}

TEST_F(SoftTakeoverTestWithValue, willIgnore) {
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(120)));
    SoftTakeover::TestAccess::advanceTimePastThreshold();
    EXPECT_TRUE(st_control.willIgnore(&co, co.getParameterForValue(80)));
    // do not ignore a value in range;
    EXPECT_FALSE(st_control.willIgnore(&co, co.getParameterForValue(51)));
    // but still ignore a value outside range because willIgnore just tests
    EXPECT_TRUE(st_control.ignore(&co, co.getParameterForValue(80)));
}

// For the ignore cases, check that they work correctly with various signed values
// TODO

}  // namespace
