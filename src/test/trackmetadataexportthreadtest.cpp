#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTemporaryDir>

#include "library/trackmetadataexportthread.h"
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

class TrackMetadataExportThreadTest : public MixxxTest,
                                      private SoundSourceProviderRegistration {
  public:
    TrackMetadataExportThreadTest()
            : m_testDataDir(getTestDir().absoluteFilePath(QStringLiteral("id3-test-data"))) {
    }

  protected:
    const QDir m_testDataDir;
    QTemporaryDir m_exportTempDir;
    GlobalTrackCacheHelper m_globalTrackCacheHelper;
};

TEST_F(TrackMetadataExportThreadTest, exportQueuedTrackMetadata) {
    ASSERT_TRUE(m_exportTempDir.isValid());
    const QString exportTrackPath = m_exportTempDir.filePath("queued-export.mp3");
    mixxxtest::copyFile(m_testDataDir.absoluteFilePath(kEmptyFile), exportTrackPath);

    TrackMetadataExportThread exportThread(nullptr);
    QSignalSpy exportedSpy(
            &exportThread,
            &TrackMetadataExportThread::trackExported);
    exportThread.start();

    auto pTrack = Track::newTemporary(exportTrackPath);
    pTrack->setTitle(QStringLiteral("Queued Title"));
    pTrack->setArtist(QStringLiteral("Queued Artist"));
    // Stamp a synchronization timestamp as if the track had been
    // imported from the file, otherwise the export is skipped for
    // untagged files.
    pTrack->replaceMetadataFromSource(
            pTrack->getMetadata(), QDateTime::currentDateTimeUtc());
    pTrack->markForMetadataExport();
    exportThread.enqueueExport(*pTrack, SyncTrackMetadataParams());
    pTrack.reset();

    ASSERT_TRUE(exportedSpy.wait(10000));
    ASSERT_EQ(exportedSpy.size(), 1);
    EXPECT_EQ(exportedSpy.at(0).at(1).value<ExportTrackMetadataResult>(),
            ExportTrackMetadataResult::Succeeded);

    // Verify that the file tags have actually been written
    TrackPointer pReimportedTrack = Track::newTemporary(exportTrackPath);
    SoundSourceProxy(pReimportedTrack).updateTrackFromSource(SoundSourceProxy::UpdateTrackFromSourceMode::Always, SyncTrackMetadataParams());
    EXPECT_EQ(pReimportedTrack->getTitle(), QStringLiteral("Queued Title"));
    EXPECT_EQ(pReimportedTrack->getArtist(), QStringLiteral("Queued Artist"));

    exportThread.stop();
    exportThread.wait();
}
