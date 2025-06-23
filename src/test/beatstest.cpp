#include <gtest/gtest.h>

#include <QtDebug>
#include <memory>

#include "audio/types.h"
#include "track/beats.h"
#include "track/beatutils.h"
#include "track/bpm.h"

using namespace mixxx;

namespace {

constexpr auto kBpm = Bpm(120.0);
constexpr auto kSampleRate = audio::SampleRate(48000);
constexpr auto kStartPosition = audio::FramePos(400);
const auto kEndPosition = kStartPosition + 24 * kSampleRate.value();
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

TEST(BeatsTest, ConstTempoGetBpmInRange) {
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kConstTempoBeats.getBpmInRange(kStartPosition, kEndPosition)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kConstTempoBeats.getBpmInRange(kStartPosition, kEndPosition * 0.5)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kConstTempoBeats.getBpmInRange(kStartPosition, kEndPosition * 2)
                    .value());
}

TEST(BeatsTest, ConstTempoGetBpmAroundPosition) {
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kConstTempoBeats.getBpmAroundPosition(kStartPosition, 2).value());
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kConstTempoBeats.getBpmAroundPosition(kStartPosition, 4).value());
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kConstTempoBeats.getBpmAroundPosition(kStartPosition, 8).value());
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kConstTempoBeats
                    .getBpmAroundPosition(
                            audio::FramePos{kEndPosition - kStartPosition}, 8)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value(), kConstTempoBeats.getBpmAroundPosition(kEndPosition, 2).value());
    EXPECT_DOUBLE_EQ(kBpm.value(), kConstTempoBeats.getBpmAroundPosition(kEndPosition, 4).value());
    EXPECT_DOUBLE_EQ(kBpm.value(), kConstTempoBeats.getBpmAroundPosition(kEndPosition, 8).value());
}

TEST(BeatsTest, NonConstTempoGetBpmInRange) {
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kNonConstTempoBeats
                    .getBpmInRange(kStartPosition,
                            kStartPosition + 8 * kSampleRate.value() / 2)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value() / 2,
            kNonConstTempoBeats
                    .getBpmInRange(kStartPosition + 8 * kSampleRate.value() / 2,
                            kStartPosition + 8 * kSampleRate.value() / 2 +
                                    16 * kSampleRate.value())
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kNonConstTempoBeats
                    .getBpmInRange(kStartPosition +
                                    8 * kSampleRate.value() / 2 +
                                    16 * kSampleRate.value(),
                            kEndPosition)
                    .value());

    // The section at 60 BPM is longer than the two sections at 120 BPM combined;
    EXPECT_DOUBLE_EQ(kBpm.value() / 2,
            kNonConstTempoBeats.getBpmInRange(kStartPosition, kEndPosition)
                    .value());

    // If the track is very long, the last section at 120 BPM becomes more important.
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kNonConstTempoBeats
                    .getBpmInRange(kStartPosition,
                            kEndPosition + 24 * kSampleRate.value())
                    .value());
}

