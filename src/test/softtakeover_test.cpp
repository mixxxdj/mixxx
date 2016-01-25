#include <gtest/gtest.h>
#include <QtDebug>
#include <QScopedPointer>

#include "controlpotmeter.h"
#include "controlpushbutton.h"
#include "preferences/usersettings.h"
#include "controllers/softtakeover.h"
#include "test/mixxxtest.h"
#include "util/time.h"

namespace {

class SoftTakeoverTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        Time::setTestMode(true);
        Time::setTestElapsedTime(mixxx::Duration::fromMillis(10));
    }

    virtual void TearDown() {
        Time::setTestMode(false);
    }
};

TEST_F(SoftTakeoverTest, DoesntIgnoreDisabledControl) {
    // Range -1.0 to 1.0
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -1.0, 1.0));

    SoftTakeoverCtrl st_control;
    EXPECT_FALSE(st_control.ignore(co.data(), co->get()));
}

TEST_F(SoftTakeoverTest, DoesntIgnoreNonPotmeter) {
    QScopedPointer<ControlPushButton> co(new ControlPushButton(
        ConfigKey("[Channel1]", "test_button")));

    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First update is always ignored so this proves the CPB is not enabled for
    // soft-takeover.
    EXPECT_FALSE(st_control.ignore(co.data(), 0));
}

TEST_F(SoftTakeoverTest, IgnoresFirstValue) {
    // Range -1.0 to 1.0
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -1.0, 1.0));

    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    EXPECT_TRUE(st_control.ignore(co.data(), 5));
}

TEST_F(SoftTakeoverTest, DoesntIgnoreSameValue) {
    // Range -1.0 to 1.0
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -1.0, 1.0));

    co->set(0.6);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(0.6)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(0.6)));
}

// These are corner cases that allow for quickly flicking/whipping controls
//  from a standstill when the previous knob value matches the current CO value
TEST_F(SoftTakeoverTest, SuperFastPrevEqCurrent) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    // From the bottom
    co->set(-250);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(-250)));
    // This can happen any time afterwards, so we test 10 seconds
    Time::setTestElapsedTime(mixxx::Duration::fromSeconds(10));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(200)));

    // From the top
    co->set(250);
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(250)));
    // This can happen any time afterwards, so we test 10 seconds
    Time::setTestElapsedTime(mixxx::Duration::fromSeconds(10));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(-200)));
}

// But when they don't match, this type of thing should be ignored!
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTest, DISABLED_SuperFastNotSame) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(250);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(249)));
    // This can happen any time afterwards, so we test 10 seconds
    Time::setTestElapsedTime(mixxx::Duration::fromSeconds(10));
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(-200)));
}

/* The meat of the tests
 * (See decscription in SoftTakeover::ignore() )
 *
 * The test matrix is given in the accompanying CSV file.
 * There are 32 possible cases due to five binary possibilities:
 *  - Previous value distance from current (near/far)
 *  - Previous value side of current (less/greater)
 *  - New value distance
 *  - New value side
 *  - New value arrival time (below or above threshold)
 */

// ---- Previous Near & less than current

TEST_F(SoftTakeoverTest, PrevNearLess_NewNearLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(40)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(45)));
}

TEST_F(SoftTakeoverTest, PrevNearLess_NewNearMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(40)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(60)));
}

TEST_F(SoftTakeoverTest, PrevNearLess_NewFarLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(40)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(1)));
}

TEST_F(SoftTakeoverTest, PrevNearLess_NewFarMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(40)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(100)));
}

TEST_F(SoftTakeoverTest, PrevNearLess_NewNearLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(40)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(45)));
}

TEST_F(SoftTakeoverTest, PrevNearLess_NewNearMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(40)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(60)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     close           far             later               TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTest, DISABLED_PrevNearLess_NewFarLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(40)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(1)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  opposite close           far             later               TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTest, DISABLED_PrevNearLess_NewFarMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(40)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(100)));
}

// ---- Previous Near & greater than current

TEST_F(SoftTakeoverTest, PrevNearMore_NewNearLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(55)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(45)));
}

TEST_F(SoftTakeoverTest, PrevNearMore_NewNearMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(55)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(60)));
}

TEST_F(SoftTakeoverTest, PrevNearMore_NewFarLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(55)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(1)));
}

TEST_F(SoftTakeoverTest, PrevNearMore_NewFarMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(55)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(100)));
}

TEST_F(SoftTakeoverTest, PrevNearMore_NewNearLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(55)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(45)));
}

TEST_F(SoftTakeoverTest, PrevNearMore_NewNearMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(55)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(60)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  opposite close           far             later               TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTest, DISABLED_PrevNearMore_NewFarLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(55)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(1)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     close           far             later               TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTest, DISABLED_PrevNearMore_NewFarMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(55)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(100)));
}

// ---- Previous Far & less than current

TEST_F(SoftTakeoverTest, PrevFarLess_NewNearLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(-50)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(45)));
}

TEST_F(SoftTakeoverTest, PrevFarLess_NewNearMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(-50)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(60)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     far             far             soon                TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTest, DISABLED_PrevFarLess_NewFarLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(-50)));
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(1)));
}

TEST_F(SoftTakeoverTest, PrevFarLess_NewFarMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(-50)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(100)));
}

TEST_F(SoftTakeoverTest, PrevFarLess_NewNearLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(-50)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(45)));
}

TEST_F(SoftTakeoverTest, PrevFarLess_NewNearMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(-50)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(60)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     far             far             later               TRUE
TEST_F(SoftTakeoverTest, PrevFarLess_NewFarLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(-50)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(1)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  opposite far             far             later               TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTest, DISABLED_PrevFarLess_NewFarMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(-50)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(100)));
}

// ---- Previous Far & greater than current

TEST_F(SoftTakeoverTest, PrevFarMore_NewNearLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(120)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(45)));
}

TEST_F(SoftTakeoverTest, PrevFarMore_NewNearMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(120)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(60)));
}

TEST_F(SoftTakeoverTest, PrevFarMore_NewFarLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(120)));
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(1)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     far             far             soon                TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTest, DISABLED_PrevFarMore_NewFarMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(120)));
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(100)));
}

TEST_F(SoftTakeoverTest, PrevFarMore_NewNearLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(120)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(45)));
}

TEST_F(SoftTakeoverTest, PrevFarMore_NewNearMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(120)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_FALSE(st_control.ignore(co.data(), co->getParameterForValue(60)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  opposite far             far             later               TRUE
//  FIXME: This fails on the st::ignore() implementation in 2.0.0-rc1
TEST_F(SoftTakeoverTest, DISABLED_PrevFarMore_NewFarLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(120)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(1)));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     far             far             later               TRUE
TEST_F(SoftTakeoverTest, PrevFarMore_NewFarMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));

    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(120)));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);
    EXPECT_TRUE(st_control.ignore(co.data(), co->getParameterForValue(100)));
}

// For the ignore cases, check that they work correctly with various signed values
// TODO

}  // namespace
