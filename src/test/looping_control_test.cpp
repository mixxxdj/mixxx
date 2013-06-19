#include <gtest/gtest.h>

#include <QtDebug>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include "controlobjectthread.h"
#include "engine/loopingcontrol.h"

class LoopingControlTest : public MixxxTest {
  public:
    LoopingControlTest()
            : m_pGroup("[Channel1]"),
              kTrackLengthSamples(1000) {
    }

  protected:
    virtual void SetUp() {
        m_pQuantizeEnabled.reset(
            new ControlPushButton(ConfigKey(m_pGroup, "quantize")));
        m_pQuantizeEnabled->set(1.0f);
        m_pQuantizeEnabled->setButtonMode(ControlPushButton::TOGGLE);
        m_pNextBeat.reset(new ControlObject(ConfigKey(m_pGroup, "beat_next")));
        m_pNextBeat->set(-1);
        m_pClosestBeat.reset(
            new ControlObject(ConfigKey(m_pGroup, "beat_closest")));
        m_pClosestBeat->set(-1);
        m_pTrackSamples.reset(
            new ControlObject(ConfigKey(m_pGroup, "track_samples")));
        m_pTrackSamples->set(kTrackLengthSamples);

        m_pLoopingControl.reset(new LoopingControl(m_pGroup, m_pConfig.data()));
        TrackPointer track = TrackPointer(new TrackInfoObject("foo"));
        track->setSampleRate(44100);
        m_pLoopingControl->trackLoaded(track);
        m_pButtonLoopIn.reset(getControlObjectThread(
                ConfigKey(m_pGroup, "loop_in")));
        m_pButtonLoopOut.reset(getControlObjectThread(
                ConfigKey(m_pGroup, "loop_out")));
        m_pButtonLoopExit.reset(getControlObjectThread(
                ConfigKey(m_pGroup, "loop_exit")));
        m_pButtonReloopExit.reset(getControlObjectThread(
                ConfigKey(m_pGroup, "reloop_exit")));
        m_pButtonLoopDouble.reset(getControlObjectThread(
                ConfigKey(m_pGroup, "loop_double")));
        m_pButtonLoopHalve.reset(getControlObjectThread(
                ConfigKey(m_pGroup, "loop_halve")));
        m_pLoopEnabled.reset(getControlObjectThread(
                ConfigKey(m_pGroup, "loop_enabled")));
        m_pLoopStartPoint.reset(getControlObjectThread(
                ConfigKey(m_pGroup, "loop_start_position")));
        m_pLoopEndPoint.reset(getControlObjectThread(
                ConfigKey(m_pGroup, "loop_end_position")));
    }

    virtual void TearDown() {
    }

    bool isLoopEnabled() {
        return m_pLoopEnabled->get() > 0.0;
    }

    const char* m_pGroup;
    const int kTrackLengthSamples;
    QScopedPointer<ControlObject> m_pNextBeat;
    QScopedPointer<ControlObject> m_pClosestBeat;
    QScopedPointer<ControlPushButton> m_pQuantizeEnabled;
    QScopedPointer<ControlObject> m_pTrackSamples;
    QScopedPointer<LoopingControl> m_pLoopingControl;
    QScopedPointer<ControlObjectThread> m_pButtonLoopIn;
    QScopedPointer<ControlObjectThread> m_pButtonLoopOut;
    QScopedPointer<ControlObjectThread> m_pButtonLoopExit;
    QScopedPointer<ControlObjectThread> m_pButtonReloopExit;
    QScopedPointer<ControlObjectThread> m_pButtonLoopDouble;
    QScopedPointer<ControlObjectThread> m_pButtonLoopHalve;
    QScopedPointer<ControlObjectThread> m_pLoopEnabled;
    QScopedPointer<ControlObjectThread> m_pLoopStartPoint;
    QScopedPointer<ControlObjectThread> m_pLoopEndPoint;
};

