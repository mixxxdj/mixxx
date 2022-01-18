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

    void setFileLastModified(const QDateTime& lastModified) const {
        ASSERT_TRUE(m_fileInfo.toQFile().setFileTime(
                lastModified, QFileDevice::FileModificationTime));
    }

    void updateFileLastModified(const QDateTime& newLastModified = QDateTime()) const {
        auto file = m_fileInfo.toQFile();
        const auto oldLastModified = fileLastModified();
        ASSERT_TRUE(oldLastModified.isValid());
        ASSERT_NE(newLastModified, oldLastModified); // Probably unintended
        ASSERT_TRUE(file.open(QIODevice::ReadWrite | QIODevice::ExistingOnly | QIODevice::Append));
        if (newLastModified.isValid()) {
            setFileLastModified(newLastModified);
            ASSERT_EQ(fileLastModified(), newLastModified);
            // Ensure that subsequent modification time stamps will be strictly greater.
            QThread::msleep(1);
            return;
        }
        QDateTime nowLastModified;
        do {
            nowLastModified = QDateTime::currentDateTime();
            ASSERT_GE(nowLastModified, oldLastModified);
            ASSERT_TRUE(file.setFileTime(nowLastModified, QFileDevice::FileModificationTime));
            // Loop until the actual modification time has changed
        } while (oldLastModified == fileLastModified());
        ASSERT_EQ(fileLastModified(), nowLastModified);
        // Ensure that subsequent modification time stamps will be strictly greater.
        QThread::msleep(1);
    }

    void removeFile() const {
        ASSERT_TRUE(m_fileInfo.toQFile().remove());
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
        const auto pTrack = trackCollectionManager()->getTrackById(m_trackId);
        DEBUG_ASSERT(pTrack);
        return pTrack;
    }

    void saveModifiedTrack(TrackPointer&& pTrack) const {
        ASSERT_NE(nullptr, pTrack);
        ASSERT_EQ(1, pTrack.use_count());
        const auto trackId = pTrack->getId();
        ASSERT_TRUE(trackId.isValid());
        ASSERT_TRUE(pTrack->isDirty());
        pTrack.reset();
        ASSERT_EQ(nullptr, GlobalTrackCacheLocker().lookupTrackById(trackId));
    }

    void verifySourceSyncStatusOfTrack(
            const Track& track,
            mixxx::TrackRecord::SourceSyncStatus expectedSourceSyncStatus) const {
        const auto trackRecord = track.getRecord();
        EXPECT_EQ(expectedSourceSyncStatus,
                trackRecord.checkSourceSyncStatus(track.getFileInfo()));

        // Verify all time stamps for consistency
        const auto sourceSynchronizedAt = trackRecord.getSourceSynchronizedAt();
        const auto fileLastModified = m_pTempFileSystem->fileLastModified();
        switch (expectedSourceSyncStatus) {
        case mixxx::TrackRecord::SourceSyncStatus::Synchronized:
            EXPECT_TRUE(sourceSynchronizedAt.isValid());
            EXPECT_TRUE(fileLastModified.isValid());
            EXPECT_EQ(fileLastModified, trackRecord.getSourceSynchronizedAt());
            break;
        case mixxx::TrackRecord::SourceSyncStatus::Outdated:
            EXPECT_TRUE(sourceSynchronizedAt.isValid());
            EXPECT_TRUE(fileLastModified.isValid());
            EXPECT_GT(fileLastModified, trackRecord.getSourceSynchronizedAt());
            break;
        case mixxx::TrackRecord::SourceSyncStatus::Void:
        case mixxx::TrackRecord::SourceSyncStatus::Unknown:
            EXPECT_FALSE(sourceSynchronizedAt.isValid());
            break;
        case mixxx::TrackRecord::SourceSyncStatus::Undefined:
            EXPECT_FALSE(fileLastModified.isValid());
            break;
        default:
            ASSERT_FALSE("unreachable");
        }
    }

    void loadTrackFromDatabaseAndVerifySourceSyncStatus(
            TrackPointer* pLoadedTrack,
            mixxx::TrackRecord::SourceSyncStatus expectedSourceSyncStatus) const {
        const auto syncTrackMetadataConfigScope =
                SyncTrackMetadataConfigScope(m_pConfig);

        // Disable sync
        syncTrackMetadataConfigScope.setConfig(false);

        const auto pTrack = loadTrack();
        ASSERT_NE(nullptr, pTrack);
        verifySourceSyncStatusOfTrack(*pTrack, expectedSourceSyncStatus);

        if (pLoadedTrack) {
            *pLoadedTrack = std::move(pTrack);
        }
    }

    void establishSourceSyncStatusUnknown(TrackPointer* pLoadedTrack) const {
        TrackPointer pTrack;
        loadTrackFromDatabaseAndVerifySourceSyncStatus(
                &pTrack,
                mixxx::TrackRecord::SourceSyncStatus::Synchronized);
        ASSERT_NE(nullptr, pTrack);

        const auto syncTrackMetadataConfigScope =
                SyncTrackMetadataConfigScope(m_pConfig);

        // Disable sync
        syncTrackMetadataConfigScope.setConfig(false);

        const auto fileLastModifiedBefore = m_pTempFileSystem->fileLastModified();

        // The column contains NULL as the default value, i.e. the last
        // synchronization time is unknown.
        auto trackRecord = pTrack->getRecord();
        trackRecord.setSourceSynchronizedAt(QDateTime{});
        ASSERT_TRUE(pTrack->replaceRecord(std::move(trackRecord)));
        // Write the data into the database (without modifying the file)
        saveModifiedTrack(std::move(pTrack));
        ASSERT_EQ(nullptr, pTrack);

        // Verify that the file has not been modified
        const auto fileLastModifiedAfter = m_pTempFileSystem->fileLastModified();
        ASSERT_EQ(fileLastModifiedAfter, fileLastModifiedBefore);

        loadTrackFromDatabaseAndVerifySourceSyncStatus(
                pLoadedTrack,
                mixxx::TrackRecord::SourceSyncStatus::Unknown);
    }

    void establishSourceSyncStatusOutdated(TrackPointer* pLoadedTrack) const {
        // Touch the file's modification time stamp to simulate an external
        // modification by a 3rd party app after the track has been loaded.
        m_pTempFileSystem->updateFileLastModified();

        loadTrackFromDatabaseAndVerifySourceSyncStatus(
                pLoadedTrack,
                mixxx::TrackRecord::SourceSyncStatus::Outdated);
    }

    void establishSourceSyncStatusUndefined(TrackPointer* pLoadedTrack) const {
        // Touch the file's modification time stamp to simulate an external
        // modification by a 3rd party app after the track has been loaded.
        m_pTempFileSystem->removeFile();

        loadTrackFromDatabaseAndVerifySourceSyncStatus(
                pLoadedTrack,
                mixxx::TrackRecord::SourceSyncStatus::Undefined);
    }

    void modifyAndSaveTrack(TrackPointer&& pTrack, bool syncTrackMetadata) {
        ASSERT_NE(nullptr, pTrack);

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
        saveModifiedTrack(std::move(pTrack));
        ASSERT_EQ(nullptr, pTrack);

        // Disable sync temporarily for the final verification to prevent
        // reimporting metadata from the file unintentionally, i.e. switch
        // the library into read-only mode.
        syncTrackMetadataConfigScope.setConfig(false);

        const auto fileLastModifiedAfter = m_pTempFileSystem->fileLastModified();
        ASSERT_EQ(fileLastModifiedAfter.isValid(), fileLastModifiedBefore.isValid());

        if (syncTrackMetadata) {
            // Verify that the file has been modified upon saving
            ASSERT_TRUE(
                    !fileLastModifiedAfter.isValid() ||
                    fileLastModifiedBefore < fileLastModifiedAfter);
        } else {
            // Verify that the file has not been modified upon saving
            ASSERT_EQ(fileLastModifiedAfter, fileLastModifiedBefore);
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
            ASSERT_FALSE("unreachable");
        }

        // Verify that the modified metadata is still present after
        // reloading the track from the database
        loadTrackFromDatabaseAndVerifySourceSyncStatus(&pTrack, sourceSyncStatusAfter);
        ASSERT_NE(nullptr, pTrack);
        const auto trackRecordAfter = pTrack->getRecord();

        // Reimport metadata from the file
        mixxx::TrackMetadata importedTrackMetadata;
        auto [importResult, sourceSynchronizedAt] =
                SoundSourceProxy(pTrack).importTrackMetadataAndCoverImage(
                        &importedTrackMetadata, nullptr);
        EXPECT_EQ(expectedImportResult, importResult);

        // The stream info properties might be adjusted when exporting track metadata!
        // We have to adjust trackRecordBefore and importedTrackMetadata accordingly
        // before continuing with the verification.
        if (syncTrackMetadata) {
            trackRecordBefore.refMetadata().refStreamInfo() =
                    trackRecordAfter.getMetadata().getStreamInfo();
            if (importResult == mixxx::MetadataSource::ImportResult::Succeeded) {
                importedTrackMetadata.refStreamInfo() =
                        trackRecordAfter.getMetadata().getStreamInfo();
            }
        }

        // Verify that the metadata in the database has been modified.
        EXPECT_NE(trackRecordBefore.getMetadata(), trackRecordAfter.getMetadata());
        EXPECT_EQ(newTitle, pTrack->getTitle());

        if (syncTrackMetadata) {
            // Verify that the synchronization time stamp has been updated
            // if the file still exists.
            EXPECT_TRUE(
                    !fileLastModifiedAfter.isValid() ||
                    trackRecordBefore.getSourceSynchronizedAt() <
                            trackRecordAfter.getSourceSynchronizedAt());
            // Verify that the synchronization time stamp has not been updated
            // if the file is inaccessible.
            EXPECT_TRUE(
                    fileLastModifiedAfter.isValid() ||
                    trackRecordBefore.getSourceSynchronizedAt() ==
                            trackRecordAfter.getSourceSynchronizedAt());
            if (importResult == mixxx::MetadataSource::ImportResult::Succeeded) {
                // Verify that the metadata in the file has been modified.
                EXPECT_LT(fileLastModifiedBefore, sourceSynchronizedAt);
                EXPECT_EQ(trackRecordAfter.getSourceSynchronizedAt(), sourceSynchronizedAt);
                EXPECT_EQ(trackRecordAfter.getMetadata(), importedTrackMetadata);
            }
        } else {
            // Verify that the synchronization time stamp has not been updated.
            EXPECT_EQ(
                    trackRecordAfter.getSourceSynchronizedAt(),
                    trackRecordBefore.getSourceSynchronizedAt());
            if (importResult == mixxx::MetadataSource::ImportResult::Succeeded) {
                // Verify that the metadata in the file has not been modified.
                EXPECT_EQ(fileLastModifiedBefore, sourceSynchronizedAt);
                EXPECT_EQ(trackRecordBefore.getMetadata(), importedTrackMetadata);
            }
        }
    }

    std::unique_ptr<TempFileSystem>
            m_pTempFileSystem;
    TrackId m_trackId;
};

