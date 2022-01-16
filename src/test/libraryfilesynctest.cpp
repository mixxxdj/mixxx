#include <gtest/gtest.h>

#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <memory>

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

    bool setFileLastModified(const QDateTime& lastModified) const {
        return m_fileInfo.toQFile().setFileTime(lastModified, QFileDevice::FileModificationTime);
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
            VERIFY_OR_DEBUG_ASSERT(setFileLastModified(newLastModified)) {
                return false;
            }
            VERIFY_OR_DEBUG_ASSERT(fileLastModified() == newLastModified) {
                return false;
            }
            // Ensure that subsequent modification time stamps will be strictly greater.
            QThread::msleep(1);
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
        // Ensure that subsequent modification time stamps will be strictly greater.
        QThread::msleep(1);
        return true;
    }

    bool removeFile() const {
        return m_fileInfo.toQFile().remove();
    }

  private:
    const QTemporaryDir m_tempDir;
    const mixxx::FileInfo m_fileInfo;
};

class SyncTrackMetadataConfigScope final {
  public:
    explicit SyncTrackMetadataConfigScope(
            UserSettingsPointer pConfig)
            : m_pConfig(std::move(pConfig)),
              m_syncTrackMetadataBackup(
                      m_pConfig->getValue(
                              mixxx::library::prefs::kSyncTrackMetadataConfigKey,
                              false)) {
    }
    ~SyncTrackMetadataConfigScope() {
        // Restore config
        m_pConfig->setValue(
                mixxx::library::prefs::kSyncTrackMetadataConfigKey,
                m_syncTrackMetadataBackup);
    }

    void setConfig(bool syncTrackMetadata) const {
        m_pConfig->setValue(
                mixxx::library::prefs::kSyncTrackMetadataConfigKey,
                syncTrackMetadata);
    }

  private:
    const UserSettingsPointer m_pConfig;
    const bool m_syncTrackMetadataBackup;
};

} // namespace

class LibraryFileSyncTest : public LibraryTest {
    void SetUp() override {
        m_pTempFileSystem = std::make_unique<TempFileSystem>();
        const auto trackRef = TrackRef::fromFileInfo(m_pTempFileSystem->fileInfo());
        ASSERT_FALSE(trackCollectionManager()->getTrackByRef(trackRef));
        const auto pTrack = getOrAddTrackByLocation(trackRef.getLocation());
        ASSERT_TRUE(trackCollectionManager()->getTrackByRef(trackRef));
        m_trackId = pTrack->getId();
        // Ensure that new modification time stamps will be strictly greater
        // than that of the temporary file!
        QThread::msleep(1);
    }

    void TearDown() override {
        m_pTempFileSystem.reset();
        m_trackId = TrackId{};
    }

  protected:
    TrackPointer loadTrack() const {
        return trackCollectionManager()->getTrackById(m_trackId);
    }

    bool verifySourceSyncStatusOfTrack(
            const Track& track,
            mixxx::TrackRecord::SourceSyncStatus expectedSourceSyncStatus) const {
        const auto trackRecord = track.getRecord();
        VERIFY_OR_DEBUG_ASSERT(expectedSourceSyncStatus ==
                trackRecord.checkSourceSyncStatus(track.getFileInfo())) {
            return false;
        }

        // Verify all time stamps for consistency
        const auto sourceSynchronizedAt = trackRecord.getSourceSynchronizedAt();
        const auto fileLastModified = m_pTempFileSystem->fileLastModified();
        switch (expectedSourceSyncStatus) {
        case mixxx::TrackRecord::SourceSyncStatus::Synchronized:
            VERIFY_OR_DEBUG_ASSERT(sourceSynchronizedAt.isValid() &&
                    fileLastModified.isValid() &&
                    trackRecord.getSourceSynchronizedAt() ==
                            m_pTempFileSystem->fileLastModified()) {
                return false;
            }
            break;
        case mixxx::TrackRecord::SourceSyncStatus::Outdated:
            VERIFY_OR_DEBUG_ASSERT(
                    sourceSynchronizedAt.isValid() &&
                    fileLastModified.isValid() &&
                    trackRecord.getSourceSynchronizedAt() < m_pTempFileSystem->fileLastModified()) {
                return false;
            }
            break;
        case mixxx::TrackRecord::SourceSyncStatus::Void:
        case mixxx::TrackRecord::SourceSyncStatus::Unknown:
            VERIFY_OR_DEBUG_ASSERT(!sourceSynchronizedAt.isValid()) {
                return false;
            }
            break;
        case mixxx::TrackRecord::SourceSyncStatus::Undefined:
            VERIFY_OR_DEBUG_ASSERT(!fileLastModified.isValid()) {
                return false;
            }
            break;
        default:
            DEBUG_ASSERT(!"unreachable");
            return false;
        }

        return true;
    }

