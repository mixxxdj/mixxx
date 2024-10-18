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

const QString kTestFileWithMetadata = QStringLiteral("id3-test-data/cover-test-jpg.mp3");
const QString kTestFileWithoutMetadata = QStringLiteral("id3-test-data/empty.mp3");

enum class AdjustFileTime {
    Earlier,
    Later,
};

enum class FileType {
    WithMetadata,
    WithoutMetadata,
};

constexpr qint64 kSecondsPerHour = 3600;

/// A temporary file system with a single audio file.
class TempFileSystem {
  public:
    explicit TempFileSystem(
            const QFileInfo& testFile)
            : m_fileInfo(m_tempDir.filePath(testFile.fileName())) {
        EXPECT_TRUE(m_tempDir.isValid());
        EXPECT_FALSE(m_fileInfo.exists());
        mixxxtest::copyFile(
                testFile.absoluteFilePath(),
                m_fileInfo.location());
        EXPECT_TRUE(m_fileInfo.exists());
    }

    const mixxx::FileInfo& fileInfo() const {
        return m_fileInfo;
    }

    QDateTime fileLastModified() const {
        return m_fileInfo.toQFile().fileTime(QFileDevice::FileModificationTime);
    }

    void adjustFileLastModified(AdjustFileTime adjustFileTime) const {
        const auto oldFileLastModified = fileLastModified();
        EXPECT_TRUE(oldFileLastModified.isValid());
        auto file = m_fileInfo.toQFile();
        EXPECT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Unbuffered));
        switch (adjustFileTime) {
        case AdjustFileTime::Earlier: {
            // Date back the last file modification 1 hour into the past
            const QDateTime fakeDateTime = oldFileLastModified.addSecs(-kSecondsPerHour);
            EXPECT_TRUE(file.setFileTime(
                    fakeDateTime,
                    QFileDevice::FileModificationTime));
            break;
        }
        case AdjustFileTime::Later: {
            // Predate the last file modification 1 hour into the future
            const QDateTime fakeDateTime = oldFileLastModified.addSecs(kSecondsPerHour);
            EXPECT_TRUE(file.setFileTime(
                    fakeDateTime,
                    QFileDevice::FileModificationTime));
            const auto newFileLastModified = fileLastModified();
            EXPECT_TRUE(newFileLastModified.isValid());
            EXPECT_GT(newFileLastModified, oldFileLastModified);
            break;
        }
        default:
            DEBUG_ASSERT(!"unreachable");
        }
        file.close();
    }

    void removeFile() const {
        EXPECT_TRUE(m_fileInfo.toQFile().remove());
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

class SyncTrackMetadataTest : public LibraryTest {
  protected:
    explicit SyncTrackMetadataTest(
            const QString& testFile)
            : m_tempFileSystem(QFileInfo(getTestDir().filePath(testFile))) {
        // Date back the file before adding it to the track collection
        // to ensure that all newly generated time stamps are strictly
        // greater than the synchronization time stamp in the library.
        adjustFileLastModified(AdjustFileTime::Earlier);
        const auto trackRef = TrackRef::fromFileInfo(m_tempFileSystem.fileInfo());
        EXPECT_FALSE(trackCollectionManager()->getTrackByRef(trackRef));
        const auto pTrack = getOrAddTrackByLocation(trackRef.getLocation());
        EXPECT_TRUE(trackCollectionManager()->getTrackByRef(trackRef));
        m_trackId = pTrack->getId();
    }

    TrackPointer loadTrack() const {
        const auto pTrack = trackCollectionManager()->getTrackById(m_trackId);
        DEBUG_ASSERT(pTrack);
        return pTrack;
    }

    void saveModifiedTrack(TrackPointer&& pTrack) const {
        EXPECT_NE(nullptr, pTrack);
        EXPECT_EQ(1, pTrack.use_count());
        const auto trackId = pTrack->getId();
        EXPECT_TRUE(trackId.isValid());
        EXPECT_TRUE(pTrack->isDirty());
        pTrack.reset();
        EXPECT_EQ(nullptr, GlobalTrackCacheLocker().lookupTrackById(trackId));
    }

    void verifySourceSyncStatusOfTrack(
            const Track& track,
            mixxx::TrackRecord::SourceSyncStatus expectedSourceSyncStatus) const {
        const auto trackRecord = track.getRecord();
        EXPECT_EQ(expectedSourceSyncStatus,
                trackRecord.checkSourceSyncStatus(track.getFileInfo()));

        // Verify all time stamps for consistency
        const auto sourceSynchronizedAt = trackRecord.getSourceSynchronizedAt();
        const auto fileLastModified = m_tempFileSystem.fileLastModified();
        switch (expectedSourceSyncStatus) {
        case mixxx::TrackRecord::SourceSyncStatus::Void:
        case mixxx::TrackRecord::SourceSyncStatus::Synchronized:
            EXPECT_TRUE(sourceSynchronizedAt.isValid());
            EXPECT_TRUE(fileLastModified.isValid());
            EXPECT_EQ(fileLastModified, sourceSynchronizedAt);
            break;
        case mixxx::TrackRecord::SourceSyncStatus::Outdated:
            EXPECT_TRUE(sourceSynchronizedAt.isValid());
            EXPECT_TRUE(fileLastModified.isValid());
            EXPECT_GT(fileLastModified, sourceSynchronizedAt);
            break;
        case mixxx::TrackRecord::SourceSyncStatus::Unknown:
            EXPECT_FALSE(sourceSynchronizedAt.isValid());
            // Independent of if the file is accessible or not
            break;
        case mixxx::TrackRecord::SourceSyncStatus::Undefined:
            EXPECT_TRUE(sourceSynchronizedAt.isValid());
            EXPECT_FALSE(fileLastModified.isValid());
            break;
        default:
            DEBUG_ASSERT(!"unreachable");
        }
    }

    TrackPointer reloadTrackFromDatabaseAndVerifySourceSyncStatus(
            mixxx::TrackRecord::SourceSyncStatus expectedSourceSyncStatus) const {
        const auto syncTrackMetadataConfigScope =
                SyncTrackMetadataConfigScope(m_pConfig);

        // Disable sync
        syncTrackMetadataConfigScope.setConfig(false);

        const auto pTrack = loadTrack();
        EXPECT_NE(nullptr, pTrack);
        verifySourceSyncStatusOfTrack(*pTrack, expectedSourceSyncStatus);

        return pTrack;
    }

    void modifyAndSaveTrack(
            TrackPointer&& pTrack,
            FileType fileType,
            bool syncTrackMetadata) {
        EXPECT_NE(nullptr, pTrack);

        const auto fileLastModifiedBefore = m_tempFileSystem.fileLastModified();
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
        EXPECT_EQ(nullptr, pTrack);

        // Disable sync temporarily for the final verification to prevent
        // reimporting metadata from the file unintentionally, i.e. switch
        // the library into read-only mode.
        syncTrackMetadataConfigScope.setConfig(false);

        const auto fileLastModifiedAfter = m_tempFileSystem.fileLastModified();
        EXPECT_EQ(fileLastModifiedAfter.isValid(), fileLastModifiedBefore.isValid());
        if (syncTrackMetadata && fileType == FileType::WithMetadata) {
            // Verify that the file has been modified upon saving
            EXPECT_TRUE(
                    !fileLastModifiedAfter.isValid() ||
                    fileLastModifiedAfter > fileLastModifiedBefore);
        } else {
            // Verify that the file has not been modified upon saving
            EXPECT_EQ(fileLastModifiedAfter, fileLastModifiedBefore);
        }

        mixxx::MetadataSource::ImportResult expectedImportResult;
        switch (fileType) {
        case FileType::WithMetadata:
            expectedImportResult = mixxx::MetadataSource::ImportResult::Succeeded;
            break;
        case FileType::WithoutMetadata:
            expectedImportResult = mixxx::MetadataSource::ImportResult::Unavailable;
            break;
        default:
            DEBUG_ASSERT(!"unreachable");
        }

        auto expectedSourceSyncStatusAfter = sourceSyncStatusBefore;
        switch (sourceSyncStatusBefore) {
        case mixxx::TrackRecord::SourceSyncStatus::Unknown:
        case mixxx::TrackRecord::SourceSyncStatus::Outdated:
            if (syncTrackMetadata &&
                    fileType == FileType::WithMetadata) {
                expectedSourceSyncStatusAfter = mixxx::TrackRecord::SourceSyncStatus::Synchronized;
            }
            break;
        case mixxx::TrackRecord::SourceSyncStatus::Void:
        case mixxx::TrackRecord::SourceSyncStatus::Undefined:
            expectedImportResult = mixxx::MetadataSource::ImportResult::Unavailable;
            break;
        case mixxx::TrackRecord::SourceSyncStatus::Synchronized:
            break;
        default:
            DEBUG_ASSERT(!"unreachable");
        }

        if (syncTrackMetadata &&
                !fileLastModifiedAfter.isValid()) {
            // The source sync timestamp should be reset if the track
            // metadata export failed due to an inaccessible file.
            expectedSourceSyncStatusAfter = mixxx::TrackRecord::SourceSyncStatus::Unknown;
        }

        // Verify that the modified metadata is still present after
        // reloading the track from the database
        pTrack = reloadTrackFromDatabaseAndVerifySourceSyncStatus(
                expectedSourceSyncStatusAfter);
        EXPECT_NE(nullptr, pTrack);
        const auto trackRecordAfter = pTrack->getRecord();

        // Reimport metadata from the file
        mixxx::TrackMetadata importedTrackMetadata;
        // TODO: Test both use cases, i.e. also with resetMissingTagMetadata = true.
        // Currently this option is disabled and not configurable in the UI.
        constexpr auto resetMissingTagMetadata = false;
        auto [importResult, sourceSynchronizedAt] =
                SoundSourceProxy(pTrack).importTrackMetadataAndCoverImage(
                        &importedTrackMetadata, nullptr, resetMissingTagMetadata);
        EXPECT_EQ(expectedImportResult, importResult);

        if (syncTrackMetadata) {
            // Verify that the synchronization time stamp has been reset
            // if the file is inaccessible, i.e. after export of track metadata
            // has failed.
            EXPECT_TRUE(
                    fileLastModifiedAfter.isValid() ||
                    !trackRecordAfter.getSourceSynchronizedAt().isValid());
            if (fileType == FileType::WithMetadata) {
                // The stream info properties might be adjusted when exporting track metadata!
                // We have to adjust trackRecordBefore and importedTrackMetadata accordingly
                // before continuing with the verification.
                trackRecordBefore.refMetadata().refStreamInfo() =
                        trackRecordAfter.getMetadata().getStreamInfo();
                if (importResult == mixxx::MetadataSource::ImportResult::Succeeded) {
                    importedTrackMetadata.refStreamInfo() =
                            trackRecordAfter.getMetadata().getStreamInfo();
                    // Verify that the metadata in the file has been modified.
                    EXPECT_LT(fileLastModifiedBefore, sourceSynchronizedAt);
                    EXPECT_EQ(trackRecordAfter.getSourceSynchronizedAt(), sourceSynchronizedAt);
                    EXPECT_EQ(trackRecordAfter.getMetadata(), importedTrackMetadata);
                }
                // Verify that the synchronization time stamp has been updated
                // if the file still exists.
                EXPECT_TRUE(
                        !fileLastModifiedAfter.isValid() ||
                        !trackRecordBefore.getSourceSynchronizedAt().isValid() ||
                        trackRecordBefore.getSourceSynchronizedAt() <
                                trackRecordAfter.getSourceSynchronizedAt());
            } else {
                // Verify that the file has not been modified upon saving
                EXPECT_EQ(fileLastModifiedAfter, fileLastModifiedBefore);
                // Verify that the synchronization time stamp has not been updated
                // if the file still exists.
                EXPECT_TRUE(
                        !fileLastModifiedAfter.isValid() ||
                        trackRecordBefore.getSourceSynchronizedAt() ==
                                trackRecordAfter.getSourceSynchronizedAt());
            }
        } else {
            // Verify that the file has not been modified upon saving
            EXPECT_EQ(fileLastModifiedAfter, fileLastModifiedBefore);
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

        // Verify that the metadata in the database has been modified.
        EXPECT_NE(trackRecordBefore.getMetadata(), trackRecordAfter.getMetadata());
        EXPECT_EQ(newTitle, pTrack->getTitle());
    }

    QDateTime fileLastModified() const {
        return m_tempFileSystem.fileLastModified();
    }

    void adjustFileLastModified(AdjustFileTime adjustFileTime) const {
        m_tempFileSystem.adjustFileLastModified(adjustFileTime);
    }

    void removeFile() const {
        m_tempFileSystem.removeFile();
    }

    virtual TrackPointer prepareTestTrack() const = 0;

    void checkTrackRecordSourceSyncStatus(
            mixxx::TrackRecord::SourceSyncStatus expectedSourceSyncStatus) const {
        const auto pTrack = prepareTestTrack();
        ASSERT_NE(nullptr, pTrack);
        const mixxx::TrackRecord emptyTrackRecord;
        EXPECT_EQ(mixxx::TrackRecord::SourceSyncStatus::Void,
                emptyTrackRecord.checkSourceSyncStatus(pTrack->getFileInfo()));
        const auto trackRecordOfTestTrack = pTrack->getRecord();
        EXPECT_EQ(expectedSourceSyncStatus,
                trackRecordOfTestTrack.checkSourceSyncStatus(pTrack->getFileInfo()));
    }

  private:
    TempFileSystem m_tempFileSystem;
    TrackId m_trackId;
};

class SyncTrackMetadataStatusSynchronizedTest : public SyncTrackMetadataTest {
  protected:
    explicit SyncTrackMetadataStatusSynchronizedTest(const QString& testFile)
            : SyncTrackMetadataTest(testFile) {
    }

    TrackPointer prepareTestTrack() const override {
        return reloadTrackFromDatabaseAndVerifySourceSyncStatus(
                mixxx::TrackRecord::SourceSyncStatus::Synchronized);
    }

    void checkTrackRecordSourceSyncStatus() const {
        SyncTrackMetadataTest::checkTrackRecordSourceSyncStatus(
                mixxx::TrackRecord::SourceSyncStatus::Synchronized);
    }
};

class LibraryFileWithMetadataSyncStatusSynchronizedTest
        : public SyncTrackMetadataStatusSynchronizedTest {
  public:
    LibraryFileWithMetadataSyncStatusSynchronizedTest()
            : SyncTrackMetadataStatusSynchronizedTest(kTestFileWithMetadata) {
    }
};

TEST_F(LibraryFileWithMetadataSyncStatusSynchronizedTest, checkTrackRecordSourceSyncStatus) {
    SyncTrackMetadataStatusSynchronizedTest::checkTrackRecordSourceSyncStatus();
}

TEST_F(LibraryFileWithMetadataSyncStatusSynchronizedTest, saveTrackMetadataWithSyncEnabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithMetadata, true);
}

TEST_F(LibraryFileWithMetadataSyncStatusSynchronizedTest, saveTrackMetadataWithSyncDisabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithMetadata, false);
}

