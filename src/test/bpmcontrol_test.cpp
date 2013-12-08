#include <gtest/gtest.h>

#include <QtDebug>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include "controlobjectthread.h"
#include "engine/bpmcontrol.h"
#include "track/beats.h"
#include "track/beatfactory.h"
#include "track/beatgrid.h"
#include "track/beatmap.h"
#include "trackinfoobject.h"

class BpmControlTest : public MixxxTest {
};

TEST_F(BpmControlTest, ShortestPercentageChange) {
    EXPECT_DOUBLE_EQ(-0.02, BpmControl::shortestPercentageChange(0.01, 0.99));
    EXPECT_DOUBLE_EQ(0.02, BpmControl::shortestPercentageChange(0.99, 0.01));
    EXPECT_DOUBLE_EQ(0.40, BpmControl::shortestPercentageChange(0.80, 0.20));
    EXPECT_DOUBLE_EQ(-0.40, BpmControl::shortestPercentageChange(0.20, 0.80));
}

TEST_F(BpmControlTest, BeatContext_BeatGrid) {
    const int sampleRate = 44100;
    const double bpm = 60.0;
    const int kFrameSize = 2;
    const double expectedBeatLength = (60.0 * sampleRate / bpm) * kFrameSize;
    TrackPointer pTrack = TrackPointer(new TrackInfoObject());
    pTrack->setSampleRate(sampleRate);

    BeatsPointer pBeats = BeatFactory::makeBeatGrid(pTrack.data(), bpm, 0);

    // On a beat.
    double prevBeat, nextBeat, beatLength, beatPercentage;
    EXPECT_TRUE(BpmControl::getBeatContext(pBeats, 0.0, &prevBeat, &nextBeat,
                                           &beatLength, &beatPercentage));
    EXPECT_DOUBLE_EQ(0.0, prevBeat);
    EXPECT_DOUBLE_EQ(beatLength, nextBeat);
    EXPECT_DOUBLE_EQ(expectedBeatLength, beatLength);
    EXPECT_DOUBLE_EQ(0.0, beatPercentage);
}
