#include <gtest/gtest.h>
#include <mpegfile.h>
#include <popularimeterframe.h>

#include <QTemporaryDir>
#include <limits>

#include "sources/metadatasourcetaglib.h"
#include "test/mixxxtest.h"
#include "test/soundsourceproviderregistration.h"
#include "track/taglib/fmpsplaycount.h"

namespace {

struct PlayCountFormatTestParam {
    const char* fixture;
    const char* fileType;
    const char* extension;
};

const PlayCountFormatTestParam kPlayCountFormatTestParams[] = {
        {"empty.mp3", "mp3", "mp3"},
        {"cover-test-øé~ł€˚.flac", "flac", "flac"},
        {"cover-test-øé~ł€˚.ogg", "ogg", "ogg"},
        {"cover-test-øé~ł€˚.opus", "opus", "opus"},
        {"cover-test-øé~ł€˚-ffmpeg-aac.m4a", "m4a", "m4a"},
        {"cover-test-øé~ł€˚.wav", "wav", "wav"},
        {"cover-test-øé~ł€˚.aiff", "aiff", "aiff"},
        {"cover-test-øé~ł€˚.wv", "wv", "wv"},
};

} // namespace

class PlayCountExportImportTest : public MixxxTest,
                                  private SoundSourceProviderRegistration {
  public:
    PlayCountExportImportTest()
            : m_testDataDir(getTestDir().absoluteFilePath(
                      QStringLiteral("id3-test-data"))) {
    }

  protected:
    QString copyEmptyMp3(const QString& fileName) const {
        const QString dstPath = m_tempDir.filePath(fileName);
        mixxxtest::copyFile(
                m_testDataDir.absoluteFilePath(QStringLiteral("empty.mp3")),
                dstPath);
        return dstPath;
    }

    const QDir m_testDataDir;
    QTemporaryDir m_tempDir;
};

class PlayCountExportImportFormatTest
        : public PlayCountExportImportTest,
          public ::testing::WithParamInterface<PlayCountFormatTestParam> {};

