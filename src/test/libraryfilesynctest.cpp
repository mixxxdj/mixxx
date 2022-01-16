#include <gtest/gtest.h>

#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include "library/library_prefs.h"
#include "sources/metadatasource.h"
#include "test/librarytest.h"
#include "track/track.h"
#include "util/fileinfo.h"

namespace {

const auto kTestDir = QDir(QDir::current().absoluteFilePath("src/test/id3-test-data"));

const QString kTestFileName = QStringLiteral("cover-test-jpg.mp3");

/// A temporary file system with a single audio file.
class TempFileSystem {
  public:
    explicit TempFileSystem(const QString& fileName = QString())
            : m_fileInfo(m_tempDir.filePath(fileName.isEmpty() ? kTestFileName : fileName)) {
        DEBUG_ASSERT(m_tempDir.isValid());
        DEBUG_ASSERT(!m_fileInfo.exists());
        mixxxtest::copyFile(
                kTestDir.absoluteFilePath(kTestFileName),
                m_fileInfo.location());
        DEBUG_ASSERT(m_fileInfo.exists());
    }

    const mixxx::FileInfo& fileInfo() const {
        return m_fileInfo;
    }

    QDateTime fileLastModified() const {
        return m_fileInfo.toQFile().fileTime(QFileDevice::FileModificationTime);
    }

    bool updateFileLastModified(const QDateTime& newLastModified = QDateTime()) const {
        auto file = m_fileInfo.toQFile();
        const auto oldLastModified = fileLastModified();
        VERIFY_OR_DEBUG_ASSERT(oldLastModified.isValid()) {
            return false;
        }
        VERIFY_OR_DEBUG_ASSERT(newLastModified != oldLastModified) {
            // Nothing to do but probably unintended!
            return true;
        }
        VERIFY_OR_DEBUG_ASSERT(file.open(QIODevice::ReadWrite |
                QIODevice::ExistingOnly | QIODevice::Append)) {
            return false;
        }
        if (newLastModified.isValid()) {
            VERIFY_OR_DEBUG_ASSERT(file.setFileTime(
                    newLastModified, QFileDevice::FileModificationTime)) {
                return false;
            }
            VERIFY_OR_DEBUG_ASSERT(fileLastModified() == newLastModified) {
                return false;
            }
            return true;
        }
        QDateTime nowLastModified;
        do {
            nowLastModified = QDateTime::currentDateTime();
            VERIFY_OR_DEBUG_ASSERT(nowLastModified >= oldLastModified) {
                return false;
            }
            VERIFY_OR_DEBUG_ASSERT(file.setFileTime(
                    nowLastModified, QFileDevice::FileModificationTime)) {
                return false;
            }
            // Loop until the actual modification time has changed
        } while (oldLastModified == fileLastModified());
        VERIFY_OR_DEBUG_ASSERT(fileLastModified() == nowLastModified) {
            return false;
        }
        return true;
    }

    bool removeFile() const {
        return m_fileInfo.toQFile().remove();
    }

  private:
    const QTemporaryDir m_tempDir;
    const mixxx::FileInfo m_fileInfo;
};

} // namespace

class LibraryFileSyncTest : public LibraryTest {
    void SetUp() override {
        const auto trackRef = TrackRef::fromFileInfo(m_tempFileSystem.fileInfo());
        ASSERT_FALSE(internalCollection()->getTrackDAO().getTrackByRef(trackRef));
        const auto pTrack = getOrAddTrackByLocation(trackRef.getLocation());
        ASSERT_TRUE(internalCollection()->getTrackDAO().getTrackByRef(trackRef));
        m_trackId = pTrack->getId();
        // Ensure that new modification time stamps will be strictly greater
        // than that of the temporary file!
        QThread::msleep(1);
    }

  protected:
    TrackPointer loadTrack() const {
        return trackCollectionManager()->getTrackById(m_trackId);
    }

    TempFileSystem m_tempFileSystem;
    TrackId m_trackId;
};

