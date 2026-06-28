#include <gtest/gtest.h>

#include "audio/types.h"
#include "track/beatfactory.h"
#include "track/beats.h"
#include "track/bpm.h"

using namespace mixxx;

namespace {

constexpr auto kBpm = Bpm(120.0);
constexpr auto kSampleRate = audio::SampleRate(44100);
constexpr auto kStartPosition = audio::FramePos(0);
constexpr int kBeatsPerBar = 4;

const auto kConstTempoBeats = Beats(
        kStartPosition,
        kBpm,
        kSampleRate,
        QString());

TEST(BeatsBarTest, GlobalBeatIndexFromFirstMarker) {
    auto firstMarker = kConstTempoBeats.cfirstmarker();
    auto it = firstMarker;
    for (int i = 0; i < 16; ++i) {
        int globalIndex = it - firstMarker;
        EXPECT_EQ(globalIndex, i) << "Beat " << i;
        ++it;
    }
}

TEST(BeatsBarTest, DownbeatDetectionEveryFourBeats) {
    auto firstMarker = kConstTempoBeats.cfirstmarker();
    auto it = firstMarker;
    for (int i = 0; i < 16; ++i) {
        int globalIndex = it - firstMarker;
        bool isDownbeat = (globalIndex % kBeatsPerBar) == 0;
        if (i % kBeatsPerBar == 0) {
            EXPECT_TRUE(isDownbeat) << "Beat " << i << " should be a downbeat";
        } else {
            EXPECT_FALSE(isDownbeat) << "Beat " << i << " should not be a downbeat";
        }
        ++it;
    }
}

TEST(BeatsBarTest, BarNumberCalculation) {
    auto firstMarker = kConstTempoBeats.cfirstmarker();
    auto it = firstMarker;
    for (int i = 0; i < 16; ++i) {
        int globalIndex = it - firstMarker;
        int barNumber = (globalIndex / kBeatsPerBar) + 1;
        int expectedBar = (i / kBeatsPerBar) + 1;
        EXPECT_EQ(barNumber, expectedBar) << "Beat " << i;
        ++it;
    }
}

TEST(BeatsBarTest, IteratorFromMidTrackPosition) {
    auto firstMarker = kConstTempoBeats.cfirstmarker();
    double beatLengthFrames = 60.0 * kSampleRate.value() / kBpm.value();
    auto midPosition = audio::FramePos(beatLengthFrames * 8.5);
    auto it = kConstTempoBeats.iteratorFrom(midPosition);
    int globalIndex = it - firstMarker;
    EXPECT_EQ(globalIndex, 9);
    EXPECT_EQ(globalIndex % kBeatsPerBar, 1);
}

TEST(BeatsBarTest, NegativeBeatIndexBeforeFirstMarker) {
    auto firstMarker = kConstTempoBeats.cfirstmarker();
    auto beforeFirst = firstMarker - 2;
    int globalIndex = beforeFirst - firstMarker;
    EXPECT_EQ(globalIndex, -2);
    int normalizedMod = ((globalIndex % kBeatsPerBar) + kBeatsPerBar) % kBeatsPerBar;
    EXPECT_EQ(normalizedMod, 2);
}

TEST(BeatsBarTest, ThreeQuarterTimeSignature) {
    auto firstMarker = kConstTempoBeats.cfirstmarker();
    const int beatsPerBar = 3;
    auto it = firstMarker;
    for (int i = 0; i < 12; ++i) {
        int globalIndex = it - firstMarker;
        int normalizedMod = ((globalIndex % beatsPerBar) + beatsPerBar) % beatsPerBar;
        bool isDownbeat = (normalizedMod == 0);
        int barNumber = (globalIndex / beatsPerBar) + 1;
        if (i % beatsPerBar == 0) {
            EXPECT_TRUE(isDownbeat) << "Beat " << i;
        } else {
            EXPECT_FALSE(isDownbeat) << "Beat " << i;
        }
        int expectedBar = (i / beatsPerBar) + 1;
        EXPECT_EQ(barNumber, expectedBar) << "Beat " << i;
        ++it;
    }
}

TEST(BeatsBarTest, NonConstTempoBeatIndexAcrossMarkers) {
    const auto nonConstTempoBeats = Beats(
            std::vector<BeatMarker>{
                    BeatMarker{kStartPosition, 8},
            },
            kStartPosition + 8 * kSampleRate.value() / (kBpm.value() / 60.0),
            kBpm,
            kSampleRate,
            QString());

    auto firstMarker = nonConstTempoBeats.cfirstmarker();
    auto it = firstMarker;
    for (int i = 0; i < 12; ++i) {
        int globalIndex = it - firstMarker;
        EXPECT_EQ(globalIndex, i) << "Beat " << i << " across tempo sections";
        ++it;
    }
}

TEST(BeatsBarTest, BeatWithinBarIndex) {
    auto firstMarker = kConstTempoBeats.cfirstmarker();
    auto it = firstMarker;
    for (int i = 0; i < 16; ++i) {
        int globalIndex = it - firstMarker;
        int beatWithinBar = globalIndex % kBeatsPerBar;
        EXPECT_EQ(beatWithinBar, i % kBeatsPerBar) << "Beat " << i;
        ++it;
    }
}

TEST(BeatsBarTest, NegativeBarNumberBeforeFirstMarker) {
    auto firstMarker = kConstTempoBeats.cfirstmarker();
    for (int offset = 1; offset <= 8; ++offset) {
        auto beforeFirst = firstMarker - offset;
        int globalIndex = beforeFirst - firstMarker;
        EXPECT_EQ(globalIndex, -offset);
        int normalizedMod = ((globalIndex % kBeatsPerBar) + kBeatsPerBar) % kBeatsPerBar;
        bool isDownbeat = (normalizedMod == 0);
        if (offset % kBeatsPerBar == 0) {
            EXPECT_TRUE(isDownbeat) << "Offset " << offset;
        } else {
            EXPECT_FALSE(isDownbeat) << "Offset " << offset;
        }
    }
}

TEST(BeatsBarTest, EveryBeatIsDownbeatWhenBeatsPerBarIsOne) {
    auto firstMarker = kConstTempoBeats.cfirstmarker();
    const int beatsPerBar = 1;
    auto it = firstMarker;
    for (int i = 0; i < 8; ++i) {
        int globalIndex = it - firstMarker;
        int normalizedMod = ((globalIndex % beatsPerBar) + beatsPerBar) % beatsPerBar;
        EXPECT_EQ(normalizedMod, 0) << "Beat " << i << " should be downbeat with beatsPerBar=1";
        int barNumber = (globalIndex / beatsPerBar) + 1;
        EXPECT_EQ(barNumber, i + 1) << "Beat " << i;
        ++it;
    }
}

TEST(BeatsBarTest, IteratorFromExactBeatPosition) {
    auto firstMarker = kConstTempoBeats.cfirstmarker();
    double beatLengthFrames = 60.0 * kSampleRate.value() / kBpm.value();
    auto exactPosition = audio::FramePos(beatLengthFrames * 4.0);
    auto it = kConstTempoBeats.iteratorFrom(exactPosition);
    int globalIndex = it - firstMarker;
    EXPECT_EQ(globalIndex, 4);
    EXPECT_EQ(globalIndex % kBeatsPerBar, 0);
}

TEST(BeatsBarTest, LargeBarNumbers) {
    auto firstMarker = kConstTempoBeats.cfirstmarker();
    double beatLengthFrames = 60.0 * kSampleRate.value() / kBpm.value();
    auto farPosition = audio::FramePos(beatLengthFrames * 400.5);
    auto it = kConstTempoBeats.iteratorFrom(farPosition);
    int globalIndex = it - firstMarker;
    EXPECT_EQ(globalIndex, 401);
    int barNumber = (globalIndex / kBeatsPerBar) + 1;
    EXPECT_EQ(barNumber, 101);
}

TEST(BeatsBarTest, NonConstTempoDownbeatDetection) {
    const auto nonConstTempoBeats = Beats(
            std::vector<BeatMarker>{
                    BeatMarker{kStartPosition, 8},
            },
            kStartPosition + 8 * kSampleRate.value() / (kBpm.value() / 60.0),
            kBpm,
            kSampleRate,
            QString());

    auto firstMarker = nonConstTempoBeats.cfirstmarker();
    auto it = firstMarker;
    for (int i = 0; i < 12; ++i) {
        int globalIndex = it - firstMarker;
        int normalizedMod = ((globalIndex % kBeatsPerBar) + kBeatsPerBar) % kBeatsPerBar;
        bool isDownbeat = (normalizedMod == 0);
        if (i % kBeatsPerBar == 0) {
            EXPECT_TRUE(isDownbeat) << "Beat " << i << " should be downbeat across tempo change";
        } else {
            EXPECT_FALSE(isDownbeat) << "Beat " << i << " should not be downbeat across tempo change";
        }
        ++it;
    }
}

TEST(BeatsBarTest, NonZeroStartPosition) {
    const auto offsetBeats = Beats(
            audio::FramePos(400),
            kBpm,
            kSampleRate,
            QString());

    auto firstMarker = offsetBeats.cfirstmarker();
    auto it = firstMarker;
    for (int i = 0; i < 8; ++i) {
        int globalIndex = it - firstMarker;
        EXPECT_EQ(globalIndex, i) << "Beat " << i << " with offset start";
        int barNumber = (globalIndex / kBeatsPerBar) + 1;
        int expectedBar = (i / kBeatsPerBar) + 1;
        EXPECT_EQ(barNumber, expectedBar) << "Beat " << i;
        ++it;
    }
}

TEST(BeatsBarTest, DefaultBeatsPerBarIsZero) {
    EXPECT_EQ(kConstTempoBeats.beatsPerBar(), 0);
    EXPECT_EQ(kConstTempoBeats.downbeatOffset(), 0);
}

TEST(BeatsBarTest, TrySetBeatsPerBar) {
    auto pBeats = Beats::fromConstTempo(
            kSampleRate, kStartPosition, kBpm);
    ASSERT_NE(pBeats, nullptr);
    EXPECT_EQ(pBeats->beatsPerBar(), 0);

    auto result = pBeats->trySetBeatsPerBar(3);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)->beatsPerBar(), 3);
    EXPECT_EQ(pBeats->beatsPerBar(), 0);
}

