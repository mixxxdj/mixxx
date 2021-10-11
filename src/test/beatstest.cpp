#include <gtest/gtest.h>

#include <QtDebug>

#include "audio/types.h"
#include "track/beats.h"
#include "track/bpm.h"
#include "util/memory.h"

using namespace mixxx;

namespace {

constexpr auto kBpm = Bpm(120.0);
constexpr auto kSampleRate = audio::SampleRate(48000);
constexpr auto kStartPosition = audio::FramePos(400);
constexpr double kMaxBeatError = 1e-9;

const auto kConstTempoBeats = Beats(
        kStartPosition,
        kBpm,
        kSampleRate,
        QString());

// Create beats with 8 beats at 120 BPM, then 16 beats at 60 Bpm, followed by 120 BPM.
const auto kNonConstTempoBeats = Beats(
        std::vector<BeatMarker>{
                BeatMarker{kStartPosition, 8},
                BeatMarker{kStartPosition + 8 * kSampleRate.value() / 2, 16},
        },
        kStartPosition + 8 * kSampleRate.value() / 2 + 16 * kSampleRate.value(),
        kBpm,
        kSampleRate,
        QString());

TEST(BeatsTest, ConstTempoGetBpm) {
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kConstTempoBeats
                    .getBpmInRange(
                            kStartPosition, audio::FramePos{60 * kSampleRate})
                    .value());
}

TEST(BeatsTest, ConstTempoIteratorSubtract) {
    EXPECT_EQ(kConstTempoBeats.cbegin(), kConstTempoBeats.cend() - 1);
    EXPECT_EQ(kConstTempoBeats.cbegin() - 5, kConstTempoBeats.cend() - 6);
    EXPECT_EQ(kConstTempoBeats.cbegin() - 10, kConstTempoBeats.cend() - 11);
}

TEST(BeatsTest, ConstTempoIteratorAddSubtractNegativeIsEquivalent) {
    EXPECT_EQ(kConstTempoBeats.cbegin() + 1, kConstTempoBeats.cbegin() - (-1));
    EXPECT_EQ(kConstTempoBeats.cbegin() + 5, kConstTempoBeats.cbegin() - (-5));
    EXPECT_EQ(kConstTempoBeats.cbegin() - 1, kConstTempoBeats.cbegin() + (-1));
    EXPECT_EQ(kConstTempoBeats.cbegin() - 5, kConstTempoBeats.cbegin() + (-5));
    EXPECT_EQ(kConstTempoBeats.cend() + 1, kConstTempoBeats.cend() - (-1));
    EXPECT_EQ(kConstTempoBeats.cend() + 5, kConstTempoBeats.cend() - (-5));
    EXPECT_EQ(kConstTempoBeats.cend() - 1, kConstTempoBeats.cend() + (-1));
    EXPECT_EQ(kConstTempoBeats.cend() - 5, kConstTempoBeats.cend() + (-5));
}

TEST(BeatsTest, ConstTempoIteratorSubtractPosition) {
    const audio::FrameDiff_t beatLengthFrames = 60.0 * kSampleRate.value() / kBpm.value();
    EXPECT_NEAR((*(kConstTempoBeats.cbegin() - 100)).value(),
            ((*kConstTempoBeats.cbegin()) - 100 * beatLengthFrames).value(),
            kMaxBeatError);
    EXPECT_NEAR((*(kConstTempoBeats.cend() - 100)).value(),
            ((*kConstTempoBeats.cend()) - 100 * beatLengthFrames).value(),
            kMaxBeatError);
}

TEST(BeatsTest, ConstTempoIteratorAdd) {
    EXPECT_EQ(kConstTempoBeats.cend(), kConstTempoBeats.cbegin() + 1);
    EXPECT_EQ(kConstTempoBeats.cend() + 5, kConstTempoBeats.cbegin() + 6);
    EXPECT_EQ(kConstTempoBeats.cend() + 10, kConstTempoBeats.cbegin() + 11);
}

TEST(BeatsTest, ConstTempoIteratorAddPosition) {
    const audio::FrameDiff_t beatLengthFrames = 60.0 * kSampleRate.value() / kBpm.value();
    EXPECT_NEAR((*(kConstTempoBeats.cbegin() + 100)).value(),
            ((*kConstTempoBeats.cbegin()) + 100 * beatLengthFrames).value(),
            kMaxBeatError);
    EXPECT_NEAR((*(kConstTempoBeats.cend() + 100)).value(),
            ((*kConstTempoBeats.cend()) + 100 * beatLengthFrames).value(),
            kMaxBeatError);
}

