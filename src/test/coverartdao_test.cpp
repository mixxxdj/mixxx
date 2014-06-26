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

TEST_F(CoverArtDAOTest, deleteUnusedCoverArts) {
    // starting with a clean cover_art table
    QSqlQuery query(m_pTrackCollection->getDatabase());
    query.prepare("DELETE FROM " % COVERART_TABLE);
    query.exec();

    // creating some tracks
    QString trackLocation_1 = "/a";
    QString trackLocation_2 = "/b";
    QString trackLocation_3 = "/c";
    TrackDAO &trackDAO = m_pTrackCollection->getTrackDAO();
    trackDAO.addTracksPrepare();
    trackDAO.addTracksAdd(new TrackInfoObject(
                    trackLocation_1, SecurityTokenPointer(), false), false);
    trackDAO.addTracksAdd(new TrackInfoObject(
                    trackLocation_2, SecurityTokenPointer(), false), false);
    trackDAO.addTracksAdd(new TrackInfoObject(
                    trackLocation_3, SecurityTokenPointer(), false), false);
    trackDAO.addTracksFinish(false);

    // getting some track ids
    int trackId_1 = trackDAO.getTrackId(trackLocation_1);
    int trackId_2 = trackDAO.getTrackId(trackLocation_2);
    ASSERT_TRUE(trackId_1 != -1);
    ASSERT_TRUE(trackId_2 != -1);

    // adding some covers
    CoverArtDAO m_CoverArtDAO = m_pTrackCollection->getCoverArtDAO();
    QString coverLocation_1 = "foo/cover.jpg";
    QString coverLocation_2 = "foo/folder.jpg";
    QString coverLocation_3 = "foo/album.jpg";
    QString coverLocation_4 = "foo/front.jpg";
    int coverId_1 = m_CoverArtDAO.saveCoverLocation(coverLocation_1);
    int coverId_2 = m_CoverArtDAO.saveCoverLocation(coverLocation_2);
    int coverId_3 = m_CoverArtDAO.saveCoverLocation(coverLocation_3);
    int coverId_4 = m_CoverArtDAO.saveCoverLocation(coverLocation_4);
    ASSERT_TRUE(coverId_1 > 0);
    ASSERT_TRUE(coverId_2 > 0);
    ASSERT_TRUE(coverId_3 > 0);
    ASSERT_TRUE(coverId_4 > 0);

    //associating some covers to some tracks
    trackDAO.updateCoverArt(trackId_1, coverId_1);
    trackDAO.updateCoverArt(trackId_2, coverId_2);

    // removing all unused covers (3 and 4)
    m_CoverArtDAO.deleteUnusedCoverArts();

    // checking current id of each cover
    int coverId_1t = m_CoverArtDAO.getCoverArtId(coverLocation_1);
    int coverId_2t = m_CoverArtDAO.getCoverArtId(coverLocation_2);
    int coverId_3t = m_CoverArtDAO.getCoverArtId(coverLocation_3);
    int coverId_4t = m_CoverArtDAO.getCoverArtId(coverLocation_4);
    ASSERT_EQ(coverId_1, coverId_1t);
    ASSERT_EQ(coverId_2, coverId_2t);
    ASSERT_EQ(0, coverId_3t);
    ASSERT_EQ(0, coverId_4t);
}

TEST_F(CoverArtDAOTest, getCoverArtInfo) {
    // creating a track
    QString trackLocation = "/getCoverArtInfo/track.mp3";
    QFileInfo file = QFileInfo(trackLocation);
    TrackDAO &trackDAO = m_pTrackCollection->getTrackDAO();
    trackDAO.addTracksPrepare();
    trackDAO.addTracksAdd(new TrackInfoObject(
                    trackLocation, SecurityTokenPointer(), false), false);
    trackDAO.addTracksFinish(false);
    int trackId = trackDAO.getTrackId(trackLocation);

    // setting album name
    QString album = "album_name";
    QSqlQuery query(m_pTrackCollection->getDatabase());
    query.prepare(QString(
        "UPDATE " LIBRARY_TABLE " SET " % LIBRARYTABLE_ALBUM % "=:album "
        "WHERE " % LIBRARYTABLE_ID % "=:trackId"));
    query.bindValue(":album", album);
    query.bindValue(":trackId", trackId);
    query.exec();

    // adding cover art
    CoverArtDAO m_CoverArtDAO = m_pTrackCollection->getCoverArtDAO();
    QString coverLocation = "/getCoverArtInfo/cover.jpg";
    int coverId = m_CoverArtDAO.saveCoverLocation(coverLocation);
    trackDAO.updateCoverArt(trackId, coverId);

    // getting cover art info from coverartdao
    CoverArtDAO::CoverArtInfo coverInfo;
    coverInfo = m_CoverArtDAO.getCoverArtInfo(trackId);
    ASSERT_EQ(trackId, coverInfo.trackId);
    EXPECT_QSTRING_EQ(coverLocation, coverInfo.coverLocation);
    EXPECT_QSTRING_EQ(album, coverInfo.album);
    EXPECT_QSTRING_EQ(file.fileName(), coverInfo.trackFilename);
    EXPECT_QSTRING_EQ(file.absolutePath(), coverInfo.trackDirectory);
    EXPECT_QSTRING_EQ(trackLocation, coverInfo.trackLocation);
}
