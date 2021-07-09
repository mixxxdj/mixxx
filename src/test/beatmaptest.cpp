#include <gtest/gtest.h>
#include <memory.h>

#include <QtDebug>

#include "track/beatmap.h"
#include "track/track.h"

using namespace mixxx;

namespace {

class BeatMapTest : public testing::Test {
  protected:
    BeatMapTest()
            : m_pTrack(Track::newTemporary()),
              m_iSampleRate(10000),
              m_iFrameSize(2) {
        m_pTrack->setAudioProperties(
                mixxx::audio::ChannelCount(2),
                mixxx::audio::SampleRate(m_iSampleRate),
                mixxx::audio::Bitrate(),
                mixxx::Duration::fromSeconds(180));
    }

    mixxx::audio::FrameDiff_t getBeatLengthFrames(mixxx::Bpm bpm) {
        return (60.0 * m_iSampleRate / bpm.value());
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
    int m_iSampleRate;
    int m_iFrameSize;
};

TEST_F(BeatMapTest, Scale) {
    constexpr mixxx::Bpm bpm(60.0);
    m_pTrack->trySetBpm(bpm.value());
    mixxx::audio::FrameDiff_t beatLengthFrames = getBeatLengthFrames(bpm);
    const auto startOffsetFrames = mixxx::audio::FramePos(7);
    const int numBeats = 100;
    // Note beats must be in frames, not samples.
    QVector<mixxx::audio::FramePos> beats =
            createBeatVector(startOffsetFrames, numBeats, beatLengthFrames);
    auto pMap = BeatMap::makeBeatMap(m_pTrack->getSampleRate(), QString(), beats);

    EXPECT_DOUBLE_EQ(bpm.value(), pMap->getBpm().value());
    pMap = pMap->scale(Beats::BpmScale::Double);
    EXPECT_DOUBLE_EQ(2 * bpm.value(), pMap->getBpm().value());

    pMap = pMap->scale(Beats::BpmScale::Halve);
    EXPECT_DOUBLE_EQ(bpm.value(), pMap->getBpm().value());

    pMap = pMap->scale(Beats::BpmScale::TwoThirds);
    EXPECT_DOUBLE_EQ(bpm.value() * 2 / 3, pMap->getBpm().value());

    pMap = pMap->scale(Beats::BpmScale::ThreeHalves);
    EXPECT_DOUBLE_EQ(bpm.value(), pMap->getBpm().value());

    pMap = pMap->scale(Beats::BpmScale::ThreeFourths);
    EXPECT_DOUBLE_EQ(bpm.value() * 3 / 4, pMap->getBpm().value());

    pMap = pMap->scale(Beats::BpmScale::FourThirds);
    EXPECT_DOUBLE_EQ(bpm.value(), pMap->getBpm().value());
}

TEST_F(BeatMapTest, TestNthBeat) {
    constexpr mixxx::Bpm bpm(60.0);
    m_pTrack->trySetBpm(bpm.value());
    mixxx::audio::FrameDiff_t beatLengthFrames = getBeatLengthFrames(bpm);
    const auto startOffsetFrames = mixxx::audio::FramePos(7);
    const int numBeats = 100;
    // Note beats must be in frames, not samples.
    QVector<mixxx::audio::FramePos> beats =
            createBeatVector(startOffsetFrames, numBeats, beatLengthFrames);
    auto pMap = BeatMap::makeBeatMap(m_pTrack->getSampleRate(), QString(), beats);

    // Check edge cases
    const mixxx::audio::FramePos firstBeat = startOffsetFrames + beatLengthFrames * 0;
    const mixxx::audio::FramePos lastBeat = startOffsetFrames + beatLengthFrames * (numBeats - 1);
    EXPECT_EQ(lastBeat, pMap->findNthBeat(lastBeat, 1));
    EXPECT_EQ(lastBeat, pMap->findNextBeat(lastBeat));
    EXPECT_FALSE(pMap->findNthBeat(lastBeat, 2).isValid());
    EXPECT_EQ(firstBeat, pMap->findNthBeat(firstBeat, -1));
    EXPECT_EQ(firstBeat, pMap->findPrevBeat(firstBeat));
    EXPECT_FALSE(pMap->findNthBeat(firstBeat, -2).isValid());

    mixxx::audio::FramePos prevBeat, nextBeat;
    pMap->findPrevNextBeats(lastBeat, &prevBeat, &nextBeat, true);
    EXPECT_EQ(lastBeat, prevBeat);
    EXPECT_FALSE(nextBeat.isValid());

    pMap->findPrevNextBeats(firstBeat, &prevBeat, &nextBeat, true);
    EXPECT_EQ(firstBeat, prevBeat);
    EXPECT_EQ(firstBeat + beatLengthFrames, nextBeat);
}

TEST_F(BeatMapTest, TestNthBeatWhenOnBeat) {
    constexpr mixxx::Bpm bpm(60.0);
    m_pTrack->trySetBpm(bpm.value());
    mixxx::audio::FrameDiff_t beatLengthFrames = getBeatLengthFrames(bpm);
    const auto startOffsetFrames = mixxx::audio::FramePos(7);
    const int numBeats = 100;
    // Note beats must be in frames, not samples.
    QVector<mixxx::audio::FramePos> beats =
            createBeatVector(startOffsetFrames, numBeats, beatLengthFrames);
    auto pMap = BeatMap::makeBeatMap(m_pTrack->getSampleRate(), QString(), beats);

    // Pretend we're on the 20th beat;
    const int curBeat = 20;
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

TEST_F(BeatMapTest, TestNthBeatWhenOnBeat_BeforeEpsilon) {
    constexpr mixxx::Bpm bpm(60.0);
    m_pTrack->trySetBpm(bpm.value());
    mixxx::audio::FrameDiff_t beatLengthFrames = getBeatLengthFrames(bpm);
    const auto startOffsetFrames = mixxx::audio::FramePos(7);
    const int numBeats = 100;
    // Note beats must be in frames, not samples.
    QVector<mixxx::audio::FramePos> beats =
            createBeatVector(startOffsetFrames, numBeats, beatLengthFrames);
    auto pMap = BeatMap::makeBeatMap(m_pTrack->getSampleRate(), QString(), beats);

    // Pretend we're just before the 20th beat;
    const int curBeat = 20;
    const mixxx::audio::FramePos kClosestBeat = startOffsetFrames + curBeat * beatLengthFrames;
    const mixxx::audio::FramePos position = kClosestBeat - beatLengthFrames * 0.005;

    // The spec dictates that a value of 0 is always invalid and returns -1
    EXPECT_FALSE(pMap->findNthBeat(position, 0).isValid());

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < curBeat; ++i) {
        EXPECT_DOUBLE_EQ((kClosestBeat + beatLengthFrames * (i - 1)).value(),
                pMap->findNthBeat(position, i).value());
        EXPECT_DOUBLE_EQ((kClosestBeat + beatLengthFrames * (-i + 1)).value(),
                pMap->findNthBeat(position, -i).value());
    }

    // Also test prev/next beat calculation
    mixxx::audio::FramePos prevBeat, nextBeat;
    pMap->findPrevNextBeats(position, &prevBeat, &nextBeat, true);
    EXPECT_EQ(kClosestBeat, prevBeat);
    EXPECT_EQ(kClosestBeat + beatLengthFrames, nextBeat);

    // Also test prev/next beat calculation without snapping tolerance
    pMap->findPrevNextBeats(position, &prevBeat, &nextBeat, false);
    EXPECT_EQ(kClosestBeat - beatLengthFrames, prevBeat);
    EXPECT_EQ(kClosestBeat, nextBeat);

    // Both previous and next beat should return the closest beat.
    EXPECT_EQ(kClosestBeat, pMap->findNextBeat(position));
    EXPECT_EQ(kClosestBeat, pMap->findPrevBeat(position));

}

TEST_F(BeatMapTest, TestNthBeatWhenOnBeat_AfterEpsilon) {
    constexpr mixxx::Bpm bpm(60.0);
    m_pTrack->trySetBpm(bpm.value());
    mixxx::audio::FrameDiff_t beatLengthFrames = getBeatLengthFrames(bpm);
    const auto startOffsetFrames = mixxx::audio::FramePos(7);
    const int numBeats = 100;
    // Note beats must be in frames, not samples.
    QVector<mixxx::audio::FramePos> beats =
            createBeatVector(startOffsetFrames, numBeats, beatLengthFrames);
    auto pMap = BeatMap::makeBeatMap(m_pTrack->getSampleRate(), QString(), beats);

    // Pretend we're just after the 20th beat;
    const int curBeat = 20;
    const mixxx::audio::FramePos kClosestBeat = startOffsetFrames + curBeat * beatLengthFrames;
    const mixxx::audio::FramePos position = kClosestBeat + beatLengthFrames * 0.005;

    // The spec dictates that a value of 0 is always invalid and returns -1
    EXPECT_FALSE(pMap->findNthBeat(position, 0).isValid());

    EXPECT_EQ(kClosestBeat, pMap->findClosestBeat(position));

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < curBeat; ++i) {
        EXPECT_DOUBLE_EQ((kClosestBeat + beatLengthFrames * (i - 1)).value(),
                pMap->findNthBeat(position, i).value());
        EXPECT_DOUBLE_EQ((kClosestBeat + beatLengthFrames * (-i + 1)).value(),
                pMap->findNthBeat(position, -i).value());
    }

