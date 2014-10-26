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
        ASSERT_TRUE(query.exec());
        query.prepare("DELETE FROM " % DIRECTORYDAO_TABLE);
        ASSERT_TRUE(query.exec());
        query.prepare("DELETE FROM library");
        ASSERT_TRUE(query.exec());
        query.prepare("DELETE FROM track_locations");
        ASSERT_TRUE(query.exec());

        delete m_pTrackCollection;
    }

    TrackCollection* m_pTrackCollection;

    // this method gives support to saveCoverArts (only)
    // it is just to make 'saveCoverArts' more readable and less repetitive
    void iterateQHashAndQSet(QHash<int, QPair<QString, QString> > covers,
                             QSet<QPair<int, int> > res,
                             QList<int> invalidCoverKeys = QList<int>());
};

TEST_F(CoverArtDAOTest, saveCoverArt) {
    CoverArtDAO m_CoverArtDAO = m_pTrackCollection->getCoverArtDAO();
    QString testCoverLoc, testHash;

    // adding a empty cover location
    int coverId = m_CoverArtDAO.saveCoverArt(testCoverLoc, testHash);
    EXPECT_EQ(-1, coverId);

    // adding a new cover
    testCoverLoc = "/a/b/cover1.jpg";
    testHash = "abc123xxxCOVER1";
    coverId = m_CoverArtDAO.saveCoverArt(testCoverLoc, testHash);
    EXPECT_TRUE(coverId > 0);

    // adding an existing cover
    int e_coverId = m_CoverArtDAO.saveCoverArt(testCoverLoc, testHash);
    EXPECT_EQ(coverId, e_coverId);

    // we do not trust what coverartdao thinks and better check up on it
    QSqlQuery query(m_pTrackCollection->getDatabase());
    query.prepare(QString(
        "SELECT " % COVERARTTABLE_ID % " FROM " % COVERART_TABLE %
        " WHERE " % COVERARTTABLE_LOCATION % "=:location"
        " AND " % COVERARTTABLE_HASH % "=:hash"));
    query.bindValue(":location", testCoverLoc);
    query.bindValue(":hash", testHash);
    ASSERT_TRUE(query.exec());
    int testCoverId = -1;
    if (query.next()) {
        testCoverId = query.value(0).toInt();
    }
    ASSERT_EQ(coverId, testCoverId);
}

// saving many covers at once
TEST_F(CoverArtDAOTest, saveCoverArts) {
    CoverArtDAO m_CoverArtDAO = m_pTrackCollection->getCoverArtDAO();
    // <trackId, <coverLoc, coverHash> >
    QHash<int, QPair<QString, QString> > covers;
    // <trackId, coverId>
    QSet<QPair<int, int> > res;

    // adding a empty hash
    res = m_CoverArtDAO.saveCoverArt(covers);
    ASSERT_TRUE(covers.isEmpty());
    ASSERT_TRUE(res.isEmpty());

    // adding new covers (all valid and new)
    covers.insert(1, qMakePair(QString("/cover1.jpg"), QString("COVER1")));
    covers.insert(2, qMakePair(QString("/cover2.jpg"), QString("COVER2")));
    covers.insert(3, qMakePair(QString("/cover3.jpg"), QString("COVER3")));
    res = m_CoverArtDAO.saveCoverArt(covers);
    iterateQHashAndQSet(covers, res);

    // adding existing covers
    QSet<QPair<int, int> > resAux;
    resAux = m_CoverArtDAO.saveCoverArt(covers);
    EXPECT_TRUE(res == resAux);

    // adding new and existing covers
    // it trust that "QHash covers" has existing covers
    covers.insert(4, qMakePair(QString("/cover4.jpg"), QString("COVER4")));
    covers.insert(5, qMakePair(QString("/cover5.jpg"), QString("COVER5")));
    covers.insert(6, qMakePair(QString("/cover6.jpg"), QString("COVER6")));
    res = m_CoverArtDAO.saveCoverArt(covers);
    iterateQHashAndQSet(covers, res);

    // adding invalid covers (empty hash)
    QHash<int, QPair<QString, QString> > invalidCovers;
    QList<int> invalidCoverKeys;
    invalidCoverKeys << 7 << 8;
    invalidCovers.insert(invalidCoverKeys.at(0),
                         qMakePair(QString(""), QString("")));
    invalidCovers.insert(invalidCoverKeys.at(1),
                         qMakePair(QString("/coverInv.jpg"), QString("")));
    res = m_CoverArtDAO.saveCoverArt(invalidCovers);
    iterateQHashAndQSet(invalidCovers, res, invalidCoverKeys);

    // adding 'invalid', 'existing' and 'new' covers
    // it trust that "QHash covers" has existing covers
    covers.insert(9, qMakePair(QString("/newCover1.png"), QString("NEWCOVER")));
    covers.insert(10, qMakePair(QString("/newCover2.jpg"), QString("NEWCOVER2")));
    covers.unite(invalidCovers);
    res = m_CoverArtDAO.saveCoverArt(covers);
    iterateQHashAndQSet(covers, res, invalidCoverKeys);
}

