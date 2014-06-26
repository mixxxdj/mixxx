#include <gtest/gtest.h>

#include "library/coverartcache.h"
#include "library/dao/coverartdao.h"
#include "library/trackcollection.h"
#include "test/mixxxtest.h"

class CoverArtCacheTest : public CoverArtCache, public MixxxTest {
  protected:
    virtual void SetUp() {
        // make sure to use the current schema.xml file in the repo
        config()->set(ConfigKey("[Config]","Path"),
                      QDir::currentPath().append("/res"));
        m_pTrackCollection = new TrackCollection(config());

        CoverArtCache::create();
        CoverArtCache::instance()->setCoverArtDAO(&m_pTrackCollection->getCoverArtDAO());
        CoverArtCache::instance()->setTrackDAO(&m_pTrackCollection->getTrackDAO());
        m_pCoverArtCache = CoverArtCache::instance();
    }

    virtual void TearDown() {
        // make sure we clean up the db
        QSqlQuery query(m_pTrackCollection->getDatabase());
        query.prepare("DELETE FROM " % COVERART_TABLE);
        query.exec();
        query.prepare("DELETE FROM library");
        query.exec();
        query.prepare("DELETE FROM track_locations");
        query.exec();

        m_pCoverArtCache->destroy();
        delete m_pTrackCollection;
    }

    CoverArtCache* m_pCoverArtCache;
    TrackCollection* m_pTrackCollection;
};

const QString coverLocationTest = ":/images/library/vinyl-record.png";

TEST_F(CoverArtCacheTest, loadImage) {
    int trackId = 1;
    QImage img = QImage(coverLocationTest);

    CoverArtCache::FutureResult res;
    res = CoverArtCache::loadImage(coverLocationTest, trackId);
    ASSERT_EQ(trackId, res.trackId);
    EXPECT_QSTRING_EQ(coverLocationTest, res.coverLocation);
    ASSERT_TRUE(img.operator==(res.img));
}
