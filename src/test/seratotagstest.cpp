#include <gtest/gtest.h>

#include <QDir>
#include <QFile>

#include "track/serato/tags.h"
#include "util/color/predefinedcolorpalettes.h"

namespace {

bool dumpToFile(const QString& filename, const QByteArray& data) {
    QFile outfile(filename);
    const bool openOk = outfile.open(QIODevice::WriteOnly);
    if (!openOk) {
        return false;
    }
    return data.size() == outfile.write(data);
}

} // namespace

class SeratoTagsTest : public testing::Test {
  protected:
    void trackColorRoundtrip(const mixxx::RgbColor::optional_t& displayedColor) {
        mixxx::RgbColor storedColor =
                mixxx::SeratoTags::displayedToStoredTrackColor(displayedColor);
        mixxx::RgbColor::optional_t actualDisplayedColor = mixxx::SeratoTags::storedToDisplayedTrackColor(storedColor);
        EXPECT_EQ(displayedColor, actualDisplayedColor);
    }
    void trackColorRoundtripWithKnownStoredColor(
            const mixxx::RgbColor::optional_t& displayedColor,
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

TEST_F(SeratoTagsTest, SetCueInfos) {
    mixxx::SeratoTags seratoTags;
    EXPECT_TRUE(seratoTags.getCueInfos().isEmpty());

    // To be able to compare the whole list easily, the reference CueInfo list
    // needs to be in a specific order:
    //
    //    1. Cue 1
    //    2. Cue 2
    //       ...
    //    5. Cue 5
    //    6. Loop 1
    //       ...
    //   14. Loop 9
    const QList<mixxx::CueInfo> cueInfos = {
            mixxx::CueInfo(mixxx::CueType::HotCue,
                    0,
                    std::nullopt,
                    0,
                    QString(),
                    mixxx::RgbColor(0xFF0000)),
            mixxx::CueInfo(mixxx::CueType::HotCue,
                    1337,
                    std::nullopt,
                    2,
                    QStringLiteral("Hello World!"),
                    mixxx::RgbColor(0x123456)),
            mixxx::CueInfo(mixxx::CueType::HotCue,
                    2500,
                    std::nullopt,
                    3,
                    QStringLiteral("Foo"),
                    mixxx::RgbColor(0x123456)),
            mixxx::CueInfo(mixxx::CueType::HotCue,
                    100,
                    std::nullopt,
                    7,
                    QStringLiteral("Bar"),
                    mixxx::RgbColor(0x123456)),
            mixxx::CueInfo(mixxx::CueType::Loop,
                    1000,
                    2000,
                    8,
                    QString(),
                    std::nullopt),
            mixxx::CueInfo(mixxx::CueType::Loop,
                    4000,
                    2000,
                    11,
                    QStringLiteral("Some Loop"),
                    std::nullopt),
    };

    seratoTags.setCueInfos(cueInfos);
    EXPECT_EQ(seratoTags.getCueInfos().size(), cueInfos.size());
    EXPECT_EQ(seratoTags.getCueInfos(), cueInfos);
}

TEST_F(SeratoTagsTest, CueColorConversionRoundtrip) {
    for (const auto color : mixxx::PredefinedColorPalettes::
                    kSeratoTrackMetadataHotcueColorPalette) {
        const auto displayedColor = mixxx::SeratoTags::storedToDisplayedSeratoDJProCueColor(color);
        const auto storedColor =
                mixxx::SeratoTags::displayedToStoredSeratoDJProCueColor(
                        displayedColor);
        EXPECT_EQ(color, storedColor);
    }

    for (const auto color : mixxx::PredefinedColorPalettes::kSeratoDJProHotcueColorPalette) {
        const auto storedColor = mixxx::SeratoTags::displayedToStoredSeratoDJProCueColor(color);
        const auto displayedColor =
                mixxx::SeratoTags::storedToDisplayedSeratoDJProCueColor(
                        storedColor);
        EXPECT_EQ(color, displayedColor);
    }
}

TEST_F(SeratoTagsTest, MarkersParseDumpRoundtrip) {
    const auto filetype = mixxx::taglib::FileType::MP3;
    QDir dir(QStringLiteral("src/test/serato/data/mp3/markers_/"));
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << "*.octet-stream");
    const QFileInfoList fileList = dir.entryInfoList();
    for (const QFileInfo& fileInfo : fileList) {
        mixxx::SeratoTags seratoTags;
        EXPECT_TRUE(seratoTags.getCueInfos().isEmpty());

        auto file = QFile(fileInfo.filePath());
        const bool openOk = file.open(QIODevice::ReadOnly);
        EXPECT_TRUE(openOk);
        const QByteArray inputData = file.readAll();
        const bool parseOk = seratoTags.parseMarkers(inputData, filetype);
        EXPECT_TRUE(parseOk);

        const auto trackColor = seratoTags.getTrackColor();
        const auto cueInfos = seratoTags.getCueInfos();
        seratoTags.setTrackColor(trackColor);
        seratoTags.setCueInfos(cueInfos);

        const QByteArray outputData = seratoTags.dumpMarkers(filetype);
        EXPECT_EQ(inputData, outputData);
        if (inputData != outputData) {
            qWarning() << "parsed" << cueInfos;
            EXPECT_TRUE(dumpToFile(
                    file.fileName() + QStringLiteral(".seratotagstest.actual"),
                    outputData));
        }
    }
}

TEST_F(SeratoTagsTest, Markers2RoundTrip) {
    const auto filetype = mixxx::taglib::FileType::MP3;
    QDir dir(QStringLiteral("src/test/serato/data/mp3/markers2/"));
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << "*.octet-stream");
    const QFileInfoList fileList = dir.entryInfoList();
    for (const QFileInfo& fileInfo : fileList) {
        mixxx::SeratoTags seratoTags;
        EXPECT_TRUE(seratoTags.getCueInfos().isEmpty());

        auto file = QFile(fileInfo.filePath());
        const bool openOk = file.open(QIODevice::ReadOnly);
        EXPECT_TRUE(openOk);
        const QByteArray inputData = file.readAll();
        const bool parseOk = seratoTags.parseMarkers2(inputData, filetype);
        EXPECT_TRUE(parseOk);

        const auto bpmLocked = seratoTags.isBpmLocked();
        const auto trackColor = seratoTags.getTrackColor();
        const auto cueInfos = seratoTags.getCueInfos();
        seratoTags.setBpmLocked(bpmLocked);
        seratoTags.setTrackColor(trackColor);
        seratoTags.setCueInfos(cueInfos);

        const QByteArray outputData = seratoTags.dumpMarkers2(filetype);
        EXPECT_EQ(inputData, outputData);
        if (inputData != outputData) {
            qWarning() << "parsed" << cueInfos;
            EXPECT_TRUE(dumpToFile(
                    file.fileName() + QStringLiteral(".seratotagstest.actual"),
                    outputData));
        }
    }
}