TEST(BeatsTest, NonConstTempoGetBpmAroundPosition) {
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kNonConstTempoBeats.getBpmAroundPosition(kStartPosition, 4)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kNonConstTempoBeats.getBpmAroundPosition(kStartPosition, 8)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 2 * kSampleRate.value(), 4)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value() / 2,
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 8 * kSampleRate.value(), 4)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value() / 2,
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 12 * kSampleRate.value(), 8)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value() / 2,
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 16 * kSampleRate.value(), 4)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 24 * kSampleRate.value(), 4)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 28 * kSampleRate.value(), 8)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value(),
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 32 * kSampleRate.value(), 4)
                    .value());

    // The relation between beatlength and BPM is defined as:
    //
    //  beatlen = 60 * sampleRate / bpm
    //  <=> bpm = 60 * sampleRate / beatlen
    //
    // We have a section with 2 beats with regular bpm and 2 beats of double
    // bpm, we can calculate the expected average bpm in relation to the
    // original bpm like this:
    //
    //   bpm' = 60 * sampleRate / beatlen'
    //        = 60 * sampleRate / ((2 * beatlen + 2 * 2 * beatlen) / 4)
    //        = 60 * sampleRate / ((6 / 4) * beatlen)
    //        = 60 * sampleRate / ((3 / 2) * beatlen)
    //        = 60 * sampleRate * (2 / 3) / beatlen
    //        = (2 / 3) * 60 * sampleRate / beatlen
    //        = (2 / 3) * bpm
    EXPECT_DOUBLE_EQ(kBpm.value() * (2.0 / 3.0),
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 4 * kSampleRate.value(), 2)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value() * (2.0 / 3.0),
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 4 * kSampleRate.value(), 4)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value() * (2.0 / 3.0),
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 4 * kSampleRate.value(), 8)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value() * (2.0 / 3.0),
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 20 * kSampleRate.value(), 2)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value() * (2.0 / 3.0),
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 20 * kSampleRate.value(), 4)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value() * (2.0 / 3.0),
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 20 * kSampleRate.value(), 8)
                    .value());

    //   bpm' = (8 / (6 + 2 * 2)) * bpm
    //        = (8 / 10) * bpm
    //        = 0.8 * bpm
    EXPECT_DOUBLE_EQ(kBpm.value() * 0.8,
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 3 * kSampleRate.value(), 4)
                    .value());
    EXPECT_DOUBLE_EQ(kBpm.value() * 0.8,
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 2 * kSampleRate.value(), 8)
                    .value());

    //   bpm' = (4 / (2 * 1 + 3)) * bpm
    //        = (4 / 5) * bpm
    //        = 0.8 * bpm
    EXPECT_DOUBLE_EQ(kBpm.value() * 0.8,
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 20.5 * kSampleRate.value(), 2)
                    .value());

    //   bpm' = (8 / (2 * 1 + 7)) * bpm
    //        = (8 / 9) * bpm
    EXPECT_DOUBLE_EQ(kBpm.value() * (8.0 / 9.0),
            kNonConstTempoBeats
                    .getBpmAroundPosition(
                            kStartPosition + 21.5 * kSampleRate.value(), 4)
                    .value());
}

TEST(BeatsTest, ConstTempoIteratorSubtract) {
    EXPECT_EQ(kConstTempoBeats.cfirstmarker(), kConstTempoBeats.clastmarker());
    EXPECT_EQ(kConstTempoBeats.cfirstmarker() - 5, kConstTempoBeats.clastmarker() - 5);
    EXPECT_EQ(kConstTempoBeats.cfirstmarker() - 10, kConstTempoBeats.clastmarker() - 10);
}

TEST(BeatsTest, ConstTempoIteratorAddSubtractNegativeIsEquivalent) {
    EXPECT_EQ(kConstTempoBeats.cfirstmarker() + 1, kConstTempoBeats.cfirstmarker() - (-1));
    EXPECT_EQ(kConstTempoBeats.cfirstmarker() + 5, kConstTempoBeats.cfirstmarker() - (-5));
    EXPECT_EQ(kConstTempoBeats.cfirstmarker() - 1, kConstTempoBeats.cfirstmarker() + (-1));
    EXPECT_EQ(kConstTempoBeats.cfirstmarker() - 5, kConstTempoBeats.cfirstmarker() + (-5));
    EXPECT_EQ(kConstTempoBeats.clastmarker() + 1, kConstTempoBeats.clastmarker() - (-1));
    EXPECT_EQ(kConstTempoBeats.clastmarker() + 5, kConstTempoBeats.clastmarker() - (-5));
    EXPECT_EQ(kConstTempoBeats.clastmarker() - 1, kConstTempoBeats.clastmarker() + (-1));
    EXPECT_EQ(kConstTempoBeats.clastmarker() - 5, kConstTempoBeats.clastmarker() + (-5));
}

TEST(BeatsTest, ConstTempoIteratorSubtractPosition) {
    const audio::FrameDiff_t beatLengthFrames = 60.0 * kSampleRate.value() / kBpm.value();
    EXPECT_NEAR((*(kConstTempoBeats.cfirstmarker() - 100)).value(),
            ((*kConstTempoBeats.cfirstmarker()) - 100 * beatLengthFrames).value(),
            kMaxBeatError);
    EXPECT_NEAR((*(kConstTempoBeats.clastmarker() - 100)).value(),
            ((*kConstTempoBeats.clastmarker()) - 100 * beatLengthFrames).value(),
            kMaxBeatError);
}

TEST(BeatsTest, ConstTempoIteratorAdd) {
    EXPECT_EQ(kConstTempoBeats.clastmarker(), kConstTempoBeats.cfirstmarker());
    EXPECT_EQ(kConstTempoBeats.clastmarker() + 5, kConstTempoBeats.cfirstmarker() + 5);
    EXPECT_EQ(kConstTempoBeats.clastmarker() + 10, kConstTempoBeats.cfirstmarker() + 10);
}

