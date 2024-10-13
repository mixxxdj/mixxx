#include <gtest/gtest.h>

#include <QDir>
#include <QFile>

#include "test/mixxxtest.h"
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
        mixxx::SeratoStoredTrackColor storedColor =
                mixxx::SeratoStoredTrackColor::fromDisplayedColor(displayedColor);
        mixxx::RgbColor::optional_t actualDisplayedColor = storedColor.toDisplayedColor();
        EXPECT_EQ(displayedColor, actualDisplayedColor);
    }
    void trackColorRoundtripWithKnownStoredColor(
            const mixxx::RgbColor::optional_t& displayedColor,
            mixxx::SeratoStoredTrackColor storedColor) {
        mixxx::SeratoStoredTrackColor actualStoredColor =
                mixxx::SeratoStoredTrackColor::fromDisplayedColor(displayedColor);
        EXPECT_EQ(actualStoredColor, storedColor);

        mixxx::RgbColor::optional_t actualDisplayedColor =
                storedColor.toDisplayedColor();
        EXPECT_EQ(displayedColor, actualDisplayedColor);
    }
};

TEST_F(SeratoTagsTest, TrackColorConversionRoundtripWithKnownStoredColor) {
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x993399), mixxx::SeratoStoredTrackColor(0xFF99FF));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x993377), mixxx::SeratoStoredTrackColor(0xFF99DD));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x993355), mixxx::SeratoStoredTrackColor(0xFF99BB));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x993333), mixxx::SeratoStoredTrackColor(0xFF9999));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x995533), mixxx::SeratoStoredTrackColor(0xFFBB99));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x997733), mixxx::SeratoStoredTrackColor(0xFFDD99));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x999933), mixxx::SeratoStoredTrackColor(0xFFFF99));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x779933), mixxx::SeratoStoredTrackColor(0xDDFF99));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x559933), mixxx::SeratoStoredTrackColor(0xBBFF99));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x339933), mixxx::SeratoStoredTrackColor(0x99FF99));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x339955), mixxx::SeratoStoredTrackColor(0x99FFBB));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x339977), mixxx::SeratoStoredTrackColor(0x99FFDD));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x339999), mixxx::SeratoStoredTrackColor(0x99FFFF));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x337799), mixxx::SeratoStoredTrackColor(0x99DDFF));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x335599), mixxx::SeratoStoredTrackColor(0x99BBFF));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x333399), mixxx::SeratoStoredTrackColor(0x9999FF));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x553399), mixxx::SeratoStoredTrackColor(0xBB99FF));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x773399), mixxx::SeratoStoredTrackColor(0xDD99FF));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x333333), mixxx::SeratoStoredTrackColor(0x000000));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x555555), mixxx::SeratoStoredTrackColor(0xBBBBBB));
    trackColorRoundtripWithKnownStoredColor(
            mixxx::RgbColor::optional(0x090909), mixxx::SeratoStoredTrackColor(0x999999));
    trackColorRoundtripWithKnownStoredColor(
            std::nullopt, mixxx::SeratoStoredTrackColor(0xFFFFFF));
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
    //    0. Cue 0
    //    2. Cue 2
    //    3. Cue 3
    //    7. Cue 7
    //    8. Loop 1
    //   14. Loop 9
    const QList<mixxx::CueInfo> cueInfos = {
            mixxx::CueInfo(mixxx::CueType::HotCue,
                    0,
                    std::nullopt,
                    0,
                    QString(),
                    mixxx::RgbColor(0xFF0000),
                    100,
                    100,
                    100,
                    100),
            mixxx::CueInfo(mixxx::CueType::HotCue,
                    1337,
                    std::nullopt,
                    2,
                    QStringLiteral("Hello World!"),
                    mixxx::RgbColor(0x123456),
                    100,
                    100,
                    100,
                    100),
            mixxx::CueInfo(mixxx::CueType::HotCue,
                    2500,
                    std::nullopt,
                    3,
                    QStringLiteral("Foo"),
                    mixxx::RgbColor(0x123456),
                    100,
                    100,
                    100,
                    100),
            mixxx::CueInfo(mixxx::CueType::HotCue,
                    100,
                    std::nullopt,
                    7,
                    QStringLiteral("Bar"),
                    mixxx::RgbColor(0x123456),
                    100,
                    100,
                    100,
                    100),
            mixxx::CueInfo(mixxx::CueType::Loop,
                    1000,
                    2000,
                    8,
                    QString(),
                    std::nullopt,
                    100,
                    100,
                    100,
                    100),
            mixxx::CueInfo(mixxx::CueType::Loop,
                    4000,
                    2000,
                    11,
                    QStringLiteral("Some Loop"),
                    std::nullopt,
                    100,
                    100,
                    100,
                    100),
    };

    seratoTags.setCueInfos(cueInfos);
    EXPECT_EQ(seratoTags.getCueInfos().size(), cueInfos.size());
    EXPECT_EQ(seratoTags.getCueInfos(), cueInfos);
}