TEST_F(LibraryFileSyncTest, checkSourceSyncStatus) {
    const auto pTrack = loadTrack();
    EXPECT_EQ(mixxx::TrackRecord::SourceSyncStatus::Void,
            mixxx::TrackRecord{}.checkSourceSyncStatus(pTrack->getFileInfo()));
    const auto trackRecord = pTrack->getRecord();
    EXPECT_EQ(mixxx::TrackRecord::SourceSyncStatus::Synchronized,
            trackRecord.checkSourceSyncStatus(pTrack->getFileInfo()));
    m_pTempFileSystem->updateFileLastModified();
    EXPECT_EQ(mixxx::TrackRecord::SourceSyncStatus::Outdated,
            trackRecord.checkSourceSyncStatus(pTrack->getFileInfo()));
    m_pTempFileSystem->removeFile();
    EXPECT_EQ(mixxx::TrackRecord::SourceSyncStatus::Undefined,
            trackRecord.checkSourceSyncStatus(pTrack->getFileInfo()));
}

TEST_F(LibraryFileSyncTest, saveTrackMetadataWithSourceSyncStatusSynchronizedAndSyncEnabled) {
    TrackPointer pTrack;
    loadTrackFromDatabaseAndVerifySourceSyncStatus(
            &pTrack,
            mixxx::TrackRecord::SourceSyncStatus::Synchronized);
    modifyAndSaveTrack(std::move(pTrack), true);
}

