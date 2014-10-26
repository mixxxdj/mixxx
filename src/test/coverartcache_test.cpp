#include <gtest/gtest.h>
#include <QStringBuilder>
#include <QFileInfo>

#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "library/dao/coverartdao.h"
#include "library/trackcollection.h"
#include "test/mixxxtest.h"
#include "soundsourceproxy.h"

// first inherit from MixxxTest to construct a QApplication to be able to
// construct the default QPixmap in CoverArtCache
class CoverArtCacheTest : public MixxxTest, public CoverArtCache {
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
};

const QString kCoverLocationTest("res/images/library/default_cover.png");
const QString kTrackLocationTest(QDir::currentPath() %
                                 "/src/test/id3-test-data/cover-test.mp3");

TEST_F(CoverArtCacheTest, extractEmbeddedCover) {
    const QString kTestPath(QDir::currentPath() % "/src/test/id3-test-data/");
    QImage cover;
    // aiff
    cover = CoverArtUtils::extractEmbeddedCover(kTestPath % "cover-test.aiff");
    EXPECT_TRUE(!cover.isNull());
    // flac
    cover = CoverArtUtils::extractEmbeddedCover(kTestPath % "cover-test.flac");
    EXPECT_TRUE(!cover.isNull());
    // mp3
    cover = CoverArtUtils::extractEmbeddedCover(kTestPath % "cover-test.mp3");
    EXPECT_TRUE(!cover.isNull());
    // ogg
    cover = CoverArtUtils::extractEmbeddedCover(kTestPath % "cover-test.ogg");
    EXPECT_TRUE(!cover.isNull());
    // wav
    cover = CoverArtUtils::extractEmbeddedCover(kTestPath % "cover-test.wav");
    EXPECT_TRUE(!cover.isNull());
}

TEST_F(CoverArtCacheTest, loadImage) {
    QImage img = QImage(kCoverLocationTest);
    CoverArtDAO::CoverArtInfo info;
    info.trackId = 1;
    info.coverLocation = kCoverLocationTest;
    info.hash = "coverhash"; // fake cover hash

    CoverArtCache::FutureResult res;
    res = CoverArtCache::loadImage(info, QSize(0,0), false);
    EXPECT_EQ(info.trackId, res.trackId);
    EXPECT_QSTRING_EQ(kCoverLocationTest, res.coverLocation);
    EXPECT_QSTRING_EQ(info.hash, res.hash);
    EXPECT_EQ(img, res.img);

    info.trackId = 1;
    info.coverLocation = "ID3TAG";
    info.trackLocation = kTrackLocationTest;
    res = CoverArtCache::loadImage(info, QSize(0,0), false);
    EXPECT_EQ(info.trackId, res.trackId);
    EXPECT_QSTRING_EQ("ID3TAG", res.coverLocation);
    EXPECT_QSTRING_EQ(info.hash, res.hash);

    SecurityTokenPointer securityToken = Sandbox::openSecurityToken(
        QDir(kTrackLocationTest), true);
    SoundSourceProxy proxy(kTrackLocationTest, securityToken);
    Mixxx::SoundSource* pProxiedSoundSource = proxy.getProxiedSoundSource();
    ASSERT_TRUE(pProxiedSoundSource != NULL);
    img = proxy.parseCoverArt();

    EXPECT_EQ(img, res.img);
}

TEST_F(CoverArtCacheTest, changeImage) {
    CoverArtDAO m_CoverArtDAO = m_pTrackCollection->getCoverArtDAO();
    TrackDAO &trackDAO = m_pTrackCollection->getTrackDAO();

    QString testdir(QDir::tempPath() + "/CoverDir");
    QString testCoverLoc = testdir + "/b/cover1.jpg";
    QString testHash = "abc123xxxCOVER1";
    QString trackLocation_1 = testdir % "/b/test.mp3";

    // adding a new cover
    int coverId = m_CoverArtDAO.saveCoverArt(testCoverLoc, testHash);
    EXPECT_TRUE(coverId > 0);

    // add Track
    trackDAO.addTracksPrepare();
    trackDAO.addTracksAdd(new TrackInfoObject(
                    kTrackLocationTest, SecurityTokenPointer(), false), false);
    trackDAO.addTracksFinish(false);
    int trackId_1 = trackDAO.getTrackId(kTrackLocationTest);

    //associating some covers to some tracks
    trackDAO.updateCoverArt(trackId_1, coverId);

    setCoverArtDAO(&m_CoverArtDAO);
    setTrackDAO(&trackDAO);
    EXPECT_TRUE(changeCoverArt(trackId_1, "ID3TAG"));
}

