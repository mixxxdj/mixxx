#include <gtest/gtest.h>

#include <QtDebug>
#include <memory>

#include "track/beats.h"
#include "track/track.h"

using namespace mixxx;

namespace {

class BeatMapTest : public testing::Test {
  protected:
    BeatMapTest()
            : m_pTrack(Track::newTemporary()),
              m_sampleRate(mixxx::audio::SampleRate(10000)),
              m_iFrameSize(2) {
        m_pTrack->setAudioProperties(
                mixxx::audio::ChannelCount(2),
                m_sampleRate,
                mixxx::audio::Bitrate(),
                mixxx::Duration::fromSeconds(180));
    }

    mixxx::audio::FrameDiff_t getBeatLengthFrames(mixxx::Bpm bpm) {
        return (60.0 * m_sampleRate.value() / bpm.value());
    }

    QVector<mixxx::audio::FramePos> createBeatVector(mixxx::audio::FramePos first_beat,
            unsigned int num_beats,
            mixxx::audio::FrameDiff_t beat_length) {
        QVector<mixxx::audio::FramePos> beats;
        for (unsigned int i = 0; i < num_beats; ++i) {
            beats.append(first_beat + i * beat_length);
        }
        return beats;
    }

    TrackPointer m_pTrack;
    mixxx::audio::SampleRate m_sampleRate;
    int m_iFrameSize;
};

TEST_F(BeatMapTest, Scale) {
    constexpr mixxx::Bpm bpm(60.0);
    m_pTrack->trySetBpm(bpm.value());
    mixxx::audio::FrameDiff_t beatLengthFrames = getBeatLengthFrames(bpm);
    const auto startOffsetFrames = mixxx::audio::FramePos(7);
    constexpr int numBeats = 120;
    // Note beats must be in frames, not samples.
    QVector<mixxx::audio::FramePos> beats =
            createBeatVector(startOffsetFrames, numBeats + 1, beatLengthFrames);
    auto pMap = Beats::fromBeatPositions(m_pTrack->getSampleRate(), beats);
    const auto trackEndPosition = audio::FramePos{60.0 * pMap->getSampleRate()};

    EXPECT_DOUBLE_EQ(bpm.value(),
            pMap->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value());
    pMap = *pMap->tryScale(Beats::BpmScale::Double);
    EXPECT_DOUBLE_EQ(2 * bpm.value(),
            pMap->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value());

    pMap = *pMap->tryScale(Beats::BpmScale::Halve);
    EXPECT_DOUBLE_EQ(bpm.value(),
            pMap->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value());

    pMap = *pMap->tryScale(Beats::BpmScale::TwoThirds);
    EXPECT_DOUBLE_EQ(bpm.value() * 2 / 3,
            pMap->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value());

    pMap = *pMap->tryScale(Beats::BpmScale::ThreeHalves);
    EXPECT_DOUBLE_EQ(bpm.value(),
            pMap->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value());

    pMap = *pMap->tryScale(Beats::BpmScale::ThreeFourths);
    EXPECT_DOUBLE_EQ(bpm.value() * 3 / 4,
            pMap->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value());

    pMap = *pMap->tryScale(Beats::BpmScale::FourThirds);
    EXPECT_DOUBLE_EQ(bpm.value(),
            pMap->getBpmInRange(audio::kStartFramePos, trackEndPosition)
                    .value());
}

TEST_F(BeatMapTest, ScaleVariableTempo) {
    // Build a beat map with two tempo sections of 3 and 5 beats each. The
    // odd beat counts cannot be scaled to an integer number of beats, so
    // scaling has to keep the beat counts and scale the marker positions
    // relative to the first beat instead.
    const auto anchor = mixxx::audio::FramePos(7);
    const mixxx::audio::FrameDiff_t beatLengthA = 5000;
    const mixxx::audio::FrameDiff_t beatLengthB = 3000;
    QVector<mixxx::audio::FramePos> beats;
    for (int i = 0; i <= 3; ++i) {
        beats.append(anchor + i * beatLengthA);
    }
    for (int i = 1; i <= 5; ++i) {
        beats.append(anchor + 3 * beatLengthA + i * beatLengthB);
    }
    const auto pMap = Beats::fromBeatPositions(m_pTrack->getSampleRate(), beats);
    ASSERT_TRUE(pMap);
    ASSERT_FALSE(pMap->hasConstantTempo());
    ASSERT_EQ(2u, pMap->getMarkers().size());
    ASSERT_EQ(3, pMap->getMarkers()[0].beatsTillNextMarker());
    ASSERT_EQ(5, pMap->getMarkers()[1].beatsTillNextMarker());

    const auto& originalMarkers = pMap->getMarkers();
    const auto originalLastMarkerPosition = pMap->getLastMarkerPosition();
    const auto originalLastMarkerBpm = pMap->getLastMarkerBpm();
    const auto sampleRate = pMap->getSampleRate().value();

    const struct {
        Beats::BpmScale bpmScale;
        double factor;
    } scales[] = {
            {Beats::BpmScale::Halve, 0.5},
            {Beats::BpmScale::TwoThirds, 2.0 / 3.0},
            {Beats::BpmScale::ThreeHalves, 3.0 / 2.0},
            {Beats::BpmScale::Double, 2.0},
    };

    for (const auto& scale : scales) {
        const auto scaled = pMap->tryScale(scale.bpmScale);
        ASSERT_TRUE(scaled.has_value());
        const auto pScaledMap = scaled.value();
        ASSERT_FALSE(pScaledMap->hasConstantTempo());

        const auto& newMarkers = pScaledMap->getMarkers();
        ASSERT_EQ(originalMarkers.size(), newMarkers.size());

        // The first beat keeps its position and all beat counts are kept.
        EXPECT_EQ(anchor, newMarkers.front().position());
        auto originalIt = originalMarkers.cbegin();
        auto newIt = newMarkers.cbegin();
        for (; originalIt != originalMarkers.cend(); ++originalIt, ++newIt) {
            EXPECT_EQ(originalIt->beatsTillNextMarker(), newIt->beatsTillNextMarker());
            // All other marker positions are scaled relative to the first
            // beat and rounded down to the frame boundary.
            const auto expectedPosition =
                    (anchor + (originalIt->position() - anchor) / scale.factor)
                            .toLowerFrameBoundary();
            EXPECT_EQ(expectedPosition, newIt->position());
        }

        const auto expectedLastMarkerPosition =
                (anchor + (originalLastMarkerPosition - anchor) / scale.factor)
                        .toLowerFrameBoundary();
        EXPECT_EQ(expectedLastMarkerPosition, pScaledMap->getLastMarkerPosition());
        EXPECT_DOUBLE_EQ(originalLastMarkerBpm.value() * scale.factor,
                pScaledMap->getLastMarkerBpm().value());

        // The effective BPM of both sections is scaled by the same factor,
        // up to the error introduced by rounding to frame boundaries.
        const auto oldLengthA =
                originalMarkers[1].position() - originalMarkers[0].position();
        const auto newLengthA = newMarkers[1].position() - newMarkers[0].position();
        const auto oldLengthB = originalLastMarkerPosition - originalMarkers[1].position();
        const auto newLengthB =
                pScaledMap->getLastMarkerPosition() - newMarkers[1].position();
        const double oldBpmA = 60.0 * sampleRate *
                originalMarkers[0].beatsTillNextMarker() / oldLengthA;
        const double newBpmA = 60.0 * sampleRate *
                newMarkers[0].beatsTillNextMarker() / newLengthA;
        const double oldBpmB = 60.0 * sampleRate *
                originalMarkers[1].beatsTillNextMarker() / oldLengthB;
        const double newBpmB = 60.0 * sampleRate *
                newMarkers[1].beatsTillNextMarker() / newLengthB;
        EXPECT_NEAR(newBpmA, oldBpmA * scale.factor, 0.05);
        EXPECT_NEAR(newBpmB, oldBpmB * scale.factor, 0.05);
    }
}

TEST_F(BeatMapTest, ScaleMarkerCollision) {
    // Two markers that are only one frame apart would collapse onto the
    // same frame position when scaled by 2. This cannot be represented, so
    // the scale process must fail gracefully.
    const auto pBeats = Beats::fromBeatMarkers(m_sampleRate,
            {BeatMarker(mixxx::audio::FramePos(7), 1),
                    BeatMarker(mixxx::audio::FramePos(8), 1)},
            mixxx::audio::FramePos(9),
            mixxx::Bpm(200));
    ASSERT_TRUE(pBeats);
    EXPECT_TRUE(pBeats->tryScale(Beats::BpmScale::Halve).has_value());
    EXPECT_FALSE(pBeats->tryScale(Beats::BpmScale::Double).has_value());
}

TEST_F(BeatMapTest, TestNthBeat) {
    constexpr mixxx::Bpm bpm(60.0);
    m_pTrack->trySetBpm(bpm.value());
    mixxx::audio::FrameDiff_t beatLengthFrames = getBeatLengthFrames(bpm);
    const auto startOffsetFrames = mixxx::audio::FramePos(7);
    constexpr int numBeats = 100;
    // Note beats must be in frames, not samples.
    QVector<mixxx::audio::FramePos> beats =
            createBeatVector(startOffsetFrames, numBeats, beatLengthFrames);
    auto pMap = Beats::fromBeatPositions(m_pTrack->getSampleRate(), beats);

    // Check edge cases
    const mixxx::audio::FramePos firstBeat = startOffsetFrames + beatLengthFrames * 0;
    const mixxx::audio::FramePos lastBeat = startOffsetFrames + beatLengthFrames * (numBeats - 1);
    EXPECT_EQ(lastBeat, pMap->findNthBeat(lastBeat, -1));
    EXPECT_EQ(lastBeat, pMap->findNthBeat(lastBeat + (beatLengthFrames / 2), -1));
    EXPECT_EQ(lastBeat - beatLengthFrames, pMap->findNthBeat(lastBeat, -2));
    EXPECT_EQ(lastBeat - beatLengthFrames,
            pMap->findNthBeat(lastBeat + (beatLengthFrames / 2), -2));
    EXPECT_EQ(lastBeat - 2 * beatLengthFrames, pMap->findNthBeat(lastBeat, -3));
    EXPECT_EQ(lastBeat, pMap->findPrevBeat(lastBeat));
    EXPECT_EQ(lastBeat, pMap->findNthBeat(lastBeat, 1));
    EXPECT_EQ(lastBeat, pMap->findNextBeat(lastBeat));
    EXPECT_TRUE(pMap->findNthBeat(lastBeat, 2).isValid());
    EXPECT_TRUE(pMap->findNthBeat(lastBeat + beatLengthFrames, 2).isValid());

    EXPECT_EQ(firstBeat, pMap->findNthBeat(firstBeat, 1));
    EXPECT_EQ(firstBeat, pMap->findNthBeat(firstBeat - (beatLengthFrames / 2), 1));
    EXPECT_EQ(firstBeat + beatLengthFrames, pMap->findNthBeat(firstBeat, 2));
    EXPECT_EQ(firstBeat + beatLengthFrames,
            pMap->findNthBeat(firstBeat - (beatLengthFrames / 2), 2));
    EXPECT_EQ(firstBeat + 2 * beatLengthFrames, pMap->findNthBeat(firstBeat, 3));
    EXPECT_EQ(firstBeat, pMap->findNextBeat(firstBeat));
    EXPECT_EQ(firstBeat, pMap->findNthBeat(firstBeat, -1));
    EXPECT_EQ(firstBeat, pMap->findPrevBeat(firstBeat));
    EXPECT_TRUE(pMap->findNthBeat(firstBeat, -2).isValid());
    EXPECT_TRUE(pMap->findNthBeat(firstBeat - beatLengthFrames, -1).isValid());

    mixxx::audio::FramePos prevBeat, nextBeat;
    pMap->findPrevNextBeats(lastBeat, &prevBeat, &nextBeat, true);
    EXPECT_EQ(lastBeat, prevBeat);
    EXPECT_TRUE(nextBeat.isValid());

    pMap->findPrevNextBeats(firstBeat, &prevBeat, &nextBeat, true);
    EXPECT_EQ(firstBeat, prevBeat);
    EXPECT_EQ(firstBeat + beatLengthFrames, nextBeat);
}

TEST_F(BeatMapTest, TestNthBeatWhenOnBeat) {
    constexpr mixxx::Bpm bpm(60.0);
    m_pTrack->trySetBpm(bpm.value());
    mixxx::audio::FrameDiff_t beatLengthFrames = getBeatLengthFrames(bpm);
    const auto startOffsetFrames = mixxx::audio::FramePos(7);
    constexpr int numBeats = 100;
    // Note beats must be in frames, not samples.
    QVector<mixxx::audio::FramePos> beats =
            createBeatVector(startOffsetFrames, numBeats, beatLengthFrames);
    auto pMap = Beats::fromBeatPositions(m_pTrack->getSampleRate(), beats);

    // Pretend we're on the 20th beat;
    constexpr int curBeat = 20;
    const mixxx::audio::FramePos position = startOffsetFrames + beatLengthFrames * curBeat;

    // The spec dictates that a value of 0 is always invalid and returns an invalid position
    EXPECT_FALSE(pMap->findNthBeat(position, 0).isValid());

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < curBeat; ++i) {
        EXPECT_DOUBLE_EQ((position + beatLengthFrames * (i - 1)).value(),
                pMap->findNthBeat(position, i).value());
        EXPECT_DOUBLE_EQ((position + beatLengthFrames * (-i + 1)).value(),
                pMap->findNthBeat(position, -i).value());
    }