    TrackPointer loadTrackFromDatabaseAndVerifySourceSyncStatus(
            mixxx::TrackRecord::SourceSyncStatus expectedSourceSyncStatus) const {
        const auto syncTrackMetadataConfigScope =
                SyncTrackMetadataConfigScope(m_pConfig);

        // Disable sync
        syncTrackMetadataConfigScope.setConfig(false);

        const auto pTrack = loadTrack();
        VERIFY_OR_DEBUG_ASSERT(pTrack) {
            return nullptr;
        }
        VERIFY_OR_DEBUG_ASSERT(
                verifySourceSyncStatusOfTrack(*pTrack, expectedSourceSyncStatus)) {
            return nullptr;
        }

        return pTrack;
    }

    TrackPointer establishSourceSyncStatusUnknown() const {
        auto pTrack = loadTrackFromDatabaseAndVerifySourceSyncStatus(
                mixxx::TrackRecord::SourceSyncStatus::Synchronized);
        VERIFY_OR_DEBUG_ASSERT(pTrack) {
            return nullptr;
        }

        const auto syncTrackMetadataConfigScope =
                SyncTrackMetadataConfigScope(m_pConfig);

        // Disable sync
        syncTrackMetadataConfigScope.setConfig(false);

        const auto fileLastModifiedBefore = m_pTempFileSystem->fileLastModified();

        // The column contains NULL as the default value, i.e. the last
        // synchronization time is unknown.
        auto trackRecord = pTrack->getRecord();
        trackRecord.setSourceSynchronizedAt(QDateTime{});
        VERIFY_OR_DEBUG_ASSERT(pTrack->replaceRecord(std::move(trackRecord))) {
            return nullptr;
        }
        // Write the data into the database (without modifying the file)
        pTrack.reset();

        // Verify that the file has not been modified
        const auto fileLastModifiedAfter = m_pTempFileSystem->fileLastModified();
        VERIFY_OR_DEBUG_ASSERT(fileLastModifiedAfter == fileLastModifiedBefore) {
            return nullptr;
        }

        return loadTrackFromDatabaseAndVerifySourceSyncStatus(
                mixxx::TrackRecord::SourceSyncStatus::Unknown);
    }

    TrackPointer establishSourceSyncStatusOutdated() const {
        // Touch the file's modification time stamp to simulate an external
        // modification by a 3rd party app after the track has been loaded.
        VERIFY_OR_DEBUG_ASSERT(m_pTempFileSystem->updateFileLastModified()) {
            return nullptr;
        }

        return loadTrackFromDatabaseAndVerifySourceSyncStatus(
                mixxx::TrackRecord::SourceSyncStatus::Outdated);
    }

    TrackPointer establishSourceSyncStatusUndefined() const {
        // Touch the file's modification time stamp to simulate an external
        // modification by a 3rd party app after the track has been loaded.
        VERIFY_OR_DEBUG_ASSERT(m_pTempFileSystem->removeFile()) {
            return nullptr;
        }

        return loadTrackFromDatabaseAndVerifySourceSyncStatus(
                mixxx::TrackRecord::SourceSyncStatus::Undefined);
    }