TEST_F(LibraryFileSyncTest, checkSourceSyncStatus) {
    const auto pTrack = loadTrack();
    ASSERT_EQ(mixxx::TrackRecord::SourceSyncStatus::Void,
            mixxx::TrackRecord{}.checkSourceSyncStatus(pTrack->getFileInfo()));
    const auto trackRecord = pTrack->getRecord();
    ASSERT_EQ(mixxx::TrackRecord::SourceSyncStatus::Synchronized,
            trackRecord.checkSourceSyncStatus(pTrack->getFileInfo()));
    ASSERT_TRUE(m_tempFileSystem.updateFileLastModified());
    ASSERT_EQ(mixxx::TrackRecord::SourceSyncStatus::Outdated,
            trackRecord.checkSourceSyncStatus(pTrack->getFileInfo()));
    ASSERT_TRUE(m_tempFileSystem.removeFile());
    ASSERT_EQ(mixxx::TrackRecord::SourceSyncStatus::Undefined,
            trackRecord.checkSourceSyncStatus(pTrack->getFileInfo()));
}

TEST_F(LibraryFileSyncTest, exportMetadataAfterMigrationWithUndefinedSyncStatus) {
    const auto origFileSynchronizedAt = mixxx::MetadataSource::getFileSynchronizedAt(
            m_tempFileSystem.fileInfo().toQFile());

    auto pTrack = loadTrack();
    auto trackRecord = pTrack->getRecord();
    ASSERT_EQ(mixxx::TrackRecord::SourceSyncStatus::Synchronized,
            trackRecord.checkSourceSyncStatus(pTrack->getFileInfo()));
    // The column contains NULL as the default value, i.e. the last
    // synchronization time is unknown.
    trackRecord.setSourceSynchronizedAt(QDateTime{});
    ASSERT_TRUE(pTrack->replaceRecord(std::move(trackRecord)));

    // Write the data into the database
    pTrack.reset();

    // Reload the prepared track
    pTrack = loadTrack();
    trackRecord = pTrack->getRecord();
    ASSERT_FALSE(trackRecord.getSourceSynchronizedAt().isValid());
    // The file has not been modified by saving the metadata
    ASSERT_EQ(origFileSynchronizedAt,
            mixxx::MetadataSource::getFileSynchronizedAt(
                    m_tempFileSystem.fileInfo().toQFile()));
    EXPECT_EQ(mixxx::TrackRecord::SourceSyncStatus::Unknown,
            trackRecord.checkSourceSyncStatus(pTrack->getFileInfo()));

    // Enable export of metadata
    m_pConfig->setValue(
            mixxx::library::prefs::kSyncTrackMetadataConfigKey,
            true);

    // Modify the track metadata
    const QString newTitle = pTrack->getTitle() + QStringLiteral("modified");
    pTrack->setTitle(newTitle);
    // Save the modified track
    pTrack.reset();

    // Reload the modified track
    pTrack = loadTrack();
    trackRecord = pTrack->getRecord();
    const auto updatedFileSynchronizedAt = mixxx::MetadataSource::getFileSynchronizedAt(
            m_tempFileSystem.fileInfo().toQFile());
    ASSERT_TRUE(updatedFileSynchronizedAt > origFileSynchronizedAt);
    EXPECT_EQ(updatedFileSynchronizedAt,
            trackRecord.getSourceSynchronizedAt());
    EXPECT_EQ(mixxx::TrackRecord::SourceSyncStatus::Synchronized,
            trackRecord.checkSourceSyncStatus(pTrack->getFileInfo()));

    // Verify that the metadata in the file has actually been modified
    mixxx::TrackMetadata trackMetadata;
    auto [importResult, sourceSynchronizedAt] =
            SoundSourceProxy(pTrack).importTrackMetadataAndCoverImage(&trackMetadata, nullptr);
    EXPECT_EQ(mixxx::MetadataSource::ImportResult::Succeeded, importResult);
    EXPECT_EQ(updatedFileSynchronizedAt, sourceSynchronizedAt);
    EXPECT_EQ(newTitle, trackMetadata.getTrackInfo().getTitle());
}
