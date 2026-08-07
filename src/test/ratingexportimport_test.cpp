#include <apeitem.h>
#include <apetag.h>
#include <flacfile.h>
#include <gtest/gtest.h>
#include <id3v1tag.h>
#include <mpegfile.h>
#include <popularimeterframe.h>
#include <textidentificationframe.h>

#include <QTemporaryDir>

#include "sources/metadatasourcetaglib.h"
#include "test/mixxxtest.h"
#include "test/soundsourceproviderregistration.h"
#include "track/taglib/fmpsrating.h"

namespace {

struct FormatTestParam {
    const char* fixture;
    const char* fileType;
    const char* extension;
};

const FormatTestParam kFormatTestParams[] = {
        {"empty.mp3", "mp3", "mp3"},
        {"cover-test.flac", "flac", "flac"},
        {"cover-test.ogg", "ogg", "ogg"},
        {"cover-test.opus", "opus", "opus"},
        {"cover-test-ffmpeg-aac.m4a", "m4a", "m4a"},
        {"cover-test.wav", "wav", "wav"},
        {"cover-test.aiff", "aiff", "aiff"},
        {"cover-test.wv", "wv", "wv"},
};

} // namespace

class RatingExportImportTest : public MixxxTest,
                               private SoundSourceProviderRegistration {
  public:
    RatingExportImportTest()
            : m_testDataDir(getTestDir().absoluteFilePath(
                      QStringLiteral("id3-test-data"))) {
    }

  protected:
    const QDir m_testDataDir;
    QTemporaryDir m_tempDir;
};

class RatingExportImportFormatTest
        : public RatingExportImportTest,
          public ::testing::WithParamInterface<FormatTestParam> {};

TEST_P(RatingExportImportFormatTest, RoundTrip) {
    const auto& param = GetParam();
    const QString srcPath = m_testDataDir.absoluteFilePath(
            QString::fromLatin1(param.fixture));
    const QString dstPath = m_tempDir.filePath(
            QStringLiteral("rating_test.") +
            QString::fromLatin1(param.extension));
    mixxxtest::copyFile(srcPath, dstPath);

    const int kTestRating = 3;

    // Export
    {
        mixxx::MetadataSourceTagLib source(dstPath, QString::fromLatin1(param.fileType));
        ASSERT_TRUE(source.exportRating(kTestRating));
    }

    // Import
    {
        mixxx::MetadataSourceTagLib source(dstPath, QString::fromLatin1(param.fileType));
        const auto rating = source.importRating();
        ASSERT_TRUE(rating.has_value());
        EXPECT_EQ(rating.value(), kTestRating);
    }
}

INSTANTIATE_TEST_SUITE_P(
        AllFormats,
        RatingExportImportFormatTest,
        ::testing::ValuesIn(kFormatTestParams),
        [](const ::testing::TestParamInfo<FormatTestParam>& info) {
            return std::string(info.param.fileType);
        });

TEST_F(RatingExportImportTest, AllRatingValues_MP3) {
    for (int rating = 1; rating <= 5; ++rating) {
        const QString dstPath = m_tempDir.filePath(
                QStringLiteral("rating_%1.mp3").arg(rating));
        mixxxtest::copyFile(
                m_testDataDir.absoluteFilePath(QStringLiteral("empty.mp3")),
                dstPath);

        {
            mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
            ASSERT_TRUE(source.exportRating(rating))
                    << "Failed to export rating " << rating;
        }

        {
            mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
            const auto imported = source.importRating();
            ASSERT_TRUE(imported.has_value())
                    << "Failed to import rating " << rating;
            EXPECT_EQ(imported.value(), rating)
                    << "Rating mismatch for value " << rating;
        }
    }
}

TEST_F(RatingExportImportTest, ClearRating_MP3) {
    const QString dstPath = m_tempDir.filePath(QStringLiteral("clear_rating.mp3"));
    mixxxtest::copyFile(
            m_testDataDir.absoluteFilePath(QStringLiteral("empty.mp3")),
            dstPath);

    // First export a non-zero rating
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        ASSERT_TRUE(source.exportRating(4));
    }

    // Now clear it by exporting 0
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        ASSERT_TRUE(source.exportRating(0));
    }

    // Import should return nullopt (no rating)
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        const auto rating = source.importRating();
        EXPECT_FALSE(rating.has_value());
    }
}

