#include <QtDebug>

#include "library/coverart.h"
#include "sources/soundsourceproxy.h"
#include "test/mixxxtest.h"
#include "track/track.h"

namespace {

const QDir kTestDir(QDir::current().absoluteFilePath("src/test/id3-test-data"));

} // anonymous namespace

// Test for updating track metadata and cover art from files.
class TrackUpdateTest: public MixxxTest {
  protected:
    static bool hasTrackMetadata(const TrackPointer& pTrack) {
        return !pTrack->getArtist().isEmpty();
    }

    static bool hasCoverArt(const TrackPointer& pTrack) {
        return pTrack->getCoverInfo().type != CoverInfo::NONE;
    }

    static TrackPointer newTestTrack() {
        return Track::newTemporary(kTestDir, "TOAL_TPE2.mp3");
    }

    static TrackPointer newTestTrackParsed() {
        auto pTrack = newTestTrack();
        EXPECT_TRUE(SoundSourceProxy(pTrack).updateTrackFromSource());
        EXPECT_TRUE(pTrack->isMetadataSynchronized());
        EXPECT_TRUE(hasTrackMetadata(pTrack));
        EXPECT_TRUE(hasCoverArt(pTrack));
        pTrack->markClean();
        EXPECT_FALSE(pTrack->isDirty());
        return pTrack;
    }

    static TrackPointer newTestTrackParsedModified() {
        auto pTrack = newTestTrackParsed();
        pTrack->setArtist(pTrack->getArtist() + pTrack->getArtist());
        auto coverInfo = pTrack->getCoverInfo();
        coverInfo.type = CoverInfo::FILE;
        coverInfo.source = CoverInfo::USER_SELECTED;
        coverInfo.setImage(QImage(1, 1, QImage::Format_Mono));
        pTrack->setCoverInfo(coverInfo);
        EXPECT_TRUE(pTrack->isDirty());
        return pTrack;
    }
};

TEST_F(TrackUpdateTest, parseModifiedCleanOnce) {
    auto pTrack = newTestTrackParsedModified();
    pTrack->markClean();

    const auto trackMetadataBefore = pTrack->getMetadata();
    const auto coverInfoBefore = pTrack->getCoverInfo();

    // Re-update from source should have no effect
    ASSERT_FALSE(SoundSourceProxy(pTrack).updateTrackFromSource(
            SoundSourceProxy::UpdateTrackFromSourceMode::Once));

    const auto trackMetadataAfter = pTrack->getMetadata();
    const auto coverInfoAfter = pTrack->getCoverInfo();

    // Verify that the track has not been modified
    ASSERT_TRUE(pTrack->isMetadataSynchronized());
    ASSERT_FALSE(pTrack->isDirty());
    ASSERT_EQ(trackMetadataBefore, trackMetadataAfter);
    ASSERT_EQ(coverInfoBefore, coverInfoAfter);
}

TEST_F(TrackUpdateTest, parseModifiedCleanAgainSkipCover) {
    auto pTrack = newTestTrackParsedModified();
    pTrack->markClean();

    const auto trackMetadataBefore = pTrack->getMetadata();
    const auto coverInfoBefore = pTrack->getCoverInfo();

    EXPECT_TRUE(SoundSourceProxy(pTrack).updateTrackFromSource(
            SoundSourceProxy::UpdateTrackFromSourceMode::Again));

    const auto trackMetadataAfter = pTrack->getMetadata();
    const auto coverInfoAfter = pTrack->getCoverInfo();

    // Updated
    EXPECT_TRUE(pTrack->isMetadataSynchronized());
    EXPECT_TRUE(pTrack->isDirty());
    EXPECT_NE(trackMetadataBefore, trackMetadataAfter);
    EXPECT_EQ(coverInfoBefore, coverInfoAfter);
}

TEST_F(TrackUpdateTest, parseModifiedCleanAgainUpdateCover) {
    auto pTrack = newTestTrackParsedModified();
    auto coverInfo = pTrack->getCoverInfo();
    coverInfo.type = CoverInfo::METADATA;
    coverInfo.source = CoverInfo::GUESSED;
    pTrack->setCoverInfo(coverInfo);
    pTrack->markClean();

    const auto trackMetadataBefore = pTrack->getMetadata();
    const auto coverInfoBefore = pTrack->getCoverInfo();

    EXPECT_TRUE(SoundSourceProxy(pTrack).updateTrackFromSource(
            SoundSourceProxy::UpdateTrackFromSourceMode::Again));

    const auto trackMetadataAfter = pTrack->getMetadata();
    const auto coverInfoAfter = pTrack->getCoverInfo();

    // Updated
    EXPECT_TRUE(pTrack->isMetadataSynchronized());
    EXPECT_TRUE(pTrack->isDirty());
    EXPECT_NE(trackMetadataBefore, trackMetadataAfter);
    EXPECT_NE(coverInfoBefore, coverInfoAfter);
}

TEST_F(TrackUpdateTest, parseModifiedDirtyAgain) {
    auto pTrack = newTestTrackParsedModified();

    const auto trackMetadataBefore = pTrack->getMetadata();
    const auto coverInfoBefore = pTrack->getCoverInfo();

    EXPECT_TRUE(SoundSourceProxy(pTrack).updateTrackFromSource(
            SoundSourceProxy::UpdateTrackFromSourceMode::Again));

    const auto trackMetadataAfter = pTrack->getMetadata();
    const auto coverInfoAfter = pTrack->getCoverInfo();

    // Updated
    EXPECT_TRUE(pTrack->isMetadataSynchronized());
    EXPECT_TRUE(pTrack->isDirty());
    EXPECT_NE(trackMetadataBefore, trackMetadataAfter);
    EXPECT_EQ(coverInfoBefore, coverInfoAfter);
}