TEST_F(SeratoTagsTest, SetIncompatibleCueInfos) {
    mixxx::SeratoTags seratoTags;
    EXPECT_TRUE(seratoTags.getCueInfos().isEmpty());

    // These are invalid entries or entries not expressible in Serato
    const QList<mixxx::CueInfo> cueInfos = {
            // Invalid Loop Index
            mixxx::CueInfo(mixxx::CueType::Loop,
                    4444,
                    5555,
                    -2,
                    QStringLiteral("Hello World!"),
                    mixxx::RgbColor(0x123456),
                    100,
                    100,
                    100,
                    100),
            // Invalid HotCue Index
            mixxx::CueInfo(mixxx::CueType::HotCue,
                    100,
                    std::nullopt,
                    -1,
                    QStringLiteral("Bar"),
                    mixxx::RgbColor(0x123456),
                    100,
                    100,
                    100,
                    100),
            // Hotcue without a start position
            mixxx::CueInfo(mixxx::CueType::HotCue,
                    std::nullopt,
                    std::nullopt,
                    0,
                    QString(),
                    mixxx::RgbColor(0xFF0000),
                    100,
                    100,
                    100,
                    100),
            // Incompatible cue type
            mixxx::CueInfo(mixxx::CueType::MainCue,
                    100,
                    std::nullopt,
                    -1,
                    QStringLiteral("Bar"),
                    mixxx::RgbColor(0x123456),
                    100,
                    100,
                    100,
                    100),
            // Loop at in the HotCue bank
            mixxx::CueInfo(mixxx::CueType::Loop,
                    1337,
                    2222,
                    2,
                    QStringLiteral("Hello World!"),
                    mixxx::RgbColor(0x123456),
                    100,
                    100,
                    100,
                    100),
            // HotCue in the Loop bank
            mixxx::CueInfo(mixxx::CueType::HotCue,
                    2500,
                    std::nullopt,
                    8,
                    QStringLiteral("Foo"),
                    mixxx::RgbColor(0x123456),
                    100,
                    100,
                    100,
                    100),
            // Hotcue with out of range Index
            mixxx::CueInfo(mixxx::CueType::Loop,
                    4000,
                    2000,
                    16,
                    QStringLiteral("Some Loop"),
                    std::nullopt,
                    100,
                    100,
                    100,
                    100),
            // Hotcue with out of range Index
            mixxx::CueInfo(mixxx::CueType::HotCue,
                    100,
                    std::nullopt,
                    17,
                    QStringLiteral("Bar"),
                    mixxx::RgbColor(0x123456),
                    100,
                    100,
                    100,
                    100),
            // Hotcue without an index
            mixxx::CueInfo(mixxx::CueType::HotCue,
                    1,
                    std::nullopt,
                    std::nullopt,
                    QString(),
                    mixxx::RgbColor(0xFF0000),
                    100,
                    100,
                    100,
                    100),
    };

    seratoTags.setCueInfos(cueInfos);
    EXPECT_EQ(seratoTags.getCueInfos().size(), 0);
}

TEST_F(SeratoTagsTest, CueColorConversionRoundtrip) {
    for (const auto color : mixxx::PredefinedColorPalettes::
                    kSeratoTrackMetadataHotcueColorPalette) {
        const auto displayedColor = mixxx::SeratoStoredHotcueColor(color).toDisplayedColor();
        const auto storedColor = mixxx::SeratoStoredHotcueColor::fromDisplayedColor(displayedColor);
        EXPECT_EQ(color, storedColor.toQRgb());
    }

    for (const auto color : mixxx::PredefinedColorPalettes::kSeratoDJProHotcueColorPalette) {
        const auto storedColor = mixxx::SeratoStoredHotcueColor::fromDisplayedColor(color);
        const auto displayedColor = storedColor.toDisplayedColor();
        EXPECT_EQ(color, displayedColor);
    }
}

TEST_F(SeratoTagsTest, MarkersParseDumpRoundtrip) {
    const auto filetype = mixxx::taglib::FileType::MPEG;
    QDir dir(MixxxTest::getOrInitTestDir().filePath(QStringLiteral("/serato/data/mp3/markers_/")));
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
        if (trackColor) {
            seratoTags.setTrackColor(*trackColor);
        }
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
    const auto filetype = mixxx::taglib::FileType::MPEG;
    QDir dir(MixxxTest::getOrInitTestDir().filePath(QStringLiteral("serato/data/mp3/markers2/")));
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
        if (trackColor) {
            seratoTags.setTrackColor(*trackColor);
        }
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
