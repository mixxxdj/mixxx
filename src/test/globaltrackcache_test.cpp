#include <QThread>
#include <QtDebug>

#include <atomic>

#include "test/mixxxtest.h"

#include "track/globaltrackcache.h"


namespace {

const QDir kTestDir(QDir::current().absoluteFilePath("src/test/id3-test-data"));

const QFileInfo kTestFile(kTestDir.absoluteFilePath("cover-test.flac"));
const QFileInfo kTestFile2(kTestDir.absoluteFilePath("cover-test.ogg"));

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
        while (!m_stop.load()) {
            m_recentTrackPtr.reset();
            const TrackId trackId(loopCount % 2);
            auto track = GlobalTrackCache::instance().lookupById(trackId);
            if (track) {
                ASSERT_EQ(trackId, track->getId());
                // lp1744550: Accessing the track from multiple threads is
                // required to cause a SIGSEGV
                if (track->getTitle().isEmpty()) {
                    track->setTitle(
                            QString("Title %1").arg(QString::number(loopCount)));
                } else {
                    track->setTitle(QString());
                }
                ASSERT_TRUE(track->isDirty());
            }
            m_recentTrackPtr = std::move(track);
            ++loopCount;
        }
        m_recentTrackPtr.reset();
        qDebug() << "Finished" << loopCount << " thread loops";
    }

  private:
    TrackPointer m_recentTrackPtr;

    std::atomic<bool> m_stop;
};

} // anonymous namespace

class GlobalTrackCacheTest: public MixxxTest, public virtual GlobalTrackCacheEvictor {
  public:
    void afterEvictedTrackFromCache(
        GlobalTrackCacheLocker* /*nullable*/ pCacheLocker,
        Track* pTrack) override {
        if (pCacheLocker) {
            pCacheLocker->unlockCache();
        }
        ASSERT_FALSE(pTrack == nullptr);
    }

  protected:
    GlobalTrackCacheTest() {
        GlobalTrackCache::createInstance(this);
    }
    ~GlobalTrackCacheTest() {
        GlobalTrackCache::destroyInstance();
    }

    GlobalTrackCache& instance() const {
        return GlobalTrackCache::instance();
    }

    TrackPointer m_recentTrackPtr;
};

TEST_F(GlobalTrackCacheTest, resolveByFileInfo) {
    ASSERT_TRUE(instance().isEmpty());

    const TrackId trackId(1);

    TrackPointer track;
    {
        auto resolver = instance().resolve(kTestFile);
        track = resolver;
        EXPECT_TRUE(static_cast<bool>(track));
        EXPECT_EQ(2, track.use_count());

        resolver.initTrackIdAndUnlockCache(trackId);
    }
    EXPECT_EQ(1, track.use_count());

    TrackWeakPointer trackWeak(track);
    EXPECT_EQ(1, trackWeak.use_count());

    TrackPointer trackCopy = track;
    EXPECT_EQ(2, trackCopy.use_count());
    EXPECT_EQ(2, track.use_count());
    EXPECT_EQ(2, trackWeak.use_count());

    trackCopy.reset();
    EXPECT_EQ(1, track.use_count());
    EXPECT_EQ(1, trackWeak.use_count());

    auto trackById = instance().lookupById(trackId);
    EXPECT_EQ(track, trackById);
    EXPECT_EQ(2, trackById.use_count());
    EXPECT_EQ(2, track.use_count());
    EXPECT_EQ(2, trackWeak.use_count());

    trackById.reset();
    EXPECT_EQ(1, trackWeak.use_count());
    EXPECT_EQ(track, TrackPointer(trackWeak));

    track.reset();
    EXPECT_EQ(0, trackWeak.use_count());
    EXPECT_EQ(TrackPointer(), TrackPointer(trackWeak));

    trackById = instance().lookupById(trackId);
    EXPECT_EQ(TrackPointer(), trackById);

    EXPECT_TRUE(instance().isEmpty());
}

TEST_F(GlobalTrackCacheTest, concurrentDelete) {
    ASSERT_TRUE(instance().isEmpty());

    TrackTitleThread workerThread;
    workerThread.start();

    // lp1744550: A decent number of iterations is needed to reliably
    // reveal potential race conditions while evicting tracks from
    // the cache!
    for (int i = 0; i < 250000; ++i) {
        m_recentTrackPtr.reset();

        TrackId trackId;

        TrackPointer track;
        {
            auto resolver = instance().resolve(kTestFile);
            track = resolver;
            EXPECT_TRUE(static_cast<bool>(track));
            trackId = track->getId();
            if (!trackId.isValid()) {
                trackId = TrackId(i % 2);
                resolver.initTrackIdAndUnlockCache(trackId);
            }
        }

        track = instance().lookupById(trackId);
        EXPECT_TRUE(static_cast<bool>(track));

        // lp1744550: Accessing the track from multiple threads is
        // required to cause a SIGSEGV
        track->setArtist(track->getTitle());

        m_recentTrackPtr = std::move(track);
    }
    m_recentTrackPtr.reset();

    workerThread.stop();
    workerThread.wait();

    EXPECT_TRUE(instance().isEmpty());
}

TEST_F(GlobalTrackCacheTest, evictWhileMoving) {
    ASSERT_TRUE(instance().isEmpty());

    TrackPointer track1 = instance().resolve(kTestFile);
    EXPECT_TRUE(static_cast<bool>(track1));

    TrackPointer track2 = instance().resolve(kTestFile2);
    EXPECT_TRUE(static_cast<bool>(track2));

    track1 = std::move(track2);

    EXPECT_TRUE(static_cast<bool>(track1));
    EXPECT_FALSE(static_cast<bool>(track2));
}
