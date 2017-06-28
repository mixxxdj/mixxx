#include <QtDebug>

#include "test/mixxxtest.h"

#include "track/track.h"
#include "sources/soundsourceproxy.h"

namespace {

const QDir kTestDir(QDir::current().absoluteFilePath("src/test/id3-test-data"));

} // anonymous namespace

class TrackObjectTest: public MixxxTest {
  protected:
    static TrackPointer newTestTrack() {
        return Track::newTemporary(
                kTestDir.absoluteFilePath("TOAL_TPE2.mp3"));
    }
};

TEST_F(TrackObjectTest, initTrackType) {
    TrackPointer pTrack = newTestTrack();

    // Verify preconditions
    ASSERT_TRUE(pTrack->getType().isEmpty());
    ASSERT_FALSE(pTrack->isDirty());

    pTrack->initType("mp3");

    // Initially setting the track type once should not set the dirty flag
    EXPECT_FALSE(pTrack->getType().isEmpty());
    EXPECT_FALSE(pTrack->isDirty());
    EXPECT_FALSE(pTrack->isHeaderParsed());

    pTrack->initType("mp3");

    // Initializing the track type again with the same value is allowed
    // and should not affect the track object
    EXPECT_FALSE(pTrack->getType().isEmpty());
    EXPECT_FALSE(pTrack->isDirty());
    EXPECT_FALSE(pTrack->isHeaderParsed());
}

TEST_F(TrackObjectTest, readAndUpdateMetadata) {
    TrackPointer pTrack = newTestTrack();

    // Parse initially
    SoundSourceProxy(pTrack).updateTrack(
            SoundSourceProxy::ParseFileTagsMode::Once);
    EXPECT_TRUE(pTrack->isDirty());
    EXPECT_TRUE(pTrack->isHeaderParsed());
    EXPECT_FALSE(pTrack->getArtist().isEmpty());
    EXPECT_EQ(CoverInfo::METADATA, pTrack->getCoverInfo().type);

    // Modify (-> reset) artist and cover art
    pTrack->setArtist(QString());
    ASSERT_TRUE(pTrack->getArtist().isEmpty());
    pTrack->setCoverInfo(CoverArt());
    ASSERT_EQ(CoverInfo::NONE, pTrack->getCoverInfo().type);

    // Parse once more -> ignore both
    ASSERT_TRUE(pTrack->isDirty());
    ASSERT_TRUE(pTrack->isHeaderParsed());
    SoundSourceProxy(pTrack).updateTrack(
            SoundSourceProxy::ParseFileTagsMode::Once);
    EXPECT_TRUE(pTrack->getArtist().isEmpty());
    EXPECT_EQ(CoverInfo::NONE, pTrack->getCoverInfo().type);

    // Parse again -> ignore both while dirty
    ASSERT_TRUE(pTrack->isDirty());
    ASSERT_TRUE(pTrack->isHeaderParsed());
    SoundSourceProxy(pTrack).updateTrack(
            SoundSourceProxy::ParseFileTagsMode::AgainWithoutCoverArt);
    EXPECT_TRUE(pTrack->getArtist().isEmpty());
    EXPECT_EQ(CoverInfo::NONE, pTrack->getCoverInfo().type);

    // Parse once more -> ignore both even if not dirty
    pTrack->markClean();
    ASSERT_FALSE(pTrack->isDirty());
    ASSERT_TRUE(pTrack->isHeaderParsed());
    SoundSourceProxy(pTrack).updateTrack(
            SoundSourceProxy::ParseFileTagsMode::Once);
    EXPECT_TRUE(pTrack->getArtist().isEmpty());
    EXPECT_EQ(CoverInfo::NONE, pTrack->getCoverInfo().type);

    // Parse again -> parse metadata, ignore cover art
    ASSERT_FALSE(pTrack->isDirty());
    ASSERT_TRUE(pTrack->isHeaderParsed());
    SoundSourceProxy(pTrack).updateTrack(
            SoundSourceProxy::ParseFileTagsMode::AgainWithoutCoverArt);
    EXPECT_TRUE(pTrack->isDirty());
    EXPECT_TRUE(pTrack->isHeaderParsed());
    EXPECT_FALSE(pTrack->getArtist().isEmpty());
    EXPECT_EQ(CoverInfo::NONE, pTrack->getCoverInfo().type);
}
