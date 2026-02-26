#include <gtest/gtest.h>

#include <QTemporaryDir>

#include "sources/metadatasourcetaglib.h"
#include "test/mixxxtest.h"
#include "test/soundsourceproviderregistration.h"

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