    // Also test prev/next beat calculation.
    mixxx::audio::FramePos prevBeat, nextBeat;
    pMap->findPrevNextBeats(position, &prevBeat, &nextBeat, true);
    EXPECT_EQ(kClosestBeat, prevBeat);
    EXPECT_EQ(kClosestBeat + beatLengthFrames, nextBeat);

    // Also test prev/next beat calculation without snapping tolerance
    pMap->findPrevNextBeats(position, &prevBeat, &nextBeat, false);
    EXPECT_EQ(kClosestBeat, prevBeat);
    EXPECT_EQ(kClosestBeat + beatLengthFrames, nextBeat);

    // Both previous and next beat should return the closest beat.
    EXPECT_EQ(kClosestBeat, pMap->findNextBeat(position));
    EXPECT_EQ(kClosestBeat, pMap->findPrevBeat(position));
}

TEST_F(BeatMapTest, TestNthBeatWhenNotOnBeat) {
    constexpr mixxx::Bpm bpm(60.0);
    m_pTrack->trySetBpm(bpm.value());
    mixxx::audio::FrameDiff_t beatLengthFrames = getBeatLengthFrames(bpm);
    const auto startOffsetFrames = mixxx::audio::FramePos(7);
    const int numBeats = 100;
    // Note beats must be in frames, not samples.
    QVector<mixxx::audio::FramePos> beats =
            createBeatVector(startOffsetFrames, numBeats, beatLengthFrames);
    auto pMap = BeatMap::makeBeatMap(m_pTrack->getSampleRate(), QString(), beats);

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

TEST_F(BeatMapTest, TestBpmAround) {
    constexpr mixxx::Bpm filebpm(60.0);
    double approx_beat_length = getBeatLengthFrames(filebpm);
    m_pTrack->trySetBpm(filebpm.value());
    const int numBeats = 64;

    QVector<mixxx::audio::FramePos> beats;
    mixxx::audio::FramePos beat_pos = mixxx::audio::kStartFramePos;
    for (unsigned int i = 0, bpmValue = 60; i < numBeats; ++i, ++bpmValue) {
        const mixxx::audio::FrameDiff_t beat_length = getBeatLengthFrames(mixxx::Bpm(bpmValue));
        beats.append(beat_pos);
        beat_pos += beat_length;
    }

    auto pMap = BeatMap::makeBeatMap(m_pTrack->getSampleRate(), QString(), beats);

    // The average of the first 8 beats should be different than the average
    // of the last 8 beats.
    EXPECT_DOUBLE_EQ(63.937645572318047,
            pMap->getBpmAroundPosition(
                        mixxx::audio::kStartFramePos + 4 * approx_beat_length,
                        4)
                    .value());
    EXPECT_DOUBLE_EQ(118.96668932698844,
            pMap->getBpmAroundPosition(
                        mixxx::audio::kStartFramePos + 60 * approx_beat_length,
                        4)
                    .value());
    // Also test at the beginning and end of the track
    EXPECT_DOUBLE_EQ(62.937377309576974,
            pMap->getBpmAroundPosition(mixxx::audio::kStartFramePos, 4).value());
    EXPECT_DOUBLE_EQ(118.96668932698844,
            pMap->getBpmAroundPosition(
                        mixxx::audio::kStartFramePos + 65 * approx_beat_length,
                        4)
                    .value());

    // Try a really, really short track
    constexpr auto startFramePos = mixxx::audio::FramePos(10);
    beats = createBeatVector(startFramePos, 3, approx_beat_length);
    pMap = BeatMap::makeBeatMap(m_pTrack->getSampleRate(), QString(), beats);
    EXPECT_DOUBLE_EQ(filebpm.value(),
            pMap->getBpmAroundPosition(
                        mixxx::audio::kStartFramePos + 1 * approx_beat_length,
                        4)
                    .value());
}

}  // namespace
