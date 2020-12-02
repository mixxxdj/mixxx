#include "engine/controls/bpmcontrol.h"

#include <gtest/gtest-message.h>
#include <gtest/gtest-test-part.h>

#include <memory>

#include "audio/types.h"
#include "gtest/gtest_pred_impl.h"
#include "mixxxtest.h"
#include "track/beatfactory.h"
#include "track/beats.h"
#include "track/track.h"
#include "track/track_decl.h"
#include "util/duration.h"

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

    const double bpm = 60.0;
    const int kFrameSize = 2;
    const double expectedBeatLength = (60.0 * sampleRate / bpm) * kFrameSize;

    mixxx::BeatsPointer pBeats = BeatFactory::makeBeatGrid(*pTrack, bpm, 0);

    // On a beat.
    double prevBeat, nextBeat, beatLength, beatPercentage;
    EXPECT_TRUE(BpmControl::getBeatContext(pBeats, 0.0, &prevBeat, &nextBeat,
                                           &beatLength, &beatPercentage));
    EXPECT_DOUBLE_EQ(0.0, prevBeat);
    EXPECT_DOUBLE_EQ(beatLength, nextBeat);
    EXPECT_DOUBLE_EQ(expectedBeatLength, beatLength);
    EXPECT_DOUBLE_EQ(0.0, beatPercentage);
}