TEST_F(LoopingControlTest, LoopSet) {
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(100);
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    ControlObject::sync();
    EXPECT_FALSE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopSetOddSamples) {
    m_pLoopStartPoint->slotSet(1);
    m_pLoopEndPoint->slotSet(101);
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    ControlObject::sync();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopInSetInsideLoopContinues) {
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(100);
    m_pButtonReloopExit->slotSet(1);
    m_pButtonReloopExit->slotSet(0);
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    ControlObject::sync();
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
    m_pLoopStartPoint->slotSet(10);
    ControlObject::sync();
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(10, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopInSetAfterLoopOutStops) {
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(100);
    m_pButtonReloopExit->slotSet(1);
    m_pButtonReloopExit->slotSet(0);
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    ControlObject::sync();
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
    m_pLoopStartPoint->slotSet(110);
    ControlObject::sync();
    EXPECT_FALSE(isLoopEnabled());
    EXPECT_EQ(110, m_pLoopStartPoint->get());
    EXPECT_EQ(-1, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopOutSetInsideLoopContinues) {
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(100);
    m_pButtonReloopExit->slotSet(1);
    m_pButtonReloopExit->slotSet(0);
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    ControlObject::sync();
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
    m_pLoopEndPoint->slotSet(80);
    ControlObject::sync();
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(80, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopOutSetBeforeLoopInIgnored) {
    m_pLoopStartPoint->slotSet(10);
    m_pLoopEndPoint->slotSet(100);
    m_pButtonReloopExit->slotSet(1);
    m_pButtonReloopExit->slotSet(0);
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    ControlObject::sync();
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(10, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
    m_pLoopEndPoint->slotSet(0);
    ControlObject::sync();
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(10, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopInButton_QuantizeDisabled) {
    m_pQuantizeEnabled->set(0);
    m_pClosestBeat->set(100);
    m_pNextBeat->set(100);
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    m_pButtonLoopIn->slotSet(1);
    m_pButtonLoopIn->slotSet(0);
    ControlObject::sync();
    EXPECT_EQ(50, m_pLoopStartPoint->get());
}

TEST_F(LoopingControlTest, LoopInButton_QuantizeEnabledNoBeats) {
    m_pQuantizeEnabled->set(1);
    m_pClosestBeat->set(-1);
    m_pNextBeat->set(-1);
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    m_pButtonLoopIn->slotSet(1);
    m_pButtonLoopIn->slotSet(0);
    ControlObject::sync();
    EXPECT_EQ(50, m_pLoopStartPoint->get());
}

TEST_F(LoopingControlTest, LoopInButton_QuantizeEnabledClosestBeat) {
    m_pQuantizeEnabled->set(1);
    m_pClosestBeat->set(100);
    m_pNextBeat->set(110);
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    m_pButtonLoopIn->slotSet(1);
    m_pButtonLoopIn->slotSet(0);
    ControlObject::sync();
    EXPECT_EQ(100, m_pLoopStartPoint->get());
}

TEST_F(LoopingControlTest, LoopOutButton_QuantizeDisabled) {
    m_pQuantizeEnabled->set(0);
    m_pClosestBeat->set(100);
    m_pNextBeat->set(100);
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    m_pLoopStartPoint->slotSet(0);
    m_pButtonLoopOut->slotSet(1);
    m_pButtonLoopOut->slotSet(0);
    ControlObject::sync();
    EXPECT_EQ(50, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopOutButton_QuantizeEnabledNoBeats) {
    m_pQuantizeEnabled->set(1);
    m_pClosestBeat->set(-1);
    m_pNextBeat->set(-1);
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    m_pLoopStartPoint->slotSet(0);
    m_pButtonLoopOut->slotSet(1);
    m_pButtonLoopOut->slotSet(0);
    ControlObject::sync();
    EXPECT_EQ(50, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopOutButton_QuantizeEnabledClosestBeat) {
    m_pQuantizeEnabled->set(1);
    m_pClosestBeat->set(100);
    m_pNextBeat->set(110);
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    m_pLoopStartPoint->slotSet(0);
    m_pButtonLoopOut->slotSet(1);
    m_pButtonLoopOut->slotSet(0);
    ControlObject::sync();
    EXPECT_EQ(100, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, ReloopExitButton_TogglesLoop) {
    m_pQuantizeEnabled->set(0);
    m_pClosestBeat->set(-1);
    m_pNextBeat->set(-1);
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    m_pLoopStartPoint->slotSet(0);
    m_pButtonLoopOut->slotSet(1);
    m_pButtonLoopOut->slotSet(0);
    ControlObject::sync();
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(50, m_pLoopEndPoint->get());
    m_pButtonReloopExit->slotSet(1);
    m_pButtonReloopExit->slotSet(0);
    ControlObject::sync();
    EXPECT_FALSE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(50, m_pLoopEndPoint->get());
    m_pButtonReloopExit->slotSet(1);
    m_pButtonReloopExit->slotSet(0);
    ControlObject::sync();
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(50, m_pLoopEndPoint->get());
    // Ensure that the Loop Exit button works, and that it doesn't act as a
    // toggle.
    m_pButtonLoopExit->slotSet(1);
    m_pButtonLoopExit->slotSet(0);
    ControlObject::sync();
    EXPECT_FALSE(isLoopEnabled());
    m_pButtonLoopExit->slotSet(1);
    m_pButtonLoopExit->slotSet(0);
    ControlObject::sync();
    EXPECT_FALSE(isLoopEnabled());
}

TEST_F(LoopingControlTest, LoopDoubleButton_DoublesLoop) {
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(50);
    ControlObject::sync();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(50, m_pLoopEndPoint->get());
    m_pButtonLoopDouble->slotSet(1);
    m_pButtonLoopDouble->slotSet(0);
    ControlObject::sync();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
    m_pButtonLoopDouble->slotSet(1);
    m_pButtonLoopDouble->slotSet(0);
    ControlObject::sync();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(200, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopDoubleButton_IgnoresPastTrackEnd) {
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(600);
    ControlObject::sync();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(600, m_pLoopEndPoint->get());
    m_pButtonLoopDouble->slotSet(1);
    m_pButtonLoopDouble->slotSet(0);
    ControlObject::sync();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(600, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopHalveButton_HalvesLoop) {
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(200);
    ControlObject::sync();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(200, m_pLoopEndPoint->get());
    m_pButtonLoopHalve->slotSet(1);
    m_pButtonLoopHalve->slotSet(0);
    ControlObject::sync();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
    m_pButtonLoopHalve->slotSet(1);
    m_pButtonLoopHalve->slotSet(0);
    ControlObject::sync();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(50, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopHalveButton_IgnoresTooSmall) {
    m_pLoopingControl->process(1.0, 50, kTrackLengthSamples, 128);
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(40);
    ControlObject::sync();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(40, m_pLoopEndPoint->get());
    m_pButtonLoopHalve->slotSet(1);
    m_pButtonLoopHalve->slotSet(0);
    ControlObject::sync();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(40, m_pLoopEndPoint->get());
}
