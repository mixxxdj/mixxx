#include "engine/controls/bpmcontrol.h"

#include <gtest/gtest.h>

#include <QScopedPointer>
#include <QtDebug>

#include "audio/types.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "mixxxtest.h"
#include "track/beats.h"
#include "track/track.h"

class BpmControlTest : public MixxxTest {
};

TEST_F(BpmControlTest, ShortestPercentageChange) {
    const double kEpsilon = 0.0000000001;
    EXPECT_NEAR(-0.02, BpmControl::shortestPercentageChange(0.01, 0.99), kEpsilon);
    EXPECT_NEAR(0.02, BpmControl::shortestPercentageChange(0.99, 0.01), kEpsilon);
    EXPECT_NEAR(0.40, BpmControl::shortestPercentageChange(0.80, 0.20), kEpsilon);
    EXPECT_NEAR(-0.40, BpmControl::shortestPercentageChange(0.20, 0.80), kEpsilon);
}

TEST_F(BpmControlTest, BeatContext_BeatGrid) {
    constexpr auto sampleRate = mixxx::audio::SampleRate(44100);

    TrackPointer pTrack = Track::newTemporary();
    pTrack->setAudioProperties(
            mixxx::audio::ChannelCount(2),
            mixxx::audio::SampleRate(sampleRate),
            mixxx::audio::Bitrate(),
            mixxx::Duration::fromSeconds(180));

    const auto bpm = mixxx::Bpm(60.0);
    const mixxx::audio::FrameDiff_t expectedBeatLengthFrames = (60.0 * sampleRate / bpm.value());

    const mixxx::BeatsPointer pBeats = mixxx::Beats::fromConstTempo(
            pTrack->getSampleRate(), mixxx::audio::kStartFramePos, bpm);

    // On a beat.
    mixxx::audio::FramePos prevBeatPosition;
    mixxx::audio::FramePos nextBeatPosition;
    mixxx::audio::FrameDiff_t beatLengthFrames;
    double beatPercentage;
    EXPECT_TRUE(BpmControl::getBeatContext(pBeats,
            mixxx::audio::kStartFramePos,
            &prevBeatPosition,
            &nextBeatPosition,
            &beatLengthFrames,
            &beatPercentage));
    EXPECT_EQ(mixxx::audio::kStartFramePos, prevBeatPosition);
    EXPECT_EQ(mixxx::audio::FramePos{beatLengthFrames}, nextBeatPosition);
    EXPECT_DOUBLE_EQ(expectedBeatLengthFrames, beatLengthFrames);
    EXPECT_DOUBLE_EQ(0.0, beatPercentage);
}