class LibraryFileWithoutMetadataSyncStatusSynchronizedTest
        : public SyncTrackMetadataStatusSynchronizedTest {
  public:
    LibraryFileWithoutMetadataSyncStatusSynchronizedTest()
            : SyncTrackMetadataStatusSynchronizedTest(kTestFileWithoutMetadata) {
    }
};

TEST_F(LibraryFileWithoutMetadataSyncStatusSynchronizedTest, checkTrackRecordSourceSyncStatus) {
    SyncTrackMetadataStatusSynchronizedTest::checkTrackRecordSourceSyncStatus();
}

TEST_F(LibraryFileWithoutMetadataSyncStatusSynchronizedTest, saveTrackMetadataWithSyncEnabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithoutMetadata, true);
}

TEST_F(LibraryFileWithoutMetadataSyncStatusSynchronizedTest, saveTrackMetadataWithSyncDisabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithoutMetadata, false);
}

class SyncTrackMetadataStatusOutdatedTest : public SyncTrackMetadataTest {
  public:
    explicit SyncTrackMetadataStatusOutdatedTest(const QString& testFile)
            : SyncTrackMetadataTest(testFile) {
        // Predate the file after it has already been added to the library
        // in the base class constructor.
        adjustFileLastModified(AdjustFileTime::Later);
    }

