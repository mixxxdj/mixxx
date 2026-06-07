#include <gtest/gtest.h>

#include "audio/types.h"
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

} // namespace