TEST(BeatsBarTest, TrySetDownbeatOffset) {
    auto pBeats = Beats::fromConstTempo(
            kSampleRate, kStartPosition, kBpm);
    ASSERT_NE(pBeats, nullptr);

    auto result = pBeats->trySetDownbeatOffset(2);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)->downbeatOffset(), 2);
    EXPECT_EQ(pBeats->downbeatOffset(), 0);
}

TEST(BeatsBarTest, SerializationRoundTripBeatGrid) {
    auto pBeats = Beats::fromConstTempo(
            kSampleRate, kStartPosition, kBpm);
    ASSERT_NE(pBeats, nullptr);

    auto pWithTimeSig = pBeats->trySetBeatsPerBar(3);
    ASSERT_TRUE(pWithTimeSig.has_value());
    auto pWithBoth = (*pWithTimeSig)->trySetDownbeatOffset(1);
    ASSERT_TRUE(pWithBoth.has_value());

    QByteArray serialized = (*pWithBoth)->toByteArray();
    auto pDeserialized = Beats::fromByteArray(
            kSampleRate,
            BEAT_GRID_2_VERSION,
            QString(),
            serialized);
    ASSERT_NE(pDeserialized, nullptr);
    EXPECT_EQ(pDeserialized->beatsPerBar(), 3);
    EXPECT_EQ(pDeserialized->downbeatOffset(), 1);
}