TEST(BeatsTest, ConstTempoIteratorDifference) {
    EXPECT_EQ(-1, kConstTempoBeats.cbegin() - kConstTempoBeats.cend());
    EXPECT_EQ(1, kConstTempoBeats.cend() - kConstTempoBeats.cbegin());

    EXPECT_EQ(0, kConstTempoBeats.cend() - kConstTempoBeats.cend());
    EXPECT_EQ(0, kConstTempoBeats.cbegin() - kConstTempoBeats.cbegin());
}

TEST(BeatsTest, ConstTempoIteratorPrefixIncrement) {
    auto it = kConstTempoBeats.cbegin();
    EXPECT_EQ(kConstTempoBeats.cbegin() + 1, ++it);
    EXPECT_EQ(kConstTempoBeats.cbegin() + 2, ++it);
}

TEST(BeatsTest, ConstTempoIteratorPostfixIncrement) {
    auto it = kConstTempoBeats.cbegin();
    EXPECT_EQ(kConstTempoBeats.cbegin(), it++);
    EXPECT_EQ(kConstTempoBeats.cbegin() + 1, it++);
    EXPECT_EQ(kConstTempoBeats.cbegin() + 2, it);
}

TEST(BeatsTest, ConstTempoIteratorPrefixDecrement) {
    auto it = kConstTempoBeats.cend();
    EXPECT_EQ(kConstTempoBeats.cend() - 1, --it);
    EXPECT_EQ(kConstTempoBeats.cend() - 2, --it);
}

TEST(BeatsTest, ConstTempoIteratorPostfixDecrement) {
    auto it = kConstTempoBeats.cend();
    EXPECT_EQ(kConstTempoBeats.cend(), it--);
    EXPECT_EQ(kConstTempoBeats.cend() - 1, it--);
    EXPECT_EQ(kConstTempoBeats.cend() - 2, it);
}

TEST(BeatsTest, NonConstTempoIteratorAdd) {
    EXPECT_EQ(kNonConstTempoBeats.cend(), kNonConstTempoBeats.cbegin() + 25);

    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 30, kNonConstTempoBeats.cend() + 5);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 35, kNonConstTempoBeats.cend() + 10);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 40, kNonConstTempoBeats.cend() + 15);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 45, kNonConstTempoBeats.cend() + 20);
}

TEST(BeatsTest, NonConstTempoIteratorSubtract) {
    EXPECT_EQ(kNonConstTempoBeats.cbegin(), kNonConstTempoBeats.cend() - 25);

    EXPECT_EQ(kNonConstTempoBeats.cbegin() - 5, kNonConstTempoBeats.cend() - 30);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() - 10, kNonConstTempoBeats.cend() - 35);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() - 15, kNonConstTempoBeats.cend() - 40);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() - 20, kNonConstTempoBeats.cend() - 45);
}

TEST(BeatsTest, NonConstTempoIteratorAddSubtract) {
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 5, kNonConstTempoBeats.cend() - 20);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 10, kNonConstTempoBeats.cend() - 15);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 15, kNonConstTempoBeats.cend() - 10);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 20, kNonConstTempoBeats.cend() - 5);
}

TEST(BeatsTest, NonConstTempoIteratorDifference) {
    EXPECT_EQ(-25, kNonConstTempoBeats.cbegin() - kNonConstTempoBeats.cend());
    EXPECT_EQ(25, kNonConstTempoBeats.cend() - kNonConstTempoBeats.cbegin());

    EXPECT_EQ(0, kNonConstTempoBeats.cend() - kNonConstTempoBeats.cend());
    EXPECT_EQ(0, kNonConstTempoBeats.cbegin() - kNonConstTempoBeats.cbegin());
}

TEST(BeatsTest, NonConstTempoIteratorPrefixIncrement) {
    auto it = kNonConstTempoBeats.cbegin();
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 1, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 2, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 3, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 4, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 5, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 6, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 7, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 8, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 9, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 9, it);
}

TEST(BeatsTest, NonConstTempoIteratorPostfixIncrement) {
    auto it = kNonConstTempoBeats.cbegin();
    EXPECT_EQ(kNonConstTempoBeats.cbegin(), it++);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 1, it++);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 2, it++);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 3, it++);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 4, it++);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 5, it++);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 6, it++);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 7, it++);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 8, it++);
    EXPECT_EQ(kNonConstTempoBeats.cbegin() + 9, it);
}

