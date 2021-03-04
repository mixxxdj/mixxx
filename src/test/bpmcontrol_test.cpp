#include <gtest/gtest.h>

#include <QtDebug>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "engine/controls/bpmcontrol.h"
#include "track/beatfactory.h"
#include "track/beatgrid.h"
#include "track/beatmap.h"
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

    const double bpm = 60.0;
    const int kFrameSize = 2;
    const double expectedBeatLength = (60.0 * sampleRate / bpm) * kFrameSize;

    const mixxx::BeatsPointer pBeats = BeatFactory::makeBeatGrid(pTrack->getSampleRate(), bpm, 0);

    // On a beat.
    double prevBeat, nextBeat, beatLength, beatPercentage;
    EXPECT_TRUE(BpmControl::getBeatContext(pBeats, 0.0, &prevBeat, &nextBeat,
                                           &beatLength, &beatPercentage));
    EXPECT_DOUBLE_EQ(0.0, prevBeat);
    EXPECT_DOUBLE_EQ(beatLength, nextBeat);
    EXPECT_DOUBLE_EQ(expectedBeatLength, beatLength);
    EXPECT_DOUBLE_EQ(0.0, beatPercentage);
}