TEST(BeatsBarTest, SerializationRoundTripBeatMap) {
    QVector<audio::FramePos> positions;
    double frame = 0.0;
    for (int i = 0; i < 20; ++i) {
        positions.append(audio::FramePos(frame));
        frame += (i < 10) ? 22050.0 : 22100.0;
    }
    auto pBeats = Beats::fromBeatPositions(kSampleRate, positions);
    ASSERT_NE(pBeats, nullptr);
    ASSERT_FALSE(pBeats->hasConstantTempo());

    auto pWithTimeSig = pBeats->trySetBeatsPerBar(6);
    ASSERT_TRUE(pWithTimeSig.has_value());
    auto pWithBoth = (*pWithTimeSig)->trySetDownbeatOffset(2);
    ASSERT_TRUE(pWithBoth.has_value());

    QByteArray serialized = (*pWithBoth)->toByteArray();
    auto pDeserialized = Beats::fromByteArray(
            kSampleRate,
            BEAT_MAP_VERSION,
            QString(),
            serialized);
    ASSERT_NE(pDeserialized, nullptr);
    EXPECT_EQ(pDeserialized->beatsPerBar(), 6);
    EXPECT_EQ(pDeserialized->downbeatOffset(), 2);
}

TEST(BeatsBarTest, BackwardCompatibilityNoTimeSig) {
    auto pBeats = Beats::fromConstTempo(
            kSampleRate, kStartPosition, kBpm);
    ASSERT_NE(pBeats, nullptr);

    QByteArray serialized = pBeats->toByteArray();
    auto pDeserialized = Beats::fromByteArray(
            kSampleRate,
            BEAT_GRID_2_VERSION,
            QString(),
            serialized);
    ASSERT_NE(pDeserialized, nullptr);
    EXPECT_EQ(pDeserialized->beatsPerBar(), 0);
    EXPECT_EQ(pDeserialized->downbeatOffset(), 0);
}

