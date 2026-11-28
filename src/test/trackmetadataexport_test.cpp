#include <gtest/gtest.h>

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include "sources/metadatasourcetaglib.h"
#include "test/mixxxtest.h"
#include "test/soundsourceproviderregistration.h"
#include "track/globaltrackcache.h"
#include "track/track.h"

namespace {

const QString kEmptyFile = QStringLiteral("empty.mp3");

void deleteTrack(Track* pTrack) {
    // Delete track objects directly in unit tests with
    // no main event loop
    delete pTrack;
};

class GlobalTrackCacheHelper : public GlobalTrackCacheSaver {
  public:
    void saveEvictedTrack(Track* pTrack) noexcept override {
        ASSERT_FALSE(pTrack == nullptr);
    }
    GlobalTrackCacheHelper() {
        GlobalTrackCache::createInstance(this, deleteTrack);
    }
    ~GlobalTrackCacheHelper() override {
        GlobalTrackCache::destroyInstance();
    }
};

} // namespace

class TrackMetadataExportTest : public MixxxTest, private SoundSourceProviderRegistration {
  public:
    TrackMetadataExportTest()
            : m_testDataDir(getTestDir().absoluteFilePath(QStringLiteral("id3-test-data"))) {
    }

  protected:
    const QDir m_testDataDir;
    QTemporaryDir m_exportTempDir;
    GlobalTrackCacheHelper m_globalTrackCacheHelper;
};

TEST_F(TrackMetadataExportTest, keepWithespaceKey) {
    const QString kWhiteSpacesKey = QStringLiteral("  A#m  ");
    const QString kNormalizedDisplayKey = QString::fromUtf8("B♭m");
    constexpr std::string_view kId3Key = "Bbm";

    // Generate a file name for exporting metadata
    const QString exportTrackPath = m_exportTempDir.filePath("keepWithespaceKey.mp3");
    mixxxtest::copyFile(m_testDataDir.absoluteFilePath(kEmptyFile), exportTrackPath);
    TrackPointer pTrack = Track::newTemporary(exportTrackPath);

    mixxx::TrackMetadata writeTrackMetadata;
    writeTrackMetadata.refTrackInfo().setKeyText(kWhiteSpacesKey);

    // the internal value is still unchanged
    EXPECT_EQ(writeTrackMetadata.getTrackInfo().getKeyText().toStdString(),
            kWhiteSpacesKey.toStdString());

    // This saves the metadata object literally, but normalizes
    // the global key value as StandardID3v2
    pTrack->replaceMetadataFromSource(
            std::move(writeTrackMetadata),
            QDateTime::currentDateTimeUtc());

    // getKeytext returns the normalized version suitable for GUI presentation
    EXPECT_EQ(pTrack->getKeyText().toStdString(), kNormalizedDisplayKey.toStdString());

    // the internal value is still unchanged
    EXPECT_EQ(pTrack->getRecord()
                      .getMetadata()
                      .getTrackInfo()
                      .getKeyText()
                      .toStdString(),
            kWhiteSpacesKey.toStdString());

    pTrack->markForMetadataExport();
    SyncTrackMetadataParams params;
    ExportTrackMetadataResult result =
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params);
    EXPECT_EQ(result, ExportTrackMetadataResult::Succeeded);

    // recreate the Track
    mixxx::TrackMetadata readTrackMetadata;
    SoundSourceProxy::importTrackMetadataAndCoverImageFromFile(
            mixxx::FileAccess(mixxx::FileInfo(exportTrackPath)),
            &readTrackMetadata,
            nullptr,
            false);

    // the internal value is still unchanged
    EXPECT_EQ(readTrackMetadata.getTrackInfo().getKeyText().toStdString(),
            kWhiteSpacesKey.toStdString());

    pTrack->replaceMetadataFromSource(
            readTrackMetadata,
            QDateTime::currentDateTimeUtc());

    // getKeytext returns the normalized version suitable for GUI presentation
    EXPECT_EQ(pTrack->getKeyText().toStdString(), kNormalizedDisplayKey.toStdString());

    // the internal value is still unchanged
    EXPECT_EQ(pTrack->getRecord()
                      .getMetadata()
                      .getTrackInfo()
                      .getKeyText()
                      .toStdString(),
            kWhiteSpacesKey.toStdString());

    // Reject edits which results to the same key
    pTrack->setKeyText(kNormalizedDisplayKey);
    EXPECT_EQ(pTrack->getRecord()
                      .getMetadata()
                      .getTrackInfo()
                      .getKeyText()
                      .toStdString(),
            kWhiteSpacesKey.toStdString());

    // Allow to remove key with an empty string
    pTrack->setKeyText("");
    EXPECT_EQ(pTrack->getRecord()
                      .getMetadata()
                      .getTrackInfo()
                      .getKeyText()
                      .toStdString(),
            QString().toStdString());

    // normalize user edits
    pTrack->setKeyText(kWhiteSpacesKey);
    // the internal value is now at the preferred ID3v2 format
    EXPECT_EQ(pTrack->getKeys().getGlobalKeyText().toStdString(), kId3Key);
}

