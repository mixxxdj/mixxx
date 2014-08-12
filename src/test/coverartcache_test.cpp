#include <gtest/gtest.h>
#include <QStringBuilder>

#include "library/coverartcache.h"
#include "library/dao/coverartdao.h"
#include "test/mixxxtest.h"

class CoverArtCacheTest : public CoverArtCache, public MixxxTest {
};

const QString& kCoverLocationTest = "res/images/library/default_cover.png";

TEST_F(CoverArtCacheTest, loadImage) {
    int trackId = 1;
    QImage img = QImage(kCoverLocationTest);
    QString md5hash = "md5hash"; // fake md5 hash

    CoverArtCache::FutureResult res;
    res = CoverArtCache::loadImage(trackId, kCoverLocationTest,
                                   md5hash, QSize(), false);
    ASSERT_EQ(trackId, res.trackId);
    EXPECT_QSTRING_EQ(kCoverLocationTest, res.coverLocation);
    EXPECT_QSTRING_EQ(md5hash, res.md5Hash);
    ASSERT_TRUE(img.operator==(res.img));
}

TEST_F(CoverArtCacheTest, searchImage) {
    // creating a temp track directory
    QString trackdir(QDir::tempPath() % "/TrackDir");
    ASSERT_FALSE(QDir().exists(trackdir)); // it must start empty
    ASSERT_TRUE(QDir().mkpath(trackdir));

    // creating CoverArtInfo with empty coverLocation
    const CoverArtDAO::CoverArtInfo cInfo = {
        1,                                             // cInfo.trackId
        "",                                            // cInfo.coverLocation
        "",                                            // cInfo.md5Hash
        "album_name",                                  // cInfo.album
        "track",                                       // cInfo.trackBaseName
        trackdir,                                      // cInfo.trackDirectory
        trackdir % "/" % cInfo.trackBaseName % ".mp3"  // cInfo.trackLocation
    };

    // looking for cover in an empty directory
    CoverArtCache::FutureResult res;
    res = CoverArtCache::searchImage(cInfo, QSize(), false);
    ASSERT_TRUE(res.coverLocation.isEmpty());

    // setting image source and default format
    QImage img(kCoverLocationTest);
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
    res = CoverArtCache::searchImage(cInfo, QSize(), false);
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
    ASSERT_EQ(7, prefCovers.size());
    for (int coverNameId = 0; coverNameId < prefCovers.size(); coverNameId++) {
        res = CoverArtCache::searchImage(cInfo, QSize(), false);
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
    res = CoverArtCache::searchImage(cInfo, QSize(), false);
    EXPECT_QSTRING_EQ(cLoc_coverJPG, res.coverLocation);

    // As we are looking for %album%.jpg and %base_track.jpg%,
    // we need to check if everything works with UTF8 chars.
    const CoverArtDAO::CoverArtInfo cInfoUtf8 = {
        2,                                             // cInfo.trackId
        "",                                            // cInfo.coverLocation
        "",                                            // cInfo.md5Hash
        QString::fromUtf8("öæäîðÑ_album"),             // cInfo.album
        QString::fromUtf8("track_ðÑöæäî"),             // cInfo.trackBaseName
        trackdir,                                      // cInfo.trackDirectory
        trackdir % "/" % cInfo.trackBaseName % ".mp3"  // cInfo.trackLocation
    };
    // 1. track_filename.jpg
    cLoc_filename = QString(trackdir % "/" % cInfoUtf8.trackBaseName % ".");
    cLoc_filename.append(format);
    EXPECT_TRUE(img.save(cLoc_filename, format));
    res = CoverArtCache::searchImage(cInfoUtf8, QSize(), false);
    EXPECT_QSTRING_EQ(cLoc_filename, res.coverLocation);
    QFile::remove(cLoc_filename);
    // 2. album_name.jpg
    cLoc_albumName = QString(trackdir % "/" % cInfoUtf8.album % ".");
    cLoc_albumName.append(format);
    EXPECT_TRUE(img.save(cLoc_albumName, format));
    res = CoverArtCache::searchImage(cInfoUtf8, QSize(), false);
    EXPECT_QSTRING_EQ(cLoc_albumName, res.coverLocation);
    QFile::remove(cLoc_albumName);

    // cleaning temp dir
    foreach (QString loc, extraCovers) {
        QFile::remove(loc);
    }
    EXPECT_TRUE(QDir(trackdir).rmdir(trackdir));
}