TEST(BeatsTest, ConstTempoIteratorAddPosition) {
    const audio::FrameDiff_t beatLengthFrames = 60.0 * kSampleRate.value() / kBpm.value();
    EXPECT_NEAR((*(kConstTempoBeats.cfirstmarker() + 100)).value(),
            ((*kConstTempoBeats.cfirstmarker()) + 100 * beatLengthFrames).value(),
            kMaxBeatError);
    EXPECT_NEAR((*(kConstTempoBeats.clastmarker() + 100)).value(),
            ((*kConstTempoBeats.clastmarker()) + 100 * beatLengthFrames).value(),
            kMaxBeatError);
}

TEST(BeatsTest, ConstTempoIteratorDifference) {
    EXPECT_EQ(0, kConstTempoBeats.cfirstmarker() - kConstTempoBeats.clastmarker());
    EXPECT_EQ(0, kConstTempoBeats.clastmarker() - kConstTempoBeats.cfirstmarker());

    EXPECT_EQ(0, kConstTempoBeats.clastmarker() - kConstTempoBeats.clastmarker());
    EXPECT_EQ(0, kConstTempoBeats.cfirstmarker() - kConstTempoBeats.cfirstmarker());

    EXPECT_EQ(5, (kConstTempoBeats.cfirstmarker() + 5) - kConstTempoBeats.clastmarker());
    EXPECT_EQ(7, (kConstTempoBeats.cfirstmarker() + 7) - kConstTempoBeats.clastmarker());
    EXPECT_EQ(-7,
            std::distance(kConstTempoBeats.clastmarker() + 7,
                    kConstTempoBeats.cfirstmarker()));
}

TEST(BeatsTest, ConstTempoIteratorPrefixIncrement) {
    auto it = kConstTempoBeats.cfirstmarker();
    EXPECT_EQ(kConstTempoBeats.cfirstmarker() + 1, ++it);
    EXPECT_EQ(kConstTempoBeats.cfirstmarker() + 2, ++it);
}

TEST(BeatsTest, ConstTempoIteratorPostfixIncrement) {
    auto it = kConstTempoBeats.cfirstmarker();
    EXPECT_EQ(kConstTempoBeats.cfirstmarker(), it++);
    EXPECT_EQ(kConstTempoBeats.cfirstmarker() + 1, it++);
}

TEST(BeatsTest, ConstTempoIteratorPrefixDecrement) {
    auto it = kConstTempoBeats.clastmarker();
    EXPECT_EQ(kConstTempoBeats.clastmarker() - 1, --it);
    EXPECT_EQ(kConstTempoBeats.clastmarker() - 2, --it);
}

TEST(BeatsTest, ConstTempoIteratorPostfixDecrement) {
    auto it = kConstTempoBeats.clastmarker();
    EXPECT_EQ(kConstTempoBeats.clastmarker(), it--);
    EXPECT_EQ(kConstTempoBeats.clastmarker() - 1, it--);
    EXPECT_EQ(kConstTempoBeats.clastmarker() - 2, it);
}

TEST(BeatsTest, NonConstTempoIteratorAdd) {
    EXPECT_EQ(kNonConstTempoBeats.clastmarker(), kNonConstTempoBeats.cfirstmarker() + 24);

    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 30, kNonConstTempoBeats.clastmarker() + 6);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 35, kNonConstTempoBeats.clastmarker() + 11);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 40, kNonConstTempoBeats.clastmarker() + 16);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 45, kNonConstTempoBeats.clastmarker() + 21);
}

TEST(BeatsTest, NonConstTempoIteratorSubtract) {
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker(), kNonConstTempoBeats.clastmarker() - 24);

    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() - 5, kNonConstTempoBeats.clastmarker() - 29);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() - 10, kNonConstTempoBeats.clastmarker() - 34);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() - 15, kNonConstTempoBeats.clastmarker() - 39);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() - 20, kNonConstTempoBeats.clastmarker() - 44);
}

TEST(BeatsTest, NonConstTempoIteratorAddSubtract) {
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 5, kNonConstTempoBeats.clastmarker() - 19);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 10, kNonConstTempoBeats.clastmarker() - 14);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 15, kNonConstTempoBeats.clastmarker() - 9);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 20, kNonConstTempoBeats.clastmarker() - 4);
}

