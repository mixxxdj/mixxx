#include "vinylcontrol/vinylcontrolxwax.h"

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "control/controlobject.h"
#include "engine/slipmodestate.h"
#include "test/mixxxtest.h"
#include "util/defs.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "vinylcontrol/vinylsignalquality.h"
#include "waveform/visualplayposition.h"

namespace {

const QString kGroup = QStringLiteral("[Channel1]");
constexpr size_t kFrames = 512;
constexpr double kTrackDuration = 100.0;

class VinylControlXwaxTest : public MixxxTest {
  protected:
    VinylControlXwaxTest() {
        addControl(QStringLiteral("[App]"), QStringLiteral("samplerate"))->set(44100);
        addControl(QStringLiteral("[App]"), QStringLiteral("output_latency_ms"))->set(20);
        addControl(kVinylPrefKey, QStringLiteral("gain"));

        for (const auto* key : {"track_samples",
                     "track_samplerate",
                     "vinylcontrol_seek",
                     "vinylcontrol_rate",
                     "rate_ratio",
                     "play",
                     "duration",
                     "vinylcontrol_mode",
                     "vinylcontrol_enabled",
                     "vinylcontrol_wantenabled",
                     "vinylcontrol_cueing",
                     "vinylcontrol_scratching",
                     "vinylcontrol_status",
                     "loop_enabled",
                     "vinylcontrol_signal_enabled",
                     "reverse",
                     "passthrough",
                     "scratch_position_enable"}) {
            addControl(kGroup, QString::fromLatin1(key));
        }

        control("duration")->set(kTrackDuration);
        control("track_samplerate")->set(44100);
        control("track_samples")->set(kTrackDuration * 44100 * 2);
        control("rate_ratio")->set(1.0);
        control("vinylcontrol_mode")->set(kVcModeRelative);

        m_pVinylControl = std::make_unique<VinylControlXwax>(config(), kGroup);
    }

    ~VinylControlXwaxTest() override {
        m_pVinylControl.reset();
        VinylControlXwax::freeLUTs();
    }

    ControlObject* addControl(const QString& group, const QString& key) {
        m_controls.push_back(std::make_unique<ControlObject>(ConfigKey(group, key)));
        return m_controls.back().get();
    }

    ControlObject* control(const char* key) const {
        return ControlObject::getControl(ConfigKey(kGroup, QString::fromLatin1(key)));
    }

    void enable() {
        control("vinylcontrol_enabled")->set(1.0);
        analyzeSilence();
    }

    void setPlayPosition(double fractionalPos) const {
        VisualPlayPosition::getVisualPlayPosition(kGroup)->set(fractionalPos,
                1.0,
                0.0,
                fractionalPos,
                1.0,
                SlipModeState::Disabled,
                false,
                false,
                false,
                0.0,
                0.0,
                kTrackDuration,
                0.0);
    }

    void analyze(CSAMPLE value, size_t frames = kFrames) {
        std::vector samples(frames * 2, value);
        m_pVinylControl->analyzeSamples(samples.data(), frames);
    }

    void analyzeSilence() {
        analyze(0.0f);
    }

    /// Analyze a buffer with a signal but without a decodable timecode, which
    /// remembers the current file position as the last one with a signal.
    void analyzeSignal() {
        analyze(0.5f);
    }

    float timecodeQuality() const {
        VinylSignalQualityReport report;
        m_pVinylControl->writeQualityReport(&report);
        return report.timecode_quality;
    }

