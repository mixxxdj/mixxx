#include <gtest/gtest.h>

#include <QTemporaryDir>

#include "sources/soundsourceproxy.h"
#include "test/mixxxtest.h"
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

class TrackMetadataExportTest : public testing::Test {
  public:
    TrackMetadataExportTest()
            : m_testDataDir(QDir::current().absoluteFilePath(
                      "src/test/id3-test-data")) {
    }

  protected:
    const QDir m_testDataDir;
    QTemporaryDir m_exportTempDir;
    GlobalTrackCacheHelper m_globalTrackCacheHelper;
};

TEST_F(TrackMetadataExportTest, keepWithespaceKey) {
    const QString kWhiteSpacesKey = QStringLiteral("  A#m  ");
    const QString kNormalizedKey = QString::fromUtf8("B♭m");
    const QString kId3Key = QStringLiteral("Bbm");

    // Generate a file name for exporting metadata
    const QString exportTrackPath = m_exportTempDir.filePath(kEmptyFile);
    mixxxtest::copyFile(m_testDataDir.absoluteFilePath(kEmptyFile), exportTrackPath);
    TrackPointer pTrack = Track::newTemporary(exportTrackPath);

    mixxx::TrackMetadata trackMetadata;
    trackMetadata.refTrackInfo().setKeyText(kWhiteSpacesKey);
    pTrack->importMetadata(
            trackMetadata,
            QDateTime::currentDateTimeUtc());

    // getKeytext returns the normalized version suitable for GUI presentation
    EXPECT_EQ(pTrack->getKeyText().toStdString(), kNormalizedKey.toStdString());

    // the internal value is still unchanged
    EXPECT_EQ(pTrack->getKeys().getGlobalKeyText().toStdString(), kWhiteSpacesKey.toStdString());

    pTrack->markForMetadataExport();
    ExportTrackMetadataResult result =
            SoundSourceProxy::exportTrackMetadataBeforeSaving(
                    pTrack.get(), nullptr);
    EXPECT_EQ(result, ExportTrackMetadataResult::Succeeded);

    // recreate the Track
    pTrack = SoundSourceProxy::importTemporaryTrack(exportTrackPath);

    // getKeytext returns the normalized version suitable for GUI presentation
    EXPECT_EQ(pTrack->getKeyText().toStdString(), kNormalizedKey.toStdString());

    // the internal value is still unchanged
    EXPECT_EQ(pTrack->getKeys().getGlobalKeyText().toStdString(), kWhiteSpacesKey.toStdString());

    // normalize user edits
    pTrack->setKeyText(kWhiteSpacesKey);
    // the internal value is now at the preferred ID3v2 format
    EXPECT_EQ(pTrack->getKeys().getGlobalKeyText().toStdString(), kId3Key.toStdString());
}
