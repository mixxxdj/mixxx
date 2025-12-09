#include "library/coverartutils.h"

#include <QFileInfo>
#include <QTemporaryDir>

#include "library/coverartcache.h"
#include "sources/soundsourceproxy.h"
#include "test/librarytest.h"
#include "track/track.h"

namespace {

const QString kReferencePNGLocationTest = QStringLiteral("id3-test-data/reference_cover.png");
const QString kReferenceJPGLocationTest = QStringLiteral("id3-test-data/cover_test.jpg");

void extractEmbeddedCover(
        const QString& trackLocation,
        const QImage& expectedImage) {
    const QImage actualImage(
            CoverArtUtils::extractEmbeddedCover(
                    mixxx::FileAccess(mixxx::FileInfo(trackLocation))));
    ASSERT_FALSE(actualImage.isNull());
    EXPECT_EQ(expectedImage, actualImage);
}

} // anonymous namespace

// First inherit from LibraryTest to construct an QApplication instance
// needed by CoverArtCache for the default QPixmapCache.
// LibraryTest is required to instantiate the GlobalTrackCache singleton.
class CoverArtUtilTest : public LibraryTest, CoverArtCache {
};

TEST_F(CoverArtUtilTest, extractEmbeddedCover) {
    QImage referencePNGImage = QImage(getTestDir().filePath(kReferencePNGLocationTest));
    QImage referenceJPGImage = QImage(getTestDir().filePath(kReferenceJPGLocationTest));

    if (SoundSourceProxy::isFileSuffixSupported(QStringLiteral("aiff"))) {
        extractEmbeddedCover(getTestFile(QStringLiteral(".aiff")), referencePNGImage);
    }

    if (SoundSourceProxy::isFileSuffixSupported(QStringLiteral("flac"))) {
        extractEmbeddedCover(getTestFile(QStringLiteral(".flac")), referencePNGImage);
    }

    if (SoundSourceProxy::isFileSuffixSupported(QStringLiteral("m4a"))) {
        extractEmbeddedCover(getTestFile(QStringLiteral("-itunes-12.3.0-aac.m4a")),
                referencePNGImage);
        extractEmbeddedCover(getTestFile(QStringLiteral("-itunes-12.7.0-aac.m4a")),
                referencePNGImage);
        extractEmbeddedCover(getTestFile(QStringLiteral("-itunes-12.7.0-alac.m4a")),
                referencePNGImage);
    }

    if (SoundSourceProxy::isFileSuffixSupported(QStringLiteral("m4v"))) {
        extractEmbeddedCover(getTestFile(QStringLiteral(".m4v")),
                referencePNGImage);
    }

    if (SoundSourceProxy::isFileSuffixSupported(QStringLiteral("mp3"))) {
        // PNG
        extractEmbeddedCover(getTestFile(QStringLiteral("-png.mp3")),
                referencePNGImage);
        // JPEG
        extractEmbeddedCover(getTestFile(QStringLiteral("-jpg.mp3")),
                referenceJPGImage);
    }

    if (SoundSourceProxy::isFileSuffixSupported(QStringLiteral("ogg"))) {
        extractEmbeddedCover(getTestFile(QStringLiteral(".ogg")),
                referencePNGImage);
    }

    if (SoundSourceProxy::isFileSuffixSupported(QStringLiteral("opus"))) {
        // opus
        extractEmbeddedCover(getTestFile(QStringLiteral(".opus")),
                referencePNGImage);
    }

    if (SoundSourceProxy::isFileSuffixSupported(QStringLiteral("wav"))) {
        extractEmbeddedCover(getTestFile(QStringLiteral(".wav")),
                referencePNGImage);
    }

    if (SoundSourceProxy::isFileSuffixSupported(QStringLiteral("wv"))) {
        extractEmbeddedCover(getTestFile(QStringLiteral(".wv")),
                referencePNGImage);
    }
}

