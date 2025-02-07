#include <gtest/gtest.h>

#include <QTemporaryDir>

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
