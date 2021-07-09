#include <gtest/gtest.h>

#include <QtDebug>

#include "track/beatgrid.h"
#include "track/track.h"
#include "util/memory.h"

using namespace mixxx;

namespace {

const double kMaxBeatError = 1e-9;

TrackPointer newTrack(int sampleRate) {
    TrackPointer pTrack(Track::newTemporary());
    pTrack->setAudioProperties(
            mixxx::audio::ChannelCount(2),
            mixxx::audio::SampleRate(sampleRate),
            mixxx::audio::Bitrate(),
            mixxx::Duration::fromSeconds(180));
    return pTrack;
}

TEST(BeatGridTest, Scale) {
    int sampleRate = 44100;
    TrackPointer pTrack = newTrack(sampleRate);

    constexpr mixxx::Bpm bpm(60.0);
    pTrack->trySetBpm(bpm.value());

    auto pGrid = BeatGrid::makeBeatGrid(pTrack->getSampleRate(),
            QString(),
            mixxx::Bpm(bpm),
            mixxx::audio::kStartFramePos);

    EXPECT_DOUBLE_EQ(bpm.value(), pGrid->getBpm().value());
    pGrid = pGrid->scale(Beats::BpmScale::Double);
    EXPECT_DOUBLE_EQ(2 * bpm.value(), pGrid->getBpm().value());

    pGrid = pGrid->scale(Beats::BpmScale::Halve);
    EXPECT_DOUBLE_EQ(bpm.value(), pGrid->getBpm().value());

    pGrid = pGrid->scale(Beats::BpmScale::TwoThirds);
    EXPECT_DOUBLE_EQ(bpm.value() * 2 / 3, pGrid->getBpm().value());

    pGrid = pGrid->scale(Beats::BpmScale::ThreeHalves);
    EXPECT_DOUBLE_EQ(bpm.value(), pGrid->getBpm().value());

    pGrid = pGrid->scale(Beats::BpmScale::ThreeFourths);
    EXPECT_DOUBLE_EQ(bpm.value() * 3 / 4, pGrid->getBpm().value());

    pGrid = pGrid->scale(Beats::BpmScale::FourThirds);
    EXPECT_DOUBLE_EQ(bpm.value(), pGrid->getBpm().value());
}

TEST(BeatGridTest, TestNthBeatWhenOnBeat) {
    constexpr int sampleRate = 44100;
    TrackPointer pTrack = newTrack(sampleRate);

    constexpr double bpm = 60.1;
    pTrack->trySetBpm(bpm);
    constexpr mixxx::audio::FrameDiff_t beatLengthFrames = 60.0 * sampleRate / bpm;

    auto pGrid = BeatGrid::makeBeatGrid(pTrack->getSampleRate(),
            QString(),
            mixxx::Bpm(bpm),
            mixxx::audio::kStartFramePos);
    // Pretend we're on the 20th beat;
    constexpr mixxx::audio::FramePos position(beatLengthFrames * 20);

    // The spec dictates that a value of 0 is always invalid and returns an invalid position
    EXPECT_FALSE(pGrid->findNthBeat(position, 0).isValid());

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < 20; ++i) {
        EXPECT_NEAR((position + beatLengthFrames * (i - 1)).value(),
                pGrid->findNthBeat(position, i).value(),
                kMaxBeatError);
        EXPECT_NEAR((position + beatLengthFrames * (-i + 1)).value(),
                pGrid->findNthBeat(position, -i).value(),
                kMaxBeatError);
    }

    // Also test prev/next beat calculation.
    mixxx::audio::FramePos prevBeat, nextBeat;
    pGrid->findPrevNextBeats(position, &prevBeat, &nextBeat, true);
    EXPECT_NEAR(position.value(), prevBeat.value(), kMaxBeatError);
    EXPECT_NEAR((position + beatLengthFrames).value(), nextBeat.value(), kMaxBeatError);

    // Also test prev/next beat calculation without snapping tolerance
    pGrid->findPrevNextBeats(position, &prevBeat, &nextBeat, false);
    EXPECT_NEAR(position.value(), prevBeat.value(), kMaxBeatError);
    EXPECT_NEAR((position + beatLengthFrames).value(), nextBeat.value(), kMaxBeatError);

