#include "engine/controls/bpmcontrol.h"

#include <gtest/gtest.h>

#include <QScopedPointer>

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
    const int sampleRate = 44100;

    TrackPointer pTrack = Track::newTemporary();
    pTrack->setAudioProperties(
            mixxx::audio::ChannelCount(2),
            mixxx::audio::SampleRate(sampleRate),
            mixxx::audio::Bitrate(),
            mixxx::Duration::fromSeconds(180));

    const mixxx::Bpm bpm = mixxx::Bpm(60.0);
    const mixxx::FrameDiff_t expectedBeatLengthFrames = (60.0 * sampleRate / bpm.getValue());

    pTrack->setBpm(bpm.getValue());

    // On a beat.
    mixxx::FramePos prevBeat, nextBeat;
    mixxx::FrameDiff_t beatLength;
    double beatPercentage;
    EXPECT_TRUE(BpmControl::getBeatContext(pTrack->getBeats(),
            mixxx::kStartFramePos,
            &prevBeat,
            &nextBeat,
            &beatLength,
            &beatPercentage));
    EXPECT_DOUBLE_EQ(0.0, prevBeat.getValue());
    EXPECT_DOUBLE_EQ(beatLength, nextBeat.getValue());
    EXPECT_DOUBLE_EQ(expectedBeatLengthFrames, beatLength);
    EXPECT_DOUBLE_EQ(0.0, beatPercentage);
}
