#include <gtest/gtest.h>

#include "track/serato/tags.h"

namespace {

class SeratoTagsTest : public testing::Test {
  protected:
    void trackColorRoundtrip(mixxx::RgbColor storedColor, mixxx::RgbColor::optional_t displayedColor) {
        mixxx::RgbColor::optional_t actualDisplayedColor = mixxx::SeratoTags::storedToDisplayedTrackColor(storedColor);
        EXPECT_EQ(displayedColor, actualDisplayedColor);

        mixxx::RgbColor actualStoredColor = mixxx::SeratoTags::displayedToStoredTrackColor(actualDisplayedColor);
        EXPECT_EQ(actualStoredColor, storedColor);
    }
};

TEST_F(SeratoTagsTest, ParseTrackColor) {
    trackColorRoundtrip(mixxx::RgbColor(0xFF99FF), mixxx::RgbColor::optional(0x993399));
    trackColorRoundtrip(mixxx::RgbColor(0xFF99DD), mixxx::RgbColor::optional(0x993377));
    trackColorRoundtrip(mixxx::RgbColor(0xFF99BB), mixxx::RgbColor::optional(0x993355));
    trackColorRoundtrip(mixxx::RgbColor(0xFF9999), mixxx::RgbColor::optional(0x993333));
    trackColorRoundtrip(mixxx::RgbColor(0xFFBB99), mixxx::RgbColor::optional(0x995533));
    trackColorRoundtrip(mixxx::RgbColor(0xFFDD99), mixxx::RgbColor::optional(0x997733));
    trackColorRoundtrip(mixxx::RgbColor(0xFFFF99), mixxx::RgbColor::optional(0x999933));
    trackColorRoundtrip(mixxx::RgbColor(0xDDFF99), mixxx::RgbColor::optional(0x779933));
    trackColorRoundtrip(mixxx::RgbColor(0xBBFF99), mixxx::RgbColor::optional(0x559933));
    trackColorRoundtrip(mixxx::RgbColor(0x99FF99), mixxx::RgbColor::optional(0x339933));
    trackColorRoundtrip(mixxx::RgbColor(0x99FFBB), mixxx::RgbColor::optional(0x339955));
    trackColorRoundtrip(mixxx::RgbColor(0x99FFDD), mixxx::RgbColor::optional(0x339977));
    trackColorRoundtrip(mixxx::RgbColor(0x99FFFF), mixxx::RgbColor::optional(0x339999));
    trackColorRoundtrip(mixxx::RgbColor(0x99DDFF), mixxx::RgbColor::optional(0x337799));
    trackColorRoundtrip(mixxx::RgbColor(0x99BBFF), mixxx::RgbColor::optional(0x335599));
    trackColorRoundtrip(mixxx::RgbColor(0x9999FF), mixxx::RgbColor::optional(0x333399));
    trackColorRoundtrip(mixxx::RgbColor(0xBB99FF), mixxx::RgbColor::optional(0x553399));
    trackColorRoundtrip(mixxx::RgbColor(0xDD99FF), mixxx::RgbColor::optional(0x773399));
    trackColorRoundtrip(mixxx::RgbColor(0x000000), mixxx::RgbColor::optional(0x333333));
    trackColorRoundtrip(mixxx::RgbColor(0xBBBBBB), mixxx::RgbColor::optional(0x555555));
    trackColorRoundtrip(mixxx::RgbColor(0x999999), mixxx::RgbColor::optional(0x090909));
    trackColorRoundtrip(mixxx::RgbColor(0xFFFFFF), std::nullopt);
}

} // namespace