TEST_F(RatingExportImportTest, NoRatingInitially_MP3) {
    const QString dstPath = m_tempDir.filePath(QStringLiteral("no_rating.mp3"));
    mixxxtest::copyFile(
            m_testDataDir.absoluteFilePath(QStringLiteral("empty.mp3")),
            dstPath);

    // A file with no FMPS_Rating tag should return nullopt
    mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
    const auto rating = source.importRating();
    EXPECT_FALSE(rating.has_value());
}

TEST_F(RatingExportImportTest, ParseFmpsRating) {
    // Exported values round-trip to the exact star rating
    EXPECT_EQ(mixxx::taglib::parseFmpsRating(QStringLiteral("0.2")), 1);
    EXPECT_EQ(mixxx::taglib::parseFmpsRating(QStringLiteral("0.4")), 2);
    EXPECT_EQ(mixxx::taglib::parseFmpsRating(QStringLiteral("0.6")), 3);
    EXPECT_EQ(mixxx::taglib::parseFmpsRating(QStringLiteral("0.8")), 4);
    EXPECT_EQ(mixxx::taglib::parseFmpsRating(QStringLiteral("1.0")), 5);
    // Explicit 0.0 is a valid FMPS value meaning "unrated"
    EXPECT_EQ(mixxx::taglib::parseFmpsRating(QStringLiteral("0.0")), 0);
    EXPECT_EQ(mixxx::taglib::parseFmpsRating(QStringLiteral("0.05")), 0);
    // Values from other applications fall into the nearest band
    EXPECT_EQ(mixxx::taglib::parseFmpsRating(QStringLiteral("0.5")), 3);
    EXPECT_EQ(mixxx::taglib::parseFmpsRating(QStringLiteral("0.9")), 5);
    // Surrounding whitespace is tolerated
    EXPECT_EQ(mixxx::taglib::parseFmpsRating(QStringLiteral(" 0.6 ")), 3);
}

TEST_F(RatingExportImportTest, ParseFmpsRatingRejectsMalformedValues) {
    EXPECT_FALSE(mixxx::taglib::parseFmpsRating(QString()).has_value());
    EXPECT_FALSE(mixxx::taglib::parseFmpsRating(QStringLiteral("abc")).has_value());
    // QString::toDouble() parses "nan" and "inf" successfully, so
    // non-finite values must be rejected explicitly
    EXPECT_FALSE(mixxx::taglib::parseFmpsRating(QStringLiteral("nan")).has_value());
    EXPECT_FALSE(mixxx::taglib::parseFmpsRating(QStringLiteral("inf")).has_value());
    EXPECT_FALSE(mixxx::taglib::parseFmpsRating(QStringLiteral("-inf")).has_value());
    // Out of the FMPS range [0.0, 1.0]
    EXPECT_FALSE(mixxx::taglib::parseFmpsRating(QStringLiteral("-0.1")).has_value());
    EXPECT_FALSE(mixxx::taglib::parseFmpsRating(QStringLiteral("1.5")).has_value());
    EXPECT_FALSE(mixxx::taglib::parseFmpsRating(QStringLiteral("5")).has_value());
    // Locale-specific decimal separators are not part of the FMPS format
    EXPECT_FALSE(mixxx::taglib::parseFmpsRating(QStringLiteral("0,5")).has_value());
}

TEST_F(RatingExportImportTest, FormatFmpsRating) {
    EXPECT_EQ(mixxx::taglib::formatFmpsRating(1), QStringLiteral("0.2"));
    EXPECT_EQ(mixxx::taglib::formatFmpsRating(2), QStringLiteral("0.4"));
    EXPECT_EQ(mixxx::taglib::formatFmpsRating(3), QStringLiteral("0.6"));
    EXPECT_EQ(mixxx::taglib::formatFmpsRating(4), QStringLiteral("0.8"));
    EXPECT_EQ(mixxx::taglib::formatFmpsRating(5), QStringLiteral("1.0"));
    // kNoRating and out-of-range values cannot be formatted
    EXPECT_FALSE(mixxx::taglib::formatFmpsRating(0).has_value());
    EXPECT_FALSE(mixxx::taglib::formatFmpsRating(-1).has_value());
    EXPECT_FALSE(mixxx::taglib::formatFmpsRating(6).has_value());
}

