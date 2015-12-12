#include <gtest/gtest.h>
#include <QtDebug>
#include <QScopedPointer>

#include "controlpotmeter.h"
#include "controlpushbutton.h"
#include "configobject.h"
#include "controllers/softtakeover.h"
#include "test/mixxxtest.h"
#include "util/time.h"

namespace {

class SoftTakeoverTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        Time::setTestMode(true);
        Time::setTestElapsedTime(0);
    }

    virtual void TearDown() {
        Time::setTestMode(false);
    }
};

TEST_F(SoftTakeoverTest, SoftTakeover_DoesntIgnoreDisabledControl) {
    // Range -1.0 to 1.0
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -1.0, 1.0));

    SoftTakeoverCtrl st_control;
    EXPECT_FALSE(st_control.ignore(co.data(), co->get()));
}

TEST_F(SoftTakeoverTest, SoftTakeover_DoesntIgnoreNonPotmeter) {
    QScopedPointer<ControlPushButton> co(new ControlPushButton(
        ConfigKey("[Channel1]", "test_button")));

    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());

    // First update is always ignored so this proves the CPB is not enabled for
    // soft-takeover.
    EXPECT_FALSE(st_control.ignore(co.data(), 0));
}

TEST_F(SoftTakeoverTest, SoftTakeover_IgnoresFirstValue) {
    // Range -1.0 to 1.0
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -1.0, 1.0));

    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    EXPECT_TRUE(st_control.ignore(co.data(), 5));
}


// This behavior is undefined since it doesn't actually matter what happens
//  setting the same value since the result is the same. 
//  E.g. in 1.11 it ignores it, in 2.0 it doesn't.

// TEST_F(SoftTakeoverTest, SoftTakeover_DoesntIgnoreSameValue) {
//     // Range -1.0 to 1.0
//     QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
//         ConfigKey("[Channel1]", "test_pot"), -1.0, 1.0));
// 
//     SoftTakeoverCtrl st_control;
//     st_control.enable(co.data());
//     // First is always ignored.
//     EXPECT_TRUE(st_control.ignore(co.data(), 0.6));
//     EXPECT_FALSE(st_control.ignore(co.data(), 0.6));
// }



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
    EXPECT_TRUE(st_control.ignore(co.data(), 40));
    EXPECT_FALSE(st_control.ignore(co.data(), 45));
}

TEST_F(SoftTakeoverTest, PrevNearLess_NewNearMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 40));
    EXPECT_FALSE(st_control.ignore(co.data(), 60));
}

TEST_F(SoftTakeoverTest, PrevNearLess_NewFarLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 40));
    EXPECT_FALSE(st_control.ignore(co.data(), 1));
}

TEST_F(SoftTakeoverTest, PrevNearLess_NewFarMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 40));
    EXPECT_FALSE(st_control.ignore(co.data(), 100));
}

TEST_F(SoftTakeoverTest, PrevNearLess_NewNearLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 40));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_FALSE(st_control.ignore(co.data(), 45));
}

TEST_F(SoftTakeoverTest, PrevNearLess_NewNearMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 40));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_FALSE(st_control.ignore(co.data(), 60));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     close           far             later               TRUE
TEST_F(SoftTakeoverTest, PrevNearLess_NewFarLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 40));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_TRUE(st_control.ignore(co.data(), 1));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  opposite close           far             later               TRUE
TEST_F(SoftTakeoverTest, PrevNearLess_NewFarMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 40));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_TRUE(st_control.ignore(co.data(), 100));
}

// ---- Previous Near & greater than current

TEST_F(SoftTakeoverTest, PrevNearMore_NewNearLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 55));
    EXPECT_FALSE(st_control.ignore(co.data(), 45));
}

TEST_F(SoftTakeoverTest, PrevNearMore_NewNearMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 55));
    EXPECT_FALSE(st_control.ignore(co.data(), 60));
}

