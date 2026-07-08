#include <gtest/gtest.h>

#include "control/controlproxy.h"
#include "engine/controls/cuecontrol.h"
#include "engine/enginebuffer.h"
#include "test/signalpathtest.h"
#include "track/beats.h"
#include "track/bpm.h"

namespace {
constexpr int kSampleRate = 44100;
constexpr double kBpm = 120.0;
// One beat at 120 BPM / 44100 Hz = 22050 frames.
constexpr double kFramesPerBeat = kSampleRate * 60.0 / kBpm;
// The test audio file (sine-30.wav) is 30 s long = 60 beats at 120 BPM. Cue
// positions used below stay within that range so the playhead can reach them.
} // namespace

/// Tests for the per-deck `beats_to_next_hotcue` control that backs the
/// on-screen bars-to-next-hotcue counter (see specs/001-hotcue-bar-counter).
/// The engine publishes the distance to the next hot cue ahead in beats
/// (always >= 0), or a negative sentinel when there is no beatgrid or no hot
/// cue ahead; the UI converts beats to bars remaining.
class HotcueBarCounterTest : public BaseSignalPathTest {
  protected:
    void SetUp() override {
        BaseSignalPathTest::SetUp();
        m_pBeatsToNextHotcue1 =
                std::make_unique<ControlProxy>(m_sGroup1, "beats_to_next_hotcue");
        m_pBeatsToNextHotcue2 =
                std::make_unique<ControlProxy>(m_sGroup2, "beats_to_next_hotcue");
        m_pBarsBeatsToNextHotcue1 =
                std::make_unique<ControlProxy>(m_sGroup1, "bars_beats_to_next_hotcue");
    }

    TrackPointer createTestTrack() const {
        const QString kTrackLocationTest =
                getTestDir().filePath(QStringLiteral("sine-30.wav"));
        const auto pTrack = Track::newTemporary(
                mixxx::FileAccess(mixxx::FileInfo(kTrackLocationTest)));
        pTrack->setAudioProperties(
                mixxx::audio::ChannelCount(2),
                mixxx::audio::SampleRate(kSampleRate),
                mixxx::audio::Bitrate(),
                mixxx::Duration::fromSeconds(180));
        return pTrack;
    }

    void setConstBeats(const TrackPointer& pTrack) const {
        pTrack->trySetBeats(mixxx::Beats::fromConstTempo(
                pTrack->getSampleRate(),
                mixxx::audio::kStartFramePos,
                mixxx::Bpm(kBpm)));
    }

    void loadTrack(const TrackPointer& pTrack) {
        BaseSignalPathTest::loadTrack(m_pMixerDeck1, pTrack);
        ProcessBuffer();
    }

    void seekTo(mixxx::audio::FramePos position) {
        m_pChannel1->getEngineBuffer()->queueNewPlaypos(
                position, EngineBuffer::SEEK_STANDARD);
        ProcessBuffer();
    }

    static mixxx::audio::FramePos beatsToFramePos(double beats) {
        return mixxx::audio::FramePos(beats * kFramesPerBeat);
    }

    std::unique_ptr<ControlProxy> m_pBeatsToNextHotcue1;
    std::unique_ptr<ControlProxy> m_pBeatsToNextHotcue2;
    std::unique_ptr<ControlProxy> m_pBarsBeatsToNextHotcue1;
};

// T006: value equals the beats to the next hot cue ahead and counts down.
TEST_F(HotcueBarCounterTest, CountsDownBeatsToNextHotcue) {
    TrackPointer pTrack = createTestTrack();
    setConstBeats(pTrack);
    // Hot cue 40 beats (10 bars) ahead of the start, within the audio range.
    pTrack->createAndAddCue(mixxx::CueType::HotCue,
            0,
            beatsToFramePos(40),
            mixxx::audio::kInvalidFramePos);
    loadTrack(pTrack);

    seekTo(mixxx::audio::kStartFramePos);
    EXPECT_DOUBLE_EQ(40.0, m_pBeatsToNextHotcue1->get());

    seekTo(beatsToFramePos(20));
    EXPECT_DOUBLE_EQ(20.0, m_pBeatsToNextHotcue1->get());

    // At the cue position the counter reads 0.
    seekTo(beatsToFramePos(40));
    EXPECT_DOUBLE_EQ(0.0, m_pBeatsToNextHotcue1->get());
}