  protected:
    TrackPointer prepareTestTrack() const override {
        // The track is loaded directly from the database and will still
        // have the previous synchronization time stamp. It should then
        // be detected as outdated.
        return reloadTrackFromDatabaseAndVerifySourceSyncStatus(
                mixxx::TrackRecord::SourceSyncStatus::Outdated);
    }

    void checkTrackRecordSourceSyncStatus() const {
        SyncTrackMetadataTest::checkTrackRecordSourceSyncStatus(
                mixxx::TrackRecord::SourceSyncStatus::Outdated);
    }
};

class LibraryFileWithMetadataSyncStatusOutdatedTest : public SyncTrackMetadataStatusOutdatedTest {
  public:
    LibraryFileWithMetadataSyncStatusOutdatedTest()
            : SyncTrackMetadataStatusOutdatedTest(kTestFileWithMetadata) {
    }
};

TEST_F(LibraryFileWithMetadataSyncStatusOutdatedTest, checkTrackRecordSourceSyncStatus) {
    SyncTrackMetadataStatusOutdatedTest::checkTrackRecordSourceSyncStatus();
}

TEST_F(LibraryFileWithMetadataSyncStatusOutdatedTest, saveTrackMetadataWithSyncEnabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithMetadata, true);
}

TEST_F(LibraryFileWithMetadataSyncStatusOutdatedTest, saveTrackMetadataWithSyncDisabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithMetadata, false);
}

