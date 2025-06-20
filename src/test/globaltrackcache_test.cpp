#include "track/globaltrackcache.h"

#include <QThread>
#include <QtDebug>
#include <atomic>

#include "test/mixxxtest.h"
#include "track/track.h"

namespace {

const QString kTestFile = QStringLiteral("id3-test-data/cover-test.flac");
const QString kTestFile2 = QStringLiteral("id3-test-data/cover-test.ogg");

class TrackTitleThread: public QThread {
  public:
    explicit TrackTitleThread()
        : m_stop(false) {
    }

    void stop() {
        m_stop.store(true);
    }

    void run() override {
        int loopCount = 0;
        while (!(m_stop.load() && GlobalTrackCacheLocker().isEmpty())) {
            // Drop the previous reference to avoid resolving the
            // same track twice
            m_recentTrackPtr.reset();
            // Try to resolve the next track by guessing the id
            const TrackId trackId(QVariant(loopCount % 2));
            auto track = GlobalTrackCacheLocker().lookupTrackById(trackId);
            if (track) {
                ASSERT_EQ(trackId, track->getId());
                // #9097: Accessing the track from multiple threads is
                // required to cause a SIGSEGV
                if (track->getTitle().isEmpty()) {
                    track->setTitle(
                            QString("Title %1").arg(QString::number(loopCount)));
                } else {
                    track->setTitle(QString());
                }
                ASSERT_TRUE(track->isDirty());
            }
            // Replace the current reference with this one and keep it alive
            // until the next loop cycle
            m_recentTrackPtr = std::move(track);
            ++loopCount;
        }
        // If the cache is empty all references must have been dropped.
        // Why? m_recentTrackPtr is only valid if a pointer has been found
        // in the cache during the previous cycle, i.e. the cache could not
        // have been empty. In this case at least another loop cycle follow,
        // and so on...
        ASSERT_TRUE(!m_recentTrackPtr);
        qDebug() << "Finished" << loopCount << " thread loops";
    }

  private:
    TrackPointer m_recentTrackPtr;

    std::atomic<bool> m_stop;
};

void deleteTrack(Track* pTrack) {
    // Delete track objects directly in unit tests with
    // no main event loop
    delete pTrack;
};

} // anonymous namespace

class GlobalTrackCacheTest: public MixxxTest, public virtual GlobalTrackCacheSaver {
  public:
    void saveEvictedTrack(Track* pTrack) noexcept override {
        ASSERT_FALSE(pTrack == nullptr);
    }

  protected:
    GlobalTrackCacheTest() {
        GlobalTrackCache::createInstance(this, deleteTrack);
    }
    ~GlobalTrackCacheTest() {
        GlobalTrackCache::destroyInstance();
    }

    TrackPointer m_recentTrackPtr;
};

TEST_F(GlobalTrackCacheTest, resolveByFileInfo) {
    ASSERT_TRUE(GlobalTrackCacheLocker().isEmpty());

    const TrackId trackId(QVariant(1));

    TrackPointer pTrack;
    { // resolver scope
        auto testFileAccess = mixxx::FileAccess(mixxx::FileInfo(getTestDir().filePath(kTestFile)));
        auto resolver = GlobalTrackCacheResolver(testFileAccess);
        pTrack = resolver.getTrack();
        EXPECT_TRUE(static_cast<bool>(pTrack));
        // track, GlobalTrackCacheResolver::m_strongPtr and GlobalTrackCache::m_incompleteTrack
        EXPECT_EQ(3, pTrack.use_count());

        resolver.initTrackIdAndUnlockCache(trackId);
        EXPECT_EQ(2, pTrack.use_count());
    }
    EXPECT_EQ(1, pTrack.use_count());

    TrackWeakPointer trackWeak(pTrack);
    EXPECT_EQ(1, trackWeak.use_count());

    TrackPointer trackCopy = pTrack;
    EXPECT_EQ(2, trackCopy.use_count());
    EXPECT_EQ(2, pTrack.use_count());
    EXPECT_EQ(2, trackWeak.use_count());

    trackCopy.reset();
    EXPECT_EQ(1, pTrack.use_count());
    EXPECT_EQ(1, trackWeak.use_count());

    auto trackById = GlobalTrackCacheLocker().lookupTrackById(trackId);
    EXPECT_EQ(pTrack, trackById);
    EXPECT_EQ(2, trackById.use_count());
    EXPECT_EQ(2, pTrack.use_count());
    EXPECT_EQ(2, trackWeak.use_count());

    trackById.reset();
    EXPECT_EQ(1, trackWeak.use_count());
    EXPECT_EQ(pTrack, TrackPointer(trackWeak.lock()));

    pTrack.reset();
    EXPECT_EQ(0, trackWeak.use_count());
    EXPECT_EQ(TrackPointer(), TrackPointer(trackWeak.lock()));

    {
        GlobalTrackCacheLocker cacheLocker;
        trackById = cacheLocker.lookupTrackById(trackId);
        EXPECT_EQ(TrackPointer(), trackById);
        EXPECT_TRUE(cacheLocker.isEmpty());
    }
}

