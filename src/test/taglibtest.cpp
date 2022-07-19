#include <QDir>
#include <QtDebug>

#include "sources/metadatasourcetaglib.h"
#include "test/mixxxtest.h"


class TagLibTest : public testing::Test {
};

TEST_F(TagLibTest, WriteID3v2Tag) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    // Generate a file name for the temporary file
    const QString tmpFileName = tempDir.filePath("no_id3v1_mp3");

    // Create the temporary file by copying an existing file
    mixxxtest::copyFile(
            MixxxTest::getOrInitTestDir().filePath(QStringLiteral("id3-test-data/empty.mp3")),
            tmpFileName);

    // Verify that the file has no tags
    {
        TagLib::MPEG::File mpegFile(
                TAGLIB_FILENAME_FROM_QSTRING(tmpFileName));
        EXPECT_FALSE(mixxx::taglib::hasID3v1Tag(mpegFile));
        EXPECT_FALSE(mixxx::taglib::hasID3v2Tag(mpegFile));
        EXPECT_FALSE(mixxx::taglib::hasAPETag(mpegFile));
    }

    qDebug() << "Setting track title";

    // Write metadata -> only an ID3v2 tag should be added
    mixxx::TrackMetadata trackMetadata;
    trackMetadata.refTrackInfo().setTitle(QStringLiteral("title"));
    const auto exported =
            mixxx::MetadataSourceTagLib(
                    tmpFileName, mixxx::taglib::FileType::MP3)
                    .exportTrackMetadata(trackMetadata);
    ASSERT_EQ(mixxx::MetadataSource::ExportResult::Succeeded, exported.first);
    ASSERT_FALSE(exported.second.isNull());

    // Check that the file only has an ID3v2 tag after writing metadata
    {
        TagLib::MPEG::File mpegFile(
                TAGLIB_FILENAME_FROM_QSTRING(tmpFileName));
        EXPECT_FALSE(mixxx::taglib::hasID3v1Tag(mpegFile));
        EXPECT_TRUE(mixxx::taglib::hasID3v2Tag(mpegFile));
        EXPECT_FALSE(mixxx::taglib::hasAPETag(mpegFile));
    }

    qDebug() << "Updating track title";

    // Write metadata again -> only the ID3v2 tag should be modified
    trackMetadata.refTrackInfo().setTitle(QStringLiteral("title2"));
    const auto exported2 =
            mixxx::MetadataSourceTagLib(
                    tmpFileName, mixxx::taglib::FileType::MP3)
                    .exportTrackMetadata(trackMetadata);
    ASSERT_EQ(mixxx::MetadataSource::ExportResult::Succeeded, exported.first);
    ASSERT_FALSE(exported.second.isNull());

    // Check that the file (still) only has an ID3v2 tag after writing metadata
    {
        TagLib::MPEG::File mpegFile(
                TAGLIB_FILENAME_FROM_QSTRING(tmpFileName));
        EXPECT_FALSE(mixxx::taglib::hasID3v1Tag(mpegFile));
        EXPECT_TRUE(mixxx::taglib::hasID3v2Tag(mpegFile));
        EXPECT_FALSE(mixxx::taglib::hasAPETag(mpegFile));
    }
}
