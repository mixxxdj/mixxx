#include <gtest/gtest.h>
#include <QStringBuilder>

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

const QString& coverLocationTest = ":/images/library/vinyl-record.png";

TEST_F(CoverArtCacheTest, loadImage) {
    int trackId = 1;
    QImage img = QImage(coverLocationTest);

    CoverArtCache::FutureResult res;
    res = CoverArtCache::loadImage(coverLocationTest, trackId);
    ASSERT_EQ(trackId, res.trackId);
    EXPECT_QSTRING_EQ(coverLocationTest, res.coverLocation);
    ASSERT_TRUE(img.operator==(res.img));
}

TEST_F(CoverArtCacheTest, searchImage) {
    // creating a temp track directory
    QString trackdir(QDir::tempPath() % "/TrackDir");
    ASSERT_FALSE(QDir().exists(trackdir)); // it must start empty
    ASSERT_TRUE(QDir().mkpath(trackdir));
    QStringList files;

    // creating CoverArtInfo with empty coverLocation
    CoverArtDAO::CoverArtInfo cInfo;
    cInfo.trackId = 1;
    cInfo.album = "album_name";
    cInfo.trackFilename = "track.mp3";
    cInfo.trackDirectory = trackdir;
    cInfo.trackLocation = trackdir % "/" % cInfo.trackFilename;

    // looking for cover in an empty directory
    CoverArtCache::FutureResult res;
    res = CoverArtCache::searchImage(cInfo);
    EXPECT_QSTRING_EQ("", res.coverLocation);

    // setting image and format
    QImage img(coverLocationTest);
    const char* format("jpg");

    // saving just one cover in our temp track dir
    QString cLoc_foo = QString(trackdir % "/" % "foo.").append(format);
    EXPECT_TRUE(img.save(cLoc_foo, format));
    files << cLoc_foo;

    // looking for cover in an directory with one image
    res = CoverArtCache::searchImage(cInfo);
    EXPECT_QSTRING_EQ(cLoc_foo, res.coverLocation);

    // saving more covers
    // track_filename.jpg
    QString trackFilename = cInfo.trackFilename;
    trackFilename.remove(trackFilename.lastIndexOf("."), trackFilename.size() - 1);
    QString cLoc_filename = QString(trackdir % "/" % trackFilename % ".").append(format);
    EXPECT_TRUE(img.scaled(500,500).save(cLoc_filename, format));
    files << cLoc_filename;
    // album_name.jpg
    QString cLoc_album = QString(trackdir % "/" % cInfo.album % ".").append(format);
    EXPECT_TRUE(img.scaled(500,500).save(cLoc_album, format));
    files << cLoc_album;
    // cover.jpg
    QString cLoc_cover = QString(trackdir % "/" % "cover.").append(format);
    EXPECT_TRUE(img.scaled(400,400).save(cLoc_cover, format));
    files << cLoc_cover;
    // front.jpg
    QString cLoc_front = QString(trackdir % "/" % "front.").append(format);
    EXPECT_TRUE(img.scaled(300,300).save(cLoc_front, format));
    files << cLoc_front;
    // folder.jpg
    QString cLoc_folder = QString(trackdir % "/" % "folder.").append(format);
    EXPECT_TRUE(img.scaled(100,100).save(cLoc_folder, format));
    files << cLoc_folder;
    // lighter.jpg
    QString cLoc_lighter = QString(trackdir % "/" % "lighter.").append(format);
    EXPECT_TRUE(img.scaled(10,10).save(cLoc_lighter, format));
    files << cLoc_lighter;

    // we must find covers in the right order
    // 1. %track-file-base%.jpg in the track directory for %track-file-base%.mp3
    // 2. %album%.jpg
    // 3. cover.jpg
    // 4. front.jpg
    // 5. folder.jpg
    // 6. anything else found in the folder
    // (if we have more than one file,get the lighter one)
    res = CoverArtCache::searchImage(cInfo);
    EXPECT_QSTRING_EQ(cLoc_filename, res.coverLocation);
    QFile::remove(cLoc_filename);

    res = CoverArtCache::searchImage(cInfo);
    EXPECT_QSTRING_EQ(cLoc_album, res.coverLocation);
    QFile::remove(cLoc_album);

    res = CoverArtCache::searchImage(cInfo);
    EXPECT_QSTRING_EQ(cLoc_cover, res.coverLocation);
    QFile::remove(cLoc_cover);

    res = CoverArtCache::searchImage(cInfo);
    EXPECT_QSTRING_EQ(cLoc_front, res.coverLocation);
    QFile::remove(cLoc_front);

    res = CoverArtCache::searchImage(cInfo);
    EXPECT_QSTRING_EQ(cLoc_folder, res.coverLocation);
    QFile::remove(cLoc_folder);

    // adding some big images
    QString cLoc_big1 = QString(trackdir % "/" % "big1.").append(format);
    EXPECT_TRUE(img.scaled(1000,1000).save(cLoc_big1, format));
    files << cLoc_big1;
    QString cLoc_big2 = QString(trackdir % "/" % "big2.").append(format);
    EXPECT_TRUE(img.scaled(900,900).save(cLoc_big2, format));
    files << cLoc_big2;

    // as all covers do not have any of the favorite names,
    // it must get the lighter one.
    res = CoverArtCache::searchImage(cInfo);
    EXPECT_QSTRING_EQ(cLoc_lighter, res.coverLocation);

    // what is chosen when cover.jpg and cover.JPG exists?
    // (it must always prefer the lighter cover)
    QString cLoc_coverJPG = trackdir % "/" % "cover." % "JPG";
    EXPECT_TRUE(img.scaled(200,200).save(cLoc_coverJPG, "JPG"));
    QString cLoc_coverjpg = trackdir % "/" % "cover." % "jpg";
    EXPECT_TRUE(img.scaled(400,400).save(cLoc_coverjpg, "jpg"));
    files << cLoc_coverJPG << cLoc_coverjpg;
    res = CoverArtCache::searchImage(cInfo);
    EXPECT_QSTRING_EQ(cLoc_coverJPG, res.coverLocation);

    // cleaning temp dir
    foreach (QString loc, files) {
        QFile::remove(loc);
    }
    EXPECT_TRUE(QDir(trackdir).rmdir(trackdir));
}