namespace {

std::optional<int> readRatingFromFile(const QString& filePath) {
    return mixxx::MetadataSourceTagLib(filePath, QStringLiteral("mp3"))
            .importRating();
}

// Prepare a temporary track for metadata export: without an initial
// metadata import checkSourceSyncStatus() reports Void and exports
// are skipped.
TrackPointer newTrackWithImportedMetadata(const QString& filePath) {
    TrackPointer pTrack = Track::newTemporary(filePath);
    mixxx::TrackMetadata trackMetadata;
    SoundSourceProxy::importTrackMetadataAndCoverImageFromFile(
            mixxx::FileAccess(mixxx::FileInfo(filePath)),
            &trackMetadata,
            nullptr,
            false);
    pTrack->replaceMetadataFromSource(
            std::move(trackMetadata),
            QDateTime::currentDateTimeUtc());
    return pTrack;
}

} // namespace

TEST_F(TrackMetadataExportTest, ClearRatingRemovesFileTag) {
    const QString exportTrackPath = m_exportTempDir.filePath("clear_rating.mp3");
    mixxxtest::copyFile(m_testDataDir.absoluteFilePath(kEmptyFile), exportTrackPath);
    TrackPointer pTrack = newTrackWithImportedMetadata(exportTrackPath);

    SyncTrackMetadataParams params;
    params.exportRatingToFile = true;

    // Export a rating into the file
    pTrack->setRating(4);
    pTrack->markForMetadataExport();
    EXPECT_EQ(ExportTrackMetadataResult::Succeeded,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));
    EXPECT_EQ(readRatingFromFile(exportTrackPath).value_or(0), 4);

    // Clearing the rating must remove the tag from the file, otherwise
    // the stale value would be re-imported on the next library scan
    pTrack->setRating(0);
    EXPECT_EQ(ExportTrackMetadataResult::Succeeded,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));
    EXPECT_FALSE(readRatingFromFile(exportTrackPath).has_value());
}

TEST_F(TrackMetadataExportTest, UnchangedRatingSkipsExport) {
    const QString exportTrackPath = m_exportTempDir.filePath("unchanged_rating.mp3");
    mixxxtest::copyFile(m_testDataDir.absoluteFilePath(kEmptyFile), exportTrackPath);
    TrackPointer pTrack = newTrackWithImportedMetadata(exportTrackPath);

    SyncTrackMetadataParams params;
    params.exportRatingToFile = true;

    pTrack->setRating(3);
    pTrack->markForMetadataExport();
    EXPECT_EQ(ExportTrackMetadataResult::Succeeded,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));
    EXPECT_EQ(readRatingFromFile(exportTrackPath).value_or(0), 3);

    // A second export with neither metadata nor rating changes must not
    // rewrite the file
    EXPECT_EQ(ExportTrackMetadataResult::Skipped,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));
}