    // Both previous and next beat should return the current position.
    EXPECT_NEAR(position.value(), pGrid->findNextBeat(position).value(), kMaxBeatError);
    EXPECT_NEAR(position.value(), pGrid->findPrevBeat(position).value(), kMaxBeatError);
}

TEST(BeatGridTest, TestNthBeatWhenOnBeat_BeforeEpsilon) {
    constexpr int sampleRate = 44100;
    TrackPointer pTrack = newTrack(sampleRate);

    constexpr double bpm = 60.1;
    pTrack->trySetBpm(bpm);
    constexpr double beatLengthFrames = 60.0 * sampleRate / bpm;

    auto pGrid = BeatGrid::makeBeatGrid(pTrack->getSampleRate(),
            QString(),
            mixxx::Bpm(bpm),
            mixxx::audio::kStartFramePos);

    // Pretend we're just before the 20th beat.
    constexpr mixxx::audio::FramePos kClosestBeat(20 * beatLengthFrames);
    const mixxx::audio::FramePos position(kClosestBeat - beatLengthFrames * 0.005);

    // The spec dictates that a value of 0 is always invalid and returns an invalid position
    EXPECT_FALSE(pGrid->findNthBeat(position, 0).isValid());

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < 20; ++i) {
        EXPECT_NEAR((kClosestBeat + beatLengthFrames * (i - 1)).value(),
                pGrid->findNthBeat(position, i).value(),
                kMaxBeatError);
        EXPECT_NEAR((kClosestBeat + beatLengthFrames * (-i + 1)).value(),
                pGrid->findNthBeat(position, -i).value(),
                kMaxBeatError);
    }

    // Also test prev/next beat calculation.
    mixxx::audio::FramePos prevBeat, nextBeat;
    pGrid->findPrevNextBeats(position, &prevBeat, &nextBeat, true);
    EXPECT_NEAR(kClosestBeat.value(), prevBeat.value(), kMaxBeatError);
    EXPECT_NEAR((kClosestBeat + beatLengthFrames).value(), nextBeat.value(), kMaxBeatError);

    // Also test prev/next beat calculation without snapping tolerance
    pGrid->findPrevNextBeats(position, &prevBeat, &nextBeat, false);
    EXPECT_NEAR((kClosestBeat - beatLengthFrames).value(), prevBeat.value(), kMaxBeatError);
    EXPECT_NEAR(kClosestBeat.value(), nextBeat.value(), kMaxBeatError);

    // Both previous and next beat should return the closest beat.
    EXPECT_NEAR(kClosestBeat.value(), pGrid->findNextBeat(position).value(), kMaxBeatError);
    EXPECT_NEAR(kClosestBeat.value(), pGrid->findPrevBeat(position).value(), kMaxBeatError);
}

TEST(BeatGridTest, TestNthBeatWhenOnBeat_AfterEpsilon) {
    constexpr int sampleRate = 44100;
    TrackPointer pTrack = newTrack(sampleRate);

    constexpr double bpm = 60.1;
    pTrack->trySetBpm(bpm);
    constexpr double beatLengthFrames = 60.0 * sampleRate / bpm;

    auto pGrid = BeatGrid::makeBeatGrid(pTrack->getSampleRate(),
            QString(),
            mixxx::Bpm(bpm),
            mixxx::audio::kStartFramePos);

    // Pretend we're just before the 20th beat.
    constexpr mixxx::audio::FramePos kClosestBeat(20 * beatLengthFrames);
    const mixxx::audio::FramePos position(kClosestBeat + beatLengthFrames * 0.005);

    // The spec dictates that a value of 0 is always invalid and returns an invalid position
    EXPECT_FALSE(pGrid->findNthBeat(position, 0).isValid());

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < 20; ++i) {
        EXPECT_NEAR((kClosestBeat + beatLengthFrames * (i - 1)).value(),
                pGrid->findNthBeat(position, i).value(),
                kMaxBeatError);
        EXPECT_NEAR((kClosestBeat + beatLengthFrames * (-i + 1)).value(),
                pGrid->findNthBeat(position, -i).value(),
                kMaxBeatError);
    }

    // Also test prev/next beat calculation.
    mixxx::audio::FramePos prevBeat, nextBeat;
    pGrid->findPrevNextBeats(position, &prevBeat, &nextBeat, true);
    EXPECT_NEAR(kClosestBeat.value(), prevBeat.value(), kMaxBeatError);
    EXPECT_NEAR((kClosestBeat + beatLengthFrames).value(), nextBeat.value(), kMaxBeatError);

    // Also test prev/next beat calculation without snapping tolerance
    pGrid->findPrevNextBeats(position, &prevBeat, &nextBeat, false);
    EXPECT_NEAR(kClosestBeat.value(), prevBeat.value(), kMaxBeatError);
    EXPECT_NEAR((kClosestBeat + beatLengthFrames).value(), nextBeat.value(), kMaxBeatError);

    // Both previous and next beat should return the closest beat.
    EXPECT_NEAR(kClosestBeat.value(), pGrid->findNextBeat(position).value(), kMaxBeatError);
    EXPECT_NEAR(kClosestBeat.value(), pGrid->findPrevBeat(position).value(), kMaxBeatError);
}

