#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QtDebug>

#include "track/trackmetadatataglib.h"

namespace {

const QDir kTestDir(QDir::current().absoluteFilePath("src/test/id3-test-data"));

class TagLibTest : public testing::Test {
  protected:

    TagLibTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F(TagLibTest, WriteID3v2Tag) {
    QTemporaryFile tmpFile("no_id3v1_mp3");

    // Generate file name
    ASSERT_TRUE(tmpFile.open());
    ASSERT_TRUE(tmpFile.exists());
    const QString tmpFileName(tmpFile.fileName());
    tmpFile.close(); // close before copying

    // Delete empty temporary file
    ASSERT_TRUE(QFile::remove(tmpFileName));

    // Copy original to temporary file
    QFile copiedFile(tmpFileName);
    ASSERT_FALSE(copiedFile.exists());
    {
        const QString origFileName(kTestDir.absoluteFilePath("empty.mp3"));
        QFile origFile(origFileName);
        ASSERT_TRUE(origFile.exists());
        qDebug() << "Copying file"
                << origFileName
                << "->"
                <<tmpFileName;
        ASSERT_TRUE(origFile.copy(tmpFileName));
        ASSERT_TRUE(copiedFile.exists());
        ASSERT_EQ(copiedFile.size(), origFile.size());
    }

    // Verify that the file has no tags
    {
        TagLib::MPEG::File mpegFile(
                TAGLIB_FILENAME_FROM_QSTRING(tmpFileName));
        EXPECT_FALSE(mixxx::taglib::hasID3v1Tag(mpegFile));
        EXPECT_FALSE(mixxx::taglib::hasID3v2Tag(mpegFile));
        EXPECT_FALSE(mixxx::taglib::hasAPETag(mpegFile));
    }

    // Write metadata -> only an ID3v2 tag should be added
    mixxx::TrackMetadata trackMetadata;
    trackMetadata.setTitle("title");
    ASSERT_EQ(OK, mixxx::taglib::writeTrackMetadataIntoFile(
            trackMetadata, tmpFileName, mixxx::taglib::FileType::MP3));

    // Check that the file only has an ID3v2 tag after writing metadata
    {
        TagLib::MPEG::File mpegFile(
                TAGLIB_FILENAME_FROM_QSTRING(tmpFileName));
        EXPECT_FALSE(mixxx::taglib::hasID3v1Tag(mpegFile));
        EXPECT_TRUE(mixxx::taglib::hasID3v2Tag(mpegFile));
        EXPECT_FALSE(mixxx::taglib::hasAPETag(mpegFile));
    }

    // Write metadata again -> only the ID3v2 tag should be modified
    trackMetadata.setTitle("title2");
    ASSERT_EQ(OK, mixxx::taglib::writeTrackMetadataIntoFile(
            trackMetadata, tmpFileName, mixxx::taglib::FileType::MP3));

    // Check that the file (still) only has an ID3v2 tag after writing metadata
    {
        TagLib::MPEG::File mpegFile(
                TAGLIB_FILENAME_FROM_QSTRING(tmpFileName));
        EXPECT_FALSE(mixxx::taglib::hasID3v1Tag(mpegFile));
        EXPECT_TRUE(mixxx::taglib::hasID3v2Tag(mpegFile));
        EXPECT_FALSE(mixxx::taglib::hasAPETag(mpegFile));
    }
}

}  // anonymous namespace