    bool modifyAndSaveTrack(TrackPointer&& pTrack, bool syncTrackMetadata) {
        VERIFY_OR_DEBUG_ASSERT(pTrack) {
            return false;
        }

        const auto fileLastModifiedBefore = m_pTempFileSystem->fileLastModified();
        /*non-const*/ auto trackRecordBefore = pTrack->getRecord();
        const auto sourceSyncStatusBefore =
                trackRecordBefore.checkSourceSyncStatus(pTrack->getFileInfo());

        const auto syncTrackMetadataConfigScope =
                SyncTrackMetadataConfigScope(m_pConfig);

        // Configure sync
        syncTrackMetadataConfigScope.setConfig(syncTrackMetadata);

        // Modify track metadata
        const QString newTitle = pTrack->getTitle() + QStringLiteral("modified");
        pTrack->setTitle(newTitle);

        // Save the track
        pTrack.reset();

        // Disable sync for the final verification
        syncTrackMetadataConfigScope.setConfig(false);

        const auto fileLastModifiedAfter = m_pTempFileSystem->fileLastModified();
        if (syncTrackMetadata) {
            // Verify that the file has been modified upon saving
            VERIFY_OR_DEBUG_ASSERT(
                    !fileLastModifiedBefore.isValid() ||
                    !fileLastModifiedAfter.isValid() ||
                    fileLastModifiedBefore < fileLastModifiedAfter) {
                return false;
            }
        } else {
            // Verify that the file has not been modified upon saving
            VERIFY_OR_DEBUG_ASSERT(fileLastModifiedBefore == fileLastModifiedAfter) {
                return false;
            }
        }

        auto sourceSyncStatusAfter = sourceSyncStatusBefore;
        auto expectedImportResult = mixxx::MetadataSource::ImportResult::Succeeded;
        switch (sourceSyncStatusBefore) {
        case mixxx::TrackRecord::SourceSyncStatus::Unknown:
        case mixxx::TrackRecord::SourceSyncStatus::Outdated:
            if (syncTrackMetadata) {
                sourceSyncStatusAfter = mixxx::TrackRecord::SourceSyncStatus::Synchronized;
            }
            break;
        case mixxx::TrackRecord::SourceSyncStatus::Void:
        case mixxx::TrackRecord::SourceSyncStatus::Undefined:
            expectedImportResult = mixxx::MetadataSource::ImportResult::Unavailable;
            break;
        case mixxx::TrackRecord::SourceSyncStatus::Synchronized:
            break;
        default:
            VERIFY_OR_DEBUG_ASSERT(!"unreachable") {
                return false;
            }
        }

        // Verify that the modified metadata is still present after
        // reloading the track from the database
        pTrack = loadTrackFromDatabaseAndVerifySourceSyncStatus(sourceSyncStatusAfter);
        VERIFY_OR_DEBUG_ASSERT(pTrack) {
            return false;
        }
        const auto trackRecordAfter = pTrack->getRecord();

        // Reimport metadata from the file
        mixxx::TrackMetadata importedTrackMetadata;
        auto [importResult, sourceSynchronizedAt] =
                SoundSourceProxy(pTrack).importTrackMetadataAndCoverImage(
                        &importedTrackMetadata, nullptr);
        VERIFY_OR_DEBUG_ASSERT(expectedImportResult == importResult) {
            return false;
        }

        // The stream info properties might be adjusted when exporting track metadata!
        // We have to adjust trackRecordBefore and importedTrackMetadata accordingly
        // before continuing with the verification.
        if (syncTrackMetadata) {
            trackRecordBefore.refMetadata().refStreamInfo() =
                    trackRecordAfter.getMetadata().getStreamInfo();
            importedTrackMetadata.refStreamInfo() = trackRecordAfter.getMetadata().getStreamInfo();
        }

        // Verify that the metadata in the database has been modified.
        VERIFY_OR_DEBUG_ASSERT(
                trackRecordBefore.getMetadata() != trackRecordAfter.getMetadata()) {
            return false;
        }
        VERIFY_OR_DEBUG_ASSERT(newTitle == pTrack->getTitle()) {
            return false;
        }

        if (syncTrackMetadata) {
            // Verify that the synchronization time stamp has been updated.
            VERIFY_OR_DEBUG_ASSERT(trackRecordAfter.getSourceSynchronizedAt() >
                    trackRecordBefore.getSourceSynchronizedAt()) {
                return false;
            }
            if (expectedImportResult == mixxx::MetadataSource::ImportResult::Succeeded) {
                // Verify that the metadata in the file has been modified.
                VERIFY_OR_DEBUG_ASSERT(sourceSynchronizedAt > fileLastModifiedBefore) {
                    return false;
                }
                VERIFY_OR_DEBUG_ASSERT(sourceSynchronizedAt ==
                        trackRecordAfter.getSourceSynchronizedAt()) {
                    return false;
                }
                VERIFY_OR_DEBUG_ASSERT(importedTrackMetadata == trackRecordAfter.getMetadata()) {
                    return false;
                }
            }
        } else {
            // Verify that the synchronization time stamp has not been updated.
            VERIFY_OR_DEBUG_ASSERT(trackRecordAfter.getSourceSynchronizedAt() ==
                    trackRecordBefore.getSourceSynchronizedAt()) {
                return false;
            }
            if (expectedImportResult == mixxx::MetadataSource::ImportResult::Succeeded) {
                // Verify that the metadata in the file has not been modified.
                VERIFY_OR_DEBUG_ASSERT(sourceSynchronizedAt == fileLastModifiedBefore) {
                    return false;
                }
                VERIFY_OR_DEBUG_ASSERT(importedTrackMetadata == trackRecordBefore.getMetadata()) {
                    return false;
                }
            }
        }

        return true;
    }

    std::unique_ptr<TempFileSystem>
            m_pTempFileSystem;
    TrackId m_trackId;
};

