#include "track/cue.h"

#include <gtest/gtest.h>

#include "test/mixxxtest.h"
#include "util/color/color.h"

namespace mixxx {

TEST(CueTest, DefaultCueToCueInfoTest) {
    const Cue cueObject;
    const auto cueInfo = cueObject.getCueInfo(
            AudioSignal::SampleRate(44100));
    EXPECT_EQ(CueInfo(), cueInfo);
}

TEST(CueTest, DefaultCueInfoToCueRoundtrip) {
    const CueInfo cueInfo1;
    const Cue cueObject(
            cueInfo1,
            AudioSignal::SampleRate(44100));
    const auto cueInfo2 = cueObject.getCueInfo(
            AudioSignal::SampleRate(44100));
    EXPECT_EQ(cueInfo1, cueInfo2);
}

TEST(CueTest, ConvertCueInfoToCueRoundtrip) {
    // Due to rounding errors this test may fail if the
    // cue position/sample conversions don't always result
    // in integer numbers.
    const auto predefinedColor =
            Color::kPredefinedColorsSet.predefinedColorFromRgbColor(
                    RgbColor::optional(0xabcdef))->m_defaultRgba;
    const auto cueInfo1 = CueInfo(
            CueType::HotCue,
            std::make_optional(1.0 * 44100 * 2),
            std::make_optional(2.0 * 44100 * 2),
            std::make_optional(3),
            QStringLiteral("label"),
            RgbColor::fromQColor(predefinedColor));
    const Cue cueObject(
            cueInfo1,
            AudioSignal::SampleRate(44100));
    const auto cueInfo2 = cueObject.getCueInfo(
            AudioSignal::SampleRate(44100));
    EXPECT_EQ(cueInfo1, cueInfo2);
}

} // namespace mixxx