TEST_F(SoftTakeoverTest, PrevNearMore_NewFarLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 55));
    EXPECT_FALSE(st_control.ignore(co.data(), 1));
}

TEST_F(SoftTakeoverTest, PrevNearMore_NewFarMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 55));
    EXPECT_FALSE(st_control.ignore(co.data(), 100));
}

TEST_F(SoftTakeoverTest, PrevNearMore_NewNearLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 55));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_FALSE(st_control.ignore(co.data(), 45));
}

TEST_F(SoftTakeoverTest, PrevNearMore_NewNearMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 55));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_FALSE(st_control.ignore(co.data(), 60));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  opposite close           far             later               TRUE
TEST_F(SoftTakeoverTest, PrevNearMore_NewFarLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 55));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_TRUE(st_control.ignore(co.data(), 1));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     close           far             later               TRUE
TEST_F(SoftTakeoverTest, PrevNearMore_NewFarMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 55));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_TRUE(st_control.ignore(co.data(), 100));
}

// ---- Previous Far & less than current

TEST_F(SoftTakeoverTest, PrevFarLess_NewNearLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -50));
    EXPECT_FALSE(st_control.ignore(co.data(), 45));
}

TEST_F(SoftTakeoverTest, PrevFarLess_NewNearMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -50));
    EXPECT_FALSE(st_control.ignore(co.data(), 60));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     far             far             soon                TRUE
TEST_F(SoftTakeoverTest, PrevFarLess_NewFarLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -50));
    EXPECT_TRUE(st_control.ignore(co.data(), 1));
}

TEST_F(SoftTakeoverTest, PrevFarLess_NewFarMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -50));
    EXPECT_FALSE(st_control.ignore(co.data(), 100));
}

TEST_F(SoftTakeoverTest, PrevFarLess_NewNearLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -50));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_FALSE(st_control.ignore(co.data(), 45));
}

TEST_F(SoftTakeoverTest, PrevFarLess_NewNearMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -50));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_FALSE(st_control.ignore(co.data(), 60));
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
    EXPECT_TRUE(st_control.ignore(co.data(), -50));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_TRUE(st_control.ignore(co.data(), 1));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  opposite far             far             later               TRUE
TEST_F(SoftTakeoverTest, PrevFarLess_NewFarMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -50));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_TRUE(st_control.ignore(co.data(), 100));
}

// ---- Previous Far & greater than current

TEST_F(SoftTakeoverTest, PrevFarMore_NewNearLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 120));
    EXPECT_FALSE(st_control.ignore(co.data(), 45));
}

TEST_F(SoftTakeoverTest, PrevFarMore_NewNearMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 120));
    EXPECT_FALSE(st_control.ignore(co.data(), 60));
}

TEST_F(SoftTakeoverTest, PrevFarMore_NewFarLess_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 120));
    EXPECT_FALSE(st_control.ignore(co.data(), 1));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  same     far             far             soon                TRUE
TEST_F(SoftTakeoverTest, PrevFarMore_NewFarMore_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 120));
    EXPECT_TRUE(st_control.ignore(co.data(), 100));
}

TEST_F(SoftTakeoverTest, PrevFarMore_NewNearLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 120));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_FALSE(st_control.ignore(co.data(), 45));
}

TEST_F(SoftTakeoverTest, PrevFarMore_NewNearMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 120));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_FALSE(st_control.ignore(co.data(), 60));
}

// Ignore this case:
//  Sides    prev distance   new distance    new value arrives   Ignore
//  opposite far             far             later               TRUE
TEST_F(SoftTakeoverTest, PrevFarMore_NewFarLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 120));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_TRUE(st_control.ignore(co.data(), 1));
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
    EXPECT_TRUE(st_control.ignore(co.data(), 120));
    Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold()*2);
    EXPECT_TRUE(st_control.ignore(co.data(), 100));
}

// For the ignore cases, check that they work correctly with various signed values
// TODO

}  // namespace
