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
        QImage img;
        // Both resetMissingTagMetadata = false/true have the same effect
        constexpr auto resetMissingTagMetadata = false;
        SoundSourceProxy::importTrackMetadataAndCoverImageFromFile(
                mixxx::FileAccess(mixxx::FileInfo(trackLocation)),
                nullptr,
                &img,
                resetMissingTagMetadata);
        ASSERT_FALSE(img.isNull());

        CoverInfo info;
        info.type = CoverInfo::METADATA;
        info.source = CoverInfo::GUESSED;
        ASSERT_TRUE(info.coverLocation.isNull());
        info.trackLocation = trackLocation;

        CoverArtCache::FutureResult res;
        res = CoverArtCache::loadCover(TrackPointer(), info, 0);
        EXPECT_EQ(img, res.coverArt.loadedImage.image);
        EXPECT_TRUE(res.coverArt.coverLocation.isNull());
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
        res = CoverArtCache::loadCover(TrackPointer(), info, 0);
        EXPECT_EQ(img, res.coverArt.loadedImage.image);
        EXPECT_QSTRING_EQ(info.coverLocation, res.coverArt.coverLocation);
    }
};

const QString kCoverFileTest = QStringLiteral("cover_test.jpg");
const QString kCoverLocationTest = QStringLiteral("id3-test-data/") + kCoverFileTest;
const QString kTrackLocationTest = QStringLiteral("id3-test-data/cover-test-png.mp3");

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
    loadCoverFromMetadata(getTestDir().filePath(kTrackLocationTest));
}

TEST_F(CoverArtCacheTest, loadCoverFromFileRelative) {
    loadCoverFromFile(
            getTestDir().filePath(kTrackLocationTest),
            kCoverFileTest,
            getTestDir().filePath(kCoverLocationTest));
}

TEST_F(CoverArtCacheTest, loadCoverFromFileAbsolute) {
    loadCoverFromFile(
            QString(),
            getTestDir().filePath(kCoverLocationTest),
            getTestDir().filePath(kCoverLocationTest));
}