TEST(BeatsTest, NonConstTempoIteratorPrefixDecrement) {
    auto it = kNonConstTempoBeats.cend();
    EXPECT_EQ(kNonConstTempoBeats.cend() - 1, --it);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 2, --it);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 3, --it);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 4, --it);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 5, --it);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 6, --it);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 7, --it);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 8, --it);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 9, --it);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 9, it);
}

TEST(BeatsTest, NonConstTempoIteratorPostfixDecrement) {
    auto it = kNonConstTempoBeats.cend();
    EXPECT_EQ(kNonConstTempoBeats.cend(), it--);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 1, it--);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 2, it--);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 3, it--);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 4, it--);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 5, it--);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 6, it--);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 7, it--);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 8, it--);
    EXPECT_EQ(kNonConstTempoBeats.cend() - 9, it);
}

TEST(BeatsTest, ConstTempoSerialization) {
    const QByteArray byteArray = kConstTempoBeats.toByteArray();
    ASSERT_EQ(BEAT_GRID_2_VERSION, kConstTempoBeats.getVersion());

    auto pBeats = Beats::fromByteArray(
            kSampleRate, BEAT_GRID_2_VERSION, QString(), byteArray);
    ASSERT_NE(nullptr, pBeats);

    EXPECT_EQ(byteArray, pBeats->toByteArray());
}

TEST(BeatsTest, NonConstTempoSerialization) {
    const QByteArray byteArray = kNonConstTempoBeats.toByteArray();
    ASSERT_EQ(BEAT_MAP_VERSION, kNonConstTempoBeats.getVersion());

    auto pBeats = Beats::fromByteArray(kSampleRate, BEAT_MAP_VERSION, QString(), byteArray);
    ASSERT_NE(nullptr, pBeats);

    EXPECT_EQ(byteArray, pBeats->toByteArray());
}

TEST(BeatsTest, NonConstTempoFromBeatPositions) {
    QVector<audio::FramePos> beatPositions;
    const audio::FrameDiff_t beatLengthFrames = 60.0 * kSampleRate.value() / kBpm.value();
    qWarning() << beatLengthFrames;
    for (int i = 0; i < 8; i++) {
        beatPositions.append(kStartPosition + i * beatLengthFrames);
        qWarning() << beatPositions.last();
    }

    for (int i = 0; i < 16; i++) {
        beatPositions.append(kStartPosition + 8 * beatLengthFrames + i * beatLengthFrames * 2);
        qWarning() << beatPositions.last();
    }

    for (int i = 0; i <= 16; i++) {
        beatPositions.append(kStartPosition + 40 * beatLengthFrames + i * beatLengthFrames / 2);
    }

    auto pBeats = Beats::fromBeatPositions(kSampleRate, beatPositions);
    ASSERT_NE(nullptr, pBeats);

    auto markers = pBeats->getMarkers();
    ASSERT_EQ(3, markers.size());

    EXPECT_DOUBLE_EQ(kStartPosition.value(), markers[0].position().value());
    EXPECT_EQ(8, markers[0].beatsTillNextMarker());

    EXPECT_DOUBLE_EQ((kStartPosition + 8 * beatLengthFrames).value(),
            markers[1].position().value());
    EXPECT_EQ(16, markers[1].beatsTillNextMarker());

    EXPECT_DOUBLE_EQ((kStartPosition + 40 * beatLengthFrames).value(),
            markers[2].position().value());
    EXPECT_EQ(16, markers[2].beatsTillNextMarker());

    EXPECT_DOUBLE_EQ((kStartPosition + 48 * beatLengthFrames).value(),
            pBeats->getEndMarkerPosition().value());
    EXPECT_EQ(kBpm * 2, pBeats->getEndMarkerBpm());
}

