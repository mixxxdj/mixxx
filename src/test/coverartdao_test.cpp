#include <gtest/gtest.h>

#include "library/dao/coverartdao.h"
#include "library/trackcollection.h"
#include "test/mixxxtest.h"

class CoverArtDAOTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        // make sure to use the current schema.xml file in the repo
        config()->set(ConfigKey("[Config]","Path"),
                      QDir::currentPath().append("/res"));
        m_pTrackCollection = new TrackCollection(config());
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

        delete m_pTrackCollection;
    }

    TrackCollection* m_pTrackCollection;
};

TEST_F(CoverArtDAOTest, saveCoverLocation) {
    CoverArtDAO m_CoverArtDAO = m_pTrackCollection->getCoverArtDAO();
    QString testCoverLoc;

    // adding a empty cover location
    int coverId = m_CoverArtDAO.saveCoverLocation(testCoverLoc);
    EXPECT_EQ(0, coverId);

    // adding a new cover
    testCoverLoc = "/a/b/cover1.jpg";
    coverId = m_CoverArtDAO.saveCoverLocation(testCoverLoc);
    EXPECT_TRUE(coverId > 0);

    // adding an existing cover
    int e_coverId = m_CoverArtDAO.saveCoverLocation(testCoverLoc);
    EXPECT_EQ(coverId, e_coverId);

    // we do not trust what coverartdao thinks and better check up on it
    QSqlQuery query(m_pTrackCollection->getDatabase());
    query.prepare(QString(
        "SELECT " % COVERARTTABLE_ID % " FROM " % COVERART_TABLE %
        " WHERE " % COVERARTTABLE_LOCATION % "=:location"));
    query.bindValue(":location", testCoverLoc);
    query.exec();
    int testCoverId = 0;
    if (query.next()) {
        testCoverId = query.value(0).toInt();
    }
    ASSERT_EQ(coverId, testCoverId);
}

TEST_F(CoverArtDAOTest, getCoverArtId) {
    CoverArtDAO m_CoverArtDAO = m_pTrackCollection->getCoverArtDAO();
    QString testCoverLoc = "a/b/cover2.jpg";

    int coverIdSaved = m_CoverArtDAO.saveCoverLocation(testCoverLoc);
    int coverId = m_CoverArtDAO.getCoverArtId(testCoverLoc);

    ASSERT_EQ(coverIdSaved, coverId);
}
