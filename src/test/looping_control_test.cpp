#include <gtest/gtest.h>

#include <QtDebug>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "control/controlproxy.h"
#include "engine/controls/loopingcontrol.h"
#include "test/mockedenginebackendtest.h"
#include "util/memory.h"

// Due to rounding errors loop positions should be compared with EXPECT_NEAR instead of EXPECT_EQ.
// NOTE(uklotzde, 2017-12-10): The rounding errors currently only appeared with GCC 7.2.1.
constexpr double kLoopPositionMaxAbsError = 0.000000001;

class LoopingControlTest : public MockedEngineBackendTest {
  public:
    LoopingControlTest()
            : kTrackLengthSamples(300000000) {
    }

  protected:
    void SetUp() override {
        MockedEngineBackendTest::SetUp();
        m_pQuantizeEnabled = std::make_unique<ControlProxy>(m_sGroup1, "quantize");
        m_pQuantizeEnabled->set(1.0);
        m_pNextBeat = std::make_unique<ControlProxy>(m_sGroup1, "beat_next");

        m_pNextBeat->set(-1);
        m_pClosestBeat = std::make_unique<ControlProxy>(m_sGroup1, "beat_closest");
        m_pClosestBeat->set(-1);
        m_pTrackSamples = std::make_unique<ControlProxy>(m_sGroup1, "track_samples");
        m_pTrackSamples->set(kTrackLengthSamples);
        m_pButtonLoopIn = std::make_unique<ControlProxy>(m_sGroup1, "loop_in");
        m_pButtonLoopOut = std::make_unique<ControlProxy>(m_sGroup1, "loop_out");
        m_pButtonLoopExit = std::make_unique<ControlProxy>(m_sGroup1, "loop_exit");
        m_pButtonReloopToggle = std::make_unique<ControlProxy>(m_sGroup1, "reloop_toggle");
        m_pButtonReloopAndStop = std::make_unique<ControlProxy>(m_sGroup1, "reloop_andstop");
        m_pButtonLoopDouble = std::make_unique<ControlProxy>(m_sGroup1, "loop_double");
        m_pButtonLoopHalve = std::make_unique<ControlProxy>(m_sGroup1, "loop_halve");
        m_pLoopEnabled = std::make_unique<ControlProxy>(m_sGroup1, "loop_enabled");
        m_pLoopStartPoint = std::make_unique<ControlProxy>(m_sGroup1, "loop_start_position");
        m_pLoopEndPoint = std::make_unique<ControlProxy>(m_sGroup1, "loop_end_position");
        m_pLoopScale = std::make_unique<ControlProxy>(m_sGroup1, "loop_scale");
        m_pButtonPlay = std::make_unique<ControlProxy>(m_sGroup1, "play");
        m_pPlayPosition = std::make_unique<ControlProxy>(m_sGroup1, "playposition");
        m_pButtonBeatMoveForward = std::make_unique<ControlProxy>(m_sGroup1, "loop_move_1_forward");
        m_pButtonBeatMoveBackward = std::make_unique<ControlProxy>(m_sGroup1, "loop_move_1_backward");
        m_pButtonBeatLoop2Activate = std::make_unique<ControlProxy>(m_sGroup1, "beatloop_2_activate");
        m_pButtonBeatLoop4Activate = std::make_unique<ControlProxy>(m_sGroup1, "beatloop_4_activate");
        m_pBeatLoop1Enabled = std::make_unique<ControlProxy>(m_sGroup1, "beatloop_1_enabled");
        m_pBeatLoop2Enabled = std::make_unique<ControlProxy>(m_sGroup1, "beatloop_2_enabled");
        m_pBeatLoop4Enabled = std::make_unique<ControlProxy>(m_sGroup1, "beatloop_4_enabled");
        m_pBeatLoop64Enabled = std::make_unique<ControlProxy>(m_sGroup1, "beatloop_64_enabled");
        m_pBeatLoop = std::make_unique<ControlProxy>(m_sGroup1, "beatloop");
        m_pBeatLoopSize = std::make_unique<ControlProxy>(m_sGroup1, "beatloop_size");
        m_pButtonBeatLoopActivate = std::make_unique<ControlProxy>(m_sGroup1, "beatloop_activate");
        m_pBeatJumpSize = std::make_unique<ControlProxy>(m_sGroup1, "beatjump_size");
        m_pButtonBeatJumpForward = std::make_unique<ControlProxy>(m_sGroup1, "beatjump_forward");
        m_pButtonBeatJumpBackward = std::make_unique<ControlProxy>(m_sGroup1, "beatjump_backward");
        m_pButtonBeatLoopRoll1Activate = std::make_unique<ControlProxy>(m_sGroup1, "beatlooproll_1_activate");
        m_pButtonBeatLoopRoll2Activate = std::make_unique<ControlProxy>(m_sGroup1, "beatlooproll_2_activate");
        m_pButtonBeatLoopRoll4Activate = std::make_unique<ControlProxy>(m_sGroup1, "beatlooproll_4_activate");
    }