TEST(BeatGridTest, TestNthBeatWhenNotOnBeat) {
    constexpr int sampleRate = 44100;
    TrackPointer pTrack = newTrack(sampleRate);

    constexpr mixxx::Bpm bpm(60.1);
    pTrack->trySetBpm(bpm.value());
    const mixxx::audio::FrameDiff_t beatLengthFrames = 60.0 * sampleRate / bpm.value();

    auto pGrid = BeatGrid::makeBeatGrid(pTrack->getSampleRate(),
            QString(),
            bpm,
            mixxx::audio::kStartFramePos);

    // Pretend we're half way between the 20th and 21st beat
    const mixxx::audio::FramePos previousBeat(beatLengthFrames * 20.0);
    const mixxx::audio::FramePos nextBeat(beatLengthFrames * 21.0);
    const mixxx::audio::FramePos position = previousBeat + (nextBeat - previousBeat) / 2.0;

    // The spec dictates that a value of 0 is always invalid and returns an invalid position
    EXPECT_FALSE(pGrid->findNthBeat(position, 0).isValid());

    // findNthBeat should return multiples of beats starting from the next or
    // previous beat, depending on whether N is positive or negative.
    for (int i = 1; i < 20; ++i) {
        EXPECT_NEAR((nextBeat + beatLengthFrames * (i - 1)).value(),
                pGrid->findNthBeat(position, i).value(),
                kMaxBeatError);
        EXPECT_NEAR((previousBeat + beatLengthFrames * (-i + 1)).value(),
                pGrid->findNthBeat(position, -i).value(),
                kMaxBeatError);
    }

    // Also test prev/next beat calculation
    mixxx::audio::FramePos foundPrevBeat, foundNextBeat;
    pGrid->findPrevNextBeats(position, &foundPrevBeat, &foundNextBeat, true);
    EXPECT_NEAR(previousBeat.value(), foundPrevBeat.value(), kMaxBeatError);
    EXPECT_NEAR(nextBeat.value(), foundNextBeat.value(), kMaxBeatError);

    // Also test prev/next beat calculation without snapping tolerance
    pGrid->findPrevNextBeats(position, &foundPrevBeat, &foundNextBeat, false);
    EXPECT_NEAR(previousBeat.value(), foundPrevBeat.value(), kMaxBeatError);
    EXPECT_NEAR(nextBeat.value(), foundNextBeat.value(), kMaxBeatError);
}

TEST(BeatGridTest, FromMetadata) {
    int sampleRate = 44100;
    TrackPointer pTrack = newTrack(sampleRate);

    constexpr mixxx::Bpm bpm(60.1);
    ASSERT_TRUE(pTrack->trySetBpm(bpm.value()));
    EXPECT_DOUBLE_EQ(pTrack->getBpm(), bpm.value());

    auto pBeats = pTrack->getBeats();
    EXPECT_DOUBLE_EQ(pBeats->getBpm().value(), bpm.value());

    // Invalid bpm resets the bpm
    ASSERT_TRUE(pTrack->trySetBpm(-60.1));
    EXPECT_DOUBLE_EQ(pTrack->getBpm(), mixxx::Bpm::kValueUndefined);

    pBeats = pTrack->getBeats();
    EXPECT_EQ(pBeats.isNull(), true);
}

}  // namespace