TEST_F(CoverArtUtilTest, searchImage) {
    // creating a temp track directory
    QTemporaryDir tempTrackDir;
    ASSERT_TRUE(tempTrackDir.isValid());
    QString trackdir = QString(tempTrackDir.path());

    const QString kTrackLocationTest(getTestFile(QStringLiteral("-png.mp3")));

    TrackPointer pTrack(Track::newTemporary(kTrackLocationTest));
    QList<QFileInfo> covers;
    CoverInfoRelative res;
    // looking for cover in an empty directory
    res = CoverArtUtils::selectCoverArtForTrack(*pTrack, covers);
    CoverInfoRelative expected1;
    expected1.source = CoverInfo::GUESSED;
    EXPECT_EQ(expected1, res);

    // Looking for a track with embedded cover.
    pTrack = Track::newTemporary(kTrackLocationTest);
    EXPECT_EQ(
            SoundSourceProxy::UpdateTrackFromSourceResult::MetadataImportedAndUpdated,
            SoundSourceProxy(pTrack).updateTrackFromSource(
                    SoundSourceProxy::UpdateTrackFromSourceMode::Once,
                    SyncTrackMetadataParams{}));
    CoverInfo result = pTrack->getCoverInfoWithLocation();
    EXPECT_EQ(result.type, CoverInfo::METADATA);
    EXPECT_EQ(result.source, CoverInfo::GUESSED);
    EXPECT_EQ(result.coverLocation, QString());
    EXPECT_NE(result.cacheKey(), CoverInfoRelative().cacheKey());

    const char* format("jpg");
    const QString qFormat(format);

    // Since we already parsed this image from the metadata in
    // kTrackLocationTest, hang on to it since we use it as a template for
    // stuff below.

    result.trackLocation = kTrackLocationTest;
    const QImage img = result.loadImage().image;
    EXPECT_EQ(img.isNull(), false);

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
    CoverInfoRelative expected2;

    // All the following expect FILE and GUESSED.
    expected2.type = CoverInfo::FILE;
    expected2.source = CoverInfo::GUESSED;

    // 0. saving just one cover in our temp track dir
    QString cLoc_foo = QString(trackdir % "/" % "foo." % qFormat);
    EXPECT_TRUE(img.save(cLoc_foo, format));

    // looking for cover in an directory with one image will select that one.
    expected2.coverLocation = "foo.jpg";
    expected2.setImageDigest(QImage(cLoc_foo));
    covers << QFileInfo(cLoc_foo);
    CoverInfoRelative res2 = CoverArtUtils::selectCoverArtForTrack(
            mixxx::FileInfo(trackBaseName), trackAlbum, covers);
    EXPECT_EQ(expected2, res2);
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
    QList<QFileInfo> prefCovers;
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
            expected2.type = CoverInfo::NONE;
            expected2.coverLocation = QString();
            expected2.setImageDigest(QImage());
        } else {
            expected2.type = CoverInfo::FILE;
            expected2.coverLocation = cover.fileName();
            expected2.setImageDigest(QImage(cover.filePath()));
        }
        res2 = CoverArtUtils::selectCoverArtForTrack(
                mixxx::FileInfo(trackBaseName), trackAlbum, prefCovers);
        EXPECT_QSTRING_EQ(expected2.coverLocation, res2.coverLocation);
        EXPECT_EQ(expected2.cacheKey(), res2.cacheKey());
        EXPECT_EQ(expected2, res2);

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

    res2 = CoverArtUtils::selectCoverArtForTrack(
            mixxx::FileInfo(trackBaseName), trackAlbum, prefCovers);
    expected2.setImageDigest(QImage(cLoc_coverJPG));
    expected2.coverLocation = "cover.JPG";
    EXPECT_EQ(expected2, res2);

    // As we are looking for %album%.jpg and %base_track.jpg%,
    // we need to check if everything works with UTF8 chars.
    trackBaseName = QString::fromUtf8("track_ðÑöæäî");
    trackAlbum = QString::fromUtf8("öæäîðÑ_album");

    prefCovers.clear();

    // 2. album_name.jpg
    cLoc_albumName = QString(trackdir % "/" % trackAlbum % "." % qFormat);
    EXPECT_TRUE(img.save(cLoc_albumName, format));
    prefCovers.append(QFileInfo(cLoc_albumName));
    res2 = CoverArtUtils::selectCoverArtForTrack(
            mixxx::FileInfo(trackBaseName), trackAlbum, prefCovers);
    expected2.setImageDigest(QImage(cLoc_albumName));
    expected2.coverLocation = trackAlbum % ".jpg";
    EXPECT_EQ(expected2, res2);

    // 1. track_filename.jpg
    cLoc_filename = QString(trackdir % "/" % trackBaseName % "." % qFormat);
    EXPECT_TRUE(img.save(cLoc_filename, format));
    prefCovers.append(QFileInfo(cLoc_filename));
    res2 = CoverArtUtils::selectCoverArtForTrack(
            mixxx::FileInfo(trackBaseName), trackAlbum, prefCovers);

    expected2.setImageDigest(QImage(cLoc_filename));
    expected2.coverLocation = trackBaseName % ".jpg";
    EXPECT_EQ(expected2, res2);

    QFile::remove(cLoc_filename);
    QFile::remove(cLoc_albumName);

    // cleaning temp dir
    foreach (QString loc, extraCovers) {
        QFile::remove(loc);
    }
}