TEST_F(RatingExportImportTest, PopmFromRating) {
    // Byte values written by Windows Media Player and kid3
    EXPECT_EQ(mixxx::taglib::popmFromRating(1), 1);
    EXPECT_EQ(mixxx::taglib::popmFromRating(2), 64);
    EXPECT_EQ(mixxx::taglib::popmFromRating(3), 128);
    EXPECT_EQ(mixxx::taglib::popmFromRating(4), 196);
    EXPECT_EQ(mixxx::taglib::popmFromRating(5), 255);
    EXPECT_EQ(mixxx::taglib::popmFromRating(0), 0);
    EXPECT_EQ(mixxx::taglib::popmFromRating(-1), 0);
    EXPECT_EQ(mixxx::taglib::popmFromRating(6), 0);
    // The values must round-trip through the import bands
    for (int rating = 1; rating <= 5; ++rating) {
        EXPECT_EQ(mixxx::taglib::ratingFromPopm(
                          mixxx::taglib::popmFromRating(rating)),
                rating);
    }
}

TEST_F(RatingExportImportTest, RatingFromPopm) {
    // POPM ratings originate from a single byte (0-255), but guard
    // the full int domain of TagLib's rating() accessor
    EXPECT_EQ(mixxx::taglib::ratingFromPopm(-1), 0);
    EXPECT_EQ(mixxx::taglib::ratingFromPopm(0), 0);
    EXPECT_EQ(mixxx::taglib::ratingFromPopm(1), 1);
    EXPECT_EQ(mixxx::taglib::ratingFromPopm(31), 1);
    EXPECT_EQ(mixxx::taglib::ratingFromPopm(32), 2);
    EXPECT_EQ(mixxx::taglib::ratingFromPopm(95), 2);
    EXPECT_EQ(mixxx::taglib::ratingFromPopm(96), 3);
    EXPECT_EQ(mixxx::taglib::ratingFromPopm(159), 3);
    EXPECT_EQ(mixxx::taglib::ratingFromPopm(160), 4);
    EXPECT_EQ(mixxx::taglib::ratingFromPopm(223), 4);
    EXPECT_EQ(mixxx::taglib::ratingFromPopm(224), 5);
    EXPECT_EQ(mixxx::taglib::ratingFromPopm(255), 5);
    EXPECT_EQ(mixxx::taglib::ratingFromPopm(300), 5);
}

TEST_F(RatingExportImportTest, MalformedFmpsTagValueIgnored_MP3) {
    const char* kMalformedValues[] = {"abc", "nan", "inf", "-0.1", "1.5", "0,5"};
    for (const char* malformedValue : kMalformedValues) {
        const QString dstPath = m_tempDir.filePath(
                QStringLiteral("malformed_%1.mp3")
                        .arg(QString::fromLatin1(malformedValue).replace(
                                QLatin1Char(','), QLatin1Char('_'))));
        mixxxtest::copyFile(
                m_testDataDir.absoluteFilePath(QStringLiteral("empty.mp3")),
                dstPath);

        // Write a malformed FMPS_Rating TXXX frame directly with TagLib
        {
            TagLib::MPEG::File file(TAGLIB_FILENAME_FROM_QSTRING(dstPath));
            ASSERT_TRUE(file.isOpen());
            auto* pFrame = new TagLib::ID3v2::UserTextIdentificationFrame(
                    TagLib::String::UTF8);
            pFrame->setDescription("FMPS_Rating");
            pFrame->setText(malformedValue);
            file.ID3v2Tag(true)->addFrame(pFrame);
            ASSERT_TRUE(file.save(TagLib::MPEG::File::ID3v2));
        }

        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        const auto rating = source.importRating();
        EXPECT_FALSE(rating.has_value())
                << "Malformed value accepted: " << malformedValue;
    }
}

