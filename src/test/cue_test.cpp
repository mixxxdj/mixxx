#include "track/cue.h"

#include <gtest/gtest.h>

#include "engine/engine.h"
#include "test/mixxxtest.h"
#include "util/color/color.h"

namespace mixxx {

TEST(CueTest, NewCueIsDirty) {
    const auto cue = Cue(
            mixxx::CueType::HotCue,
            1,
            mixxx::audio::kStartFramePos,
            mixxx::audio::kInvalidFramePos);
    EXPECT_TRUE(cue.isDirty());
}

TEST(CueTest, DefaultCueInfoToCueRoundtrip) {
    const CueInfo cueInfo1;
    const Cue cueObject(
            cueInfo1,
            audio::SampleRate(44100),
            true);
    auto cueInfo2 = cueObject.getCueInfo(
            audio::SampleRate(44100));
    cueInfo2.setColor(std::nullopt);
    EXPECT_EQ(cueInfo1, cueInfo2);
}

TEST(CueTest, ConvertCueInfoToCueRoundtrip) {
    // Due to rounding errors this test may fail if the
    // cue position/sample conversions don't always result
    // in integer numbers.
    const auto cueInfo1 = CueInfo(
            CueType::HotCue,
            std::make_optional(1.0 * 44100 * mixxx::kEngineChannelCount),
            std::make_optional(2.0 * 44100 * mixxx::kEngineChannelCount),
            std::make_optional(3),
            QStringLiteral("label"),
            RgbColor::optional(0xABCDEF));
    const Cue cueObject(
            cueInfo1,
            audio::SampleRate(44100),
            true);
    EXPECT_TRUE(cueObject.isDirty());
    const auto cueInfo2 = cueObject.getCueInfo(
            audio::SampleRate(44100));
    EXPECT_EQ(cueInfo1, cueInfo2);
}

} // namespace mixxx