TEST(BeatsBarTest, MutationsPreserveBeatsPerBar) {
    auto pBeats = Beats::fromConstTempo(
            kSampleRate, kStartPosition, kBpm);
    ASSERT_NE(pBeats, nullptr);

    auto pWithBpb = pBeats->trySetBeatsPerBar(5);
    ASSERT_TRUE(pWithBpb.has_value());

    auto pTranslated = (*pWithBpb)->tryTranslate(100.0);
    ASSERT_TRUE(pTranslated.has_value());
    EXPECT_EQ((*pTranslated)->beatsPerBar(), 5);

    auto pScaled = (*pWithBpb)->tryScale(Beats::BpmScale::Double);
    ASSERT_TRUE(pScaled.has_value());
    EXPECT_EQ((*pScaled)->beatsPerBar(), 5);

    auto pNewBpm = (*pWithBpb)->trySetBpm(Bpm(140.0));
    ASSERT_TRUE(pNewBpm.has_value());
    EXPECT_EQ((*pNewBpm)->beatsPerBar(), 5);
}

TEST(BeatsBarTest, FactoryPreservesBeatsPerBar) {
    QVector<audio::FramePos> beatPositions;
    for (int i = 0; i < 100; ++i) {
        beatPositions.append(audio::FramePos(i * 22050.0));
    }

    QHash<QString, QString> extraVersionInfo;
    auto pBeats = BeatFactory::makePreferredBeats(
            beatPositions,
            extraVersionInfo,
            true,
            kSampleRate,
            3,
            1);
    ASSERT_NE(pBeats, nullptr);
    EXPECT_EQ(pBeats->beatsPerBar(), 3);
    EXPECT_EQ(pBeats->downbeatOffset(), 1);

    QByteArray serialized = pBeats->toByteArray();
    auto pDeserialized = Beats::fromByteArray(
            kSampleRate,
            pBeats->getVersion(),
            pBeats->getSubVersion(),
            serialized);
    ASSERT_NE(pDeserialized, nullptr);
    EXPECT_EQ(pDeserialized->beatsPerBar(), 3);
    EXPECT_EQ(pDeserialized->downbeatOffset(), 1);
}

} // namespace
