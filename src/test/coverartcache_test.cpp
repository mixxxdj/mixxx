#include <gtest/gtest.h>
#include <QFileInfo>

#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "library/trackcollection.h"
#include "test/librarytest.h"
#include "sources/soundsourceproxy.h"

// first inherit from MixxxTest to construct a QApplication to be able to
// construct the default QPixmap in CoverArtCache
class CoverArtCacheTest : public LibraryTest, public CoverArtCache {
  protected:
    void loadCoverFromMetadata(const QString& trackLocation) {
        CoverInfo info;
        info.type = CoverInfo::METADATA;
        info.source = CoverInfo::GUESSED;
        info.coverLocation = QString();
        info.trackLocation = trackLocation;

        CoverArtCache::FutureResult res;
        res = CoverArtCache::loadCover(nullptr, TrackPointer(), info, 0, false);
        EXPECT_QSTRING_EQ(QString(), res.coverArt.coverLocation);
        EXPECT_TRUE(CoverImageUtils::isValidHash(res.coverArt.hash));
        EXPECT_TRUE(res.coverInfoUpdated);

        SecurityTokenPointer securityToken =
                Sandbox::openSecurityToken(QDir(trackLocation), true);
        QImage img = SoundSourceProxy::importTemporaryCoverImage(
                trackLocation, securityToken);
        EXPECT_FALSE(img.isNull());
        EXPECT_EQ(img, res.coverArt.loadedImage.image);
    }

    void loadCoverFromFile(const QString& trackLocation,
            const QString& coverLocation,
            const QString& absoluteCoverLocation) {
        const QImage img = QImage(absoluteCoverLocation);
        ASSERT_FALSE(img.isNull());

        CoverInfo info;
        info.type = CoverInfo::FILE;
        info.source = CoverInfo::GUESSED;
        info.coverLocation = coverLocation;
        info.trackLocation = trackLocation;

        CoverArtCache::FutureResult res;
        res = CoverArtCache::loadCover(nullptr, TrackPointer(), info, 0, false);
        EXPECT_TRUE(res.coverInfoUpdated); // hash updated
        EXPECT_EQ(img, res.coverArt.loadedImage.image);
        EXPECT_EQ(CoverImageUtils::calculateHash(img), res.coverArt.hash);
        EXPECT_QSTRING_EQ(info.coverLocation, res.coverArt.coverLocation);
    }
};

const QString kCoverFileTest("cover_test.jpg");
const QString kCoverLocationTest(QDir::currentPath() %
                                 "/src/test/id3-test-data/" % kCoverFileTest);
const QString kTrackLocationTest(QDir::currentPath() %
                                 "/src/test/id3-test-data/cover-test-png.mp3");


// We need 3 separate test cases:
// 1) loadCoverFromMetadata()
// - CoverInfo::METADATA
// - absolute trackLocation
// - empty coverLocation (will be ignored)
// 2) loadCoverFromFileRelative()
// - CoverInfo::FILE
// - absolute trackLocation
// - coverLocation="cover_test.jpg"
// 3) loadCoverFromFileAbsolute()
// - CoverInfo::FILE
// - empty trackLocation
// - absolute coverLocation

TEST_F(CoverArtCacheTest, loadCoverFromMetadata) {
    loadCoverFromMetadata(kTrackLocationTest);
}

TEST_F(CoverArtCacheTest, loadCoverFromFileRelative) {
    loadCoverFromFile(kTrackLocationTest, kCoverFileTest, kCoverLocationTest);
}

TEST_F(CoverArtCacheTest, loadCoverFromFileAbsolute) {
    loadCoverFromFile(QString(), kCoverLocationTest, kCoverLocationTest);
}