TEST_F(LibraryFileWithMetadataSyncStatusOutdatedTest, reimportTrackMetadataWithSyncEnabled) {
    const mixxx::TrackRecord outdatedTrackRecord =
            prepareTestTrack()->getRecord();

    const auto syncTrackMetadataConfigScope =
            SyncTrackMetadataConfigScope(m_pConfig);

    // Enable sync
    syncTrackMetadataConfigScope.setConfig(true);

    auto pTrack = loadTrack();
    EXPECT_TRUE(pTrack);
    const auto importedTrackRecord = pTrack->getRecord();
    verifySourceSyncStatusOfTrack(
            *pTrack, mixxx::TrackRecord::SourceSyncStatus::Synchronized);
    EXPECT_LT(
            outdatedTrackRecord.getSourceSynchronizedAt(),
            importedTrackRecord.getSourceSynchronizedAt());
}

TEST_F(LibraryFileWithMetadataSyncStatusOutdatedTest, doNotReimportTrackMetadataWithSyncDisabled) {
    const mixxx::TrackRecord outdatedTrackRecord =
            prepareTestTrack()->getRecord();

    const auto syncTrackMetadataConfigScope =
            SyncTrackMetadataConfigScope(m_pConfig);

    // Disable sync
    syncTrackMetadataConfigScope.setConfig(false);

    auto pTrack = loadTrack();
    EXPECT_TRUE(pTrack);
    const auto loadedTrackRecord = pTrack->getRecord();
    verifySourceSyncStatusOfTrack(
            *pTrack, mixxx::TrackRecord::SourceSyncStatus::Outdated);
    EXPECT_EQ(outdatedTrackRecord, loadedTrackRecord);
}