TEST_F(LibraryFileSyncTest, saveTrackMetadataWithSourceSyncStatusSynchronizedAndSyncDisabled) {
    TrackPointer pTrack;
    loadTrackFromDatabaseAndVerifySourceSyncStatus(
            &pTrack,
            mixxx::TrackRecord::SourceSyncStatus::Synchronized);
    modifyAndSaveTrack(std::move(pTrack), false);
}

TEST_F(LibraryFileSyncTest, saveTrackMetadataWithSourceSyncStatusUnknownAndSyncEnabled) {
    TrackPointer pTrack;
    establishSourceSyncStatusUnknown(&pTrack);
    modifyAndSaveTrack(std::move(pTrack), true);
}

TEST_F(LibraryFileSyncTest, saveTrackMetadataWithSourceSyncStatusUnknownAndSyncDisabled) {
    TrackPointer pTrack;
    establishSourceSyncStatusUnknown(&pTrack);
    modifyAndSaveTrack(std::move(pTrack), false);
}

TEST_F(LibraryFileSyncTest, saveTrackMetadataWithSourceSyncStatusOutdatedAndSyncEnabled) {
    TrackPointer pTrack;
    establishSourceSyncStatusOutdated(&pTrack);
    modifyAndSaveTrack(std::move(pTrack), true);
}