TEST(BeatsTest, ConstTempoFindNthBeatWhenOnBeat) {
    const auto it = kConstTempoBeats.cbegin() + 10;
    const audio::FrameDiff_t beatLengthFrames = 60.0 * kSampleRate.value() / kBpm.value();
    const audio::FramePos position = *it;

    // The spec dictates that a value of 0 is always invalid and returns an invalid position
    EXPECT_FALSE(kConstTempoBeats.findNthBeat(position, 0).isValid());

    // findNthBeat should return exactly the current beat if we ask for 1 or
    // -1. For all other values, it should return n times the beat length.
    for (int i = 1; i < 20; ++i) {
        EXPECT_NEAR(
                (position + beatLengthFrames * (i - 1)).value(),
                kConstTempoBeats.findNthBeat(position, i).value(),
                kMaxBeatError);
        EXPECT_NEAR((position + beatLengthFrames * (-i + 1)).value(),
                kConstTempoBeats.findNthBeat(position, -i).value(),
                kMaxBeatError);
    }
}

TEST(BeatsTest, ConstTempoFindNthBeatWhenNotOnBeat) {
    auto it = kConstTempoBeats.cbegin() + 10;
    const mixxx::audio::FramePos previousBeat = *it;
    it++;
    const mixxx::audio::FramePos nextBeat = *it;
    const audio::FramePos position = previousBeat + ((nextBeat - previousBeat) / 2.0);

    const audio::FrameDiff_t beatLengthFrames = 60.0 * kSampleRate.value() / kBpm.value();

    // The spec dictates that a value of 0 is always invalid and returns an invalid position
    EXPECT_FALSE(kConstTempoBeats.findNthBeat(position, 0).isValid());

    // findNthBeat should return multiples of beats starting from the next or
    // previous beat, depending on whether N is positive or negative.
    for (int i = 1; i < 20; ++i) {
        EXPECT_NEAR(
                (nextBeat + beatLengthFrames * (i - 1)).value(),
                kConstTempoBeats.findNthBeat(position, i).value(),
                kMaxBeatError);
        EXPECT_NEAR(
                (previousBeat + beatLengthFrames * (-i + 1)).value(),
                kConstTempoBeats.findNthBeat(position, -i).value(),
                kMaxBeatError);
    }
}

TEST(BeatsTest, ConstTempoFindPrevNextBeatWhenOnBeat) {
    const auto it = kConstTempoBeats.cbegin() + 10;
    const audio::FramePos position = *it;
    const audio::FrameDiff_t beatLengthFrames = 60.0 * kSampleRate.value() / kBpm.value();

    // Test prev/next beat calculation.
    audio::FramePos prevBeatPosition, nextBeatPosition;
    kConstTempoBeats.findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, true);
    EXPECT_NEAR(position.value(), prevBeatPosition.value(), kMaxBeatError);
    EXPECT_NEAR((position + beatLengthFrames).value(), nextBeatPosition.value(), kMaxBeatError);

    // Also test prev/next beat calculation without snapping tolerance
    kConstTempoBeats.findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, false);
    EXPECT_NEAR(position.value(), prevBeatPosition.value(), kMaxBeatError);
    EXPECT_NEAR((position + beatLengthFrames).value(), nextBeatPosition.value(), kMaxBeatError);

    // Both previous and next beat should return the current position.
    EXPECT_NEAR(position.value(), kConstTempoBeats.findNextBeat(position).value(), kMaxBeatError);
    EXPECT_NEAR(position.value(), kConstTempoBeats.findPrevBeat(position).value(), kMaxBeatError);
}

TEST(BeatsTest, ConstTempoFindPrevNextBeatWhenNotOnBeat) {
    auto it = kConstTempoBeats.cbegin() + 10;
    const mixxx::audio::FramePos previousBeat = *it;
    it++;
    const mixxx::audio::FramePos nextBeat = *it;
    const audio::FramePos position = previousBeat + ((nextBeat - previousBeat) / 2.0);

    // Test prev/next beat calculation
    mixxx::audio::FramePos foundPrevBeat, foundNextBeat;
    kConstTempoBeats.findPrevNextBeats(position, &foundPrevBeat, &foundNextBeat, true);
    EXPECT_NEAR(previousBeat.value(), foundPrevBeat.value(), kMaxBeatError);
    EXPECT_NEAR(nextBeat.value(), foundNextBeat.value(), kMaxBeatError);

    // Also test prev/next beat calculation without snapping tolerance
    kConstTempoBeats.findPrevNextBeats(position, &foundPrevBeat, &foundNextBeat, false);
    EXPECT_NEAR(previousBeat.value(), foundPrevBeat.value(), kMaxBeatError);
    EXPECT_NEAR(nextBeat.value(), foundNextBeat.value(), kMaxBeatError);
}

} // namespace
