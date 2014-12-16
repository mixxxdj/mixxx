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
class CoverArtCacheTest : public MixxxTest, public CoverArtCache {
  protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

};

const QString kCoverLocationTest("res/images/library/default_cover.png");
const QString kTrackLocationTest(QDir::currentPath() %
                                 "/src/test/id3-test-data/cover-test.mp3");

TEST_F(CoverArtCacheTest, loadCover) {
    QImage img = QImage(kCoverLocationTest);
    CoverInfo info;
    info.type = CoverInfo::FILE;
    info.source = CoverInfo::GUESSED;
    info.coverLocation = "../../../" % kCoverLocationTest;
    info.trackLocation = kTrackLocationTest;
    info.hash = 4321; // fake cover hash

    CoverArtCache::FutureResult res;
    res = CoverArtCache::loadCover(info, NULL, 1234, 0, false);
    EXPECT_EQ(1234, res.requestReference);
    EXPECT_QSTRING_EQ(info.coverLocation, res.cover.info.coverLocation);
    EXPECT_QSTRING_EQ(info.hash, res.cover.info.hash);
    EXPECT_EQ(img, res.cover.image);

    info.type = CoverInfo::METADATA;
    info.source = CoverInfo::GUESSED;
    info.coverLocation = QString();
    info.trackLocation = kTrackLocationTest;
    res = CoverArtCache::loadCover(info, NULL, 1234, 0, false);
    EXPECT_EQ(1234, res.requestReference);
    EXPECT_QSTRING_EQ(QString(), res.cover.info.coverLocation);
    EXPECT_QSTRING_EQ(info.hash, res.cover.info.hash);

    SecurityTokenPointer securityToken = Sandbox::openSecurityToken(
        QDir(kTrackLocationTest), true);
    SoundSourceProxy proxy(kTrackLocationTest, securityToken);
    Mixxx::SoundSourcePointer pSoundSource(proxy.getSoundSource());
    ASSERT_TRUE(pSoundSource);
    img = pSoundSource->parseCoverArt();

    EXPECT_EQ(img, res.cover.image);
}