    // Also test prev/next beat calculation.
    mixxx::audio::FramePos prevBeat, nextBeat;
    pMap->findPrevNextBeats(position, &prevBeat, &nextBeat, true);
    EXPECT_EQ(position, prevBeat);
    EXPECT_EQ(position + beatLengthFrames, nextBeat);

    // Also test prev/next beat calculation without snapping tolerance
    pMap->findPrevNextBeats(position, &prevBeat, &nextBeat, false);
    EXPECT_EQ(position, prevBeat);
    EXPECT_EQ(position + beatLengthFrames, nextBeat);

    // Both previous and next beat should return the current position.
    EXPECT_EQ(position, pMap->findNextBeat(position));
    EXPECT_EQ(position, pMap->findPrevBeat(position));
}

TEST_F(BeatMapTest, TestNthBeatWhenNotOnBeat) {
    constexpr mixxx::Bpm bpm(60.0);
    m_pTrack->trySetBpm(bpm.value());
    mixxx::audio::FrameDiff_t beatLengthFrames = getBeatLengthFrames(bpm);
    const auto startOffsetFrames = mixxx::audio::FramePos(7);
    constexpr int numBeats = 100;
    // Note beats must be in frames, not samples.
    QVector<mixxx::audio::FramePos> beats =
            createBeatVector(startOffsetFrames, numBeats, beatLengthFrames);
    auto pMap = Beats::fromBeatPositions(m_pTrack->getSampleRate(), beats);

    // Pretend we're half way between the 20th and 21st beat
    const mixxx::audio::FramePos previousBeat = startOffsetFrames + beatLengthFrames * 20.0;
    const mixxx::audio::FramePos nextBeat = startOffsetFrames + beatLengthFrames * 21.0;
    const mixxx::audio::FramePos position = previousBeat + (nextBeat - previousBeat) / 2.0;

    // The spec dictates that a value of 0 is always invalid and returns -1
    EXPECT_FALSE(pMap->findNthBeat(position, 0).isValid());

    // findNthBeat should return multiples of beats starting from the next or
    // previous beat, depending on whether N is positive or negative.
    for (int i = 1; i < 20; ++i) {
        EXPECT_DOUBLE_EQ((nextBeat + beatLengthFrames * (i - 1)).value(),
                pMap->findNthBeat(position, i).value());
        EXPECT_DOUBLE_EQ((previousBeat - beatLengthFrames * (i - 1)).value(),
                pMap->findNthBeat(position, -i).value());
    }

    // Also test prev/next beat calculation
    mixxx::audio::FramePos foundPrevBeat, foundNextBeat;
    pMap->findPrevNextBeats(position, &foundPrevBeat, &foundNextBeat, true);
    EXPECT_EQ(previousBeat, foundPrevBeat);
    EXPECT_EQ(nextBeat, foundNextBeat);

    // Also test prev/next beat calculation without snapping tolerance
    pMap->findPrevNextBeats(position, &foundPrevBeat, &foundNextBeat, false);
    EXPECT_EQ(previousBeat, foundPrevBeat);
    EXPECT_EQ(nextBeat, foundNextBeat);
}

