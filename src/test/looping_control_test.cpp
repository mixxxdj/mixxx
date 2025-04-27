#include <gtest/gtest.h>
#include <qstringliteral.h>

#include <QScopedPointer>
#include <QtDebug>
#include <memory>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "control/pollingcontrolproxy.h"
#include "engine/controls/loopingcontrol.h"
#include "mixxxtest.h"
#include "test/mockedenginebackendtest.h"

namespace {

// Due to rounding errors loop positions should be compared with EXPECT_NEAR instead of EXPECT_EQ.
// NOTE(uklotzde, 2017-12-10): The rounding errors currently only appeared with GCC 7.2.1.
constexpr double kLoopPositionMaxAbsError = 0.000000001;

constexpr auto kTrackEndPosition = mixxx::audio::FramePos{150000000};
} // namespace

class LoopingControlTest : public MockedEngineBackendTest {
  protected:
    void SetUp() override {
        MockedEngineBackendTest::SetUp();
        m_pQuantizeEnabled = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("quantize")});
        m_pQuantizeEnabled->set(1.0);
        m_pSlipEnabled = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("slip_enabled")});
        m_pNextBeat = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("beat_next")});

        m_pNextBeat->set(-1);
        m_pClosestBeat = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("beat_closest")});
        m_pClosestBeat->set(-1);
        m_pTrackSamples = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("track_samples")});
        m_pTrackSamples->set(kTrackEndPosition.toEngineSamplePos());
        m_pButtonLoopIn = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("loop_in")});
        m_pButtonLoopOut = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("loop_out")});
        m_pButtonLoopExit = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("loop_exit")});
        m_pButtonReloopToggle = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("reloop_toggle")});
        m_pButtonReloopAndStop = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("reloop_andstop")});
        m_pButtonLoopDouble = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("loop_double")});
        m_pButtonLoopHalve = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("loop_halve")});
        m_pLoopEnabled = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("loop_enabled")});
        m_pLoopStartPoint = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("loop_start_position")});
        m_pLoopEndPoint = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("loop_end_position")});
        m_pLoopScale = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("loop_scale")});
        m_pButtonPlay = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("play")});
        m_pPlayPosition = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, QStringLiteral("playposition")});
        m_pButtonBeatMoveForward = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "loop_move_1_forward"});
        m_pButtonBeatMoveBackward = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "loop_move_1_backward"});
        m_pButtonBeatLoop2Activate = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatloop_2_activate"});
        m_pButtonBeatLoop4Activate = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatloop_4_activate"});
        m_pBeatLoop1Enabled = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatloop_1_enabled"});
        m_pBeatLoop2Enabled = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatloop_2_enabled"});
        m_pBeatLoop4Enabled = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatloop_4_enabled"});
        m_pBeatLoop64Enabled = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatloop_64_enabled"});
        m_pBeatLoop = std::make_unique<PollingControlProxy>(ConfigKey{m_sGroup1, "beatloop"});
        m_pBeatLoopSize = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatloop_size"});
        m_pButtonBeatLoopActivate = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatloop_activate"});
        m_pBeatJumpSize = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatjump_size"});
        m_pButtonBeatJumpSizeDouble = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatjump_size_double"});
        m_pButtonBeatJumpSizeHalve = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatjump_size_halve"});
        m_pButtonBeatJumpForward = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatjump_forward"});
        m_pButtonBeatJumpBackward = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatjump_backward"});
        m_pButtonBeatLoopRoll1Activate = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatlooproll_1_activate"});
        m_pButtonBeatLoopRoll2Activate = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatlooproll_2_activate"});
        m_pButtonBeatLoopRoll4Activate = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "beatlooproll_4_activate"});
        m_pButtonBeatLoopAnchor = std::make_unique<PollingControlProxy>(
                ConfigKey{m_sGroup1, "loop_anchor"});

        ProcessBuffer();
    }

    mixxx::audio::FramePos currentFramePos() {
        return m_pChannel1->getEngineBuffer()->m_pLoopingControl->frameInfo().currentPosition;
    }

    bool isLoopEnabled() {
        return m_pLoopEnabled->get() > 0.0;
    }

    void setCurrentPosition(mixxx::audio::FramePos position) {
        m_pChannel1->getEngineBuffer()->queueNewPlaypos(position, EngineBuffer::SEEK_STANDARD);
        ProcessBuffer();
    }

    std::unique_ptr<PollingControlProxy> m_pNextBeat;
    std::unique_ptr<PollingControlProxy> m_pClosestBeat;
    std::unique_ptr<PollingControlProxy> m_pQuantizeEnabled;
    std::unique_ptr<PollingControlProxy> m_pSlipEnabled;
    std::unique_ptr<PollingControlProxy> m_pTrackSamples;
    std::unique_ptr<PollingControlProxy> m_pButtonLoopIn;
    std::unique_ptr<PollingControlProxy> m_pButtonLoopOut;
    std::unique_ptr<PollingControlProxy> m_pButtonLoopExit;
    std::unique_ptr<PollingControlProxy> m_pButtonReloopToggle;
    std::unique_ptr<PollingControlProxy> m_pButtonReloopAndStop;
    std::unique_ptr<PollingControlProxy> m_pButtonLoopDouble;
    std::unique_ptr<PollingControlProxy> m_pButtonLoopHalve;
    std::unique_ptr<PollingControlProxy> m_pLoopEnabled;
    std::unique_ptr<PollingControlProxy> m_pLoopStartPoint;
    std::unique_ptr<PollingControlProxy> m_pLoopEndPoint;
    std::unique_ptr<PollingControlProxy> m_pLoopScale;
    std::unique_ptr<PollingControlProxy> m_pPlayPosition;
    std::unique_ptr<PollingControlProxy> m_pButtonPlay;
    std::unique_ptr<PollingControlProxy> m_pButtonBeatMoveForward;
    std::unique_ptr<PollingControlProxy> m_pButtonBeatMoveBackward;
    std::unique_ptr<PollingControlProxy> m_pButtonBeatLoopAnchor;
    std::unique_ptr<PollingControlProxy> m_pButtonBeatLoop2Activate;
    std::unique_ptr<PollingControlProxy> m_pButtonBeatLoop4Activate;
    std::unique_ptr<PollingControlProxy> m_pBeatLoop1Enabled;
    std::unique_ptr<PollingControlProxy> m_pBeatLoop2Enabled;
    std::unique_ptr<PollingControlProxy> m_pBeatLoop4Enabled;
    std::unique_ptr<PollingControlProxy> m_pBeatLoop64Enabled;
    std::unique_ptr<PollingControlProxy> m_pBeatLoop;
    std::unique_ptr<PollingControlProxy> m_pBeatLoopSize;
    std::unique_ptr<PollingControlProxy> m_pButtonBeatLoopActivate;
    std::unique_ptr<PollingControlProxy> m_pBeatJumpSize;
    std::unique_ptr<PollingControlProxy> m_pButtonBeatJumpSizeHalve;
    std::unique_ptr<PollingControlProxy> m_pButtonBeatJumpSizeDouble;
    std::unique_ptr<PollingControlProxy> m_pButtonBeatJumpForward;
    std::unique_ptr<PollingControlProxy> m_pButtonBeatJumpBackward;
    std::unique_ptr<PollingControlProxy> m_pButtonBeatLoopRoll1Activate;
    std::unique_ptr<PollingControlProxy> m_pButtonBeatLoopRoll2Activate;
    std::unique_ptr<PollingControlProxy> m_pButtonBeatLoopRoll4Activate;
};