TEST_F(TrackMetadataExportTest, RatingExportKeepsSourceSynchronized) {
    const QString exportTrackPath = m_exportTempDir.filePath("rating_synced.mp3");
    mixxxtest::copyFile(m_testDataDir.absoluteFilePath(kEmptyFile), exportTrackPath);
    TrackPointer pTrack = newTrackWithImportedMetadata(exportTrackPath);

    SyncTrackMetadataParams params;
    params.exportRatingToFile = true;

    pTrack->setRating(3);
    pTrack->markForMetadataExport();
    EXPECT_EQ(ExportTrackMetadataResult::Succeeded,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));

    // The rating export writes the file after the metadata export
    // recorded its synchronization time stamp. The time stamp must be
    // refreshed afterwards, otherwise the file appears externally
    // modified and triggers a spurious metadata re-import on next load.
    EXPECT_TRUE(pTrack->checkSourceSynchronized());
}

TEST_F(TrackMetadataExportTest, RatingOnlyPrefDoesNotExportOtherTags) {
    const QString exportTrackPath = m_exportTempDir.filePath("rating_only_pref.mp3");
    mixxxtest::copyFile(m_testDataDir.absoluteFilePath(kEmptyFile), exportTrackPath);
    TrackPointer pTrack = newTrackWithImportedMetadata(exportTrackPath);

    // Only rating export is enabled: a full metadata synchronization
    // into file tags has not been opted into
    SyncTrackMetadataParams params;
    params.syncTrackMetadata = false;
    params.exportRatingToFile = true;

    pTrack->setTitle(QStringLiteral("Only In Mixxx"));
    pTrack->setRating(3);
    EXPECT_EQ(ExportTrackMetadataResult::Succeeded,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));

    // The rating must be in the file, the title must not
    EXPECT_EQ(readRatingFromFile(exportTrackPath).value_or(0), 3);
    mixxx::TrackMetadata fileMetadata;
    SoundSourceProxy::importTrackMetadataAndCoverImageFromFile(
            mixxx::FileAccess(mixxx::FileInfo(exportTrackPath)),
            &fileMetadata,
            nullptr,
            false);
    EXPECT_NE(fileMetadata.getTrackInfo().getTitle(),
            QStringLiteral("Only In Mixxx"));
}

// AI-generated: The two tests below cover deferred exports, which are
// scheduled whenever a modified track is saved while still in use.
// (AI-generated)
TEST_F(TrackMetadataExportTest, DeferredExportLeavesUnmodifiedFileUntouched) {
    const QString exportTrackPath = m_exportTempDir.filePath("deferred_unmodified.mp3");
    mixxxtest::copyFile(m_testDataDir.absoluteFilePath(kEmptyFile), exportTrackPath);

    SyncTrackMetadataParams params;
    params.syncTrackMetadata = true;

    // Write some tags into the file by an explicitly requested export
    {
        TrackPointer pTrack = newTrackWithImportedMetadata(exportTrackPath);
        pTrack->setTitle(QStringLiteral("Deferred Title"));
        pTrack->markForMetadataExport();
        ASSERT_EQ(ExportTrackMetadataResult::Succeeded,
                SoundSourceProxy::exportTrackMetadataBeforeSaving(
                        pTrack.get(), params));
    }

    // Backdate the file to detect any subsequent write
    const QDateTime backdated = QDateTime::currentDateTimeUtc().addSecs(-3600);
    {
        QFile file(exportTrackPath);
        ASSERT_TRUE(file.open(QIODevice::ReadWrite));
        ASSERT_TRUE(file.setFileTime(backdated, QFileDevice::FileModificationTime));
    }
    const QDateTime modifiedBefore = QFileInfo(exportTrackPath).lastModified();
    ASSERT_LT(modifiedBefore.secsTo(QDateTime::currentDateTimeUtc()), 3700);
    ASSERT_GT(modifiedBefore.secsTo(QDateTime::currentDateTimeUtc()), 3500);

    // A deferred export of a track whose metadata equals the file tags
    // must not write the file
    TrackPointer pTrack = newTrackWithImportedMetadata(exportTrackPath);
    pTrack->markForDeferredMetadataExport();
    EXPECT_TRUE(pTrack->isMarkedForMetadataExport());
    EXPECT_EQ(ExportTrackMetadataResult::Skipped,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));
    EXPECT_FALSE(pTrack->isMarkedForMetadataExport());
    EXPECT_EQ(modifiedBefore, QFileInfo(exportTrackPath).lastModified());

    // An explicitly requested export still writes the unmodified metadata
    pTrack->markForMetadataExport();
    EXPECT_EQ(ExportTrackMetadataResult::Succeeded,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));
    EXPECT_NE(modifiedBefore, QFileInfo(exportTrackPath).lastModified());
}

