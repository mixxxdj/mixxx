#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QtDebug>

#include "track/trackmetadatataglib.h"

namespace {

const QDir kTestDir(QDir::current().absoluteFilePath("src/test/id3-test-data"));

class TagLibTest : public testing::Test {};

class FileRemover final {
public:
    explicit FileRemover(const QString& fileName)
        : m_fileName(fileName) {
    }
    ~FileRemover() {
        QFile::remove(m_fileName);
    }
private:
    QString m_fileName;
};

QString generateTemporaryFileName(const QString& fileNameTemplate) {
    QTemporaryFile tmpFile(fileNameTemplate);
    tmpFile.open();
    DEBUG_ASSERT(tmpFile.exists());
    const QString tmpFileName(tmpFile.fileName());
    FileRemover tmpFileRemover(tmpFileName);
    return tmpFileName;
}

void copyFile(const QString& srcFileName, const QString& dstFileName) {
    QFile srcFile(srcFileName);
    DEBUG_ASSERT(srcFile.exists());
    qDebug() << "Copying file"
            << srcFileName
            << "->"
            <<dstFileName;
    srcFile.copy(dstFileName);
    QFile dstFile(dstFileName);
    DEBUG_ASSERT(dstFile.exists());
    DEBUG_ASSERT(srcFile.size() == dstFile.size());
}

TEST_F(TagLibTest, WriteID3v2Tag) {
    // Generate a file name for the temporary file
    const QString tmpFileName = generateTemporaryFileName("no_id3v1_mp3");

    // Create the temporary file by copying an exiting file
    copyFile(kTestDir.absoluteFilePath("empty.mp3"), tmpFileName);

    // Ensure that the temporary file is removed after the test
    FileRemover tmpFileRemover(tmpFileName);

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