TEST_F(LoopingControlTest, LoopSet) {
    m_pLoopStartPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{100}.toEngineSamplePos());
    setCurrentPosition(mixxx::audio::FramePos{50});
    EXPECT_FALSE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{100}, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, LoopSetFractionalFrames) {
    m_pLoopStartPoint->set(mixxx::audio::FramePos{0.5}.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{50.75}.toEngineSamplePos());
    setCurrentPosition(mixxx::audio::FramePos{25});
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{0.5}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{50.75}, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, LoopInSetInsideLoopContinues) {
    m_pLoopStartPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{100}.toEngineSamplePos());
    m_pButtonReloopToggle->set(1);
    m_pButtonReloopToggle->set(0);
    setCurrentPosition(mixxx::audio::FramePos{50});
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{100}, m_pLoopEndPoint);
    m_pLoopStartPoint->set(mixxx::audio::FramePos{10}.toEngineSamplePos());
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{10}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{100}, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, LoopInSetAfterLoopOutStops) {
    m_pLoopStartPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{100}.toEngineSamplePos());
    m_pButtonReloopToggle->set(1);
    m_pButtonReloopToggle->set(0);
    setCurrentPosition(mixxx::audio::FramePos{50});
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{100}, m_pLoopEndPoint);
    m_pLoopStartPoint->set(mixxx::audio::FramePos{110}.toEngineSamplePos());
    EXPECT_FALSE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{110}, m_pLoopStartPoint);
    EXPECT_EQ(-1, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopInSetAtLoopOutClearsLoopOut) {
    m_pLoopStartPoint->set(0);
    m_pLoopEndPoint->set(100);
    m_pLoopStartPoint->set(100);
    EXPECT_EQ(100, m_pLoopStartPoint->get());
    EXPECT_EQ(-1, m_pLoopEndPoint->get());
    EXPECT_FALSE(isLoopEnabled());
}

TEST_F(LoopingControlTest, LoopOutSetAtLoopInIgnored) {
    m_pLoopStartPoint->set(0);
    m_pLoopEndPoint->set(100);
    m_pLoopEndPoint->set(0);
    EXPECT_EQ(0, m_pLoopStartPoint->get());
    EXPECT_EQ(100, m_pLoopEndPoint->get());
    m_pLoopEndPoint->set(-1);
    EXPECT_EQ(-1, m_pLoopEndPoint->get());
    m_pLoopEndPoint->set(0);
    EXPECT_FALSE(isLoopEnabled());
    EXPECT_EQ(-1, m_pLoopEndPoint->get());
}

TEST_F(LoopingControlTest, LoopOutSetInsideLoopContinues) {
    m_pLoopStartPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{100}.toEngineSamplePos());
    m_pButtonReloopToggle->set(1);
    m_pButtonReloopToggle->set(0);
    setCurrentPosition(mixxx::audio::FramePos{50});
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{100}, m_pLoopEndPoint);
    m_pLoopEndPoint->set(mixxx::audio::FramePos{80}.toEngineSamplePos());
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{80}, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, LoopOutSetBeforeLoopInIgnored) {
    m_pLoopStartPoint->set(mixxx::audio::FramePos{10}.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{100}.toEngineSamplePos());
    m_pButtonReloopToggle->set(1);
    m_pButtonReloopToggle->set(0);
    setCurrentPosition(mixxx::audio::FramePos{50});
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{10}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{100}, m_pLoopEndPoint);
    m_pLoopEndPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{10}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{100}, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, LoopInButton_QuantizeDisabled) {
    m_pQuantizeEnabled->set(0);
    m_pClosestBeat->set(100);
    m_pNextBeat->set(100);
    setCurrentPosition(mixxx::audio::FramePos{50});
    m_pButtonLoopIn->set(1);
    m_pButtonLoopIn->set(0);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{50}, m_pLoopStartPoint);
}

TEST_F(LoopingControlTest, LoopInButton_AdjustLoopInPointOutsideLoop) {
    m_pQuantizeEnabled->set(0);
    m_pLoopStartPoint->set(mixxx::audio::FramePos{1000}.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{2000}.toEngineSamplePos());
    m_pButtonReloopToggle->set(1);
    m_pButtonReloopToggle->set(0);
    m_pButtonLoopIn->set(1);
    setCurrentPosition(mixxx::audio::FramePos{50});
    m_pButtonLoopIn->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{50}, m_pLoopStartPoint);
}

TEST_F(LoopingControlTest, LoopInButton_AdjustLoopInPointInsideLoop) {
    m_pQuantizeEnabled->set(0);
    m_pLoopStartPoint->set(mixxx::audio::FramePos{1000}.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{2000}.toEngineSamplePos());
    m_pButtonReloopToggle->set(1);
    m_pButtonReloopToggle->set(0);
    m_pButtonLoopIn->set(1);
    setCurrentPosition(mixxx::audio::FramePos{1500});
    m_pButtonLoopIn->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{1500}, m_pLoopStartPoint);
}