TEST_F(TrackMetadataExportTest, DeferredExportWritesModifiedMetadata) {
    const QString exportTrackPath = m_exportTempDir.filePath("deferred_modified.mp3");
    mixxxtest::copyFile(m_testDataDir.absoluteFilePath(kEmptyFile), exportTrackPath);

    SyncTrackMetadataParams params;
    params.syncTrackMetadata = true;

    {
        TrackPointer pTrack = newTrackWithImportedMetadata(exportTrackPath);
        pTrack->setTitle(QStringLiteral("First Title"));
        pTrack->markForMetadataExport();
        ASSERT_EQ(ExportTrackMetadataResult::Succeeded,
                SoundSourceProxy::exportTrackMetadataBeforeSaving(
                        pTrack.get(), params));
    }

    TrackPointer pTrack = newTrackWithImportedMetadata(exportTrackPath);
    pTrack->setTitle(QStringLiteral("Second Title"));
    pTrack->markForDeferredMetadataExport();
    EXPECT_EQ(ExportTrackMetadataResult::Succeeded,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));

    mixxx::TrackMetadata fileMetadata;
    SoundSourceProxy::importTrackMetadataAndCoverImageFromFile(
            mixxx::FileAccess(mixxx::FileInfo(exportTrackPath)),
            &fileMetadata,
            nullptr,
            false);
    EXPECT_EQ(fileMetadata.getTrackInfo().getTitle(),
            QStringLiteral("Second Title"));
}

TEST_F(TrackMetadataExportTest, RatingOnlyChangeExports) {
    const QString exportTrackPath = m_exportTempDir.filePath("rating_only.mp3");
    mixxxtest::copyFile(m_testDataDir.absoluteFilePath(kEmptyFile), exportTrackPath);
    TrackPointer pTrack = newTrackWithImportedMetadata(exportTrackPath);

    SyncTrackMetadataParams params;
    params.exportRatingToFile = true;

    pTrack->setRating(3);
    pTrack->markForMetadataExport();
    EXPECT_EQ(ExportTrackMetadataResult::Succeeded,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));

    // Changing only the rating (not part of TrackMetadata) must still
    // trigger an export without an explicit re-export request
    pTrack->setRating(5);
    EXPECT_EQ(ExportTrackMetadataResult::Succeeded,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));
    EXPECT_EQ(readRatingFromFile(exportTrackPath).value_or(0), 5);
}

namespace {

std::optional<int> readPlayCountFromFile(const QString& filePath) {
    return mixxx::MetadataSourceTagLib(filePath, QStringLiteral("mp3"))
            .importPlayCount();
}

} // namespace