TEST_P(PlayCountExportImportFormatTest, RoundTrip) {
    const auto& param = GetParam();
    const QString srcPath = m_testDataDir.absoluteFilePath(
            QString::fromUtf8(param.fixture));
    const QString dstPath = m_tempDir.filePath(
            QStringLiteral("playcount_test.") +
            QString::fromLatin1(param.extension));
    mixxxtest::copyFile(srcPath, dstPath);

    // No play count before the first export
    {
        mixxx::MetadataSourceTagLib source(dstPath, QString::fromLatin1(param.fileType));
        EXPECT_FALSE(source.importPlayCount().has_value());
    }

    for (const int playCount : {1, 42, 2306}) {
        {
            mixxx::MetadataSourceTagLib source(dstPath, QString::fromLatin1(param.fileType));
            ASSERT_TRUE(source.exportPlayCount(playCount));
        }
        {
            mixxx::MetadataSourceTagLib source(dstPath, QString::fromLatin1(param.fileType));
            const auto imported = source.importPlayCount();
            ASSERT_TRUE(imported.has_value());
            EXPECT_EQ(imported.value(), playCount);
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
        AllFormats,
        PlayCountExportImportFormatTest,
        ::testing::ValuesIn(kPlayCountFormatTestParams),
        [](const ::testing::TestParamInfo<PlayCountFormatTestParam>& info) {
            return std::string(info.param.fileType);
        });

TEST_F(PlayCountExportImportTest, ParseFmpsPlayCount) {
    using mixxx::taglib::parseFmpsPlayCount;
    EXPECT_EQ(parseFmpsPlayCount(QStringLiteral("0")).value_or(-1), 0);
    EXPECT_EQ(parseFmpsPlayCount(QStringLiteral("1")).value_or(-1), 1);
    EXPECT_EQ(parseFmpsPlayCount(QStringLiteral(" 17 ")).value_or(-1), 17);
    // Fractional values are legal according to the specification
    EXPECT_EQ(parseFmpsPlayCount(QStringLiteral("3.0")).value_or(-1), 3);
    EXPECT_EQ(parseFmpsPlayCount(QStringLiteral("3.9")).value_or(-1), 3);
    // Values beyond the range of int are clamped
    EXPECT_EQ(parseFmpsPlayCount(QStringLiteral("99999999999999999999")).value_or(-1),
            std::numeric_limits<int>::max());
}

TEST_F(PlayCountExportImportTest, ParseFmpsPlayCountRejectsMalformedValues) {
    using mixxx::taglib::parseFmpsPlayCount;
    EXPECT_FALSE(parseFmpsPlayCount(QString()).has_value());
    EXPECT_FALSE(parseFmpsPlayCount(QStringLiteral("-1")).has_value());
    EXPECT_FALSE(parseFmpsPlayCount(QStringLiteral("abc")).has_value());
    EXPECT_FALSE(parseFmpsPlayCount(QStringLiteral("nan")).has_value());
    EXPECT_FALSE(parseFmpsPlayCount(QStringLiteral("inf")).has_value());
    EXPECT_FALSE(parseFmpsPlayCount(QStringLiteral("1e3")).has_value());
    EXPECT_FALSE(parseFmpsPlayCount(QStringLiteral("1,5")).has_value());
}

TEST_F(PlayCountExportImportTest, FormatFmpsPlayCount) {
    using mixxx::taglib::formatFmpsPlayCount;
    EXPECT_EQ(formatFmpsPlayCount(0).value_or(QString()), QStringLiteral("0"));
    EXPECT_EQ(formatFmpsPlayCount(12).value_or(QString()), QStringLiteral("12"));
    EXPECT_FALSE(formatFmpsPlayCount(-1).has_value());
}

TEST_F(PlayCountExportImportTest, NegativePlayCountIsNotExported_MP3) {
    const QString dstPath = copyEmptyMp3(QStringLiteral("negative.mp3"));
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        EXPECT_FALSE(source.exportPlayCount(-1));
    }
    mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
    EXPECT_FALSE(source.importPlayCount().has_value());
}

TEST_F(PlayCountExportImportTest, PopmCounterFallback_MP3) {
    const QString dstPath = copyEmptyMp3(QStringLiteral("popm_fallback.mp3"));
    // Seed only a POPM frame as written by another application
    {
        TagLib::MPEG::File file(TAGLIB_FILENAME_FROM_QSTRING(dstPath));
        ASSERT_TRUE(file.isOpen());
        auto* pFrame = new TagLib::ID3v2::PopularimeterFrame();
        pFrame->setEmail("other@app");
        pFrame->setRating(128);
        pFrame->setCounter(7);
        file.ID3v2Tag(true)->addFrame(pFrame);
        ASSERT_TRUE(file.save(TagLib::MPEG::File::ID3v2));
    }
    mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
    EXPECT_EQ(source.importPlayCount().value_or(-1), 7);
}

TEST_F(PlayCountExportImportTest, ExportUpdatesExistingPopmCounter_MP3) {
    const QString dstPath = copyEmptyMp3(QStringLiteral("popm_update.mp3"));
    {
        TagLib::MPEG::File file(TAGLIB_FILENAME_FROM_QSTRING(dstPath));
        ASSERT_TRUE(file.isOpen());
        auto* pFrame = new TagLib::ID3v2::PopularimeterFrame();
        pFrame->setEmail("other@app");
        pFrame->setRating(128);
        pFrame->setCounter(7);
        file.ID3v2Tag(true)->addFrame(pFrame);
        ASSERT_TRUE(file.save(TagLib::MPEG::File::ID3v2));
    }
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        ASSERT_TRUE(source.exportPlayCount(9));
    }
    {
        TagLib::MPEG::File file(TAGLIB_FILENAME_FROM_QSTRING(dstPath));
        ASSERT_TRUE(file.isOpen());
        const auto popmFrames = file.ID3v2Tag()->frameListMap()["POPM"];
        ASSERT_EQ(popmFrames.size(), 1u);
        auto* pFrame = dynamic_cast<TagLib::ID3v2::PopularimeterFrame*>(
                popmFrames.front());
        ASSERT_TRUE(pFrame);
        EXPECT_EQ(pFrame->counter(), 9u);
        // The rating of the other application must be preserved
        EXPECT_EQ(pFrame->rating(), 128);
    }
    mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
    EXPECT_EQ(source.importPlayCount().value_or(-1), 9);
}

TEST_F(PlayCountExportImportTest, ExportDoesNotTouchRating_MP3) {
    const QString dstPath = copyEmptyMp3(QStringLiteral("rating_kept.mp3"));
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        ASSERT_TRUE(source.exportRating(4));
    }
    {
        mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
        ASSERT_TRUE(source.exportPlayCount(5));
    }
    mixxx::MetadataSourceTagLib source(dstPath, QStringLiteral("mp3"));
    EXPECT_EQ(source.importRating().value_or(-1), 4);
    EXPECT_EQ(source.importPlayCount().value_or(-1), 5);
}