TEST_F(RatingExportImportTest, ExportUpdatesExistingPopm_MP3) {
    const QString dstPath = m_tempDir.filePath(QStringLiteral("popm_update.mp3"));
    mixxxtest::copyFile(
            m_testDataDir.absoluteFilePath(QStringLiteral("empty.mp3")),
            dstPath);

    // Seed a POPM frame as written by another application
    {
        TagLib::MPEG::File file(TAGLIB_FILENAME_FROM_QSTRING(dstPath));
        ASSERT_TRUE(file.isOpen());
        auto* pFrame = new TagLib::ID3v2::PopularimeterFrame();
        pFrame->setEmail("other@app");
        pFrame->setRating(255);
        pFrame->setCounter(7);
        file.ID3v2Tag(true)->addFrame(pFrame);
        ASSERT_TRUE(file.save(TagLib::MPEG::File::ID3v2));
    }

    // Exporting a different rating must keep the POPM frame in sync,
    // otherwise the two tags contradict each other
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        ASSERT_TRUE(source.exportRating(2));
    }
    {
        TagLib::MPEG::File file(TAGLIB_FILENAME_FROM_QSTRING(dstPath));
        ASSERT_TRUE(file.isOpen());
        const auto popmFrames = file.ID3v2Tag()->frameListMap()["POPM"];
        ASSERT_EQ(popmFrames.size(), 1u);
        auto* pFrame = dynamic_cast<TagLib::ID3v2::PopularimeterFrame*>(
                popmFrames.front());
        ASSERT_TRUE(pFrame);
        EXPECT_EQ(pFrame->rating(), 64);
        // The play counter of the other application must be preserved
        EXPECT_EQ(pFrame->counter(), 7u);
    }
}

TEST_F(RatingExportImportTest, ClearRatingZeroesPopm_MP3) {
    const QString dstPath = m_tempDir.filePath(QStringLiteral("popm_clear.mp3"));
    mixxxtest::copyFile(
            m_testDataDir.absoluteFilePath(QStringLiteral("empty.mp3")),
            dstPath);

    // Seed a POPM frame as written by another application
    {
        TagLib::MPEG::File file(TAGLIB_FILENAME_FROM_QSTRING(dstPath));
        ASSERT_TRUE(file.isOpen());
        auto* pFrame = new TagLib::ID3v2::PopularimeterFrame();
        pFrame->setEmail("other@app");
        pFrame->setRating(196);
        pFrame->setCounter(3);
        file.ID3v2Tag(true)->addFrame(pFrame);
        ASSERT_TRUE(file.save(TagLib::MPEG::File::ID3v2));
    }

    // Clearing must also zero the POPM rating, otherwise the cleared
    // rating would be resurrected via the POPM import fallback
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        ASSERT_TRUE(source.exportRating(0));
    }
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        const auto rating = source.importRating();
        // A zeroed POPM frame reads back as explicitly unrated
        ASSERT_TRUE(rating.has_value());
        EXPECT_EQ(rating.value(), 0);
    }
    {
        TagLib::MPEG::File file(TAGLIB_FILENAME_FROM_QSTRING(dstPath));
        ASSERT_TRUE(file.isOpen());
        const auto popmFrames = file.ID3v2Tag()->frameListMap()["POPM"];
        ASSERT_EQ(popmFrames.size(), 1u);
        auto* pFrame = dynamic_cast<TagLib::ID3v2::PopularimeterFrame*>(
                popmFrames.front());
        ASSERT_TRUE(pFrame);
        EXPECT_EQ(pFrame->rating(), 0);
        EXPECT_EQ(pFrame->counter(), 3u);
    }
}

TEST_F(RatingExportImportTest, ExportPreservesOtherMpegTags) {
    const QString dstPath = m_tempDir.filePath(QStringLiteral("preserve_tags.mp3"));
    mixxxtest::copyFile(
            m_testDataDir.absoluteFilePath(QStringLiteral("empty.mp3")),
            dstPath);

    // Seed an ID3v1 tag and an APE item as left behind by other tools
    // (e.g. mp3gain)
    {
        TagLib::MPEG::File file(TAGLIB_FILENAME_FROM_QSTRING(dstPath));
        ASSERT_TRUE(file.isOpen());
        file.ID3v1Tag(true)->setTitle("Keep Me");
        file.APETag(true)->setItem("MP3GAIN_MINMAX",
                TagLib::APE::Item("MP3GAIN_MINMAX", TagLib::String("052,209")));
        ASSERT_TRUE(file.save(TagLib::MPEG::File::ID3v1 | TagLib::MPEG::File::APE,
                TagLib::File::StripTags::StripNone));
    }

    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        ASSERT_TRUE(source.exportRating(3));
    }

    // Exporting a rating must not strip the other tags from the file
    {
        TagLib::MPEG::File file(TAGLIB_FILENAME_FROM_QSTRING(dstPath));
        ASSERT_TRUE(file.isOpen());
        EXPECT_TRUE(file.hasID3v1Tag());
        ASSERT_TRUE(file.hasAPETag());
        EXPECT_TRUE(file.APETag()->itemListMap().contains("MP3GAIN_MINMAX"));
    }
}

