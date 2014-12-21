#include <gtest/gtest.h>
#include <QStringBuilder>
#include <QFileInfo>

#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "library/trackcollection.h"
#include "test/mixxxtest.h"
#include "soundsourceproxy.h"

// first inherit from MixxxTest to construct a QApplication to be able to
// construct the default QPixmap in CoverArtCache
class CoverArtUtilTest : public MixxxTest, public CoverArtCache {
  protected:
    virtual void SetUp() {
        m_pTrackCollection = new TrackCollection(config());
    }

    virtual void TearDown() {
        // make sure we clean up the db
        QSqlQuery query(m_pTrackCollection->getDatabase());
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

TEST_F(CoverArtUtilTest, extractEmbeddedCover) {
    const QString kTestPath(QDir::currentPath() % "/src/test/id3-test-data/");
    QImage cover;
    // We never need to acquire security tokens for tests since we don't run
    // them in a sandboxed environment.
    SecurityTokenPointer pToken;
    // aiff
    cover = CoverArtUtils::extractEmbeddedCover(kTestPath % "cover-test.aiff",
                                                pToken);
    EXPECT_TRUE(!cover.isNull());
    // flac
    cover = CoverArtUtils::extractEmbeddedCover(kTestPath % "cover-test.flac",
                                                pToken);
    EXPECT_TRUE(!cover.isNull());
    // mp3
    cover = CoverArtUtils::extractEmbeddedCover(kTestPath % "cover-test.mp3",
                                                pToken);
    EXPECT_TRUE(!cover.isNull());
    // ogg
    cover = CoverArtUtils::extractEmbeddedCover(kTestPath % "cover-test.ogg",
                                                pToken);
    EXPECT_TRUE(!cover.isNull());
    // wav
    cover = CoverArtUtils::extractEmbeddedCover(kTestPath % "cover-test.wav",
                                                pToken);
    EXPECT_TRUE(!cover.isNull());

#ifdef __OPUS__
    // opus
    cover = CoverArtUtils::extractEmbeddedCover(kTestPath % "cover-test.opus",
                                                pToken);
    EXPECT_TRUE(!cover.isNull());
#endif
}

TEST_F(CoverArtUtilTest, searchImage) {
    // creating a temp track directory
    QString trackdir(QDir::tempPath() % "/TrackDir");
    ASSERT_FALSE(QDir().exists(trackdir)); // it must start empty
    ASSERT_TRUE(QDir().mkpath(trackdir));

    TrackPointer pTrack(new TrackInfoObject(kTrackLocationTest));
    QLinkedList<QFileInfo> covers;
    CoverArt res;
    // looking for cover in an empty directory
    res = CoverArtUtils::selectCoverArtForTrack(pTrack.data(), covers);
    CoverArt expected;
    expected.info.source = CoverInfo::GUESSED;
    EXPECT_EQ(expected, res);

    // Looking for a track with embedded cover.
    pTrack = TrackPointer(new TrackInfoObject(kTrackLocationTest,
                                              SecurityTokenPointer(),
                                              true, true));
    expected = CoverArt();
    expected.image = pTrack->getCoverArt().image;
    expected.info.type = CoverInfo::METADATA;
    expected.info.source = CoverInfo::GUESSED;
    expected.info.coverLocation = QString();
    expected.info.hash = CoverArtUtils::calculateHash(expected.image);
    EXPECT_EQ(expected, pTrack->getCoverArt());

    const char* format("jpg");
    const QString qFormat(format);

    // Since we already parsed this image from the matadata in
    // kTrackLocationTest, hang on to it since we use it as a template for
    // stuff below.
    const QImage img = expected.image;


    QString trackBaseName = "cover-test";
    QString trackAlbum = "album_name";

    // Search Strategy
    // 0. If we have just one file, we will get it.
    // 1. %track-file-base%.jpg in the track directory for %track-file-base%.mp3
    // 2. %album%.jpg
    // 3. cover.jpg
    // 4. front.jpg
    // 5. album.jpg
    // 6. folder.jpg
    // 7. if just one file exists take that otherwise none.

    // All the following expect the same image/hash to be selected.
    expected.image = img;
    expected.info.hash = CoverArtUtils::calculateHash(expected.image);

    // All the following expect FILE and GUESSED.
    expected.info.type = CoverInfo::FILE;
    expected.info.source = CoverInfo::GUESSED;

    // 0. saving just one cover in our temp track dir
    QString cLoc_foo = QString(trackdir % "/" % "foo." % qFormat);
    EXPECT_TRUE(img.save(cLoc_foo, format));

    // looking for cover in an directory with one image will select that one.
    expected.image = QImage(cLoc_foo);
    expected.info.coverLocation = "foo.jpg";
    expected.info.hash = CoverArtUtils::calculateHash(expected.image);
    covers << QFileInfo(cLoc_foo);
    res = CoverArtUtils::selectCoverArtForTrack(trackBaseName, trackAlbum,
                                                covers);
    EXPECT_EQ(expected, res);
    QFile::remove(cLoc_foo);

    QStringList extraCovers;
    // adding some extra images (bigger) just to populate the track dir.
    QString cLoc_big1 = QString(trackdir % "/" % "big1." % qFormat);
    EXPECT_TRUE(img.scaled(1000,1000).save(cLoc_big1, format));
    extraCovers << cLoc_big1;
    QString cLoc_big2 = QString(trackdir % "/" % "big2." % qFormat);
    EXPECT_TRUE(img.scaled(900,900).save(cLoc_big2, format));
    extraCovers << cLoc_big2;
    QString cLoc_big3 = QString(trackdir % "/" % "big3." % qFormat);
    EXPECT_TRUE(img.scaled(800,800).save(cLoc_big3, format));
    extraCovers << cLoc_big3;

    // saving more covers using the preferred names in the right order
    QLinkedList<QFileInfo> prefCovers;
    // 1. track_filename.jpg
    QString cLoc_filename = QString(trackdir % "/cover-test." % qFormat);
    EXPECT_TRUE(img.scaled(500,500).save(cLoc_filename, format));
    prefCovers << QFileInfo(cLoc_filename);
    // 2. album_name.jpg
    QString cLoc_albumName = QString(trackdir % "/album_name." % qFormat);
    EXPECT_TRUE(img.scaled(500,500).save(cLoc_albumName, format));
    prefCovers << QFileInfo(cLoc_albumName);
    // 3. cover.jpg
    QString cLoc_cover = QString(trackdir % "/" % "cover." % qFormat);
    EXPECT_TRUE(img.scaled(400,400).save(cLoc_cover, format));
    prefCovers << QFileInfo(cLoc_cover);
    // 4. front.jpg
    QString cLoc_front = QString(trackdir % "/" % "front." % qFormat);
    EXPECT_TRUE(img.scaled(300,300).save(cLoc_front, format));
    prefCovers << QFileInfo(cLoc_front);
    // 5. album.jpg
    QString cLoc_album = QString(trackdir % "/" % "album." % qFormat);
    EXPECT_TRUE(img.scaled(100,100).save(cLoc_album, format));
    prefCovers << QFileInfo(cLoc_album);
    // 6. folder.jpg
    QString cLoc_folder = QString(trackdir % "/" % "folder." % qFormat);
    EXPECT_TRUE(img.scaled(100,100).save(cLoc_folder, format));
    prefCovers << QFileInfo(cLoc_folder);
    // 8. other1.jpg
    QString cLoc_other1 = QString(trackdir % "/" % "other1." % qFormat);
    EXPECT_TRUE(img.scaled(10,10).save(cLoc_other1, format));
    prefCovers << QFileInfo(cLoc_other1);
    // 7. other2.jpg
    QString cLoc_other2 = QString(trackdir % "/" % "other2." % qFormat);
    EXPECT_TRUE(img.scaled(10,10).save(cLoc_other2, format));
    prefCovers << QFileInfo(cLoc_other2);

    // we must find covers in the right order
    EXPECT_EQ(8, prefCovers.size());

    // Remove the covers one by one from the front, checking that each one is
    // selected as we remove the previously-most-preferable cover.
    while (!prefCovers.isEmpty()) {
        QFileInfo cover = prefCovers.first();

        // We expect no cover selected for other1 since there are 2 covers,
        // neither of which match our preferred cover names. other2 will be
        // selected once we get to it since it is the only cover available.
        if (cover.baseName() == "other1") {
            expected.image = QImage();
            expected.info.type = CoverInfo::NONE;
            expected.info.coverLocation = QString();
            expected.info.hash = 0;
        } else {
            expected.image = QImage(cover.filePath());
            expected.info.type = CoverInfo::FILE;
            expected.info.coverLocation = cover.fileName();
            expected.info.hash = CoverArtUtils::calculateHash(expected.image);
        }
        res = CoverArtUtils::selectCoverArtForTrack(trackBaseName, trackAlbum,
                                                    prefCovers);
        EXPECT_QSTRING_EQ(expected.info.coverLocation, res.info.coverLocation);
        EXPECT_QSTRING_EQ(expected.info.hash, res.info.hash);
        EXPECT_EQ(expected, res);

        QFile::remove(cover.filePath());
        prefCovers.pop_front();
    }

    //
    // Additional tests
    //

    // what is chosen when cover.jpg and cover.JPG exists?
    // (it must always prefer the lighter cover)
    QString cLoc_coverJPG = trackdir % "/" % "cover." % "JPG";
    EXPECT_TRUE(img.scaled(200,200).save(cLoc_coverJPG, "JPG"));
    prefCovers.append(QFileInfo(cLoc_coverJPG));
    QString cLoc_coverjpg = trackdir % "/" % "cover." % "jpg";
    EXPECT_TRUE(img.scaled(400,400).save(cLoc_coverjpg, "jpg"));
    prefCovers.append(QFileInfo(cLoc_coverjpg));
    extraCovers << cLoc_coverJPG << cLoc_coverjpg;

    res = CoverArtUtils::selectCoverArtForTrack(trackBaseName, trackAlbum,
                                                prefCovers);
    expected.image = QImage(cLoc_coverJPG);
    expected.info.hash = CoverArtUtils::calculateHash(expected.image);
    expected.info.coverLocation = "cover.JPG";
    EXPECT_EQ(expected, res);

    // As we are looking for %album%.jpg and %base_track.jpg%,
    // we need to check if everything works with UTF8 chars.
    trackBaseName = QString::fromUtf8("track_ðÑöæäî");
    trackAlbum = QString::fromUtf8("öæäîðÑ_album");

    prefCovers.clear();

    // 2. album_name.jpg
    cLoc_albumName = QString(trackdir % "/" % trackAlbum % "." % qFormat);
    EXPECT_TRUE(img.save(cLoc_albumName, format));
    prefCovers.append(QFileInfo(cLoc_albumName));
    res = CoverArtUtils::selectCoverArtForTrack(trackBaseName, trackAlbum,
                                                prefCovers);
    expected.image = QImage(cLoc_albumName);
    expected.info.hash = CoverArtUtils::calculateHash(expected.image);
    expected.info.coverLocation = trackAlbum % ".jpg";
    EXPECT_EQ(expected, res);

    // 1. track_filename.jpg
    cLoc_filename = QString(trackdir % "/" % trackBaseName % "." % qFormat);
    EXPECT_TRUE(img.save(cLoc_filename, format));
    prefCovers.append(QFileInfo(cLoc_filename));
    res = CoverArtUtils::selectCoverArtForTrack(trackBaseName, trackAlbum,
                                                prefCovers);
    expected.image = QImage(cLoc_filename);
    expected.info.hash = CoverArtUtils::calculateHash(expected.image);
    expected.info.coverLocation = trackBaseName % ".jpg";
    EXPECT_EQ(expected, res);

    QFile::remove(cLoc_filename);
    QFile::remove(cLoc_albumName);

    // cleaning temp dir
    foreach (QString loc, extraCovers) {
        QFile::remove(loc);
    }
    EXPECT_TRUE(QDir(trackdir).rmdir(trackdir));
}