TEST_F(LoopingControlTest, LoopOutButton_QuantizeDisabled) {
    m_pQuantizeEnabled->set(0);
    m_pClosestBeat->set(1000);
    m_pNextBeat->set(1000);
    setCurrentPosition(mixxx::audio::FramePos{500});
    m_pLoopStartPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    m_pButtonLoopOut->set(1);
    m_pButtonLoopOut->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{500}, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, LoopOutButton_AdjustLoopOutPointOutsideLoop) {
    m_pQuantizeEnabled->set(0);
    m_pLoopStartPoint->set(mixxx::audio::FramePos{1000}.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{2000}.toEngineSamplePos());
    m_pButtonReloopToggle->set(1);
    m_pButtonReloopToggle->set(0);
    m_pButtonLoopOut->set(1);
    setCurrentPosition(mixxx::audio::FramePos{3000});
    m_pButtonLoopOut->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{3000}, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, LoopOutButton_AdjustLoopOutPointInsideLoop) {
    m_pQuantizeEnabled->set(0);
    m_pLoopStartPoint->set(mixxx::audio::FramePos{100}.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{2000}.toEngineSamplePos());
    m_pButtonReloopToggle->set(1);
    m_pButtonReloopToggle->set(0);
    m_pButtonLoopOut->set(1);
    setCurrentPosition(mixxx::audio::FramePos{1500});
    m_pButtonLoopOut->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{1500}, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, LoopInOutButtons_QuantizeEnabled) {
    const auto bpm = mixxx::Bpm{60};
    m_pTrack1->trySetBpm(bpm);
    m_pQuantizeEnabled->set(1);
    // Move short after the first beat
    setCurrentPosition(mixxx::audio::FramePos{250});
    m_pButtonLoopIn->set(1);
    m_pButtonLoopIn->set(0);
    EXPECT_EQ(m_pClosestBeat->get(), m_pLoopStartPoint->get());

    // Move short after the 5th beat
    m_pBeatJumpSize->set(4);
    m_pButtonBeatJumpForward->set(1);
    m_pButtonBeatJumpForward->set(0);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ(currentFramePos(), mixxx::audio::FramePos{(44100 * 4) + 250});
    // This should make loop_out snap to 5th beat and queue
    // a seek to first beat + initial offset.
    m_pButtonLoopOut->set(1);
    m_pButtonLoopOut->set(0);
    ProcessBuffer(); // first process to schedule seek in a stopped deck
    ProcessBuffer(); // then seek
    EXPECT_EQ(m_pLoopEndPoint->get(), 44100 * 2 * 4);
    EXPECT_FRAMEPOS_EQ(currentFramePos(), mixxx::audio::FramePos{250});
    // Should adopt the loop size and enable the correct loop control
    EXPECT_EQ(4, m_pBeatLoopSize->get());
    EXPECT_TRUE(m_pBeatLoop4Enabled->toBool());

    // Repeat the procedure and verify a seek is triggered even though
    // the loop is identical.
    m_pLoopEnabled->set(0);
    m_pButtonBeatJumpForward->set(1);
    m_pButtonBeatJumpForward->set(0);
    ProcessBuffer();
    m_pButtonLoopOut->set(1);
    m_pButtonLoopOut->set(0);
    ProcessBuffer(); // first process to schedule seek in a stopped deck
    ProcessBuffer(); // then seek
    EXPECT_FRAMEPOS_EQ(currentFramePos(), mixxx::audio::FramePos{250});
    EXPECT_EQ(m_pLoopEndPoint->get(), 44100 * 2 * 4);
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
    setCurrentPosition(mixxx::audio::FramePos{500});
    m_pLoopStartPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    m_pButtonLoopOut->set(1);
    m_pButtonLoopOut->set(0);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{500}, m_pLoopEndPoint);
    m_pButtonReloopToggle->set(1);
    m_pButtonReloopToggle->set(0);
    EXPECT_FALSE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{500}, m_pLoopEndPoint);
    m_pButtonReloopToggle->set(1);
    m_pButtonReloopToggle->set(0);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{500}, m_pLoopEndPoint);
    // Ensure that the Loop Exit button works, and that it doesn't act as a
    // toggle.
    m_pButtonLoopExit->set(1);
    m_pButtonLoopExit->set(0);
    EXPECT_FALSE(isLoopEnabled());
    m_pButtonLoopExit->set(1);
    m_pButtonLoopExit->set(0);
    EXPECT_FALSE(isLoopEnabled());
}

TEST_F(LoopingControlTest, ReloopToggleButton_DoesNotJumpAhead) {
    m_pLoopStartPoint->set(mixxx::audio::FramePos{1000}.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{2000}.toEngineSamplePos());
    setCurrentPosition(mixxx::audio::kStartFramePos);

    m_pButtonReloopToggle->set(1);
    m_pButtonReloopToggle->set(0);
    setCurrentPosition(mixxx::audio::FramePos{50});
    EXPECT_LE(currentFramePos().toEngineSamplePos(), m_pLoopStartPoint->get());
}

TEST_F(LoopingControlTest, ReloopAndStopButton) {
    m_pLoopStartPoint->set(mixxx::audio::FramePos{1000}.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{2000}.toEngineSamplePos());
    setCurrentPosition(mixxx::audio::FramePos{1500});
    m_pButtonReloopToggle->set(1);
    m_pButtonReloopToggle->set(0);
    m_pButtonReloopAndStop->set(1);
    m_pButtonReloopAndStop->set(0);
    ProcessBuffer();
    EXPECT_EQ(currentFramePos().toEngineSamplePos(), m_pLoopStartPoint->get());
    EXPECT_TRUE(m_pLoopEnabled->toBool());
}

