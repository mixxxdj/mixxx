#include <gtest/gtest.h>

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