    std::vector<std::unique_ptr<ControlObject>> m_controls;
    std::unique_ptr<VinylControlXwax> m_pVinylControl;
};

TEST_F(VinylControlXwaxTest, disabledDoesNotTouchPlayback) {
    control("play")->set(1.0);
    control("vinylcontrol_rate")->set(0.5);

    analyzeSilence();

    EXPECT_FALSE(m_pVinylControl->isEnabled());
    EXPECT_DOUBLE_EQ(1.0, control("play")->get());
    EXPECT_DOUBLE_EQ(0.5, control("vinylcontrol_rate")->get());
    EXPECT_DOUBLE_EQ(kVinylStatusDisabled, control("vinylcontrol_status")->get());
}

TEST_F(VinylControlXwaxTest, wantEnabledEnablesVinylControl) {
    control("vinylcontrol_wantenabled")->set(1.0);

    analyzeSilence();

    EXPECT_TRUE(m_pVinylControl->isEnabled());
    EXPECT_DOUBLE_EQ(1.0, control("vinylcontrol_enabled")->get());
    EXPECT_DOUBLE_EQ(0.0, control("vinylcontrol_wantenabled")->get());
}

TEST_F(VinylControlXwaxTest, enablingSetsStatusOk) {
    enable();

    EXPECT_TRUE(m_pVinylControl->isEnabled());
    EXPECT_DOUBLE_EQ(kVinylStatusOk, control("vinylcontrol_status")->get());
}

TEST_F(VinylControlXwaxTest, disablingKeepsRateRatioAndSetsStatusDisabled) {
    enable();
    control("rate_ratio")->set(1.2);

    control("vinylcontrol_enabled")->set(0.0);
    analyzeSilence();

    EXPECT_FALSE(m_pVinylControl->isEnabled());
    EXPECT_DOUBLE_EQ(1.2, control("vinylcontrol_rate")->get());
    EXPECT_DOUBLE_EQ(kVinylStatusDisabled, control("vinylcontrol_status")->get());
}

TEST_F(VinylControlXwaxTest, silenceStopsPlaybackWhenPositionDoesNotMove) {
    setPlayPosition(0.1);
    enable();
    analyzeSignal();
    control("play")->set(1.0);
    control("vinylcontrol_rate")->set(1.0);

    analyzeSilence();

    EXPECT_DOUBLE_EQ(0.0, control("play")->get());
    EXPECT_DOUBLE_EQ(0.0, control("vinylcontrol_rate")->get());
    EXPECT_DOUBLE_EQ(1.0, control("rate_ratio")->get());
    EXPECT_DOUBLE_EQ(kVinylStatusOk, control("vinylcontrol_status")->get());
    EXPECT_FLOAT_EQ(0.0f, timecodeQuality());
}

TEST_F(VinylControlXwaxTest, silenceKeepsPlayingBrieflyWhilePositionMoves) {
    setPlayPosition(0.1);
    enable();
    analyzeSignal();
    control("play")->set(1.0);

    // Advance by 0.1 seconds, less than the 0.3 seconds grace period
    setPlayPosition(0.1 + 0.1 / kTrackDuration);
    analyzeSilence();

    EXPECT_DOUBLE_EQ(1.0, control("play")->get());
    EXPECT_DOUBLE_EQ(1.0, control("rate_ratio")->get());
}

TEST_F(VinylControlXwaxTest, silenceStopsPlaybackAfterGracePeriod) {
    setPlayPosition(0.1);
    enable();
    analyzeSignal();
    control("play")->set(1.0);

    // Advance by 0.5 seconds, more than the 0.3 seconds grace period
    setPlayPosition(0.1 + 0.5 / kTrackDuration);
    analyzeSilence();

    EXPECT_DOUBLE_EQ(0.0, control("play")->get());
}

TEST_F(VinylControlXwaxTest, switchingToAbsoluteWhilePlayingFallsBackToRelative) {
    enable();
    control("play")->set(1.0);
    control("vinylcontrol_mode")->set(kVcModeAbsolute);

    analyzeSilence();

    EXPECT_DOUBLE_EQ(kVcModeRelative, control("vinylcontrol_mode")->get());
}

TEST_F(VinylControlXwaxTest, switchingToAbsoluteWhileStoppedIsAccepted) {
    enable();
    control("vinylcontrol_mode")->set(kVcModeAbsolute);

    analyzeSilence();

    EXPECT_DOUBLE_EQ(kVcModeAbsolute, control("vinylcontrol_mode")->get());
}

TEST_F(VinylControlXwaxTest, loopForcesRelativeMode) {
    enable();
    control("vinylcontrol_mode")->set(kVcModeAbsolute);
    analyzeSilence();

    control("loop_enabled")->set(1.0);
    analyzeSilence();

    EXPECT_DOUBLE_EQ(kVcModeRelative, control("vinylcontrol_mode")->get());
}

TEST_F(VinylControlXwaxTest, absoluteModeDisablesCueing) {
    enable();
    control("vinylcontrol_cueing")->set(kRelativeCueHotCue);
    control("vinylcontrol_mode")->set(kVcModeAbsolute);

    analyzeSilence();

    EXPECT_DOUBLE_EQ(kRelativeCueOff, control("vinylcontrol_cueing")->get());
}

TEST_F(VinylControlXwaxTest, switchingToRelativeClearsErrorStatus) {
    enable();
    control("vinylcontrol_mode")->set(kVcModeAbsolute);
    analyzeSilence();
    control("vinylcontrol_status")->set(kVinylStatusError);

    // Keep the position moving so that the silence doesn't reset the status
    setPlayPosition(0.1 / kTrackDuration);
    control("vinylcontrol_mode")->set(kVcModeRelative);
    analyzeSilence();

    EXPECT_DOUBLE_EQ(kVinylStatusOk, control("vinylcontrol_status")->get());
}

TEST_F(VinylControlXwaxTest, constantModeWhilePlayingUsesRateRatio) {
    enable();
    control("play")->set(1.0);
    control("rate_ratio")->set(1.1);
    control("vinylcontrol_mode")->set(kVcModeConstant);

    analyzeSilence();

    EXPECT_DOUBLE_EQ(1.1, control("vinylcontrol_rate")->get());
    EXPECT_DOUBLE_EQ(1.0, control("play")->get());
}

TEST_F(VinylControlXwaxTest, constantModeWhileStoppedStopsRate) {
    enable();
    control("vinylcontrol_rate")->set(1.0);
    control("vinylcontrol_mode")->set(kVcModeConstant);

    analyzeSilence();

    EXPECT_DOUBLE_EQ(0.0, control("vinylcontrol_rate")->get());
}

TEST_F(VinylControlXwaxTest, signalWithoutTimecodeDoesNotStartPlayback) {
    enable();

    analyzeSignal();

    EXPECT_DOUBLE_EQ(0.0, control("play")->get());
}

TEST_F(VinylControlXwaxTest, buffersLargerThanTheWorkBufferAreAccepted) {
    enable();

    analyze(0.0f, MAX_BUFFER_LEN);

    EXPECT_TRUE(m_pVinylControl->isEnabled());
}

} // namespace