TEST_F(RatingExportImportTest, ExportKeepsMpegApeRatingInSync) {
    const QString dstPath = m_tempDir.filePath(QStringLiteral("ape_sync.mp3"));
    mixxxtest::copyFile(
            m_testDataDir.absoluteFilePath(QStringLiteral("empty.mp3")),
            dstPath);

    // Seed an APE FMPS_RATING as written by another application
    {
        TagLib::MPEG::File file(TAGLIB_FILENAME_FROM_QSTRING(dstPath));
        ASSERT_TRUE(file.isOpen());
        file.APETag(true)->setItem("FMPS_RATING",
                TagLib::APE::Item("FMPS_RATING", TagLib::String("0.8")));
        ASSERT_TRUE(file.save(TagLib::MPEG::File::APE,
                TagLib::File::StripTags::StripNone));
    }

    // Clearing the rating must also clear the APE item, otherwise the
    // old rating is resurrected via the APE import fallback
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        ASSERT_TRUE(source.exportRating(0));
    }
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        const auto rating = source.importRating();
        EXPECT_NE(rating.value_or(0), 4);
    }
}

TEST_F(RatingExportImportTest, ExportKeepsFlacId3v2RatingInSync) {
    const QString dstPath = m_tempDir.filePath(QStringLiteral("id3v2_sync.flac"));
    mixxxtest::copyFile(
            m_testDataDir.absoluteFilePath(QStringLiteral("cover-test.flac")),
            dstPath);

    // Seed an ID3v2 FMPS_Rating as written by another application
    {
        TagLib::FLAC::File file(TAGLIB_FILENAME_FROM_QSTRING(dstPath));
        ASSERT_TRUE(file.isOpen());
        auto* pFrame = new TagLib::ID3v2::UserTextIdentificationFrame(
                TagLib::String::UTF8);
        pFrame->setDescription("FMPS_Rating");
        pFrame->setText("1.0");
        file.ID3v2Tag(true)->addFrame(pFrame);
        ASSERT_TRUE(file.save());
    }

    // The import prefers ID3v2 over the Xiph comment for FLAC, so the
    // export must keep the ID3v2 value in sync
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("flac"));
        ASSERT_TRUE(source.exportRating(3));
    }
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("flac"));
        const auto rating = source.importRating();
        ASSERT_TRUE(rating.has_value());
        EXPECT_EQ(rating.value(), 3);
    }

    // Clearing must clear both locations
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("flac"));
        ASSERT_TRUE(source.exportRating(0));
    }
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("flac"));
        const auto rating = source.importRating();
        EXPECT_EQ(rating.value_or(0), 0);
    }
}

TEST_F(RatingExportImportTest, PopmFallbackBoundaries_MP3) {
    const struct {
        int popm;
        int expectedRating;
    } kPopmTestCases[] = {
            {1, 1},
            {31, 1},
            {32, 2},
            {95, 2},
            {96, 3},
            {159, 3},
            {160, 4},
            {223, 4},
            {224, 5},
            {255, 5},
    };
    for (const auto& testCase : kPopmTestCases) {
        const QString dstPath = m_tempDir.filePath(
                QStringLiteral("popm_%1.mp3").arg(testCase.popm));
        mixxxtest::copyFile(
                m_testDataDir.absoluteFilePath(QStringLiteral("empty.mp3")),
                dstPath);

        // Write a POPM frame (no FMPS_Rating) directly with TagLib
        {
            TagLib::MPEG::File file(TAGLIB_FILENAME_FROM_QSTRING(dstPath));
            ASSERT_TRUE(file.isOpen());
            auto* pFrame = new TagLib::ID3v2::PopularimeterFrame();
            pFrame->setEmail("no@email");
            pFrame->setRating(testCase.popm);
            file.ID3v2Tag(true)->addFrame(pFrame);
            ASSERT_TRUE(file.save(TagLib::MPEG::File::ID3v2));
        }

        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        const auto rating = source.importRating();
        ASSERT_TRUE(rating.has_value())
                << "No rating imported for POPM " << testCase.popm;
        EXPECT_EQ(rating.value(), testCase.expectedRating)
                << "Wrong rating for POPM " << testCase.popm;
    }
}