TEST(BeatsTest, NonConstTempoIteratorDifference) {
    EXPECT_EQ(-24, kNonConstTempoBeats.cfirstmarker() - kNonConstTempoBeats.clastmarker());
    EXPECT_EQ(24, kNonConstTempoBeats.clastmarker() - kNonConstTempoBeats.cfirstmarker());

    EXPECT_EQ(0, kNonConstTempoBeats.clastmarker() - kNonConstTempoBeats.clastmarker());
    EXPECT_EQ(0, kNonConstTempoBeats.cfirstmarker() - kNonConstTempoBeats.cfirstmarker());
}

TEST(BeatsTest, NonConstTempoIteratorPrefixIncrement) {
    auto it = kNonConstTempoBeats.cfirstmarker();
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 1, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 2, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 3, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 4, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 5, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 6, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 7, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 8, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 9, ++it);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 9, it);
}

TEST(BeatsTest, NonConstTempoIteratorPostfixIncrement) {
    auto it = kNonConstTempoBeats.cfirstmarker();
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker(), it++);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 1, it++);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 2, it++);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 3, it++);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 4, it++);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 5, it++);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 6, it++);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 7, it++);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 8, it++);
    EXPECT_EQ(kNonConstTempoBeats.cfirstmarker() + 9, it);
}

TEST(BeatsTest, NonConstTempoIteratorPrefixDecrement) {
    auto it = kNonConstTempoBeats.clastmarker();
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 1, --it);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 2, --it);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 3, --it);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 4, --it);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 5, --it);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 6, --it);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 7, --it);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 8, --it);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 9, --it);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 9, it);
}

TEST(BeatsTest, NonConstTempoIteratorPostfixDecrement) {
    auto it = kNonConstTempoBeats.clastmarker();
    EXPECT_EQ(kNonConstTempoBeats.clastmarker(), it--);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 1, it--);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 2, it--);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 3, it--);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 4, it--);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 5, it--);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 6, it--);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 7, it--);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 8, it--);
    EXPECT_EQ(kNonConstTempoBeats.clastmarker() - 9, it);
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
            pBeats->getLastMarkerPosition().value());
    EXPECT_EQ(kBpm * 2, pBeats->getLastMarkerBpm());
}

TEST(BeatsTest, ConstTempoFindNthBeatWhenOnBeat) {
    const auto it = kConstTempoBeats.cfirstmarker() + 10;
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
    auto it = kConstTempoBeats.cfirstmarker() + 10;
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
    const auto it = kConstTempoBeats.cfirstmarker() + 10;
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
    auto it = kConstTempoBeats.cfirstmarker() + 10;
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

TEST(BeatsTest, roundBpm) {
    // Integer Bpm
    mixxx::Bpm roundBpm = BeatUtils::roundBpmWithinRange(
            mixxx::Bpm(121.5), mixxx::Bpm(121.9), mixxx::Bpm(122.4));
    EXPECT_NEAR(roundBpm.value(), 122.0, kMaxBeatError);

    // Half Bpm
    roundBpm = BeatUtils::roundBpmWithinRange(mixxx::Bpm(80.1), mixxx::Bpm(80.4), mixxx::Bpm(80.7));
    EXPECT_NEAR(roundBpm.value(), 80.5, kMaxBeatError);

    // High Half Bpm -> 1/3
    roundBpm = BeatUtils::roundBpmWithinRange(
            mixxx::Bpm(121.1), mixxx::Bpm(121.4), mixxx::Bpm(122.7));
    EXPECT_NEAR(roundBpm.value(), 121.3333333333, kMaxBeatError);

    // High Half Bpm -> 1/12
    roundBpm = BeatUtils::roundBpmWithinRange(
            mixxx::Bpm(121.4), mixxx::Bpm(121.46), mixxx::Bpm(122.6));
    EXPECT_NEAR(roundBpm.value(), 121.5, kMaxBeatError);

    // 2/3 Bpm
    roundBpm = BeatUtils::roundBpmWithinRange(
            mixxx::Bpm(186.276), mixxx::Bpm(186.621), mixxx::Bpm(186.967));
    EXPECT_NEAR(roundBpm.value(), 186.666666666, kMaxBeatError);

    // 1/12 Bpm
    roundBpm = BeatUtils::roundBpmWithinRange(
            mixxx::Bpm(186.01), mixxx::Bpm(186.1), mixxx::Bpm(186.18));
    EXPECT_NEAR(roundBpm.value(), 186.0833333333333, kMaxBeatError);
}

} // namespace
