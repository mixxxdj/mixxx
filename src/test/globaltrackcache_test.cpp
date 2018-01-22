#include <QThread>
#include <QtDebug>

#include <atomic>

#include "test/mixxxtest.h"

#include "track/globaltrackcache.h"

namespace {

const QDir kTestDir(QDir::current().absoluteFilePath("src/test/id3-test-data"));

const QFileInfo kTestFile(kTestDir.absoluteFilePath("cover-test.flac"));

class TrackTitleThread: public QThread {
  public:
    explicit TrackTitleThread(TrackId trackId)
        : m_trackId(std::move(trackId)),
          m_number(0),
          m_stop(false) {
    }

    void stop() {
        m_stop.store(true);
    }

    void run() override {
        int loopCount = 0;
        while (!m_stop.load()) {
            auto track = GlobalTrackCache::instance().lookupById(m_trackId).getTrack();
            if (track) {
                ASSERT_EQ(m_trackId, track->getId());
                // lp1744550: Accessing the track from multiple threads is
                // required to cause a SIGSEGV
                if (track->getTitle().isEmpty()) {
                    track->setTitle(
                            QString("Title %1").arg(QString::number(m_number)));
                } else {
                    track->setTitle(QString());
                }
                ASSERT_TRUE(track->isDirty());
            }
            ++loopCount;
        }
        qDebug() << "Finished" << loopCount << " thread loops";
    }

  private:
    const TrackId m_trackId;

    int m_number;

    std::atomic<bool> m_stop;
};

} // anonymous namespace

class GlobalTrackCacheTest: public MixxxTest, public virtual GlobalTrackCacheEvictor {
  public:
    void onEvictingTrackFromCache(
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
};

TEST_F(GlobalTrackCacheTest, resolveByFileInfo) {
    ASSERT_TRUE(instance().isEmpty());

    const TrackId trackId(1);

    TrackPointer track;
    {
        auto resolver = instance().resolve(kTestFile);
        track = resolver.getTrack();
        EXPECT_TRUE(static_cast<bool>(track));
        EXPECT_EQ(2, track.use_count());

        resolver.initTrackId(trackId);
        EXPECT_EQ(2, track.use_count());
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

    auto trackById = instance().lookupById(trackId).getTrack();
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

    trackById = instance().lookupById(trackId).getTrack();
    EXPECT_EQ(TrackPointer(), trackById);

    EXPECT_TRUE(instance().isEmpty());
}

TEST_F(GlobalTrackCacheTest, concurrentDelete) {
    ASSERT_TRUE(instance().isEmpty());

    const TrackId trackId(1);

    TrackTitleThread workerThread(trackId);
    workerThread.start();

    // lp1744550: A decent number of iterations is needed to reliably
    // reveal potential race conditions while evicting tracks from
    // the cache!
    for (int i = 0; i < 100000; ++i) {
        TrackPointer track;
        {
            auto resolver = instance().resolve(kTestFile);
            track = resolver.getTrack();
            resolver.initTrackId(trackId);
        }
        track = instance().lookupById(trackId).getTrack();
        // lp1744550: Accessing the track from multiple threads is
        // required to cause a SIGSEGV
        track->setArtist(track->getTitle());
    }

    workerThread.stop();
    workerThread.wait();

    EXPECT_TRUE(instance().isEmpty());
}