TEST_F(GlobalTrackCacheTest, concurrentDelete) {
    ASSERT_TRUE(GlobalTrackCacheLocker().isEmpty());

    TrackTitleThread workerThread;
    workerThread.start();

    const auto testFile = mixxx::FileInfo(getTestDir().filePath(kTestFile));

    // #9097: A decent number of iterations is needed to reliably
    // reveal potential race conditions while evicting tracks from
    // the cache!
    // NOTE(2019-12-14, uklotzde): On Travis and macOS executing 10_000
    // iterations takes ~1 sec. In order to safely finish this test within
    // the timeout limit of 30 sec. we use 20 * 10_000 = 200_000 iterations.
    //
    // NOTE(2024-06-03, daschuer): Reduced to 100000 because the we hit a
    // timeout on the macos-12 GitHub workflow runner
    // With 200000 we had:
    // ubuntu-22.04 0.73 sec
    // windows-2019 9.86 sec
    // macos-11 5.81 sec
    // macos-12 timeout after 45.02 sec (24.55 sec with 100000)
    for (int i = 0; i < 100000; ++i) {
        m_recentTrackPtr.reset();

        TrackId trackId;

        TrackPointer track;
        {
            auto testFileAccess = mixxx::FileAccess(testFile);
            auto resolver = GlobalTrackCacheResolver(testFileAccess);
            track = resolver.getTrack();
            EXPECT_TRUE(static_cast<bool>(track));
            trackId = track->getId();
            if (!trackId.isValid()) {
                trackId = TrackId(QVariant(i % 2));
                resolver.initTrackIdAndUnlockCache(trackId);
            }
        }

        track = GlobalTrackCacheLocker().lookupTrackById(trackId);
        EXPECT_TRUE(static_cast<bool>(track));

        // #9097: Accessing the track from multiple threads is
        // required to cause a SIGSEGV
        track->setArtist(QString("Artist %1").arg(QString::number(i)));

        m_recentTrackPtr = std::move(track);

        // Lookup the track again
        track = GlobalTrackCacheLocker().lookupTrackById(trackId);
        EXPECT_TRUE(static_cast<bool>(track));

        // Ensure that track objects are evicted and deleted
        QCoreApplication::processEvents();
    }
    m_recentTrackPtr.reset();

    workerThread.stop();

    // Ensure that all track objects have been deleted
    while (!GlobalTrackCacheLocker().isEmpty()) {
        QCoreApplication::processEvents();
    }

    workerThread.wait();
}

TEST_F(GlobalTrackCacheTest, evictWhileMoving) {
    ASSERT_TRUE(GlobalTrackCacheLocker().isEmpty());

    TrackPointer track1 = GlobalTrackCacheResolver(
            mixxx::FileAccess(mixxx::FileInfo(getTestDir().filePath(kTestFile))))
                                  .getTrack();
    EXPECT_TRUE(static_cast<bool>(track1));

    TrackPointer track2 = GlobalTrackCacheResolver(
            mixxx::FileAccess(mixxx::FileInfo(getTestDir().filePath(kTestFile2))))
                                  .getTrack();
    EXPECT_TRUE(static_cast<bool>(track2));

    track1 = std::move(track2);

    EXPECT_TRUE(static_cast<bool>(track1));
    EXPECT_FALSE(static_cast<bool>(track2));

    track1.reset();

    EXPECT_TRUE(GlobalTrackCacheLocker().isEmpty());
}
