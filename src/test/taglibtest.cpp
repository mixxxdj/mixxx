#include <QDir>
#include <QtDebug>
#include <atomic>
#include <thread>
#include <vector>

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
                    tmpFileName, "mp3")
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
                    tmpFileName, "mp3")
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

#ifndef __WINDOWS__ // Note: Following Windows links (*.lnk shortcuts) is not supported by Mixxx yet
TEST_F(TagLibTest, WriteID3v2TagViaLink) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    // Generate a file name for the temporary file
    const QString tmpFileName = tempDir.filePath("no_id3v1_mp3");

    // Create the temporary file by copying an existing file
    mixxxtest::copyFile(
            MixxxTest::getOrInitTestDir().filePath(QStringLiteral("id3-test-data/empty.mp3")),
            tmpFileName);

    // Access the MP3 file indirectly via a symlink when reading & writing the tags
    const QString linkFileName = tempDir.filePath("no_id3v1_mp3_link");
    EXPECT_TRUE(QFile::link(tmpFileName, linkFileName));

    auto linkFileInfoBefore = QFileInfo(linkFileName);
    EXPECT_TRUE(linkFileInfoBefore.exists());
    EXPECT_TRUE(linkFileInfoBefore.isSymLink());
    auto canonicalTmpFileName = QFileInfo(tmpFileName).canonicalFilePath();
    EXPECT_EQ(linkFileInfoBefore.canonicalFilePath().toStdString(),
            canonicalTmpFileName.toStdString());

    // Verify that the file has no tags
    {
        TagLib::MPEG::File mpegFile(
                TAGLIB_FILENAME_FROM_QSTRING(linkFileName));
        EXPECT_FALSE(mixxx::taglib::hasID3v1Tag(mpegFile));
        EXPECT_FALSE(mixxx::taglib::hasID3v2Tag(mpegFile));
        EXPECT_FALSE(mixxx::taglib::hasAPETag(mpegFile));
    }

    qDebug() << "Setting track title";

    // Write metadata (via the symlink) -> only an ID3v2 tag should be added
    mixxx::TrackMetadata trackMetadata;
    trackMetadata.refTrackInfo().setTitle(QStringLiteral("title"));
    const auto exported =
            mixxx::MetadataSourceTagLib(
                    linkFileName, "mp3")
                    .exportTrackMetadata(trackMetadata);
    ASSERT_EQ(mixxx::MetadataSource::ExportResult::Succeeded, exported.first);
    ASSERT_FALSE(exported.second.isNull());

    // Check that the file only has an ID3v2 tag after writing metadata
    {
        TagLib::MPEG::File mpegFile(
                TAGLIB_FILENAME_FROM_QSTRING(linkFileName));
        EXPECT_FALSE(mixxx::taglib::hasID3v1Tag(mpegFile));
        EXPECT_TRUE(mixxx::taglib::hasID3v2Tag(mpegFile));
        EXPECT_FALSE(mixxx::taglib::hasAPETag(mpegFile));
    }

    // Verify that the symlink still exists and still points to the correct file
    auto linkFileInfoAfter = QFileInfo(linkFileName);

    EXPECT_TRUE(linkFileInfoAfter.exists());
    EXPECT_EQ(linkFileInfoAfter.canonicalFilePath().toStdString(),
            canonicalTmpFileName.toStdString());
    EXPECT_TRUE(linkFileInfoAfter.isSymLink());
}
#endif

// On Windows TagLib only allows FILE_SHARE_READ when opening a file. Opening
// it for read/write therefore fails if the file is currently open somewhere
// else, even only for reading. Importing metadata must not need write access,
// otherwise tests running in parallel randomly fail to read shared test files.
// Windows applies the sharing rules per handle, so threads reproduce this.
TEST_F(TagLibTest, ImportConcurrentlyFromSameFile) {
    const QString fileName =
            MixxxTest::getOrInitTestDir().filePath(
                    QStringLiteral("id3-test-data/cover-test-øé~ł€˚-png.mp3"));
    ASSERT_TRUE(QFileInfo::exists(fileName));

    constexpr int kThreads = 8;
    constexpr int kImportsPerThread = 100;

    std::atomic<int> readyThreads = 0;
    std::atomic<bool> start = false;
    std::atomic<int> failedImports = 0;

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&] {
            ++readyThreads;
            while (!start) {
                std::this_thread::yield();
            }
            const mixxx::MetadataSourceTagLib source(fileName, QStringLiteral("mp3"));
            for (int j = 0; j < kImportsPerThread; ++j) {
                mixxx::TrackMetadata trackMetadata;
                const auto imported = source.importTrackMetadataAndCoverImage(
                        &trackMetadata, nullptr, false);
                if (imported.first != mixxx::MetadataSource::ImportResult::Succeeded) {
                    ++failedImports;
                }
            }
        });
    }
    while (readyThreads < kThreads) {
        std::this_thread::yield();
    }
    start = true;
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(0, failedImports);
}
