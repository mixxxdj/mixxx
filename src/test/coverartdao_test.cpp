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

TEST_F(CoverArtDAOTest, saveCoverArt) {
    CoverArtDAO m_CoverArtDAO = m_pTrackCollection->getCoverArtDAO();
    QString testCoverLoc, testMd5Hash;

    // adding a empty cover location
    int coverId = m_CoverArtDAO.saveCoverArt(testCoverLoc, testMd5Hash);
    EXPECT_EQ(-1, coverId);

    // adding a new cover
    testCoverLoc = "/a/b/cover1.jpg";
    testMd5Hash = "abc123xxxCOVER1";
    coverId = m_CoverArtDAO.saveCoverArt(testCoverLoc, testMd5Hash);
    EXPECT_TRUE(coverId > 0);

    // adding an existing cover
    int e_coverId = m_CoverArtDAO.saveCoverArt(testCoverLoc, testMd5Hash);
    EXPECT_EQ(coverId, e_coverId);

    // we do not trust what coverartdao thinks and better check up on it
    QSqlQuery query(m_pTrackCollection->getDatabase());
    query.prepare(QString(
        "SELECT " % COVERARTTABLE_ID % " FROM " % COVERART_TABLE %
        " WHERE " % COVERARTTABLE_LOCATION % "=:location"
        " AND " % COVERARTTABLE_MD5 % "=:md5"));
    query.bindValue(":location", testCoverLoc);
    query.bindValue(":md5", testMd5Hash);
    query.exec();
    int testCoverId = -1;
    if (query.next()) {
        testCoverId = query.value(0).toInt();
    }
    ASSERT_EQ(coverId, testCoverId);
}

TEST_F(CoverArtDAOTest, getCoverArtId) {
    CoverArtDAO m_CoverArtDAO = m_pTrackCollection->getCoverArtDAO();
    QString testCoverLoc = "a/b/cover2.jpg";
    QString testCoverMd5 = "abc123xxxCOVER2";

    int coverIdSaved = m_CoverArtDAO.saveCoverArt(testCoverLoc, testCoverMd5);
    int coverId = m_CoverArtDAO.getCoverArtId(testCoverMd5);

    ASSERT_EQ(coverIdSaved, coverId);
}

TEST_F(CoverArtDAOTest, deleteUnusedCoverArts) {
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
    QString coverMd5_1 = "abc123xxxCOVER";
    QString coverLocation_2 = "foo/folder.jpg";
    QString coverMd5_2 = "abc123xxxFOLDER";
    QString coverLocation_3 = "foo/album.jpg";
    QString coverMd5_3 = "abc123xxxALBUM";
    QString coverLocation_4 = "foo/front.jpg";
    QString coverMd5_4 = "abc123xxxFRONT";
    int coverId_1 = m_CoverArtDAO.saveCoverArt(coverLocation_1, coverMd5_1);
    int coverId_2 = m_CoverArtDAO.saveCoverArt(coverLocation_2, coverMd5_2);
    int coverId_3 = m_CoverArtDAO.saveCoverArt(coverLocation_3, coverMd5_3);
    int coverId_4 = m_CoverArtDAO.saveCoverArt(coverLocation_4, coverMd5_4);
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
    int coverId_1t = m_CoverArtDAO.getCoverArtId(coverMd5_1);
    int coverId_2t = m_CoverArtDAO.getCoverArtId(coverMd5_2);
    int coverId_3t = m_CoverArtDAO.getCoverArtId(coverMd5_3);
    int coverId_4t = m_CoverArtDAO.getCoverArtId(coverMd5_4);
    ASSERT_EQ(coverId_1, coverId_1t);
    ASSERT_EQ(coverId_2, coverId_2t);
    ASSERT_EQ(-1, coverId_3t);
    ASSERT_EQ(-1, coverId_4t);
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
    QString coverMd5 = "abc12345xxxCOVER";
    int coverId = m_CoverArtDAO.saveCoverArt(coverLocation, coverMd5);
    trackDAO.updateCoverArt(trackId, coverId);

    // getting cover art info from coverartdao
    CoverArtDAO::CoverArtInfo coverInfo;
    coverInfo = m_CoverArtDAO.getCoverArtInfo(trackId);
    ASSERT_EQ(trackId, coverInfo.trackId);
    EXPECT_QSTRING_EQ(coverLocation, coverInfo.coverLocation);
    EXPECT_QSTRING_EQ(coverMd5, coverInfo.md5Hash);
    EXPECT_QSTRING_EQ(album, coverInfo.album);
    EXPECT_QSTRING_EQ(file.baseName(), coverInfo.trackBaseName);
    EXPECT_QSTRING_EQ(file.absolutePath(), coverInfo.trackDirectory);
    EXPECT_QSTRING_EQ(trackLocation, coverInfo.trackLocation);
}