TEST_F(LoopingControlTest, LoopScale_DoublesLoop) {
    m_pQuantizeEnabled->set(0);
    setCurrentPosition(mixxx::audio::kStartFramePos);
    m_pButtonLoopIn->set(1);
    m_pButtonLoopIn->set(0);
    setCurrentPosition(mixxx::audio::FramePos{500});
    m_pButtonLoopOut->set(1);
    m_pButtonLoopOut->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{500}, m_pLoopEndPoint);
    m_pLoopScale->set(2.0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{1000}, m_pLoopEndPoint);
    m_pLoopScale->set(2.0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{2000}, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, LoopScale_HalvesLoop) {
    m_pLoopStartPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{2000}.toEngineSamplePos());
    setCurrentPosition(mixxx::audio::FramePos{1800});
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{2000}, m_pLoopEndPoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{1800}, currentFramePos());
    EXPECT_FALSE(isLoopEnabled());
    m_pLoopScale->set(0.5);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{1000}, m_pLoopEndPoint);

    // The loop was not enabled so halving the loop should not move the playhead
    // even though it is outside the loop.
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{1800}, currentFramePos());

    m_pButtonReloopToggle->set(1);
    EXPECT_TRUE(isLoopEnabled());
    m_pLoopScale->set(0.5);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{500}, m_pLoopEndPoint);
    // Since the current sample was out of range of the new loop,
    // the current sample should reseek based on the new loop size.
    mixxx::audio::FramePos targetPosition;
    const mixxx::audio::FramePos triggerPosition =
            m_pChannel1->getEngineBuffer()->m_pLoopingControl->nextTrigger(
                    false, mixxx::audio::FramePos{1800}, &targetPosition);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{300}, targetPosition);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{1800}, triggerPosition);
}

TEST_F(LoopingControlTest, LoopDoubleButton_IgnoresPastTrackEnd) {
    setCurrentPosition(mixxx::audio::FramePos{50});
    m_pLoopStartPoint->set(kTrackEndPosition.toEngineSamplePos() / 2.0);
    m_pLoopEndPoint->set(kTrackEndPosition.toEngineSamplePos());
    EXPECT_FRAMEPOS_EQ_CONTROL(kTrackEndPosition / 2.0, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(kTrackEndPosition, m_pLoopEndPoint);
    m_pButtonLoopDouble->set(1);
    m_pButtonLoopDouble->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(kTrackEndPosition / 2.0, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(kTrackEndPosition, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, LoopDoubleButton_DoublesBeatloopSize) {
    m_pTrack1->trySetBpm(120.0);
    m_pBeatLoopSize->set(16.0);
    EXPECT_EQ(16.0, m_pBeatLoopSize->get());
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    EXPECT_EQ(16.0, m_pBeatLoopSize->get());
    m_pButtonLoopDouble->set(1.0);
    m_pButtonLoopDouble->set(0.0);
    EXPECT_EQ(32.0, m_pBeatLoopSize->get());
}

TEST_F(LoopingControlTest, LoopDoubleButton_DoesNotResizeManualLoop) {
    m_pQuantizeEnabled->set(0);
    setCurrentPosition(mixxx::audio::FramePos{500});
    m_pButtonLoopIn->set(1.0);
    m_pButtonLoopIn->set(0.0);
    setCurrentPosition(mixxx::audio::FramePos{1000});
    m_pButtonLoopOut->set(1.0);
    m_pButtonLoopOut->set(0.0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{500}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{1000}, m_pLoopEndPoint);
    m_pButtonLoopDouble->set(1);
    m_pButtonLoopDouble->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{500}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{1000}, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, LoopDoubleButton_UpdatesNumberedBeatloopActivationControls) {
    m_pTrack1->trySetBpm(120.0);
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
    m_pLoopStartPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{40}.toEngineSamplePos());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{40}, m_pLoopEndPoint);
    m_pButtonLoopHalve->set(1);
    m_pButtonLoopHalve->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{40}, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, LoopHalveButton_HalvesBeatloopSize) {
    m_pTrack1->trySetBpm(120.0);
    m_pBeatLoopSize->set(64.0);
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    m_pButtonLoopHalve->set(1);
    m_pButtonLoopHalve->set(0);
    EXPECT_EQ(32.0, m_pBeatLoopSize->get());
}

TEST_F(LoopingControlTest, LoopHalveButton_DoesNotResizeManualLoop) {
    m_pQuantizeEnabled->set(0);
    setCurrentPosition(mixxx::audio::FramePos{500});
    m_pButtonLoopIn->set(1.0);
    m_pButtonLoopIn->set(0.0);
    setCurrentPosition(mixxx::audio::FramePos{1000});
    m_pButtonLoopOut->set(1.0);
    m_pButtonLoopOut->set(0.0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{500}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{1000}, m_pLoopEndPoint);
    m_pButtonLoopHalve->set(1);
    m_pButtonLoopHalve->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{500}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{1000}, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, LoopHalveButton_UpdatesNumberedBeatloopActivationControls) {
    m_pTrack1->trySetBpm(120.0);
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
    const auto bpm = mixxx::Bpm{120};
    m_pTrack1->trySetBpm(bpm);

    m_pLoopStartPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{150}.toEngineSamplePos());
    setCurrentPosition(mixxx::audio::FramePos{5});
    m_pButtonReloopToggle->set(1);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{150}, m_pLoopEndPoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{5}, currentFramePos());

    // Move the loop out from under the playposition.
    m_pButtonBeatMoveForward->set(1.0);
    m_pButtonBeatMoveForward->set(0.0);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{22050}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{22200}, m_pLoopEndPoint);
    ProcessBuffer();
    // Should seek to the corresponding offset within the moved loop
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{22055}, currentFramePos());

    // Move backward so that the current position is outside the new location of the loop
    setCurrentPosition(mixxx::audio::FramePos{22150});
    m_pButtonBeatMoveBackward->set(1.0);
    m_pButtonBeatMoveBackward->set(0.0);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_NEAR(300, m_pLoopEndPoint->get(), kLoopPositionMaxAbsError);
    ProcessBuffer();
    EXPECT_NEAR(200,
            currentFramePos().toEngineSamplePos(),
            kLoopPositionMaxAbsError);

    // Now repeat the test with looping disabled (should not affect the
    // playhead).
    m_pButtonReloopToggle->set(1);
    EXPECT_FALSE(isLoopEnabled());

    // Move the loop out from under the playposition.
    m_pButtonBeatMoveForward->set(1.0);
    m_pButtonBeatMoveForward->set(0.0);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{22050}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{22200}, m_pLoopEndPoint);
    // Should not seek inside the moved loop when the loop is disabled
    EXPECT_NEAR(200,
            currentFramePos().toEngineSamplePos(),
            kLoopPositionMaxAbsError);

    // Move backward so that the current position is outside the new location of the loop
    setCurrentPosition(mixxx::audio::FramePos{250});
    m_pButtonBeatMoveBackward->set(1.0);
    m_pButtonBeatMoveBackward->set(0.0);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_NEAR(300, m_pLoopEndPoint->get(), kLoopPositionMaxAbsError);
    EXPECT_NEAR(500,
            currentFramePos().toEngineSamplePos(),
            kLoopPositionMaxAbsError);
}