// this method gives support to 'saveCoverArts' (only)
// it is just to make 'saveCoverArts' more readable and less repetitive
void CoverArtDAOTest::iterateQHashAndQSet(QHash<int, QPair<QString, QString> > covers,
                                          QSet<QPair<int, int> > res,
                                          QList<int> invalidCoverKeys)
{
    ASSERT_TRUE(res.size() <= covers.size());
    QSetIterator<QPair<int, int> > set(res);
    QHashIterator<int, QPair<QString, QString> > hash(covers);
    while (hash.hasNext()) {
        hash.next();
        int trackId = hash.key();
        bool hasTrackId = false;
        set.toFront();
        while (set.hasNext() && !hasTrackId) {
            QPair<int, int> p = set.next();
            hasTrackId = p.first == trackId;
            if (invalidCoverKeys.contains(p.first)) {
                EXPECT_TRUE(p.second == -1);
            } else {
                EXPECT_TRUE(p.second > 0);
            }
        }
        EXPECT_TRUE(hasTrackId);
    }
}

TEST_F(CoverArtDAOTest, getCoverArtId) {
    CoverArtDAO m_CoverArtDAO = m_pTrackCollection->getCoverArtDAO();
    QString testCoverLoc = "a/b/cover2.jpg";
    QString testCoverHash = "abc123xxxCOVER2";

    int coverIdSaved = m_CoverArtDAO.saveCoverArt(testCoverLoc, testCoverHash);
    int coverId = m_CoverArtDAO.getCoverArtId(testCoverHash);

    ASSERT_EQ(coverIdSaved, coverId);
}

TEST_F(CoverArtDAOTest, deleteUnusedCoverArts) {
    DirectoryDAO &directoryDao = m_pTrackCollection->getDirectoryDAO();
    // creating a temp dir
    QString testdir(QDir::tempPath() + "/CoverDir");
    directoryDao.addDirectory(testdir);
    // creating some tracks
    QString trackLocation_1 = testdir % "/a";
    QString trackLocation_2 = testdir % "/b";
    QString trackLocation_3 = testdir % "/c";
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
    QString coverHash_1 = "abc123xxxCOVER";
    QString coverLocation_2 = "foo/folder.jpg";
    QString coverHash_2 = "abc123xxxFOLDER";
    QString coverLocation_3 = "foo/album.jpg";
    QString coverHash_3 = "abc123xxxALBUM";
    QString coverLocation_4 = "foo/front.jpg";
    QString coverHash_4 = "abc123xxxFRONT";
    int coverId_1 = m_CoverArtDAO.saveCoverArt(coverLocation_1, coverHash_1);
    int coverId_2 = m_CoverArtDAO.saveCoverArt(coverLocation_2, coverHash_2);
    int coverId_3 = m_CoverArtDAO.saveCoverArt(coverLocation_3, coverHash_3);
    int coverId_4 = m_CoverArtDAO.saveCoverArt(coverLocation_4, coverHash_4);
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
    int coverId_1t = m_CoverArtDAO.getCoverArtId(coverHash_1);
    int coverId_2t = m_CoverArtDAO.getCoverArtId(coverHash_2);
    int coverId_3t = m_CoverArtDAO.getCoverArtId(coverHash_3);
    int coverId_4t = m_CoverArtDAO.getCoverArtId(coverHash_4);
    ASSERT_EQ(coverId_1, coverId_1t);
    ASSERT_EQ(coverId_2, coverId_2t);
    ASSERT_EQ(-1, coverId_3t);
    ASSERT_EQ(-1, coverId_4t);
}

TEST_F(CoverArtDAOTest, getCoverAndAlbumInfo) {
    DirectoryDAO &directoryDao = m_pTrackCollection->getDirectoryDAO();
    // creating a temp dir
    QString testdir(QDir::tempPath() + "/CoverDir");
    directoryDao.addDirectory(testdir);
    // creating a track
    QString trackLocation = testdir % "/getCoverAndAlbumInfo/track.mp3";
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
    ASSERT_TRUE(query.exec());

    // adding cover art
    CoverArtDAO m_CoverArtDAO = m_pTrackCollection->getCoverArtDAO();
    QString coverLocation = "/getCoverAndAlbumInfo/cover.jpg";
    QString coverHash = "abc12345xxxCOVER";
    int coverId = m_CoverArtDAO.saveCoverArt(coverLocation, coverHash);
    trackDAO.updateCoverArt(trackId, coverId);

    // getting cover art info from coverartdao
    CoverAndAlbumInfo coverInfo;
    coverInfo = m_CoverArtDAO.getCoverAndAlbumInfo(trackId);
    ASSERT_EQ(trackId, coverInfo.info.trackId);
    EXPECT_QSTRING_EQ(coverLocation, coverInfo.info.coverLocation);
    EXPECT_QSTRING_EQ(coverHash, coverInfo.info.hash);
    EXPECT_QSTRING_EQ(album, coverInfo.album);
    EXPECT_QSTRING_EQ(trackLocation, coverInfo.info.trackLocation);
}