    bool isLoopEnabled() {
        return m_pLoopEnabled->get() > 0.0;
    }

    void seekToSampleAndProcess(double new_pos) {
        m_pChannel1->getEngineBuffer()->queueNewPlaypos(new_pos, EngineBuffer::SEEK_STANDARD);
        ProcessBuffer();
    }

    const int kTrackLengthSamples;
    std::unique_ptr<ControlProxy> m_pNextBeat;
    std::unique_ptr<ControlProxy> m_pClosestBeat;
    std::unique_ptr<ControlProxy> m_pQuantizeEnabled;
    std::unique_ptr<ControlProxy> m_pTrackSamples;
    std::unique_ptr<ControlProxy> m_pButtonLoopIn;
    std::unique_ptr<ControlProxy> m_pButtonLoopOut;
    std::unique_ptr<ControlProxy> m_pButtonLoopExit;
    std::unique_ptr<ControlProxy> m_pButtonReloopToggle;
    std::unique_ptr<ControlProxy> m_pButtonReloopAndStop;
    std::unique_ptr<ControlProxy> m_pButtonLoopDouble;
    std::unique_ptr<ControlProxy> m_pButtonLoopHalve;
    std::unique_ptr<ControlProxy> m_pLoopEnabled;
    std::unique_ptr<ControlProxy> m_pLoopStartPoint;
    std::unique_ptr<ControlProxy> m_pLoopEndPoint;
    std::unique_ptr<ControlProxy> m_pLoopScale;
    std::unique_ptr<ControlProxy> m_pPlayPosition;
    std::unique_ptr<ControlProxy> m_pButtonPlay;
    std::unique_ptr<ControlProxy> m_pButtonBeatMoveForward;
    std::unique_ptr<ControlProxy> m_pButtonBeatMoveBackward;
    std::unique_ptr<ControlProxy> m_pButtonBeatLoop2Activate;
    std::unique_ptr<ControlProxy> m_pButtonBeatLoop4Activate;
    std::unique_ptr<ControlProxy> m_pBeatLoop1Enabled;
    std::unique_ptr<ControlProxy> m_pBeatLoop2Enabled;
    std::unique_ptr<ControlProxy> m_pBeatLoop4Enabled;
    std::unique_ptr<ControlProxy> m_pBeatLoop64Enabled;
    std::unique_ptr<ControlProxy> m_pBeatLoop;
    std::unique_ptr<ControlProxy> m_pBeatLoopSize;
    std::unique_ptr<ControlProxy> m_pButtonBeatLoopActivate;
    std::unique_ptr<ControlProxy> m_pBeatJumpSize;
    std::unique_ptr<ControlProxy> m_pButtonBeatJumpForward;
    std::unique_ptr<ControlProxy> m_pButtonBeatJumpBackward;
    std::unique_ptr<ControlProxy> m_pButtonBeatLoopRoll1Activate;
    std::unique_ptr<ControlProxy> m_pButtonBeatLoopRoll2Activate;
    std::unique_ptr<ControlProxy> m_pButtonBeatLoopRoll4Activate;
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
    m_pLoopEndPoint->slotSet(101.5);
    seekToSampleAndProcess(50);
    EXPECT_EQ(1, m_pLoopStartPoint->get());
    EXPECT_EQ(101.5, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopInSetInsideLoopContinues) {
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(100);
    m_pButtonReloopToggle->slotSet(1);
    m_pButtonReloopToggle->slotSet(0);
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
    m_pButtonReloopToggle->slotSet(1);
    m_pButtonReloopToggle->slotSet(0);
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
    m_pButtonReloopToggle->slotSet(1);
    m_pButtonReloopToggle->slotSet(0);
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
    m_pButtonReloopToggle->slotSet(1);
    m_pButtonReloopToggle->slotSet(0);
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

TEST_F(LoopingControlTest, LoopInButton_AdjustLoopInPointOutsideLoop) {
    m_pLoopStartPoint->slotSet(1000);
    m_pLoopEndPoint->slotSet(2000);
    m_pButtonReloopToggle->slotSet(1);
    m_pButtonReloopToggle->slotSet(0);
    m_pButtonLoopIn->slotSet(1);
    seekToSampleAndProcess(50);
    m_pButtonLoopIn->slotSet(0);
    EXPECT_EQ(50, m_pLoopStartPoint->get());
}

TEST_F(LoopingControlTest, LoopInButton_AdjustLoopInPointInsideLoop) {
    m_pLoopStartPoint->slotSet(1000);
    m_pLoopEndPoint->slotSet(2000);
    m_pButtonReloopToggle->slotSet(1);
    m_pButtonReloopToggle->slotSet(0);
    m_pButtonLoopIn->slotSet(1);
    seekToSampleAndProcess(1500);
    m_pButtonLoopIn->slotSet(0);
    EXPECT_EQ(1500, m_pLoopStartPoint->get());
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

TEST_F(LoopingControlTest, LoopOutButton_AdjustLoopOutPointOutsideLoop) {
    m_pLoopStartPoint->slotSet(1000);
    m_pLoopEndPoint->slotSet(2000);
    m_pButtonReloopToggle->slotSet(1);
    m_pButtonReloopToggle->slotSet(0);
    m_pButtonLoopOut->slotSet(1);
    seekToSampleAndProcess(3000);
    m_pButtonLoopOut->slotSet(0);
    EXPECT_EQ(3000, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopOutButton_AdjustLoopOutPointInsideLoop) {
    m_pLoopStartPoint->slotSet(100);
    m_pLoopEndPoint->slotSet(2000);
    m_pButtonReloopToggle->slotSet(1);
    m_pButtonReloopToggle->slotSet(0);
    m_pButtonLoopOut->slotSet(1);
    seekToSampleAndProcess(1500);
    m_pButtonLoopOut->slotSet(0);
    EXPECT_EQ(1500, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopInOutButtons_QuantizeEnabled) {
    m_pTrack1->setBpm(60.0);
    m_pQuantizeEnabled->set(1);
    seekToSampleAndProcess(500);
    m_pButtonLoopIn->set(1);
    m_pButtonLoopIn->set(0);
    EXPECT_EQ(m_pClosestBeat->get(), m_pLoopStartPoint->get());

    m_pBeatJumpSize->set(4);
    m_pButtonBeatJumpForward->set(1);
    m_pButtonBeatJumpForward->set(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(m_pPlayPosition->get() * kTrackLengthSamples, (44100 * 2 * 4) + 500);
    m_pButtonLoopOut->set(1);
    m_pButtonLoopOut->set(0);
    ProcessBuffer();
    EXPECT_EQ(m_pLoopEndPoint->get(), 44100 * 2 * 4);
    EXPECT_DOUBLE_EQ(m_pPlayPosition->get() * kTrackLengthSamples, (44100 * 2 * 4) + 500);

    EXPECT_EQ(4, m_pBeatLoopSize->get());
    EXPECT_TRUE(m_pBeatLoop4Enabled->toBool());

    // Check that beatloop_4_enabled is reset to 0 when changing the loop size.
    m_pBeatJumpSize->set(1);
    m_pButtonLoopOut->set(1);
    m_pButtonBeatJumpForward->set(1);
    m_pButtonBeatJumpForward->set(0);
    m_pButtonLoopOut->set(0);
    ProcessBuffer();
    EXPECT_EQ(m_pClosestBeat->get(), m_pLoopEndPoint->get());
    EXPECT_FALSE(m_pBeatLoop4Enabled->toBool());
}

TEST_F(LoopingControlTest, ReloopToggleButton_TogglesLoop) {
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
    m_pButtonReloopToggle->slotSet(1);
    m_pButtonReloopToggle->slotSet(0);
    EXPECT_FALSE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(500, m_pLoopEndPoint->get());
    m_pButtonReloopToggle->slotSet(1);
    m_pButtonReloopToggle->slotSet(0);
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

TEST_F(LoopingControlTest, ReloopToggleButton_DoesNotJumpAhead) {
    m_pLoopStartPoint->slotSet(1000);
    m_pLoopEndPoint->slotSet(2000);
    seekToSampleAndProcess(0);

    m_pButtonReloopToggle->slotSet(1);
    m_pButtonReloopToggle->slotSet(0);
    seekToSampleAndProcess(50);
    EXPECT_LE(m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current, m_pLoopStartPoint->get());
}

TEST_F(LoopingControlTest, ReloopAndStopButton) {
    m_pLoopStartPoint->slotSet(1000);
    m_pLoopEndPoint->slotSet(2000);
    seekToSampleAndProcess(1500);
    m_pButtonReloopToggle->slotSet(1);
    m_pButtonReloopToggle->slotSet(0);
    m_pButtonReloopAndStop->slotSet(1);
    m_pButtonReloopAndStop->slotSet(0);
    ProcessBuffer();
    EXPECT_EQ(m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current, m_pLoopStartPoint->get());
    EXPECT_TRUE(m_pLoopEnabled->toBool());
}

TEST_F(LoopingControlTest, LoopScale_DoublesLoop) {
    seekToSampleAndProcess(0);
    m_pButtonLoopIn->set(1);
    m_pButtonLoopIn->set(0);
    seekToSampleAndProcess(500);
    m_pButtonLoopOut->set(1);
    m_pButtonLoopOut->set(0);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(500, m_pLoopEndPoint->get());
    m_pLoopScale->set(2.0);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(1000, m_pLoopEndPoint->get());
    m_pLoopScale->set(2.0);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(2000, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopScale_HalvesLoop) {
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(2000);
    seekToSampleAndProcess(1800);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(2000, m_pLoopEndPoint->get());
    EXPECT_EQ(1800, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current);
    EXPECT_FALSE(isLoopEnabled());
    m_pLoopScale->set(0.5);
    ProcessBuffer();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(1000, m_pLoopEndPoint->get());

    // The loop was not enabled so halving the loop should not move the playhead
    // even though it is outside the loop.
    EXPECT_EQ(1800, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current);

    m_pButtonReloopToggle->slotSet(1);
    EXPECT_TRUE(isLoopEnabled());
    m_pLoopScale->set(0.5);
    ProcessBuffer();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(500, m_pLoopEndPoint->get());
    // Since the current sample was out of range of the new loop,
    // the current sample should reseek based on the new loop size.
    double target;
    double trigger = m_pChannel1->getEngineBuffer()->m_pLoopingControl->nextTrigger(
            false, 1800, &target);
    EXPECT_EQ(300, target);
    EXPECT_EQ(1800, trigger);
}

TEST_F(LoopingControlTest, LoopDoubleButton_IgnoresPastTrackEnd) {
    seekToSampleAndProcess(50);
    m_pLoopStartPoint->slotSet(kTrackLengthSamples / 2.0);
    m_pLoopEndPoint->slotSet(kTrackLengthSamples);
    EXPECT_EQ(kTrackLengthSamples / 2.0, m_pLoopStartPoint->get());
    EXPECT_EQ(kTrackLengthSamples, m_pLoopEndPoint->get());
    m_pButtonLoopDouble->slotSet(1);
    m_pButtonLoopDouble->slotSet(0);
    EXPECT_EQ(kTrackLengthSamples / 2.0, m_pLoopStartPoint->get());
    EXPECT_EQ(kTrackLengthSamples, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopDoubleButton_DoublesBeatloopSize) {
    m_pTrack1->setBpm(120.0);
    m_pBeatLoopSize->set(16.0);
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    m_pButtonLoopDouble->set(1.0);
    m_pButtonLoopDouble->set(0.0);
    EXPECT_EQ(32.0, m_pBeatLoopSize->get());
}

TEST_F(LoopingControlTest, LoopDoubleButton_DoesNotResizeManualLoop) {
    seekToSampleAndProcess(500);
    m_pButtonLoopIn->set(1.0);
    m_pButtonLoopIn->set(0.0);
    seekToSampleAndProcess(1000);
    m_pButtonLoopOut->set(1.0);
    m_pButtonLoopOut->set(0.0);
    EXPECT_EQ(500, m_pLoopStartPoint->get());
    EXPECT_EQ(1000, m_pLoopEndPoint->get());
    m_pButtonLoopDouble->slotSet(1);
    m_pButtonLoopDouble->slotSet(0);
    EXPECT_EQ(500, m_pLoopStartPoint->get());
    EXPECT_EQ(1000, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopDoubleButton_UpdatesNumberedBeatloopActivationControls) {
    m_pTrack1->setBpm(120.0);
    m_pBeatLoopSize->set(2.0);
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());
    EXPECT_FALSE(m_pBeatLoop4Enabled->toBool());

    m_pButtonLoopDouble->set(1.0);
    m_pButtonLoopDouble->set(0.0);
    EXPECT_FALSE(m_pBeatLoop2Enabled->toBool());
    EXPECT_TRUE(m_pBeatLoop4Enabled->toBool());
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

TEST_F(LoopingControlTest, LoopHalveButton_HalvesBeatloopSize) {
    m_pTrack1->setBpm(120.0);
    m_pBeatLoopSize->set(64.0);
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    m_pButtonLoopHalve->slotSet(1);
    m_pButtonLoopHalve->slotSet(0);
    EXPECT_EQ(32.0, m_pBeatLoopSize->get());
}

TEST_F(LoopingControlTest, LoopHalveButton_DoesNotResizeManualLoop) {
    seekToSampleAndProcess(500);
    m_pButtonLoopIn->set(1.0);
    m_pButtonLoopIn->set(0.0);
    seekToSampleAndProcess(1000);
    m_pButtonLoopOut->set(1.0);
    m_pButtonLoopOut->set(0.0);
    EXPECT_EQ(500, m_pLoopStartPoint->get());
    EXPECT_EQ(1000, m_pLoopEndPoint->get());
    m_pButtonLoopHalve->slotSet(1);
    m_pButtonLoopHalve->slotSet(0);
    EXPECT_EQ(500, m_pLoopStartPoint->get());
    EXPECT_EQ(1000, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopHalveButton_UpdatesNumberedBeatloopActivationControls) {
    m_pTrack1->setBpm(120.0);
    m_pBeatLoopSize->set(4.0);
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    EXPECT_FALSE(m_pBeatLoop2Enabled->toBool());
    EXPECT_TRUE(m_pBeatLoop4Enabled->toBool());

    m_pButtonLoopHalve->set(1.0);
    m_pButtonLoopHalve->set(0.0);
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());
    EXPECT_FALSE(m_pBeatLoop4Enabled->toBool());
}

TEST_F(LoopingControlTest, LoopMoveTest) {
    m_pTrack1->setBpm(120);
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(300);
    seekToSampleAndProcess(10);
    m_pButtonReloopToggle->slotSet(1);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(300, m_pLoopEndPoint->get());
    EXPECT_EQ(10, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current);

    // Move the loop out from under the playposition.
    m_pButtonBeatMoveForward->set(1.0);
    m_pButtonBeatMoveForward->set(0.0);
    ProcessBuffer();
    EXPECT_EQ(44100, m_pLoopStartPoint->get());
    EXPECT_EQ(44400, m_pLoopEndPoint->get());
    ProcessBuffer();
    // Should seek to the corresponding offset within the moved loop
    EXPECT_EQ(44110, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current);

    // Move backward so that the current position is outside the new location of the loop
    m_pChannel1->getEngineBuffer()->queueNewPlaypos(44300, EngineBuffer::SEEK_STANDARD);
    ProcessBuffer();
    m_pButtonBeatMoveBackward->set(1.0);
    m_pButtonBeatMoveBackward->set(0.0);
    ProcessBuffer();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_NEAR(300, m_pLoopEndPoint->get(), kLoopPositionMaxAbsError);
    ProcessBuffer();
    EXPECT_NEAR(200,
            m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current,
            kLoopPositionMaxAbsError);

     // Now repeat the test with looping disabled (should not affect the
    // playhead).
    m_pButtonReloopToggle->slotSet(1);
    EXPECT_FALSE(isLoopEnabled());

    // Move the loop out from under the playposition.
    m_pButtonBeatMoveForward->set(1.0);
    m_pButtonBeatMoveForward->set(0.0);
    ProcessBuffer();
    EXPECT_EQ(44100, m_pLoopStartPoint->get());
    EXPECT_EQ(44400, m_pLoopEndPoint->get());
    // Should not seek inside the moved loop when the loop is disabled
    EXPECT_NEAR(200,
            m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current,
            kLoopPositionMaxAbsError);

    // Move backward so that the current position is outside the new location of the loop
    m_pChannel1->getEngineBuffer()->queueNewPlaypos(500, EngineBuffer::SEEK_STANDARD);
    ProcessBuffer();
    m_pButtonBeatMoveBackward->set(1.0);
    m_pButtonBeatMoveBackward->set(0.0);
    ProcessBuffer();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_NEAR(300, m_pLoopEndPoint->get(), kLoopPositionMaxAbsError);
    EXPECT_NEAR(500,
            m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current,
            kLoopPositionMaxAbsError);
}

TEST_F(LoopingControlTest, LoopResizeSeek) {
    // Activating a new loop with a loop active should warp the playposition
    // the same as it does when we scale the loop larger and smaller so we
    // keep in sync with the beat.

    // Disable quantize for this test
    m_pQuantizeEnabled->set(0.0);

    m_pTrack1->setBpm(23520);
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(600);
    seekToSampleAndProcess(500);
    m_pButtonReloopToggle->slotSet(1);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(600, m_pLoopEndPoint->get());
    EXPECT_EQ(500, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current);

    // Activate a shorter loop
    m_pButtonBeatLoop2Activate->set(1.0);

    ProcessBuffer();

    // The loop is resized and we should have seeked to a mid-beat part of the
    // loop.
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(450, m_pLoopEndPoint->get());
    ProcessBuffer();
    EXPECT_EQ(50, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current);

    // But if looping is not enabled, no warping occurs.
    m_pLoopStartPoint->slotSet(0);
    m_pLoopEndPoint->slotSet(600);
    seekToSampleAndProcess(500);
    m_pButtonReloopToggle->slotSet(1);
    EXPECT_FALSE(isLoopEnabled());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(600, m_pLoopEndPoint->get());
    EXPECT_EQ(500, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current);

    m_pButtonBeatLoop2Activate->set(1.0);
    ProcessBuffer();

    EXPECT_EQ(500, m_pLoopStartPoint->get());
    EXPECT_EQ(950, m_pLoopEndPoint->get());
    EXPECT_EQ(500, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current);
}

TEST_F(LoopingControlTest, BeatLoopSize_SetAndToggle) {
    m_pTrack1->setBpm(120.0);
    // Setting beatloop_size should not activate a loop
    m_pBeatLoopSize->set(2.0);
    EXPECT_FALSE(m_pLoopEnabled->toBool());

    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());

    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());
}

TEST_F(LoopingControlTest, BeatLoopSize_SetWithoutTrackLoaded) {
    // Eject the track that is automatically loaded by the testing framework
    m_pChannel1->getEngineBuffer()->slotEjectTrack(1.0);
    m_pBeatLoopSize->set(5.0);
    EXPECT_EQ(5.0, m_pBeatLoopSize->get());
}

TEST_F(LoopingControlTest, BeatLoopSize_IgnoresPastTrackEnd) {
    // TODO: actually calculate that the beatloop would go beyond
    // the end of the track
    m_pTrack1->setBpm(60.0);
    seekToSampleAndProcess(m_pTrackSamples->get() - 400);
    m_pBeatLoopSize->set(64.0);
    EXPECT_NE(64.0, m_pBeatLoopSize->get());
    EXPECT_FALSE(m_pBeatLoop64Enabled->toBool());
}

TEST_F(LoopingControlTest, BeatLoopSize_SetsNumberedControls) {
    m_pTrack1->setBpm(120.0);
    m_pBeatLoopSize->set(2.0);
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());
    EXPECT_FALSE(m_pBeatLoop4Enabled->toBool());

    m_pBeatLoopSize->set(4.0);
    EXPECT_FALSE(m_pBeatLoop2Enabled->toBool());
    EXPECT_TRUE(m_pBeatLoop4Enabled->toBool());
    EXPECT_TRUE(m_pLoopEnabled->toBool());
}

TEST_F(LoopingControlTest, BeatLoopSize_IsSetByNumberedControl) {
    m_pTrack1->setBpm(120.0);
    m_pBeatLoopSize->set(4.0);
    m_pButtonBeatLoop2Activate->set(1.0);
    m_pButtonBeatLoop2Activate->set(0.0);
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_EQ(2.0, m_pBeatLoopSize->get());

    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_EQ(2.0, m_pBeatLoopSize->get());
}

TEST_F(LoopingControlTest, BeatLoopSize_SetDoesNotStartLoop) {
    m_pTrack1->setBpm(120.0);
    m_pBeatLoopSize->set(16.0);
    EXPECT_FALSE(m_pLoopEnabled->toBool());
}

TEST_F(LoopingControlTest, BeatLoopSize_ResizeKeepsStartPosition) {
    seekToSampleAndProcess(50);
    m_pTrack1->setBpm(160.0);
    m_pBeatLoopSize->set(2.0);
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    double oldStart = m_pLoopStartPoint->get();

    ProcessBuffer();

    m_pBeatLoopSize->set(1.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    double newStart = m_pLoopStartPoint->get();
    EXPECT_TRUE(oldStart == newStart);
}

TEST_F(LoopingControlTest, BeatLoopSize_ValueChangeDoesNotActivateLoop) {
    seekToSampleAndProcess(50);
    m_pTrack1->setBpm(160.0);
    m_pBeatLoopSize->set(2.0);
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());

    m_pButtonReloopToggle->set(1.0);
    m_pButtonReloopToggle->set(0.0);
    EXPECT_FALSE(m_pLoopEnabled->toBool());
    m_pBeatLoopSize->set(4.0);
    EXPECT_FALSE(m_pLoopEnabled->toBool());
    EXPECT_FALSE(m_pBeatLoop4Enabled->toBool());
}

TEST_F(LoopingControlTest, BeatLoopSize_ValueChangeResizesBeatLoop) {
    seekToSampleAndProcess(50);
    m_pTrack1->setBpm(160.0);
    m_pBeatLoopSize->set(2.0);
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    double oldLoopStart = m_pLoopStartPoint->get();
    double oldLoopEnd = m_pLoopEndPoint->get();
    double oldLoopLength = oldLoopEnd - oldLoopStart;

    m_pButtonReloopToggle->set(1.0);
    m_pButtonReloopToggle->set(0.0);
    EXPECT_FALSE(m_pLoopEnabled->toBool());
    m_pBeatLoopSize->set(4.0);

    double newLoopStart = m_pLoopStartPoint->get();
    double newLoopEnd = m_pLoopEndPoint->get();
    double newLoopLength = newLoopEnd - newLoopStart;
    EXPECT_EQ(oldLoopStart, newLoopStart);
    EXPECT_NE(oldLoopEnd, newLoopEnd);
    EXPECT_EQ(oldLoopLength * 2, newLoopLength);
}

TEST_F(LoopingControlTest, BeatLoopSize_ValueChangeDoesNotResizeManualLoop) {
    seekToSampleAndProcess(50);
    m_pTrack1->setBpm(160.0);
    m_pQuantizeEnabled->set(0);
    m_pBeatLoopSize->set(4.0);
    m_pButtonLoopIn->slotSet(1);
    m_pButtonLoopIn->slotSet(0);
    seekToSampleAndProcess(500);
    m_pButtonLoopOut->slotSet(1);
    m_pButtonLoopOut->slotSet(0);
    double oldLoopStart = m_pLoopStartPoint->get();
    double oldLoopEnd = m_pLoopEndPoint->get();

    m_pBeatLoopSize->set(8.0);
    double newLoopStart = m_pLoopStartPoint->get();
    double newLoopEnd = m_pLoopEndPoint->get();
    EXPECT_EQ(oldLoopStart, newLoopStart);
    EXPECT_EQ(oldLoopEnd, newLoopEnd);
}

TEST_F(LoopingControlTest, LegacyBeatLoopControl) {
    m_pTrack1->setBpm(120.0);
    m_pBeatLoop->set(2.0);
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_EQ(2.0, m_pBeatLoopSize->get());

    m_pButtonReloopToggle->set(1.0);
    m_pButtonReloopToggle->set(0.0);
    EXPECT_FALSE(m_pBeatLoop2Enabled->toBool());
    EXPECT_FALSE(m_pLoopEnabled->toBool());
    EXPECT_EQ(2.0, m_pBeatLoopSize->get());

    ProcessBuffer();

    m_pBeatLoop->set(6.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_EQ(6.0, m_pBeatLoopSize->get());
}

TEST_F(LoopingControlTest, Beatjump_JumpsByBeats) {
    m_pTrack1->setBpm(120.0);
    ProcessBuffer();

    double beatLength = m_pNextBeat->get();
    EXPECT_NE(0, beatLength);

    m_pBeatJumpSize->set(4.0);
    m_pButtonBeatJumpForward->set(1.0);
    m_pButtonBeatJumpForward->set(0.0);
    ProcessBuffer();
    EXPECT_EQ(beatLength * 4, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current);
    m_pButtonBeatJumpBackward->set(1.0);
    m_pButtonBeatJumpBackward->set(0.0);
    ProcessBuffer();
    EXPECT_EQ(0, m_pChannel1->getEngineBuffer()->m_pLoopingControl->getSampleOfTrack().current);
}

TEST_F(LoopingControlTest, Beatjump_MovesActiveLoop) {
    m_pTrack1->setBpm(120.0);
    ProcessBuffer();

    m_pBeatLoopSize->set(4.0);
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    double beatLength = m_pNextBeat->get();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(beatLength * 4, m_pLoopEndPoint->get());

    m_pBeatJumpSize->set(1.0);
    m_pButtonBeatJumpForward->set(1.0);
    m_pButtonBeatJumpForward->set(0.0);
    EXPECT_EQ(beatLength, m_pLoopStartPoint->get());
    EXPECT_EQ(beatLength * 5, m_pLoopEndPoint->get());

    // jump backward with playposition outside the loop should not move the loop
    m_pButtonBeatJumpBackward->set(1.0);
    m_pButtonBeatJumpBackward->set(0.0);
    EXPECT_EQ(beatLength, m_pLoopStartPoint->get());
    EXPECT_EQ(beatLength * 5, m_pLoopEndPoint->get());

    seekToSampleAndProcess(beatLength);
    m_pButtonBeatJumpBackward->set(1.0);
    m_pButtonBeatJumpBackward->set(0.0);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(beatLength * 4, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, Beatjump_MovesLoopBoundaries) {
    // Holding down the loop in/out buttons and using beatjump should
    // move only the loop in/out point, but not shift the entire loop forward/backward
    m_pTrack1->setBpm(120.0);
    ProcessBuffer();

    m_pBeatLoopSize->set(4.0);
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    double beatLength = m_pNextBeat->get();
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(beatLength * 4, m_pLoopEndPoint->get());

    m_pButtonLoopIn->set(1.0);
    m_pBeatJumpSize->set(1.0);
    m_pButtonBeatJumpForward->set(1.0);
    m_pButtonBeatJumpForward->set(0.0);
    ProcessBuffer();
    m_pButtonLoopIn->set(0.0);
    ProcessBuffer();
    EXPECT_EQ(beatLength, m_pLoopStartPoint->get());
    EXPECT_EQ(beatLength * 4, m_pLoopEndPoint->get());

    m_pButtonLoopOut->set(1.0);
    m_pButtonBeatJumpForward->set(1.0);
    m_pButtonBeatJumpForward->set(0.0);
    ProcessBuffer();
    m_pButtonLoopOut->set(0.0);
    ProcessBuffer();
    EXPECT_EQ(beatLength, m_pLoopStartPoint->get());
    EXPECT_EQ(beatLength * 2, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopEscape) {
    m_pLoopStartPoint->slotSet(100);
    m_pLoopEndPoint->slotSet(200);
    m_pButtonReloopToggle->set(1.0);
    m_pButtonReloopToggle->set(0.0);
    ProcessBuffer();
    EXPECT_TRUE(isLoopEnabled());
    // seek outside a loop should disable it
    seekToSampleAndProcess(300);
    EXPECT_FALSE(isLoopEnabled());

    m_pButtonReloopToggle->set(1.0);
    m_pButtonReloopToggle->set(0.0);
    ProcessBuffer();
    EXPECT_TRUE(isLoopEnabled());
    // seek outside a loop should disable it
    seekToSampleAndProcess(50);
    EXPECT_FALSE(isLoopEnabled());
}

TEST_F(LoopingControlTest, BeatLoopRoll_Activation) {
    m_pTrack1->setBpm(120.0);

    m_pButtonBeatLoopRoll2Activate->set(1.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());

    m_pButtonBeatLoopRoll2Activate->set(0.0);
    EXPECT_FALSE(m_pLoopEnabled->toBool());
    EXPECT_FALSE(m_pBeatLoop2Enabled->toBool());
}

TEST_F(LoopingControlTest, BeatLoopRoll_Overlap) {
    m_pTrack1->setBpm(120.0);

    m_pButtonBeatLoopRoll2Activate->set(1.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());

    m_pButtonBeatLoopRoll4Activate->set(1.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop4Enabled->toBool());
    EXPECT_FALSE(m_pBeatLoop2Enabled->toBool());

    m_pButtonBeatLoopRoll2Activate->set(0.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop4Enabled->toBool());
    EXPECT_FALSE(m_pBeatLoop2Enabled->toBool());

    m_pButtonBeatLoopRoll4Activate->set(0.0);
    EXPECT_FALSE(m_pLoopEnabled->toBool());
    EXPECT_FALSE(m_pBeatLoop4Enabled->toBool());
}

TEST_F(LoopingControlTest, BeatLoopRoll_OverlapStackUnwind) {
    m_pTrack1->setBpm(120.0);

    // start a 2 beat loop roll
    m_pButtonBeatLoopRoll2Activate->set(1.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());

    // start a 4 beat loop roll on top of the previous loop
    m_pButtonBeatLoopRoll4Activate->set(1.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop4Enabled->toBool());
    EXPECT_FALSE(m_pBeatLoop2Enabled->toBool());

    // start a 1 beat loop roll on top of the previous loop
    m_pButtonBeatLoopRoll1Activate->set(1.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop1Enabled->toBool());
    EXPECT_FALSE(m_pBeatLoop4Enabled->toBool());

    // stop the 4 beat loop roll, the 1 beat roll should continue
    m_pButtonBeatLoopRoll4Activate->set(0.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop1Enabled->toBool());

    // stop the 1 beat loop roll, the 2 beat roll should continue
    m_pButtonBeatLoopRoll1Activate->set(0.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());
    EXPECT_FALSE(m_pBeatLoop1Enabled->toBool());

    // stop the 2 beat loop roll
    m_pButtonBeatLoopRoll2Activate->set(0.0);
    EXPECT_FALSE(m_pLoopEnabled->toBool());
    EXPECT_FALSE(m_pBeatLoop2Enabled->toBool());
}

TEST_F(LoopingControlTest, BeatLoopRoll_StartPoint) {
    m_pTrack1->setBpm(120.0);

    // start a 4 beat loop roll, start point should be overridden to play position
    m_pLoopStartPoint->slotSet(8);
    m_pButtonBeatLoopRoll4Activate->set(1.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop4Enabled->toBool());
    EXPECT_EQ(0, m_pLoopStartPoint->get());

    // move the start point, activate a 1 beat loop roll, new start point be preserved
    m_pLoopStartPoint->slotSet(8);
    m_pButtonBeatLoopRoll1Activate->set(1.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop1Enabled->toBool());
    EXPECT_EQ(8, m_pLoopStartPoint->get());

    // end the 1 beat loop roll
    m_pButtonBeatLoopRoll1Activate->set(0.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_EQ(8, m_pLoopStartPoint->get());
    EXPECT_TRUE(m_pBeatLoop4Enabled->toBool());

    // end the 4 beat loop roll
    m_pButtonBeatLoopRoll4Activate->set(0.0);
    EXPECT_FALSE(m_pLoopEnabled->toBool());
    EXPECT_FALSE(m_pBeatLoop4Enabled->toBool());

    // new loop should start back at 0
    m_pButtonBeatLoopRoll4Activate->set(1.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop4Enabled->toBool());
    EXPECT_EQ(0, m_pLoopStartPoint->get());
}