TEST_F(LoopingControlTest, LoopResizeSeek) {
    // Activating a new loop with a loop active should warp the playposition
    // the same as it does when we scale the loop larger and smaller so we
    // keep in sync with the beat.

    // Disable quantize for this test
    m_pQuantizeEnabled->set(0.0);

    m_pTrack1->trySetBpm(23520);
    m_pLoopStartPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{300}.toEngineSamplePos());
    setCurrentPosition(mixxx::audio::FramePos{250});
    m_pButtonReloopToggle->set(1);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{300}, m_pLoopEndPoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{250}, currentFramePos());

    // Activate a shorter loop
    m_pButtonBeatLoop2Activate->set(1.0);

    ProcessBuffer();

    // The loop is resized and we should have seeked to a mid-beat part of the
    // loop.
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{225}, m_pLoopEndPoint);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{25}, currentFramePos());

    // But if looping is not enabled, no warping occurs.
    m_pLoopStartPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{300}.toEngineSamplePos());
    setCurrentPosition(mixxx::audio::FramePos{250});
    m_pButtonReloopToggle->set(1);
    EXPECT_FALSE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{300}, m_pLoopEndPoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{250}, currentFramePos());

    m_pButtonBeatLoop2Activate->set(1.0);
    ProcessBuffer();

    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{250}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{475}, m_pLoopEndPoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{250}, currentFramePos());
}

TEST_F(LoopingControlTest, EjectResetsLoopInOutPositions) {
    m_pLoopStartPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{300}.toEngineSamplePos());
    m_pChannel1->getEngineBuffer()->ejectTrack();
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, BeatLoopSize_SetAndToggle) {
    m_pTrack1->trySetBpm(120.0);
    // Setting beatloop_size should not activate a loop
    m_pBeatLoopSize->set(2.0);
    EXPECT_FALSE(m_pLoopEnabled->toBool());

    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());

    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    EXPECT_FALSE(m_pLoopEnabled->toBool());
    EXPECT_FALSE(m_pBeatLoop2Enabled->toBool());
}

TEST_F(LoopingControlTest, BeatLoopSize_SetWithoutTrackLoaded) {
    // Eject the track that is automatically loaded by the testing framework
    m_pChannel1->getEngineBuffer()->ejectTrack();
    m_pBeatLoopSize->set(5.0);
    EXPECT_EQ(5.0, m_pBeatLoopSize->get());
}

TEST_F(LoopingControlTest, BeatLoopSize_IgnoresPastTrackEnd) {
    // TODO: actually calculate that the beatloop would go beyond
    // the end of the track
    m_pTrack1->trySetBpm(60.0);
    setCurrentPosition(mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                               m_pTrackSamples->get()) -
            44100);
    m_pBeatLoopSize->set(0.5);
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    m_pBeatLoopSize->set(64.0);
    EXPECT_NE(64.0, m_pBeatLoopSize->get());
    EXPECT_FALSE(m_pBeatLoop64Enabled->toBool());
}

TEST_F(LoopingControlTest, BeatLoopSize_SetPastTrackEndIfLoopInactive) {
    // TODO: actually calculate that the beatloop would go beyond
    // the end of the track
    m_pTrack1->trySetBpm(60.0);
    setCurrentPosition(mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                               m_pTrackSamples->get()) -
            44100);
    m_pBeatLoopSize->set(64.0);
    EXPECT_EQ(64.0, m_pBeatLoopSize->get());
    EXPECT_FALSE(m_pBeatLoop64Enabled->toBool());
}

TEST_F(LoopingControlTest, BeatLoopSize_SetsNumberedControls) {
    m_pTrack1->trySetBpm(120.0);
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
    m_pTrack1->trySetBpm(120.0);
    m_pBeatLoopSize->set(4.0);
    m_pButtonBeatLoop2Activate->set(1.0);
    m_pButtonBeatLoop2Activate->set(0.0);
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_EQ(2.0, m_pBeatLoopSize->get());

    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    EXPECT_FALSE(m_pBeatLoop2Enabled->toBool());
    EXPECT_FALSE(m_pLoopEnabled->toBool());
    EXPECT_EQ(2.0, m_pBeatLoopSize->get());
}

TEST_F(LoopingControlTest, BeatLoopSize_SetRangeCheck) {
    // Set BeatLoopSize to the maximum allowed value of 512
    m_pBeatLoopSize->set(512.0);
    EXPECT_EQ(512, m_pBeatLoopSize->get());

    m_pBeatLoopSize->set(150.0);
    EXPECT_EQ(150, m_pBeatLoopSize->get());

    // Set BeatLoopSize to a value above the allowed maximum of 512 -> This must be ignored
    m_pBeatLoopSize->set(513.0);
    EXPECT_EQ(150, m_pBeatLoopSize->get());

    // Double BeatLoopSize (the result is 300 which is in the allowed range)
    m_pButtonLoopDouble->set(1.0);
    m_pButtonLoopDouble->set(0.0);
    EXPECT_EQ(300.0, m_pBeatLoopSize->get());

    // Double BeatLoopSize (the result would be 600 which is above the allowed
    // maximum of 512 -> This must be ignored)
    m_pButtonLoopDouble->set(1.0);
    m_pButtonLoopDouble->set(0.0);
    EXPECT_EQ(300.0, m_pBeatLoopSize->get());

    // Set BeatLoopSize to the minimum allowed value
    m_pBeatLoopSize->set(1 / 32.0);
    EXPECT_EQ(1 / 32.0, m_pBeatLoopSize->get());

    m_pBeatLoopSize->set(1 / 10.0);
    EXPECT_EQ(1 / 10.0, m_pBeatLoopSize->get());

    // Set BeatLoopSize to a value below the allowed minimum of 1/32 -> This must be ignored
    m_pBeatLoopSize->set(1 / 33.0);
    EXPECT_EQ(1 / 10.0, m_pBeatLoopSize->get());

    m_pBeatLoopSize->set(0);
    EXPECT_EQ(1 / 10.0, m_pBeatLoopSize->get());

    // Halve BeatLoopSize (the result is 1/20 which is in the allowed range)
    m_pButtonLoopHalve->set(1.0);
    m_pButtonLoopHalve->set(0.0);
    EXPECT_EQ(1 / 20.0, m_pBeatLoopSize->get());

    // Halve BeatLoopSize (the result would be 1/40 which is below the allowed
    // minimum of 1/32 -> This must be ignored)
    m_pButtonLoopHalve->set(1.0);
    m_pButtonLoopHalve->set(0.0);
    EXPECT_EQ(1 / 20.0, m_pBeatLoopSize->get());
}