// bars_beats_to_next_hotcue publishes a "bar.beat" countdown encoded as
// bar + beat/10, or a negative sentinel when there is no hot cue ahead.
// bar = (beats-1)/4, beat = ((beats-1) mod 4) + 1, reaching 0.0 at the cue.
TEST_F(HotcueBarCounterTest, PublishesBarBeatCountdown) {
    TrackPointer pTrack = createTestTrack();
    setConstBeats(pTrack);
    // Hot cue 40 beats (10 bars) ahead.
    pTrack->createAndAddCue(mixxx::CueType::HotCue,
            0,
            beatsToFramePos(40),
            mixxx::audio::kInvalidFramePos);
    loadTrack(pTrack);

    // N=40 beats remaining -> bar 9, beat 4 -> 9.4
    seekTo(mixxx::audio::kStartFramePos);
    EXPECT_NEAR(9.4, m_pBarsBeatsToNextHotcue1->get(), 1e-9);

    // N=20 -> bar 4, beat 4 -> 4.4
    seekTo(beatsToFramePos(20));
    EXPECT_NEAR(4.4, m_pBarsBeatsToNextHotcue1->get(), 1e-9);

    // N=17 -> bar 4, beat 1 -> 4.1
    seekTo(beatsToFramePos(23));
    EXPECT_NEAR(4.1, m_pBarsBeatsToNextHotcue1->get(), 1e-9);

    // N=16 -> bar 3, beat 4 -> 3.4
    seekTo(beatsToFramePos(24));
    EXPECT_NEAR(3.4, m_pBarsBeatsToNextHotcue1->get(), 1e-9);

    // N=4 -> bar 0, beat 4 -> 0.4 (last bar before the cue)
    seekTo(beatsToFramePos(36));
    EXPECT_NEAR(0.4, m_pBarsBeatsToNextHotcue1->get(), 1e-9);

    // N=1 (last beat before the cue) -> bar 0, beat 1 -> 0.1
    seekTo(beatsToFramePos(39));
    EXPECT_NEAR(0.1, m_pBarsBeatsToNextHotcue1->get(), 1e-9);

    // At the cue -> 0.0
    seekTo(beatsToFramePos(40));
    EXPECT_NEAR(0.0, m_pBarsBeatsToNextHotcue1->get(), 1e-9);

    // Past the only cue (no cue ahead) -> 0.0 (this display value is never
    // negative, unlike beats_to_next_hotcue which uses a -1 sentinel).
    seekTo(beatsToFramePos(44));
    EXPECT_NEAR(0.0, m_pBarsBeatsToNextHotcue1->get(), 1e-9);
    EXPECT_LT(m_pBeatsToNextHotcue1->get(), 0.0);
}

// T007: each deck reports its own value; a deck with no track shows the
// placeholder sentinel (negative).
TEST_F(HotcueBarCounterTest, IndependentPerDeck) {
    TrackPointer pTrack = createTestTrack();
    setConstBeats(pTrack);
    pTrack->createAndAddCue(mixxx::CueType::HotCue,
            0,
            beatsToFramePos(32),
            mixxx::audio::kInvalidFramePos);
    loadTrack(pTrack); // deck 1 only
    seekTo(mixxx::audio::kStartFramePos);

    EXPECT_DOUBLE_EQ(32.0, m_pBeatsToNextHotcue1->get());
    // Deck 2 has no track loaded -> placeholder sentinel.
    EXPECT_LT(m_pBeatsToNextHotcue2->get(), 0.0);
}

// T008: adding/moving a hot cue recomputes the value without a reload.
TEST_F(HotcueBarCounterTest, RecomputesWhenHotcueChanges) {
    TrackPointer pTrack = createTestTrack();
    setConstBeats(pTrack);
    pTrack->createAndAddCue(mixxx::CueType::HotCue,
            0,
            beatsToFramePos(40),
            mixxx::audio::kInvalidFramePos);
    loadTrack(pTrack);
    seekTo(mixxx::audio::kStartFramePos);
    EXPECT_DOUBLE_EQ(40.0, m_pBeatsToNextHotcue1->get());

    // A nearer hot cue becomes the next cue ahead.
    pTrack->createAndAddCue(mixxx::CueType::HotCue,
            1,
            beatsToFramePos(16),
            mixxx::audio::kInvalidFramePos);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(16.0, m_pBeatsToNextHotcue1->get());
}

// T013: no beatgrid -> placeholder sentinel.
TEST_F(HotcueBarCounterTest, NoBeatgridShowsPlaceholder) {
    TrackPointer pTrack = createTestTrack(); // no beats set
    pTrack->createAndAddCue(mixxx::CueType::HotCue,
            0,
            beatsToFramePos(16),
            mixxx::audio::kInvalidFramePos);
    loadTrack(pTrack);
    seekTo(mixxx::audio::kStartFramePos);

    EXPECT_LT(m_pBeatsToNextHotcue1->get(), 0.0);
}

// T013: no hot cue ahead of the play position -> placeholder sentinel.
TEST_F(HotcueBarCounterTest, NoCueAheadShowsPlaceholder) {
    TrackPointer pTrack = createTestTrack();
    setConstBeats(pTrack);
    pTrack->createAndAddCue(mixxx::CueType::HotCue,
            0,
            beatsToFramePos(16),
            mixxx::audio::kInvalidFramePos);
    loadTrack(pTrack);

    // Seek past the only hot cue.
    seekTo(beatsToFramePos(32));
    EXPECT_LT(m_pBeatsToNextHotcue1->get(), 0.0);
}
