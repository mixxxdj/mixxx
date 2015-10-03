#include <gtest/gtest.h>
#include <QtDebug>

#include "track/beatgrid.h"

namespace {

class BeatGridTest : public testing::Test {
  protected:

    BeatGridTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F(BeatGridTest, Scale) {
    TrackPointer pTrack(TrackInfoObject::newTemporary());

    int sampleRate = 44100;
    double bpm = 60.0;
    pTrack->setBpm(bpm);
    pTrack->setSampleRate(sampleRate);

    BeatGrid* pGrid = new BeatGrid(pTrack.data(), 0);
    pGrid->setBpm(bpm);

    EXPECT_DOUBLE_EQ(bpm, pGrid->getBpm());
    pGrid->scale(2);
    EXPECT_DOUBLE_EQ(2 * bpm, pGrid->getBpm());

    pGrid->scale(0.5);
    EXPECT_DOUBLE_EQ(bpm, pGrid->getBpm());

    pGrid->scale(0.25);
    EXPECT_DOUBLE_EQ(0.25 * bpm, pGrid->getBpm());
}

TEST_F(BeatGridTest, TestNthBeatWhenOnBeat) {
    TrackPointer pTrack(TrackInfoObject::newTemporary());

    int sampleRate = 44100;
    double bpm = 60.0;
    const int kFrameSize = 2;
    pTrack->setBpm(bpm);
    pTrack->setSampleRate(sampleRate);
    double beatLength = (60.0 * sampleRate / bpm) * kFrameSize;

    BeatGrid* pGrid = new BeatGrid(pTrack.data(), 0);
    pGrid->setBpm(bpm);
    // Pretend we're on the 20th beat;
    double position = beatLength * 20;

    // The spec dictates that a value of 0 is always invalid and returns -1
    EXPECT_EQ(-1, pGrid->findNthBeat(position, 0));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < 20; ++i) {
        EXPECT_EQ(position + beatLength*(i-1), pGrid->findNthBeat(position, i));
        EXPECT_EQ(position + beatLength*(-i+1), pGrid->findNthBeat(position, -i));
    }

    // Also test prev/next beat calculation.
    double prevBeat, nextBeat;
    pGrid->findPrevNextBeats(position, &prevBeat, &nextBeat);
    EXPECT_EQ(position, prevBeat);
    EXPECT_EQ(position + beatLength, nextBeat);

    // Both previous and next beat should return the current position.
    EXPECT_EQ(position, pGrid->findNextBeat(position));
    EXPECT_EQ(position, pGrid->findPrevBeat(position));
}

TEST_F(BeatGridTest, TestNthBeatWhenOnBeat_BeforeEpsilon) {
    TrackPointer pTrack(TrackInfoObject::newTemporary());

    int sampleRate = 44100;
    double bpm = 60.0;
    const int kFrameSize = 2;
    pTrack->setBpm(bpm);
    pTrack->setSampleRate(sampleRate);
    double beatLength = (60.0 * sampleRate / bpm) * kFrameSize;

    BeatGrid* pGrid = new BeatGrid(pTrack.data(), 0);
    pGrid->setBpm(bpm);

    // Pretend we're just before the 20th beat.
    const double kClosestBeat = 20 * beatLength;
    double position = kClosestBeat - beatLength * 0.005;

    // The spec dictates that a value of 0 is always invalid and returns -1
    EXPECT_EQ(-1, pGrid->findNthBeat(position, 0));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < 20; ++i) {
        EXPECT_EQ(kClosestBeat + beatLength*(i-1), pGrid->findNthBeat(position, i));
        EXPECT_EQ(kClosestBeat + beatLength*(-i+1), pGrid->findNthBeat(position, -i));
    }

    // Also test prev/next beat calculation.
    double prevBeat, nextBeat;
    pGrid->findPrevNextBeats(position, &prevBeat, &nextBeat);
    EXPECT_EQ(kClosestBeat, prevBeat);
    EXPECT_EQ(kClosestBeat + beatLength, nextBeat);

    // Both previous and next beat should return the closest beat.
    EXPECT_EQ(kClosestBeat, pGrid->findNextBeat(position));
    EXPECT_EQ(kClosestBeat, pGrid->findPrevBeat(position));
}

TEST_F(BeatGridTest, TestNthBeatWhenOnBeat_AfterEpsilon) {
    TrackPointer pTrack(TrackInfoObject::newTemporary());

    int sampleRate = 44100;
    double bpm = 60.0;
    const int kFrameSize = 2;
    pTrack->setBpm(bpm);
    pTrack->setSampleRate(sampleRate);
    double beatLength = (60.0 * sampleRate / bpm) * kFrameSize;

    BeatGrid* pGrid = new BeatGrid(pTrack.data(), 0);
    pGrid->setBpm(bpm);

    // Pretend we're just before the 20th beat.
    const double kClosestBeat = 20 * beatLength;
    double position = kClosestBeat + beatLength * 0.005;

    // The spec dictates that a value of 0 is always invalid and returns -1
    EXPECT_EQ(-1, pGrid->findNthBeat(position, 0));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < 20; ++i) {
        EXPECT_EQ(kClosestBeat + beatLength*(i-1), pGrid->findNthBeat(position, i));
        EXPECT_EQ(kClosestBeat + beatLength*(-i+1), pGrid->findNthBeat(position, -i));
    }

    // Also test prev/next beat calculation.
    double prevBeat, nextBeat;
    pGrid->findPrevNextBeats(position, &prevBeat, &nextBeat);
    EXPECT_EQ(kClosestBeat, prevBeat);
    EXPECT_EQ(kClosestBeat + beatLength, nextBeat);

    // Both previous and next beat should return the closest beat.
    EXPECT_EQ(kClosestBeat, pGrid->findNextBeat(position));
    EXPECT_EQ(kClosestBeat, pGrid->findPrevBeat(position));
}

TEST_F(BeatGridTest, TestNthBeatWhenNotOnBeat) {
    TrackPointer pTrack(TrackInfoObject::newTemporary());
    int sampleRate = 44100;
    double bpm = 60.0;
    const int kFrameSize = 2;
    pTrack->setBpm(bpm);
    pTrack->setSampleRate(sampleRate);
    double beatLength = (60.0 * sampleRate / bpm) * kFrameSize;

    BeatGrid* pGrid = new BeatGrid(pTrack.data(), 0);
    pGrid->setBpm(bpm);

    // Pretend we're half way between the 20th and 21st beat
    double previousBeat = beatLength * 20.0;
    double nextBeat = beatLength * 21.0;
    double position = (nextBeat + previousBeat) / 2.0;

    // The spec dictates that a value of 0 is always invalid and returns -1
    EXPECT_EQ(-1, pGrid->findNthBeat(position, 0));

    // findNthBeat should return multiples of beats starting from the next or
    // previous beat, depending on whether N is positive or negative.
    for (int i = 1; i < 20; ++i) {
        EXPECT_EQ(nextBeat + beatLength*(i-1), pGrid->findNthBeat(position, i));
        EXPECT_EQ(previousBeat + beatLength*(-i+1), pGrid->findNthBeat(position, -i));
    }

    // Also test prev/next beat calculation
    double foundPrevBeat, foundNextBeat;
    pGrid->findPrevNextBeats(position, &foundPrevBeat, &foundNextBeat);
    EXPECT_EQ(previousBeat, foundPrevBeat);
    EXPECT_EQ(nextBeat, foundNextBeat);
}

}  // namespace