TEST_F(LoopingControlTest, BeatLoopSize_SetDoesNotStartLoop) {
    m_pTrack1->trySetBpm(120.0);
    m_pBeatLoopSize->set(16.0);
    EXPECT_FALSE(m_pLoopEnabled->toBool());
}

TEST_F(LoopingControlTest, BeatLoopSize_ResizeKeepsStartPosition) {
    setCurrentPosition(mixxx::audio::FramePos{50});
    m_pTrack1->trySetBpm(160.0);
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
    setCurrentPosition(mixxx::audio::FramePos{50});
    m_pTrack1->trySetBpm(160.0);
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
    setCurrentPosition(mixxx::audio::FramePos{50});
    m_pTrack1->trySetBpm(160.0);
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
    setCurrentPosition(mixxx::audio::FramePos{50});
    m_pTrack1->trySetBpm(160.0);
    m_pQuantizeEnabled->set(0);
    m_pBeatLoopSize->set(4.0);
    m_pButtonLoopIn->set(1);
    m_pButtonLoopIn->set(0);
    setCurrentPosition(mixxx::audio::FramePos{500});
    m_pButtonLoopOut->set(1);
    m_pButtonLoopOut->set(0);
    double oldLoopStart = m_pLoopStartPoint->get();
    double oldLoopEnd = m_pLoopEndPoint->get();

    m_pBeatLoopSize->set(8.0);
    double newLoopStart = m_pLoopStartPoint->get();
    double newLoopEnd = m_pLoopEndPoint->get();
    EXPECT_EQ(oldLoopStart, newLoopStart);
    EXPECT_EQ(oldLoopEnd, newLoopEnd);
}

TEST_F(LoopingControlTest, LegacyBeatLoopControl) {
    m_pTrack1->trySetBpm(120.0);
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

TEST_F(LoopingControlTest, BeatjumpSize_SetRangeCheck) {
    // Set BeatJumpSize to the maximum allowed value
    m_pBeatJumpSize->set(512.0);
    EXPECT_EQ(512, m_pBeatJumpSize->get());

    m_pBeatJumpSize->set(150.0);
    EXPECT_EQ(150, m_pBeatJumpSize->get());

    // Set BeatJumpSize to a value above the allowed maximum of 512 -> This must be ignored
    m_pBeatJumpSize->set(513.0);
    EXPECT_EQ(150, m_pBeatJumpSize->get());

    // Double BeatJumpSize (the result is 300 which is in the allowed range)
    m_pButtonBeatJumpSizeDouble->set(1.0);
    m_pButtonBeatJumpSizeDouble->set(0.0);
    EXPECT_EQ(300.0, m_pBeatJumpSize->get());

    // Double BeatJumpSize (the result would be 600 which is above the allowed
    // maximum of 512-> This must be ignored)
    m_pButtonBeatJumpSizeDouble->set(1.0);
    m_pButtonBeatJumpSizeDouble->set(0.0);
    EXPECT_EQ(300.0, m_pBeatJumpSize->get());

    // Set BeatJumpSize to the minimum allowed value
    m_pBeatJumpSize->set(1 / 32.0);
    EXPECT_EQ(1 / 32.0, m_pBeatJumpSize->get());

    m_pBeatJumpSize->set(1 / 10.0);
    EXPECT_EQ(1 / 10.0, m_pBeatJumpSize->get());

    // Set BeatJumpSize to a value below the allowed minimum of 1/32 -> This must be ignored
    m_pBeatJumpSize->set(1 / 33.0);
    EXPECT_EQ(1 / 10.0, m_pBeatJumpSize->get());

    m_pBeatJumpSize->set(0);
    EXPECT_EQ(1 / 10.0, m_pBeatJumpSize->get());

    // Halve BeatJumpSize (the result is 1/20 which is in the allowed range)
    m_pButtonBeatJumpSizeHalve->set(1.0);
    m_pButtonBeatJumpSizeHalve->set(0.0);
    EXPECT_EQ(1 / 20.0, m_pBeatJumpSize->get());

    // Halve BeatJumpSize (the result would be 1/40 which is below the allowed
    // minimum of 1/32 -> This must be ignored)
    m_pButtonBeatJumpSizeHalve->set(1.0);
    m_pButtonBeatJumpSizeHalve->set(0.0);
    EXPECT_EQ(1 / 20.0, m_pBeatJumpSize->get());
}

TEST_F(LoopingControlTest, Beatjump_JumpsByBeats) {
    const auto bpm = mixxx::Bpm{120};
    m_pTrack1->trySetBpm(bpm);
    ProcessBuffer();

    const mixxx::audio::FrameDiff_t beatLengthFrames = 60.0 * 44100.0 / bpm.value();
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{beatLengthFrames}, m_pNextBeat);
    EXPECT_NE(0, beatLengthFrames);

    m_pBeatJumpSize->set(4.0);
    m_pButtonBeatJumpForward->set(1.0);
    m_pButtonBeatJumpForward->set(0.0);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{beatLengthFrames * 4}, currentFramePos());
    m_pButtonBeatJumpBackward->set(1.0);
    m_pButtonBeatJumpBackward->set(0.0);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ(mixxx::audio::kStartFramePos, currentFramePos());
}

