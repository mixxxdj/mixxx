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

TEST_F(SoftTakeoverTest, SoftTakeover_DoesntIgnoresDisabledControl) {
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

TEST_F(SoftTakeoverTest, SoftTakeover_DoesntIgnoreSameValue) {
    // Range -1.0 to 1.0
    QScopedPointer<ControlPotmeter> co(new ControlPotmeter(
        ConfigKey("[Channel1]", "test_pot"), -1.0, 1.0));

    SoftTakeoverCtrl st_control;
    st_control.enable(co.data());
    // First is always ignored.
    EXPECT_TRUE(st_control.ignore(co.data(), 0.6));
    EXPECT_FALSE(st_control.ignore(co.data(), 0.6));
}

}  // namespace