class LibraryFileWithoutMetadataSyncStatusOutdatedTest
        : public SyncTrackMetadataStatusOutdatedTest {
  public:
    LibraryFileWithoutMetadataSyncStatusOutdatedTest()
            : SyncTrackMetadataStatusOutdatedTest(kTestFileWithoutMetadata) {
    }
};

TEST_F(LibraryFileWithoutMetadataSyncStatusOutdatedTest, checkTrackRecordSourceSyncStatus) {
    SyncTrackMetadataStatusOutdatedTest::checkTrackRecordSourceSyncStatus();
}

TEST_F(LibraryFileWithoutMetadataSyncStatusOutdatedTest, saveTrackMetadataWithSyncEnabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithoutMetadata, true);
}

TEST_F(LibraryFileWithoutMetadataSyncStatusOutdatedTest, saveTrackMetadataWithSyncDisabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithoutMetadata, false);
}

class SyncTrackMetadataStatusUnknownTest : public SyncTrackMetadataTest {
  public:
    explicit SyncTrackMetadataStatusUnknownTest(const QString& testFile)
            : SyncTrackMetadataTest(testFile) {
    }

  protected:
    TrackPointer prepareTestTrack() const override {
        TrackPointer pTrack =
                reloadTrackFromDatabaseAndVerifySourceSyncStatus(
                        mixxx::TrackRecord::SourceSyncStatus::Synchronized);
        EXPECT_NE(nullptr, pTrack);

        const auto syncTrackMetadataConfigScope =
                SyncTrackMetadataConfigScope(m_pConfig);

        // Disable sync
        syncTrackMetadataConfigScope.setConfig(false);

        const auto fileLastModifiedBefore = fileLastModified();

        // The column contains NULL as the default value, i.e. the last
        // synchronization time is unknown.
        pTrack->resetSourceSynchronizedAt();
        // Write the data into the database (without modifying the file)
        saveModifiedTrack(std::move(pTrack));
        EXPECT_EQ(nullptr, pTrack);

        // Verify that the file has not been modified
        const auto fileLastModifiedAfter = fileLastModified();
        EXPECT_EQ(fileLastModifiedAfter, fileLastModifiedBefore);

        return reloadTrackFromDatabaseAndVerifySourceSyncStatus(
                mixxx::TrackRecord::SourceSyncStatus::Unknown);
    }

