#include <gtest/gtest.h>

#include "track/serato/tags.h"

class SeratoTagsTest : public testing::Test {
  protected:
    void trackColorRoundtrip(mixxx::RgbColor::optional_t displayedColor) {
        mixxx::RgbColor storedColor =
                mixxx::SeratoTags::displayedToStoredTrackColor(displayedColor);
        mixxx::RgbColor::optional_t actualDisplayedColor = mixxx::SeratoTags::storedToDisplayedTrackColor(storedColor);
        EXPECT_EQ(displayedColor, actualDisplayedColor);
    }
    void trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional_t displayedColor,
            mixxx::RgbColor storedColor) {
        mixxx::RgbColor actualStoredColor =
                mixxx::SeratoTags::displayedToStoredTrackColor(displayedColor);
        EXPECT_EQ(actualStoredColor, storedColor);

        mixxx::RgbColor::optional_t actualDisplayedColor =
                mixxx::SeratoTags::storedToDisplayedTrackColor(storedColor);
        EXPECT_EQ(displayedColor, actualDisplayedColor);
    }
};

TEST_F(SeratoTagsTest, TrackColorConversionRoundtripWithKnownStoredColor) {
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x993399), mixxx::RgbColor(0xFF99FF));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x993377), mixxx::RgbColor(0xFF99DD));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x993355), mixxx::RgbColor(0xFF99BB));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x993333), mixxx::RgbColor(0xFF9999));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x995533), mixxx::RgbColor(0xFFBB99));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x997733), mixxx::RgbColor(0xFFDD99));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x999933), mixxx::RgbColor(0xFFFF99));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x779933), mixxx::RgbColor(0xDDFF99));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x559933), mixxx::RgbColor(0xBBFF99));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x339933), mixxx::RgbColor(0x99FF99));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x339955), mixxx::RgbColor(0x99FFBB));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x339977), mixxx::RgbColor(0x99FFDD));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x339999), mixxx::RgbColor(0x99FFFF));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x337799), mixxx::RgbColor(0x99DDFF));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x335599), mixxx::RgbColor(0x99BBFF));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x333399), mixxx::RgbColor(0x9999FF));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x553399), mixxx::RgbColor(0xBB99FF));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x773399), mixxx::RgbColor(0xDD99FF));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x333333), mixxx::RgbColor(0x000000));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x555555), mixxx::RgbColor(0xBBBBBB));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x090909), mixxx::RgbColor(0x999999));
    trackColorRoundtripWithKnownStoredColor(std::nullopt, mixxx::RgbColor(0xFFFFFF));
}

TEST_F(SeratoTagsTest, TrackColorConversionRoundtrip) {
    trackColorRoundtrip(mixxx::RgbColor::optional(0xFF0000));
    trackColorRoundtrip(mixxx::RgbColor::optional(0x00FF00));
    trackColorRoundtrip(mixxx::RgbColor::optional(0x0000FF));
    trackColorRoundtrip(mixxx::RgbColor::optional(0xFFFF00));
    trackColorRoundtrip(mixxx::RgbColor::optional(0x00FFFF));
    trackColorRoundtrip(mixxx::RgbColor::optional(0xFF00FF));
    trackColorRoundtrip(mixxx::RgbColor::optional(0xFFFFFF));
    trackColorRoundtrip(mixxx::RgbColor::optional(0x000000));
}

TEST_F(SeratoTagsTest, SetTrackColor) {
    mixxx::SeratoTags seratoTags;
    EXPECT_EQ(seratoTags.getTrackColor(), std::nullopt);

    constexpr mixxx::RgbColor color1(0xFF8800);
    seratoTags.setTrackColor(color1);
    EXPECT_EQ(seratoTags.getTrackColor(), color1);

    constexpr mixxx::RgbColor color2(0x123456);
    seratoTags.setTrackColor(color2);
    EXPECT_EQ(seratoTags.getTrackColor(), color2);
}

TEST_F(SeratoTagsTest, SetBpmLocked) {
    mixxx::SeratoTags seratoTags;
    EXPECT_EQ(seratoTags.isBpmLocked(), false);

    seratoTags.setBpmLocked(true);
    EXPECT_EQ(seratoTags.isBpmLocked(), true);

    seratoTags.setBpmLocked(false);
    EXPECT_EQ(seratoTags.isBpmLocked(), false);
}