TEST_F(CoverArtCacheTest, searchImage) {
    // creating a temp track directory
    QString trackdir(QDir::tempPath() % "/TrackDir");
    ASSERT_FALSE(QDir().exists(trackdir)); // it must start empty
    ASSERT_TRUE(QDir().mkpath(trackdir));

    // creating CoverArtInfo with empty coverLocation
    CoverArtDAO::CoverArtInfo cInfo = {
        1,                                             // cInfo.trackId
        "",                                            // cInfo.coverLocation
        "",                                            // cInfo.hash
        "album_name",                                  // cInfo.album
        "track",                                       // cInfo.trackBaseName
        trackdir,                                      // cInfo.trackDirectory
        trackdir % "/track.mp3"                        // cInfo.trackLocation
    };

    // looking for cover in an empty directory
    CoverArtCache::FutureResult res;
    res = CoverArtCache::searchImage(cInfo, QSize(0,0), false);
    EXPECT_TRUE(res.coverLocation.isEmpty());

    // looking for a track with embedded cover
    cInfo.trackLocation = kTrackLocationTest;
    res = CoverArtCache::searchImage(cInfo, QSize(0,0), false);
    EXPECT_EQ(cInfo.trackId, res.trackId);
    EXPECT_QSTRING_EQ("ID3TAG", res.coverLocation);

    SecurityTokenPointer securityToken = Sandbox::openSecurityToken(
        QDir(kTrackLocationTest), true);
    SoundSourceProxy proxy(kTrackLocationTest, securityToken);
    Mixxx::SoundSource* pProxiedSoundSource = proxy.getProxiedSoundSource();
    ASSERT_TRUE(pProxiedSoundSource != NULL);
    QImage img = proxy.parseCoverArt();

    EXPECT_EQ(img, res.img);

    // setting image source and default format
    cInfo.trackLocation = trackdir % "/" % cInfo.trackBaseName % ".mp3";
    img = QImage(kCoverLocationTest);
    const char* format("jpg");

    //
    // Search Strategy
    // 0. If we have just one file, we will get it.
    // 1. %track-file-base%.jpg in the track directory for %track-file-base%.mp3
    // 2. %album%.jpg
    // 3. cover.jpg
    // 4. front.jpg
    // 5. album.jpg
    // 6. folder.jpg
    // 7. anything else found in the folder (get the lighter one)
    //

    // 0. saving just one cover in our temp track dir
    QString cLoc_foo = QString(trackdir % "/" % "foo.").append(format);
    EXPECT_TRUE(img.save(cLoc_foo, format));
    // looking for cover in an directory with one image
    res = CoverArtCache::searchImage(cInfo, QSize(0,0), false);
    EXPECT_QSTRING_EQ(cLoc_foo, res.coverLocation);
    QFile::remove(cLoc_foo);

    QStringList extraCovers;
    // adding some extra images (bigger) just to populate the track dir.
    QString cLoc_big1 = QString(trackdir % "/" % "big1.").append(format);
    EXPECT_TRUE(img.scaled(1000,1000).save(cLoc_big1, format));
    extraCovers << cLoc_big1;
    QString cLoc_big2 = QString(trackdir % "/" % "big2.").append(format);
    EXPECT_TRUE(img.scaled(900,900).save(cLoc_big2, format));
    extraCovers << cLoc_big2;
    QString cLoc_big3 = QString(trackdir % "/" % "big3.").append(format);
    EXPECT_TRUE(img.scaled(800,800).save(cLoc_big3, format));
    extraCovers << cLoc_big3;

    // saving more covers using the preferred names in the right order
    QStringList prefCovers;
    // 1. track_filename.jpg
    QString cLoc_filename = QString(trackdir % "/" % cInfo.trackBaseName % ".");
    cLoc_filename.append(format);
    EXPECT_TRUE(img.scaled(500,500).save(cLoc_filename, format));
    prefCovers << cLoc_filename;
    // 2. album_name.jpg
    QString cLoc_albumName = QString(trackdir % "/" % cInfo.album % ".").append(format);
    EXPECT_TRUE(img.scaled(500,500).save(cLoc_albumName, format));
    prefCovers << cLoc_albumName;
    // 3. cover.jpg
    QString cLoc_cover = QString(trackdir % "/" % "cover.").append(format);
    EXPECT_TRUE(img.scaled(400,400).save(cLoc_cover, format));
    prefCovers << cLoc_cover;
    // 4. front.jpg
    QString cLoc_front = QString(trackdir % "/" % "front.").append(format);
    EXPECT_TRUE(img.scaled(300,300).save(cLoc_front, format));
    prefCovers << cLoc_front;
    // 5. album.jpg
    QString cLoc_album = QString(trackdir % "/" % "album.").append(format);
    EXPECT_TRUE(img.scaled(100,100).save(cLoc_album, format));
    prefCovers << cLoc_album;
    // 6. folder.jpg
    QString cLoc_folder = QString(trackdir % "/" % "folder.").append(format);
    EXPECT_TRUE(img.scaled(100,100).save(cLoc_folder, format));
    prefCovers << cLoc_folder;
    // 7. lighter.jpg
    QString cLoc_lighter = QString(trackdir % "/" % "lighter.").append(format);
    EXPECT_TRUE(img.scaled(10,10).save(cLoc_lighter, format));
    prefCovers << cLoc_lighter;

    // we must find covers in the right order
    EXPECT_EQ(7, prefCovers.size());
    for (int coverNameId = 0; coverNameId < prefCovers.size(); coverNameId++) {
        res = CoverArtCache::searchImage(cInfo, QSize(0,0), false);
        EXPECT_QSTRING_EQ(prefCovers[coverNameId], res.coverLocation);
        QFile::remove(prefCovers[coverNameId]);
    }

    //
    // Additional tests
    //

    // what is chosen when cover.jpg and cover.JPG exists?
    // (it must always prefer the lighter cover)
    QString cLoc_coverJPG = trackdir % "/" % "cover." % "JPG";
    EXPECT_TRUE(img.scaled(200,200).save(cLoc_coverJPG, "JPG"));
    QString cLoc_coverjpg = trackdir % "/" % "cover." % "jpg";
    EXPECT_TRUE(img.scaled(400,400).save(cLoc_coverjpg, "jpg"));
    extraCovers << cLoc_coverJPG << cLoc_coverjpg;
    res = CoverArtCache::searchImage(cInfo, QSize(0,0), false);
    EXPECT_QSTRING_EQ(cLoc_coverJPG, res.coverLocation);

    // As we are looking for %album%.jpg and %base_track.jpg%,
    // we need to check if everything works with UTF8 chars.
    const CoverArtDAO::CoverArtInfo cInfoUtf8 = {
        2,                                             // cInfo.trackId
        "",                                            // cInfo.coverLocation
        "",                                            // cInfo.hash
        QString::fromUtf8("öæäîðÑ_album"),             // cInfo.album
        QString::fromUtf8("track_ðÑöæäî"),             // cInfo.trackBaseName
        trackdir,                                      // cInfo.trackDirectory
        trackdir % "/" % cInfo.trackBaseName % ".mp3"  // cInfo.trackLocation
    };
    // 1. track_filename.jpg
    cLoc_filename = QString(trackdir % "/" % cInfoUtf8.trackBaseName % ".");
    cLoc_filename.append(format);
    EXPECT_TRUE(img.save(cLoc_filename, format));
    res = CoverArtCache::searchImage(cInfoUtf8, QSize(0,0), false);
    EXPECT_QSTRING_EQ(cLoc_filename, res.coverLocation);
    QFile::remove(cLoc_filename);
    // 2. album_name.jpg
    cLoc_albumName = QString(trackdir % "/" % cInfoUtf8.album % ".");
    cLoc_albumName.append(format);
    EXPECT_TRUE(img.save(cLoc_albumName, format));
    res = CoverArtCache::searchImage(cInfoUtf8, QSize(0,0), false);
    EXPECT_QSTRING_EQ(cLoc_albumName, res.coverLocation);
    QFile::remove(cLoc_albumName);

    // cleaning temp dir
    foreach (QString loc, extraCovers) {
        QFile::remove(loc);
    }
    EXPECT_TRUE(QDir(trackdir).rmdir(trackdir));
}