    void checkTrackRecordSourceSyncStatus() const {
        SyncTrackMetadataTest::checkTrackRecordSourceSyncStatus(
                mixxx::TrackRecord::SourceSyncStatus::Unknown);
    }
};

class LibraryFileWithMetadataSyncStatusUnknownTest : public SyncTrackMetadataStatusUnknownTest {
  public:
    LibraryFileWithMetadataSyncStatusUnknownTest()
            : SyncTrackMetadataStatusUnknownTest(kTestFileWithMetadata) {
    }
};

TEST_F(LibraryFileWithMetadataSyncStatusUnknownTest, checkTrackRecordSourceSyncStatus) {
    SyncTrackMetadataStatusUnknownTest::checkTrackRecordSourceSyncStatus();
}

TEST_F(LibraryFileWithMetadataSyncStatusUnknownTest, saveTrackMetadataWithSyncEnabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithMetadata, true);
}

TEST_F(LibraryFileWithMetadataSyncStatusUnknownTest, saveTrackMetadataWithSyncDisabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithMetadata, false);
}

class LibraryFileWithoutMetadataSyncStatusUnknownTest : public SyncTrackMetadataStatusUnknownTest {
  public:
    LibraryFileWithoutMetadataSyncStatusUnknownTest()
            : SyncTrackMetadataStatusUnknownTest(kTestFileWithoutMetadata) {
    }
};

TEST_F(LibraryFileWithoutMetadataSyncStatusUnknownTest, checkTrackRecordSourceSyncStatus) {
    SyncTrackMetadataStatusUnknownTest::checkTrackRecordSourceSyncStatus();
}

TEST_F(LibraryFileWithoutMetadataSyncStatusUnknownTest, saveTrackMetadataWithSyncEnabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithoutMetadata, true);
}

TEST_F(LibraryFileWithoutMetadataSyncStatusUnknownTest, saveTrackMetadataWithSyncDisabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithoutMetadata, false);
}

class SyncTrackMetadataStatusUndefinedTest : public SyncTrackMetadataTest {
  public:
    explicit SyncTrackMetadataStatusUndefinedTest(const QString& testFile)
            : SyncTrackMetadataTest(testFile) {
    }

  protected:
    TrackPointer prepareTestTrack() const override {
        removeFile();

        return reloadTrackFromDatabaseAndVerifySourceSyncStatus(
                mixxx::TrackRecord::SourceSyncStatus::Undefined);
    }

    void checkTrackRecordSourceSyncStatus() const {
        SyncTrackMetadataTest::checkTrackRecordSourceSyncStatus(
                mixxx::TrackRecord::SourceSyncStatus::Undefined);
    }
};

class LibraryFileWithMetadataSyncStatusUndefinedTest : public SyncTrackMetadataStatusUndefinedTest {
  public:
    LibraryFileWithMetadataSyncStatusUndefinedTest()
            : SyncTrackMetadataStatusUndefinedTest(kTestFileWithMetadata) {
    }
};

TEST_F(LibraryFileWithMetadataSyncStatusUndefinedTest, checkTrackRecordSourceSyncStatus) {
    SyncTrackMetadataStatusUndefinedTest::checkTrackRecordSourceSyncStatus();
}

TEST_F(LibraryFileWithMetadataSyncStatusUndefinedTest, saveTrackMetadataWithSyncEnabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithMetadata, true);
}

TEST_F(LibraryFileWithMetadataSyncStatusUndefinedTest, saveTrackMetadataWithSyncDisabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithMetadata, false);
}

class LibraryFileWithoutMetadataSyncStatusUndefinedTest
        : public SyncTrackMetadataStatusUndefinedTest {
  public:
    LibraryFileWithoutMetadataSyncStatusUndefinedTest()
            : SyncTrackMetadataStatusUndefinedTest(kTestFileWithoutMetadata) {
    }
};

TEST_F(LibraryFileWithoutMetadataSyncStatusUndefinedTest, checkTrackRecordSourceSyncStatus) {
    SyncTrackMetadataStatusUndefinedTest::checkTrackRecordSourceSyncStatus();
}

TEST_F(LibraryFileWithoutMetadataSyncStatusUndefinedTest, saveTrackMetadataWithSyncEnabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithoutMetadata, true);
}

TEST_F(LibraryFileWithoutMetadataSyncStatusUndefinedTest, saveTrackMetadataWithSyncDisabled) {
    modifyAndSaveTrack(prepareTestTrack(), FileType::WithoutMetadata, false);
}