TEST_F(LibraryFileSyncTest, saveTrackMetadataWithSourceSyncStatusOutdatedAndSyncDisabled) {
    TrackPointer pTrack;
    establishSourceSyncStatusOutdated(&pTrack);
    modifyAndSaveTrack(std::move(pTrack), false);
}

TEST_F(LibraryFileSyncTest, saveTrackMetadataWithSourceSyncStatusUndefinedAndSyncEnabled) {
    TrackPointer pTrack;
    establishSourceSyncStatusUndefined(&pTrack);
    modifyAndSaveTrack(std::move(pTrack), true);
}

TEST_F(LibraryFileSyncTest, saveTrackMetadataWithSourceSyncStatusUndefinedAndSyncDisabled) {
    TrackPointer pTrack;
    establishSourceSyncStatusUndefined(&pTrack);
    modifyAndSaveTrack(std::move(pTrack), false);
}

TEST_F(LibraryFileSyncTest, reimportOutdatedTrackMetadataWithSyncEnabled) {
    mixxx::TrackRecord outdatedTrackRecord;
    {
        TrackPointer pOutdatedTrack;
        establishSourceSyncStatusOutdated(&pOutdatedTrack);
        outdatedTrackRecord = pOutdatedTrack->getRecord();
    }

    const auto syncTrackMetadataConfigScope =
            SyncTrackMetadataConfigScope(m_pConfig);

    // Enabled sync
    syncTrackMetadataConfigScope.setConfig(true);

    auto pTrack = loadTrack();
    ASSERT_TRUE(pTrack);
    const auto importedTrackRecord = pTrack->getRecord();
    verifySourceSyncStatusOfTrack(
            *pTrack, mixxx::TrackRecord::SourceSyncStatus::Synchronized);
    EXPECT_LT(
            outdatedTrackRecord.getSourceSynchronizedAt(),
            importedTrackRecord.getSourceSynchronizedAt());
}

TEST_F(LibraryFileSyncTest, doNotReimportOutdatedTrackMetadataWithSyncDisabled) {
    mixxx::TrackRecord outdatedTrackRecord;
    {
        TrackPointer pOutdatedTrack;
        establishSourceSyncStatusOutdated(&pOutdatedTrack);
        outdatedTrackRecord = pOutdatedTrack->getRecord();
    }

    const auto syncTrackMetadataConfigScope =
            SyncTrackMetadataConfigScope(m_pConfig);

    // Disable sync
    syncTrackMetadataConfigScope.setConfig(false);

    auto pTrack = loadTrack();
    ASSERT_TRUE(pTrack);
    const auto loadedTrackRecord = pTrack->getRecord();
    verifySourceSyncStatusOfTrack(
            *pTrack, mixxx::TrackRecord::SourceSyncStatus::Outdated);
    EXPECT_EQ(outdatedTrackRecord, loadedTrackRecord);
}
