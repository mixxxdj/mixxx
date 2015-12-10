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
 * The test matrix is given in the accompanying CSV file. Of the 32 possible 
 *  cases, only four should be ignored.
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
    Time::setTestElapsedTime(100);
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
    Time::setTestElapsedTime(100);
    EXPECT_FALSE(st_control.ignore(co.data(), 60));
}

TEST_F(SoftTakeoverTest, PrevNearLess_NewFarLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 40));
    Time::setTestElapsedTime(100);
    EXPECT_FALSE(st_control.ignore(co.data(), 1));
}

TEST_F(SoftTakeoverTest, PrevNearLess_NewFarMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 40));
    Time::setTestElapsedTime(100);
    EXPECT_FALSE(st_control.ignore(co.data(), 100));
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
    Time::setTestElapsedTime(100);
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
    Time::setTestElapsedTime(100);
    EXPECT_FALSE(st_control.ignore(co.data(), 60));
}

TEST_F(SoftTakeoverTest, PrevNearMore_NewFarLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 55));
    Time::setTestElapsedTime(100);
    EXPECT_FALSE(st_control.ignore(co.data(), 1));
}

TEST_F(SoftTakeoverTest, PrevNearMore_NewFarMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 55));
    Time::setTestElapsedTime(100);
    EXPECT_FALSE(st_control.ignore(co.data(), 100));
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

// This should be ignored
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
    Time::setTestElapsedTime(100);
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
    Time::setTestElapsedTime(100);
    EXPECT_FALSE(st_control.ignore(co.data(), 60));
}

// This should be ignored
TEST_F(SoftTakeoverTest, PrevFarLess_NewFarLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -50));
    Time::setTestElapsedTime(100);
    EXPECT_TRUE(st_control.ignore(co.data(), 1));
}

TEST_F(SoftTakeoverTest, PrevFarLess_NewFarMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -50));
    Time::setTestElapsedTime(100);
    EXPECT_FALSE(st_control.ignore(co.data(), 100));
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

// This should be ignored
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
    Time::setTestElapsedTime(100);
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
    Time::setTestElapsedTime(100);
    EXPECT_FALSE(st_control.ignore(co.data(), 60));
}

TEST_F(SoftTakeoverTest, PrevFarMore_NewFarLess_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 120));
    Time::setTestElapsedTime(100);
    EXPECT_FALSE(st_control.ignore(co.data(), 1));
}

// This should be ignored
TEST_F(SoftTakeoverTest, PrevFarMore_NewFarMore_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(50);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 120));
    Time::setTestElapsedTime(100);
    EXPECT_TRUE(st_control.ignore(co.data(), 100));
}

// For the ignore cases, check that they work correctly with various signed values


// Far away and less than current, within time threshold

TEST_F(SoftTakeoverTest, PrevFarLessNeg_NewFarLessNeg_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(100);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -50));
    EXPECT_TRUE(st_control.ignore(co.data(), -20));
}

TEST_F(SoftTakeoverTest, PrevFarLessNeg_NewFarLessPos_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(100);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -50));
    EXPECT_TRUE(st_control.ignore(co.data(), 20));
}

TEST_F(SoftTakeoverTest, PrevFarLessPos_NewFarLessNeg_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(100);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 50));
    EXPECT_TRUE(st_control.ignore(co.data(), -20));
}

TEST_F(SoftTakeoverTest, PrevFarLessPos_NewFarLessPos_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(100);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 50));
    EXPECT_TRUE(st_control.ignore(co.data(), 20));
}


// Far away and less than current, over time threshold

TEST_F(SoftTakeoverTest, PrevFarLessNeg_NewFarLessNeg_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(100);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -50));
    Time::setTestElapsedTime(100);
    EXPECT_TRUE(st_control.ignore(co.data(), -20));
}

TEST_F(SoftTakeoverTest, PrevFarLessNeg_NewFarLessPos_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(100);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -50));
    Time::setTestElapsedTime(100);
    EXPECT_TRUE(st_control.ignore(co.data(), 20));
}

TEST_F(SoftTakeoverTest, PrevFarLessPos_NewFarLessNeg_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(100);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 50));
    Time::setTestElapsedTime(100);
    EXPECT_TRUE(st_control.ignore(co.data(), -20));
}

TEST_F(SoftTakeoverTest, PrevFarLessPos_NewFarLessPos_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(100);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 50));
    Time::setTestElapsedTime(100);
    EXPECT_TRUE(st_control.ignore(co.data(), 20));
}

// Far away and more than current, within time threshold

TEST_F(SoftTakeoverTest, PrevFarMoreNeg_NewFarMoreNeg_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(-200);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -150));
    EXPECT_TRUE(st_control.ignore(co.data(), -100));
}

TEST_F(SoftTakeoverTest, PrevFarMoreNeg_NewFarMorePos_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(-200);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -150));
    EXPECT_TRUE(st_control.ignore(co.data(), 100));
}

TEST_F(SoftTakeoverTest, PrevFarMorePos_NewFarMoreNeg_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(-200);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 150));
    EXPECT_TRUE(st_control.ignore(co.data(), -100));
}

TEST_F(SoftTakeoverTest, PrevFarMorePos_NewFarMorePos_Soon) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(-200);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 150));
    EXPECT_TRUE(st_control.ignore(co.data(), 100));
}

// Far away and more than current, over time threshold

TEST_F(SoftTakeoverTest, PrevFarMoreNeg_NewFarMoreNeg_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(-200);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -150));
    Time::setTestElapsedTime(100);
    EXPECT_TRUE(st_control.ignore(co.data(), -100));
}

TEST_F(SoftTakeoverTest, PrevFarMoreNeg_NewFarMorePos_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(-200);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), -150));
    Time::setTestElapsedTime(100);
    EXPECT_TRUE(st_control.ignore(co.data(), 100));
}

TEST_F(SoftTakeoverTest, PrevFarMorePos_NewFarMoreNeg_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(-200);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 150));
    Time::setTestElapsedTime(100);
    EXPECT_TRUE(st_control.ignore(co.data(), -100));
}

TEST_F(SoftTakeoverTest, PrevFarMorePos_NewFarMorePos_Late) {
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -250, 250));
    
    co->set(-200);
    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 150));
    Time::setTestElapsedTime(100);
    EXPECT_TRUE(st_control.ignore(co.data(), 100));
}


}  // namespace
