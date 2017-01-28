#include <gtest/gtest.h>
#include <QStringBuilder>
#include <QFileInfo>

#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "library/trackcollection.h"
#include "test/mixxxtest.h"
#include "sources/soundsourceproxy.h"

// first inherit from MixxxTest to construct a QApplication to be able to
// construct the default QPixmap in CoverArtCache
class CoverArtCacheTest : public MixxxTest, public CoverArtCache {
  protected:
    void loadCoverFromMetadata(QString trackLocation) {
        CoverInfo info;
        info.type = CoverInfo::METADATA;
        info.source = CoverInfo::GUESSED;
        info.coverLocation = QString();
        info.trackLocation = trackLocation;

        CoverArtCache::FutureResult res;
        res = CoverArtCache::loadCover(info, NULL, 0, false);
        EXPECT_QSTRING_EQ(QString(), res.cover.coverLocation);
        EXPECT_EQ(info.hash, res.cover.hash);

        SecurityTokenPointer securityToken = Sandbox::openSecurityToken(
            QDir(trackLocation), true);
        TrackPointer pTrack(Track::newTemporary(trackLocation, securityToken));
        SoundSourceProxy proxy(pTrack);
        QImage img(SoundSourceProxy(pTrack).parseCoverImage());
        EXPECT_FALSE(img.isNull());
        EXPECT_EQ(img, res.cover.image);
    }

    void loadCoverFromFile(QString trackLocation, QString coverLocation, QString absoluteCoverLocation) {
        QImage img = QImage(absoluteCoverLocation);

        CoverInfo info;
        info.type = CoverInfo::FILE;
        info.source = CoverInfo::GUESSED;
        info.coverLocation = coverLocation;
        info.trackLocation = trackLocation;
        info.hash = 4321; // fake cover hash

        CoverArtCache::FutureResult res;
        res = CoverArtCache::loadCover(info, NULL, 0, false);
        EXPECT_QSTRING_EQ(info.coverLocation, res.cover.coverLocation);
        EXPECT_EQ(info.hash, res.cover.hash);
        EXPECT_FALSE(img.isNull());
        EXPECT_EQ(img, res.cover.image);
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

TEST_F(CoverArtCacheTest, loadCover) {
    loadCoverFromMetadata(kTrackLocationTest);
    loadCoverFromFile(kTrackLocationTest, kCoverFileTest, kCoverLocationTest); //relative
    loadCoverFromFile(QString(), kCoverLocationTest, kCoverLocationTest); //absolute
}