TEST_F(TrackMetadataExportTest, HigherPlayCountExports) {
    const QString exportTrackPath = m_exportTempDir.filePath("playcount_higher.mp3");
    mixxxtest::copyFile(m_testDataDir.absoluteFilePath(kEmptyFile), exportTrackPath);
    TrackPointer pTrack = newTrackWithImportedMetadata(exportTrackPath);

    SyncTrackMetadataParams params;
    params.exportPlayCountToFile = true;

    pTrack->resetPlayCounter(3);
    EXPECT_EQ(ExportTrackMetadataResult::Succeeded,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));
    EXPECT_EQ(readPlayCountFromFile(exportTrackPath).value_or(-1), 3);
    // The write must not make the file appear externally modified
    EXPECT_TRUE(pTrack->checkSourceSynchronized());

    // Nothing changed: the file must not be rewritten
    EXPECT_EQ(ExportTrackMetadataResult::Skipped,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));

    // Playing the track again exports the new total
    pTrack->updatePlayCounter();
    EXPECT_EQ(ExportTrackMetadataResult::Succeeded,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));
    EXPECT_EQ(readPlayCountFromFile(exportTrackPath).value_or(-1), 4);
}

TEST_F(TrackMetadataExportTest, LowerPlayCountNeverOverwritesFile) {
    const QString exportTrackPath = m_exportTempDir.filePath("playcount_lower.mp3");
    mixxxtest::copyFile(m_testDataDir.absoluteFilePath(kEmptyFile), exportTrackPath);
    {
        mixxx::MetadataSourceTagLib source(exportTrackPath, QStringLiteral("mp3"));
        ASSERT_TRUE(source.exportPlayCount(50));
    }
    TrackPointer pTrack = newTrackWithImportedMetadata(exportTrackPath);

    SyncTrackMetadataParams params;
    params.exportPlayCountToFile = true;

    // Another player counted more plays than Mixxx. Even an explicit
    // export request must not reduce the value in the file.
    pTrack->resetPlayCounter(2);
    pTrack->markForMetadataExport();
    SoundSourceProxy::exportTrackMetadataBeforeSaving(pTrack.get(), params);
    EXPECT_EQ(readPlayCountFromFile(exportTrackPath).value_or(-1), 50);
}

TEST_F(TrackMetadataExportTest, PlayCountExportDisabledWritesNothing) {
    const QString exportTrackPath = m_exportTempDir.filePath("playcount_disabled.mp3");
    mixxxtest::copyFile(m_testDataDir.absoluteFilePath(kEmptyFile), exportTrackPath);
    TrackPointer pTrack = newTrackWithImportedMetadata(exportTrackPath);

    pTrack->resetPlayCounter(3);
    pTrack->markForMetadataExport();
    SoundSourceProxy::exportTrackMetadataBeforeSaving(
            pTrack.get(), SyncTrackMetadataParams{});
    EXPECT_FALSE(readPlayCountFromFile(exportTrackPath).has_value());
}

TEST_F(TrackMetadataExportTest, PlayCountOnlyPrefDoesNotExportOtherTags) {
    const QString exportTrackPath = m_exportTempDir.filePath("playcount_only_pref.mp3");
    mixxxtest::copyFile(m_testDataDir.absoluteFilePath(kEmptyFile), exportTrackPath);
    TrackPointer pTrack = newTrackWithImportedMetadata(exportTrackPath);

    SyncTrackMetadataParams params;
    params.syncTrackMetadata = false;
    params.exportPlayCountToFile = true;

    pTrack->setTitle(QStringLiteral("Only In Mixxx"));
    pTrack->setRating(3);
    pTrack->resetPlayCounter(6);
    EXPECT_EQ(ExportTrackMetadataResult::Succeeded,
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), params));

    // Only the play count must be in the file
    EXPECT_EQ(readPlayCountFromFile(exportTrackPath).value_or(-1), 6);
    EXPECT_FALSE(readRatingFromFile(exportTrackPath).has_value());
    mixxx::TrackMetadata fileMetadata;
    SoundSourceProxy::importTrackMetadataAndCoverImageFromFile(
            mixxx::FileAccess(mixxx::FileInfo(exportTrackPath)),
            &fileMetadata,
            nullptr,
            false);
    EXPECT_NE(fileMetadata.getTrackInfo().getTitle(),
            QStringLiteral("Only In Mixxx"));
}
