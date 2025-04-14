#include <gtest/gtest.h>

#include <QtDebug>
#include <memory>

#include "audio/types.h"
#include "track/beats.h"
#include "track/track.h"

using namespace mixxx;

namespace {

constexpr auto kSampleRate = mixxx::audio::SampleRate(44100);
const double kMaxBeatError = 1e-9;

TrackPointer newTrack(mixxx::audio::SampleRate sampleRate) {
    TrackPointer pTrack(Track::newTemporary());
    pTrack->setAudioProperties(
            mixxx::audio::ChannelCount(2),
            sampleRate,
            mixxx::audio::Bitrate(),
            mixxx::Duration::fromSeconds(180));
    return pTrack;
}

TEST(BeatGridTest, TranslateBeats) {
    TrackPointer pTrack = newTrack(kSampleRate);
    constexpr mixxx::Bpm bpm(60.0);
    pTrack->trySetBpm(bpm.value());
    auto pBeat = Beats::fromConstTempo(pTrack->getSampleRate(),
            mixxx::audio::kStartFramePos,
            mixxx::Bpm(bpm));
    BeatsPointer pQuarterBeat = Beats::fromConstTempo(pTrack->getSampleRate(),
            mixxx::audio::kStartFramePos + 0.25 * kSampleRate,
            mixxx::Bpm(bpm));
    BeatsPointer pHalfBeat = Beats::fromConstTempo(pTrack->getSampleRate(),
            mixxx::audio::kStartFramePos + 0.5 * kSampleRate,
            mixxx::Bpm(bpm));
    BeatsPointer pOneBeat = Beats::fromConstTempo(pTrack->getSampleRate(),
            mixxx::audio::kStartFramePos + kSampleRate,
            mixxx::Bpm(bpm));

    // Test nothing happens for xBeats=0.0
    EXPECT_EQ(*pBeat, **pBeat->tryTranslateBeats(0.0));

    // Test quarter beat shift
    EXPECT_EQ(*pQuarterBeat, **pBeat->tryTranslateBeats(0.25));

    // Test half beat shift
    EXPECT_EQ(*pHalfBeat, **pBeat->tryTranslateBeats(0.5));
    EXPECT_EQ(*pHalfBeat, **(*pBeat->tryTranslateBeats(0.25))->tryTranslateBeats(0.25));

    // Test full beat shift
    EXPECT_EQ(*pOneBeat, **pBeat->tryTranslateBeats(1.0));
    EXPECT_EQ(*pOneBeat, **pQuarterBeat->tryTranslateBeats(0.75));
    EXPECT_EQ(*pOneBeat, **pHalfBeat->tryTranslateBeats(0.5));

    // Test negative shift
    EXPECT_EQ(*pBeat, **pHalfBeat->tryTranslateBeats(-0.5));
    EXPECT_EQ(*pBeat, **pOneBeat->tryTranslateBeats(-1.0));
}

TEST(BeatGridTest, Scale) {
    TrackPointer pTrack = newTrack(kSampleRate);

    constexpr mixxx::Bpm bpm(60.0);
    pTrack->trySetBpm(bpm.value());

    auto pGrid = Beats::fromConstTempo(pTrack->getSampleRate(),
            mixxx::audio::kStartFramePos,
            mixxx::Bpm(bpm));
    const auto trackEndPosition = audio::FramePos{pTrack->getDuration() * pGrid->getSampleRate()};

    EXPECT_DOUBLE_EQ(bpm.value(),
            pGrid->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value());
    pGrid = *pGrid->tryScale(Beats::BpmScale::Double);
    EXPECT_DOUBLE_EQ(2 * bpm.value(),
            pGrid->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value());

    pGrid = *pGrid->tryScale(Beats::BpmScale::Halve);
    EXPECT_DOUBLE_EQ(bpm.value(),
            pGrid->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value());

    pGrid = *pGrid->tryScale(Beats::BpmScale::TwoThirds);
    EXPECT_DOUBLE_EQ(bpm.value() * 2 / 3,
            pGrid->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value());

    pGrid = *pGrid->tryScale(Beats::BpmScale::ThreeHalves);
    EXPECT_DOUBLE_EQ(bpm.value(),
            pGrid->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value());

    pGrid = *pGrid->tryScale(Beats::BpmScale::ThreeFourths);
    EXPECT_DOUBLE_EQ(bpm.value() * 3 / 4,
            pGrid->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value());

    pGrid = *pGrid->tryScale(Beats::BpmScale::FourThirds);
    EXPECT_DOUBLE_EQ(bpm.value(),
            pGrid->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value());
}

TEST(BeatGridTest, TestNthBeatWhenOnBeat) {
    TrackPointer pTrack = newTrack(kSampleRate);

    constexpr double bpm = 60.1;
    pTrack->trySetBpm(bpm);
    constexpr mixxx::audio::FrameDiff_t beatLengthFrames = 60.0 * kSampleRate / bpm;

    auto pGrid = Beats::fromConstTempo(pTrack->getSampleRate(),
            mixxx::audio::kStartFramePos,
            mixxx::Bpm(bpm));
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

TEST(BeatGridTest, TestNthBeatWhenNotOnBeat) {
    TrackPointer pTrack = newTrack(kSampleRate);

    constexpr mixxx::Bpm bpm(60.1);
    pTrack->trySetBpm(bpm.value());
    const mixxx::audio::FrameDiff_t beatLengthFrames = 60.0 * kSampleRate / bpm.value();

    auto pGrid = Beats::fromConstTempo(pTrack->getSampleRate(),
            mixxx::audio::kStartFramePos,
            bpm);

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
    TrackPointer pTrack = newTrack(kSampleRate);

    constexpr mixxx::Bpm bpm(60.1);
    ASSERT_TRUE(pTrack->trySetBpm(bpm.value()));
    EXPECT_DOUBLE_EQ(pTrack->getBpm(), bpm.value());

    auto pBeats = pTrack->getBeats();
    const auto trackEndPosition = audio::FramePos{pTrack->getDuration() * pBeats->getSampleRate()};
    EXPECT_DOUBLE_EQ(
            pBeats->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value(),
            bpm.value());

    // Invalid bpm resets the bpm
    ASSERT_TRUE(pTrack->trySetBpm(-60.1));
    EXPECT_DOUBLE_EQ(pTrack->getBpm(), mixxx::Bpm::kValueUndefined);

    pBeats = pTrack->getBeats();
    EXPECT_EQ(nullptr, pBeats);
}

}  // namespace
