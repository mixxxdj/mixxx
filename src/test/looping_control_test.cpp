#include <gtest/gtest.h>

#include <QtDebug>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include "controlobjectthread.h"
#include "engine/loopingcontrol.h"
#include "test/mockedenginebackendtest.h"

class LoopingControlTest : public MockedEngineBackendTest {
  public:
    LoopingControlTest()
            : kTrackLengthSamples(3000) {
    }

  protected:
    virtual void SetUp() {
        MockedEngineBackendTest::SetUp();
        m_pQuantizeEnabled.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "quantize")));
        m_pQuantizeEnabled->set(1.0);
        m_pNextBeat.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "beat_next")));
        m_pNextBeat->set(-1);
        m_pClosestBeat.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "beat_closest")));
        m_pClosestBeat->set(-1);
        m_pTrackSamples.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "track_samples")));
        m_pTrackSamples->set(kTrackLengthSamples);

        m_pButtonLoopIn.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "loop_in")));
        m_pButtonLoopOut.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "loop_out")));
        m_pButtonLoopExit.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "loop_exit")));
        m_pButtonReloopExit.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "reloop_exit")));
        m_pButtonLoopDouble.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "loop_double")));
        m_pButtonLoopHalve.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "loop_halve")));
        m_pLoopEnabled.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "loop_enabled")));
        m_pLoopStartPoint.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "loop_start_position")));
        m_pLoopEndPoint.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "loop_end_position")));
        m_pPlayPosition.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "playposition")));
        m_pButtonBeatMoveForward.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "loop_move_1_forward")));
        m_pButtonBeatMoveBackward.reset(getControlObjectThread(
                ConfigKey(m_sGroup1, "loop_move_1_backward")));
    }

    bool isLoopEnabled() {
        return m_pLoopEnabled->get() > 0.0;
    }

    void seekToSampleAndProcess(double new_pos) {
        m_pChannel1->getEngineBuffer()->queueNewPlaypos(new_pos, EngineBuffer::SEEK_STANDARD);
        ProcessBuffer();
    }

    const int kTrackLengthSamples;
    QScopedPointer<ControlObjectThread> m_pNextBeat;
    QScopedPointer<ControlObjectThread> m_pClosestBeat;
    QScopedPointer<ControlObjectThread> m_pQuantizeEnabled;
    QScopedPointer<ControlObjectThread> m_pTrackSamples;
    QScopedPointer<ControlObjectThread> m_pButtonLoopIn;
    QScopedPointer<ControlObjectThread> m_pButtonLoopOut;
    QScopedPointer<ControlObjectThread> m_pButtonLoopExit;
    QScopedPointer<ControlObjectThread> m_pButtonReloopExit;
    QScopedPointer<ControlObjectThread> m_pButtonLoopDouble;
    QScopedPointer<ControlObjectThread> m_pButtonLoopHalve;
    QScopedPointer<ControlObjectThread> m_pLoopEnabled;
    QScopedPointer<ControlObjectThread> m_pLoopStartPoint;
    QScopedPointer<ControlObjectThread> m_pLoopEndPoint;
    QScopedPointer<ControlObjectThread> m_pPlayPosition;
    QScopedPointer<ControlObjectThread> m_pButtonBeatMoveForward;
    QScopedPointer<ControlObjectThread> m_pButtonBeatMoveBackward;
};