TEST_F(LoopingControlTest, Beatjump_MovesActiveLoop) {
    const auto bpm = mixxx::Bpm{120};
    m_pTrack1->trySetBpm(bpm);
    ProcessBuffer();

    const mixxx::audio::FrameDiff_t beatLengthFrames = 60.0 * 44100.0 / bpm.value();
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{beatLengthFrames}, m_pNextBeat);
    EXPECT_NE(0, beatLengthFrames);

    m_pBeatLoopSize->set(4.0);
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    const auto loopStartPosition = mixxx::audio::kStartFramePos;
    const auto loopEndPosition = mixxx::audio::FramePos{beatLengthFrames * 4};
    EXPECT_FRAMEPOS_EQ_CONTROL(loopStartPosition, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(loopEndPosition, m_pLoopEndPoint);

    m_pBeatJumpSize->set(1.0);
    m_pButtonBeatJumpForward->set(1.0);
    m_pButtonBeatJumpForward->set(0.0);
    EXPECT_FRAMEPOS_EQ_CONTROL(loopStartPosition + beatLengthFrames, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(loopEndPosition + beatLengthFrames, m_pLoopEndPoint);

    // jump backward with playposition outside the loop should not move the loop
    m_pButtonBeatJumpBackward->set(1.0);
    m_pButtonBeatJumpBackward->set(0.0);
    EXPECT_FRAMEPOS_EQ_CONTROL(loopStartPosition + beatLengthFrames, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(loopEndPosition + beatLengthFrames, m_pLoopEndPoint);

    setCurrentPosition(mixxx::audio::FramePos{beatLengthFrames});
    m_pButtonBeatJumpBackward->set(1.0);
    m_pButtonBeatJumpBackward->set(0.0);
    EXPECT_FRAMEPOS_EQ_CONTROL(loopStartPosition, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(loopEndPosition, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, Beatjump_MovesLoopBoundaries) {
    // Holding down the loop in/out buttons and using beatjump should
    // move only the loop in/out point, but not shift the entire loop forward/backward
    const auto bpm = mixxx::Bpm{120};
    m_pTrack1->trySetBpm(bpm);
    ProcessBuffer();

    const mixxx::audio::FrameDiff_t beatLengthFrames = 60.0 * 44100.0 / bpm.value();
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{beatLengthFrames}, m_pNextBeat);
    EXPECT_NE(0, beatLengthFrames);

    m_pBeatLoopSize->set(4.0);
    m_pButtonBeatLoopActivate->set(1.0);
    m_pButtonBeatLoopActivate->set(0.0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{beatLengthFrames * 4}, m_pLoopEndPoint);

    m_pButtonLoopIn->set(1.0);
    m_pBeatJumpSize->set(1.0);
    m_pButtonBeatJumpForward->set(1.0);
    m_pButtonBeatJumpForward->set(0.0);
    ProcessBuffer();
    m_pButtonLoopIn->set(0.0);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{beatLengthFrames}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{beatLengthFrames * 4}, m_pLoopEndPoint);

    m_pButtonLoopOut->set(1.0);
    m_pButtonBeatJumpForward->set(1.0);
    m_pButtonBeatJumpForward->set(0.0);
    ProcessBuffer();
    m_pButtonLoopOut->set(0.0);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{beatLengthFrames}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{beatLengthFrames * 2}, m_pLoopEndPoint);
}

TEST_F(LoopingControlTest, LoopEscape) {
    m_pLoopStartPoint->set(mixxx::audio::FramePos{100}.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{200}.toEngineSamplePos());
    m_pButtonReloopToggle->set(1.0);
    m_pButtonReloopToggle->set(0.0);
    ProcessBuffer();
    EXPECT_TRUE(isLoopEnabled());
    // seek outside a loop should disable it
    setCurrentPosition(mixxx::audio::FramePos{300});
    EXPECT_FALSE(isLoopEnabled());

    m_pButtonReloopToggle->set(1.0);
    m_pButtonReloopToggle->set(0.0);
    ProcessBuffer();
    EXPECT_TRUE(isLoopEnabled());
    // seek outside a loop should disable it
    setCurrentPosition(mixxx::audio::FramePos{50});
    EXPECT_FALSE(isLoopEnabled());
}

TEST_F(LoopingControlTest, BeatLoopRoll_Activation) {
    m_pTrack1->trySetBpm(120.0);

    m_pButtonBeatLoopRoll2Activate->set(1.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop2Enabled->toBool());

    m_pButtonBeatLoopRoll2Activate->set(0.0);
    EXPECT_FALSE(m_pLoopEnabled->toBool());
    EXPECT_FALSE(m_pBeatLoop2Enabled->toBool());
}

TEST_F(LoopingControlTest, BeatLoopRoll_Overlap) {
    m_pTrack1->trySetBpm(120.0);

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
    m_pTrack1->trySetBpm(120.0);

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
    m_pTrack1->trySetBpm(120.0);

    // start a 4 beat loop roll, start point should be overridden to play position
    m_pLoopStartPoint->set(mixxx::audio::FramePos{8}.toEngineSamplePos());
    m_pButtonBeatLoopRoll4Activate->set(1.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop4Enabled->toBool());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);

    // move the start point, activate a 1 beat loop roll, new start point be preserved
    m_pLoopStartPoint->set(mixxx::audio::FramePos{8}.toEngineSamplePos());
    m_pButtonBeatLoopRoll1Activate->set(1.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop1Enabled->toBool());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{8}, m_pLoopStartPoint);

    // end the 1 beat loop roll
    m_pButtonBeatLoopRoll1Activate->set(0.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{8}, m_pLoopStartPoint);
    EXPECT_TRUE(m_pBeatLoop4Enabled->toBool());

    // end the 4 beat loop roll
    m_pButtonBeatLoopRoll4Activate->set(0.0);
    EXPECT_FALSE(m_pLoopEnabled->toBool());
    EXPECT_FALSE(m_pBeatLoop4Enabled->toBool());

    // new loop should start back at 0
    m_pButtonBeatLoopRoll4Activate->set(1.0);
    EXPECT_TRUE(m_pLoopEnabled->toBool());
    EXPECT_TRUE(m_pBeatLoop4Enabled->toBool());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
}

