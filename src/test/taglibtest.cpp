#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QtDebug>

#include "sources/metadatasourcetaglib.h"

namespace {

const QDir kTestDir(QDir::current().absoluteFilePath("src/test/id3-test-data"));

class TagLibTest : public testing::Test {
  protected:
    static QString generateTemporaryFileName(const QString& fileNameTemplate) {
        QTemporaryFile tmpFile(fileNameTemplate);
        // The file must be opened to create it and to obtain
        // its file name!
        tmpFile.open();
        const QString tmpFileName = tmpFile.fileName();
        DEBUG_ASSERT(!tmpFileName.isEmpty());
        return tmpFileName;
    }

    static bool copyFile(const QString& srcFileName, const QString& dstFileName) {
        QFile srcFile(srcFileName);
        DEBUG_ASSERT(srcFile.exists());
        if (!srcFile.copy(dstFileName)) {
            qWarning()
                    << srcFile.errorString()
                    << "- Failed to copy file"
                    << srcFileName
                    << "->"
                    << dstFileName;
            return false;
        }
        QFile dstFile(dstFileName);
        DEBUG_ASSERT(dstFile.exists());
        DEBUG_ASSERT(srcFile.size() == dstFile.size());
        return true;
    }
};

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

TEST_F(TagLibTest, WriteID3v2Tag) {
    // Generate a file name for the temporary file
    const QString tmpFileName = generateTemporaryFileName("no_id3v1_mp3");

    // Create the temporary file by copying an existing file
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

    qDebug() << "Setting track title";

    // Write metadata -> only an ID3v2 tag should be added
    mixxx::TrackMetadata trackMetadata;
    trackMetadata.refTrackInfo().setTitle("title");
    const auto exported =
            mixxx::MetadataSourceTagLib(
                    tmpFileName, mixxx::taglib::FileType::MP3).exportTrackMetadata(trackMetadata);
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
    trackMetadata.refTrackInfo().setTitle("title2");
    const auto exported2 =
            mixxx::MetadataSourceTagLib(
                    tmpFileName, mixxx::taglib::FileType::MP3).exportTrackMetadata(trackMetadata);
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

}  // anonymous namespace