TEST_F(LoopingControlTest, LoopSet) {
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(100);
    seekToSampleAndProcess(50);
    EXPECT_FALSE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopSetOddSamples) {
    m_pLoopStartPoint->slotSet(1);
    m_pLoopEndPoint->slotSet(101);
    seekToSampleAndProcess(50);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopInSetInsideLoopContinues) {
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(100);
    m_pButtonReloopExit->slotSet(1);
    m_pButtonReloopExit->slotSet(0);
    seekToSampleAndProcess(50);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
    m_pLoopStartPoint->slotSet(10);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(10, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopInSetAfterLoopOutStops) {
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(100);
    m_pButtonReloopExit->slotSet(1);
    m_pButtonReloopExit->slotSet(0);
    seekToSampleAndProcess(50);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
    m_pLoopStartPoint->slotSet(110);
    EXPECT_FALSE(isLoopEnabled());
    EXPECT_EQ(110, m_pLoopStartPoint->get());
    EXPECT_EQ(-1, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopOutSetInsideLoopContinues) {
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(100);
    m_pButtonReloopExit->slotSet(1);
    m_pButtonReloopExit->slotSet(0);
    seekToSampleAndProcess(50);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
    m_pLoopEndPoint->slotSet(80);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(80, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopOutSetBeforeLoopInIgnored) {
    m_pLoopStartPoint->slotSet(10);
    m_pLoopEndPoint->slotSet(100);
    m_pButtonReloopExit->slotSet(1);
    m_pButtonReloopExit->slotSet(0);
    seekToSampleAndProcess(50);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(10, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
    m_pLoopEndPoint->slotSet(0);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(10, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopInButton_QuantizeDisabled) {
    m_pQuantizeEnabled->set(0);
    m_pClosestBeat->set(100);
    m_pNextBeat->set(100);
    seekToSampleAndProcess(50);
    m_pButtonLoopIn->slotSet(1);
    m_pButtonLoopIn->slotSet(0);
    ProcessBuffer();
    EXPECT_EQ(50, m_pLoopStartPoint->get());
}

TEST_F(LoopingControlTest, LoopInButton_QuantizeEnabledNoBeats) {
    m_pQuantizeEnabled->set(1);
    m_pClosestBeat->set(-1);
    m_pNextBeat->set(-1);
    seekToSampleAndProcess(50);
    m_pButtonLoopIn->slotSet(1);
    m_pButtonLoopIn->slotSet(0);
    EXPECT_EQ(50, m_pLoopStartPoint->get());
}

TEST_F(LoopingControlTest, LoopInButton_QuantizeEnabledClosestBeat) {
    m_pQuantizeEnabled->set(1);
    m_pClosestBeat->set(100);
    m_pNextBeat->set(110);
    seekToSampleAndProcess(50);
    m_pButtonLoopIn->slotSet(1);
    m_pButtonLoopIn->slotSet(0);
    EXPECT_EQ(100, m_pLoopStartPoint->get());
}

TEST_F(LoopingControlTest, LoopOutButton_QuantizeDisabled) {
    m_pQuantizeEnabled->set(0);
    m_pClosestBeat->set(1000);
    m_pNextBeat->set(1000);
    seekToSampleAndProcess(500);
    m_pLoopStartPoint->slotSet(0);
    m_pButtonLoopOut->slotSet(1);
    m_pButtonLoopOut->slotSet(0);
    EXPECT_EQ(500, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopOutButton_QuantizeEnabledNoBeats) {
    m_pQuantizeEnabled->set(1);
    m_pClosestBeat->set(-1);
    m_pNextBeat->set(-1);
    seekToSampleAndProcess(500);
    m_pLoopStartPoint->slotSet(0);
    m_pButtonLoopOut->slotSet(1);
    m_pButtonLoopOut->slotSet(0);
    EXPECT_EQ(500, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopOutButton_QuantizeEnabledClosestBeat) {
    m_pQuantizeEnabled->set(1);
    m_pClosestBeat->set(1000);
    m_pNextBeat->set(1100);
    seekToSampleAndProcess(500);
    m_pLoopStartPoint->slotSet(0);
    m_pButtonLoopOut->slotSet(1);
    m_pButtonLoopOut->slotSet(0);
    EXPECT_EQ(1000, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, ReloopExitButton_TogglesLoop) {
    m_pQuantizeEnabled->set(0);
    m_pClosestBeat->set(-1);
    m_pNextBeat->set(-1);
    seekToSampleAndProcess(500);
    m_pLoopStartPoint->slotSet(0);
    m_pButtonLoopOut->slotSet(1);
    m_pButtonLoopOut->slotSet(0);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(500, m_pLoopEndPoint->get());
    m_pButtonReloopExit->slotSet(1);
    m_pButtonReloopExit->slotSet(0);
    EXPECT_FALSE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(500, m_pLoopEndPoint->get());
    m_pButtonReloopExit->slotSet(1);
    m_pButtonReloopExit->slotSet(0);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(500, m_pLoopEndPoint->get());
    // Ensure that the Loop Exit button works, and that it doesn't act as a
    // toggle.
    m_pButtonLoopExit->slotSet(1);
    m_pButtonLoopExit->slotSet(0);
    EXPECT_FALSE(isLoopEnabled());
    m_pButtonLoopExit->slotSet(1);
    m_pButtonLoopExit->slotSet(0);
    EXPECT_FALSE(isLoopEnabled());
}

TEST_F(LoopingControlTest, LoopDoubleButton_DoublesLoop) {
    seekToSampleAndProcess(50);
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(500);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(500, m_pLoopEndPoint->get());
    m_pButtonLoopDouble->slotSet(1);
    m_pButtonLoopDouble->slotSet(0);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(1000, m_pLoopEndPoint->get());
    m_pButtonLoopDouble->slotSet(1);
    m_pButtonLoopDouble->slotSet(0);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(2000, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopDoubleButton_IgnoresPastTrackEnd) {
    seekToSampleAndProcess(50);
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(1600);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(1600, m_pLoopEndPoint->get());
    m_pButtonLoopDouble->slotSet(1);
    m_pButtonLoopDouble->slotSet(0);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(1600, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopHalveButton_HalvesLoop) {
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(2000);
    seekToSampleAndProcess(1800);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(2000, m_pLoopEndPoint->get());
    EXPECT_EQ(1800, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getCurrentSample());
    m_pButtonLoopHalve->slotSet(1);
    m_pButtonLoopHalve->slotSet(0);
    ProcessBuffer();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(1000, m_pLoopEndPoint->get());
    // Since the current sample was out of range of the new loop,
    // the current sample should reseek based on the new loop size.
    EXPECT_EQ(800, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getCurrentSample());
    m_pButtonLoopHalve->slotSet(1);
    m_pButtonLoopHalve->slotSet(0);
    ProcessBuffer();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(500, m_pLoopEndPoint->get());
    EXPECT_EQ(300, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getCurrentSample());
}

TEST_F(LoopingControlTest, LoopHalveButton_IgnoresTooSmall) {
    ProcessBuffer();
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(40);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(40, m_pLoopEndPoint->get());
    m_pButtonLoopHalve->slotSet(1);
    m_pButtonLoopHalve->slotSet(0);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(40, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopMoveTest) {
    // Set a crazy bpm so our super-short track of 1000 samples has a couple beats in it.
    m_pTrack1->setBpm(23520);
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(300);
    seekToSampleAndProcess(10);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(300, m_pLoopEndPoint->get());
    EXPECT_EQ(10, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getCurrentSample());

    // Move the loop out from under the playposition.
    m_pButtonBeatMoveForward->set(1.0);
    m_pButtonBeatMoveForward->set(0.0);
    ProcessBuffer();
    EXPECT_EQ(224, m_pLoopStartPoint->get());
    EXPECT_EQ(524, m_pLoopEndPoint->get());
    EXPECT_EQ(310, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getCurrentSample());

    // Move backward so that the current position is off the end of the loop.
    m_pChannel1->getEngineBuffer()->queueNewPlaypos(500, EngineBuffer::SEEK_STANDARD);
    ProcessBuffer();
    m_pButtonBeatMoveBackward->set(1.0);
    m_pButtonBeatMoveBackward->set(0.0);
    ProcessBuffer();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(300, m_pLoopEndPoint->get());
    EXPECT_EQ(200, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getCurrentSample());
}