TEST_F(LoopingControlTest, LoopResizeUsingAnchor) {
    // Activating a new loop with a loop active should warp the playposition
    // the same as it does when we scale the loop larger and smaller so we
    // keep in sync with the beat.

    // Disable quantize for this test
    m_pQuantizeEnabled->set(0.0);

    m_pTrack1->trySetBpm(23520);
    m_pLoopStartPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{300}.toEngineSamplePos());
    setCurrentPosition(mixxx::audio::FramePos{250});
    m_pButtonReloopToggle->set(1);
    EXPECT_TRUE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{300}, m_pLoopEndPoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{250}, currentFramePos());

    // Set loop anchor to end
    m_pButtonBeatLoopAnchor->set(1.0);
    // Activate a shorter loop
    m_pButtonBeatLoop2Activate->set(1.0);

    ProcessBuffer();

    // The loop is resized and we should have seeked to a mid-beat part of the
    // loop.
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{75}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{300}, m_pLoopEndPoint);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{250}, currentFramePos());

    // But if looping is not enabled, no warping occurs.
    m_pLoopStartPoint->set(mixxx::audio::kStartFramePos.toEngineSamplePos());
    m_pLoopEndPoint->set(mixxx::audio::FramePos{300}.toEngineSamplePos());
    setCurrentPosition(mixxx::audio::FramePos{250});
    m_pButtonReloopToggle->set(1);
    EXPECT_FALSE(isLoopEnabled());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{300}, m_pLoopEndPoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{250}, currentFramePos());

    setCurrentPosition(mixxx::audio::FramePos{400});

    m_pButtonBeatLoop2Activate->set(1.0);
    ProcessBuffer();

    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{175}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{400}, m_pLoopEndPoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{400}, currentFramePos());

    // Set loop anchor to start
    m_pButtonBeatLoopAnchor->set(0.0);
    // Activate a larger loop
    m_pButtonBeatLoop4Activate->set(1.0);

    ProcessBuffer();

    // The loop is resized and its end joint this time
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{175}, m_pLoopStartPoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{625}, m_pLoopEndPoint);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos{175}, currentFramePos());
}

TEST_F(LoopingControlTest, LoopBeatloopReverse) {
    // Activating a new loop with a loop active should warp the playposition
    // the same as it does when we scale the loop larger and smaller so we
    // keep in sync with the beat.

    // Disable quantize for this test
    m_pQuantizeEnabled->set(0.0);

    m_pTrack1->trySetBpm(23520);

    double dBeatSizes[] = {0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8};

    // Test activate (only set the loop)
    for (unsigned int i = 0; i < (sizeof(dBeatSizes) / sizeof(dBeatSizes[0])); ++i) {
        m_pLoopEnabled->set(0.0);
        EXPECT_FALSE(isLoopEnabled());
        EXPECT_EQ(0.0, m_pButtonBeatLoopAnchor->get());
        setCurrentPosition(mixxx::audio::FramePos{1000});
        auto control = std::make_unique<PollingControlProxy>(
                ConfigKey(m_sGroup1, QString("beatloop_r%1_activate").arg(dBeatSizes[i])));
        control->set(1.0);
        EXPECT_TRUE(isLoopEnabled());
        EXPECT_FRAMEPOS_EQ_CONTROL(
                mixxx::audio::FramePos{1000 - dBeatSizes[i] * 112.5},
                m_pLoopStartPoint);
        EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{1000}, m_pLoopEndPoint);
        EXPECT_EQ(0.0, m_pButtonBeatLoopAnchor->get());
    }

    // Test activate (set the loop and deactivate)
    for (unsigned int i = 0; i < (sizeof(dBeatSizes) / sizeof(dBeatSizes[0])); ++i) {
        m_pLoopEnabled->set(0.0);
        EXPECT_FALSE(isLoopEnabled());
        EXPECT_EQ(0.0, m_pButtonBeatLoopAnchor->get());
        setCurrentPosition(mixxx::audio::FramePos{1000});
        auto control = std::make_unique<PollingControlProxy>(
                ConfigKey(m_sGroup1, QString("beatloop_r%1_toggle").arg(dBeatSizes[i])));
        control->set(1.0);
        EXPECT_TRUE(isLoopEnabled());
        EXPECT_FRAMEPOS_EQ_CONTROL(
                mixxx::audio::FramePos{1000 - dBeatSizes[i] * 112.5},
                m_pLoopStartPoint);
        EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{1000}, m_pLoopEndPoint);
        EXPECT_EQ(0.0, m_pButtonBeatLoopAnchor->get());
        control->set(1.0);
        EXPECT_FALSE(isLoopEnabled());
        EXPECT_EQ(0.0, m_pButtonBeatLoopAnchor->get());
    }

    // Test roll activate (only set the loop and slip)
    for (unsigned int i = 0; i < (sizeof(dBeatSizes) / sizeof(dBeatSizes[0])); ++i) {
        EXPECT_FALSE(isLoopEnabled());
        EXPECT_EQ(0.0, m_pButtonBeatLoopAnchor->get());
        setCurrentPosition(mixxx::audio::FramePos{1000});
        auto control = std::make_unique<PollingControlProxy>(
                ConfigKey(m_sGroup1, QString("beatlooproll_r%1_activate").arg(dBeatSizes[i])));
        control->set(1.0);
        EXPECT_TRUE(isLoopEnabled());
        EXPECT_EQ(1.0, m_pSlipEnabled->get());
        EXPECT_FRAMEPOS_EQ_CONTROL(
                mixxx::audio::FramePos{1000 - dBeatSizes[i] * 112.5},
                m_pLoopStartPoint);
        EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos{1000}, m_pLoopEndPoint);
        EXPECT_EQ(0.0, m_pButtonBeatLoopAnchor->get());
        m_pLoopEnabled->set(0.0);
        EXPECT_FALSE(isLoopEnabled());
        EXPECT_EQ(0.0, m_pSlipEnabled->get());
    }
}