TEST_F(LibraryFileSyncTest, checkSourceSyncStatus) {
    const auto pTrack = loadTrack();
    ASSERT_EQ(mixxx::TrackRecord::SourceSyncStatus::Void,
            mixxx::TrackRecord{}.checkSourceSyncStatus(pTrack->getFileInfo()));
    const auto trackRecord = pTrack->getRecord();
    ASSERT_EQ(mixxx::TrackRecord::SourceSyncStatus::Synchronized,
            trackRecord.checkSourceSyncStatus(pTrack->getFileInfo()));
    ASSERT_TRUE(m_pTempFileSystem->updateFileLastModified());
    ASSERT_EQ(mixxx::TrackRecord::SourceSyncStatus::Outdated,
            trackRecord.checkSourceSyncStatus(pTrack->getFileInfo()));
    ASSERT_TRUE(m_pTempFileSystem->removeFile());
    ASSERT_EQ(mixxx::TrackRecord::SourceSyncStatus::Undefined,
            trackRecord.checkSourceSyncStatus(pTrack->getFileInfo()));
}

TEST_F(LibraryFileSyncTest, saveTrackMetadataWithSourceSyncStatusSynchronizedAndSyncEnabled) {
    EXPECT_TRUE(modifyAndSaveTrack(
            loadTrackFromDatabaseAndVerifySourceSyncStatus(
                    mixxx::TrackRecord::SourceSyncStatus::Synchronized),
            true));
}

TEST_F(LibraryFileSyncTest, saveTrackMetadataWithSourceSyncStatusSynchronizedAndSyncDisabled) {
    EXPECT_TRUE(modifyAndSaveTrack(
            loadTrackFromDatabaseAndVerifySourceSyncStatus(
                    mixxx::TrackRecord::SourceSyncStatus::Synchronized),
            false));
}

TEST_F(LibraryFileSyncTest, saveTrackMetadataWithSourceSyncStatusUnknownAndSyncEnabled) {
    EXPECT_TRUE(modifyAndSaveTrack(
            establishSourceSyncStatusUnknown(),
            true));
}

TEST_F(LibraryFileSyncTest, saveTrackMetadataWithSourceSyncStatusUnknownAndSyncDisabled) {
    EXPECT_TRUE(modifyAndSaveTrack(
            establishSourceSyncStatusUnknown(),
            false));
}

TEST_F(LibraryFileSyncTest, saveTrackMetadataWithSourceSyncStatusOutdatedAndSyncEnabled) {
    EXPECT_TRUE(modifyAndSaveTrack(
            establishSourceSyncStatusOutdated(),
            true));
}

TEST_F(LibraryFileSyncTest, saveTrackMetadataWithSourceSyncStatusOutdatedAndSyncDisabled) {
    EXPECT_TRUE(modifyAndSaveTrack(
            establishSourceSyncStatusOutdated(),
            false));
}

// FIXME: Fix failing test, i.e. modifications don't seem to be saved in the database as expected!
TEST_F(LibraryFileSyncTest, DISABLED_saveTrackMetadataWithSourceSyncStatusUndefinedAndSyncEnabled) {
    EXPECT_TRUE(modifyAndSaveTrack(
            establishSourceSyncStatusUndefined(),
            true));
}

// FIXME: Fix failing test, i.e. modifications don't seem to be saved in the database as expected!
TEST_F(LibraryFileSyncTest,
        DISABLED_saveTrackMetadataWithSourceSyncStatusUndefinedAndSyncDisabled) {
    EXPECT_TRUE(modifyAndSaveTrack(
            establishSourceSyncStatusUndefined(),
            false));
}

TEST_F(LibraryFileSyncTest, reimportOutdatedTrackMetadataWithSyncEnabled) {
    const auto outdatedTrackRecord =
            establishSourceSyncStatusOutdated()->getRecord();

    const auto syncTrackMetadataConfigScope =
            SyncTrackMetadataConfigScope(m_pConfig);

    // Enabled sync
    syncTrackMetadataConfigScope.setConfig(true);

    auto pTrack = loadTrack();
    ASSERT_TRUE(pTrack);
    const auto importedTrackRecord = pTrack->getRecord();
    EXPECT_TRUE(verifySourceSyncStatusOfTrack(
            *pTrack, mixxx::TrackRecord::SourceSyncStatus::Synchronized));
    EXPECT_TRUE(outdatedTrackRecord.getSourceSynchronizedAt() <
            importedTrackRecord.getSourceSynchronizedAt());
}

TEST_F(LibraryFileSyncTest, doNotReimportOutdatedTrackMetadataWithSyncDisabled) {
    const auto outdatedTrackRecord =
            establishSourceSyncStatusOutdated()->getRecord();

    const auto syncTrackMetadataConfigScope =
            SyncTrackMetadataConfigScope(m_pConfig);

    // Disable sync
    syncTrackMetadataConfigScope.setConfig(false);

    auto pTrack = loadTrack();
    ASSERT_TRUE(pTrack);
    const auto loadedTrackRecord = pTrack->getRecord();
    EXPECT_TRUE(verifySourceSyncStatusOfTrack(
            *pTrack, mixxx::TrackRecord::SourceSyncStatus::Outdated));
    EXPECT_EQ(outdatedTrackRecord, loadedTrackRecord);
}