TEST_F(BeatMapTest, HasBeatInRangeWithFractionalPos) {
    constexpr mixxx::Bpm bpm(60.0);
    constexpr int numBeats = 120;
    const mixxx::audio::FrameDiff_t beatLengthFrames = getBeatLengthFrames(bpm);
    ASSERT_EQ(beatLengthFrames, std::round(beatLengthFrames));

    mixxx::audio::FramePos beatPos = mixxx::audio::kStartFramePos;
    const mixxx::audio::FramePos lastBeatPos = beatPos + beatLengthFrames * (numBeats - 1);
    QVector<mixxx::audio::FramePos> beats;
    for (; beatPos <= lastBeatPos; beatPos += beatLengthFrames) {
        beats.append(beatPos);
    }
    const auto pMap = Beats::fromBeatPositions(m_pTrack->getSampleRate(), beats);

    const mixxx::audio::FrameDiff_t halfBeatLengthFrames = beatLengthFrames / 2;
    EXPECT_TRUE(pMap->hasBeatInRange(mixxx::audio::kStartFramePos,
            mixxx::audio::kStartFramePos + halfBeatLengthFrames));
    EXPECT_TRUE(pMap->hasBeatInRange(mixxx::audio::kStartFramePos - 0.2,
            mixxx::audio::kStartFramePos + halfBeatLengthFrames));
    // FIXME: The next comparison is broken due to fuzzy matching in BeatMap::findNthBeat()
    //EXPECT_FALSE(pMap->hasBeatInRange(mixxx::audio::kStartFramePos + 0.2, mixxx::audio::kStartFramePos + halfBeatLengthFrames));
    EXPECT_TRUE(pMap->hasBeatInRange(
            mixxx::audio::kStartFramePos - halfBeatLengthFrames,
            mixxx::audio::kStartFramePos));
    EXPECT_FALSE(pMap->hasBeatInRange(
            mixxx::audio::kStartFramePos - halfBeatLengthFrames,
            mixxx::audio::kStartFramePos - 0.2));
    EXPECT_TRUE(pMap->hasBeatInRange(
            mixxx::audio::kStartFramePos - halfBeatLengthFrames,
            mixxx::audio::kStartFramePos + 0.2));
}

}  // namespace
