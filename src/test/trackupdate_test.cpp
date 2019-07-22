#include <QtDebug>

#include "test/mixxxtest.h"

#include "track/track.h"
#include "library/coverart.h"
#include "sources/soundsourceproxy.h"

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
        return Track::newTemporary(TrackFile(
                kTestDir, "TOAL_TPE2.mp3"));
    }

    static TrackPointer newTestTrackParsed() {
        auto pTrack = newTestTrack();
        SoundSourceProxy(pTrack).updateTrackFromSource();
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
        coverInfo.hash = coverInfo.hash + 1;
        pTrack->setCoverInfo(coverInfo);
        EXPECT_TRUE(pTrack->isDirty());
        return pTrack;
    }
};

TEST_F(TrackUpdateTest, parseModifiedCleanOnce) {
    auto pTrack = newTestTrackParsedModified();
    pTrack->markClean();

    mixxx::TrackMetadata trackMetadataBefore;
    pTrack->getTrackMetadata(&trackMetadataBefore);
    auto coverInfoBefore = pTrack->getCoverInfo();

    SoundSourceProxy(pTrack).updateTrackFromSource(
            SoundSourceProxy::ImportTrackMetadataMode::Once);

    mixxx::TrackMetadata trackMetadataAfter;
    pTrack->getTrackMetadata(&trackMetadataAfter);
    auto coverInfoAfter = pTrack->getCoverInfo();

    // Not updated
    EXPECT_TRUE(pTrack->isMetadataSynchronized());
    EXPECT_FALSE(pTrack->isDirty());
    EXPECT_EQ(trackMetadataBefore, trackMetadataAfter);
    EXPECT_EQ(coverInfoBefore, coverInfoAfter);
}

TEST_F(TrackUpdateTest, parseModifiedCleanAgainSkipCover) {
    auto pTrack = newTestTrackParsedModified();
    pTrack->markClean();

    mixxx::TrackMetadata trackMetadataBefore;
    pTrack->getTrackMetadata(&trackMetadataBefore);
    auto coverInfoBefore = pTrack->getCoverInfo();

    SoundSourceProxy(pTrack).updateTrackFromSource(
            SoundSourceProxy::ImportTrackMetadataMode::Again);

    mixxx::TrackMetadata trackMetadataAfter;
    pTrack->getTrackMetadata(&trackMetadataAfter);
    auto coverInfoAfter = pTrack->getCoverInfo();

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

    mixxx::TrackMetadata trackMetadataBefore;
    pTrack->getTrackMetadata(&trackMetadataBefore);
    auto coverInfoBefore = pTrack->getCoverInfo();

    SoundSourceProxy(pTrack).updateTrackFromSource(
            SoundSourceProxy::ImportTrackMetadataMode::Again);

    mixxx::TrackMetadata trackMetadataAfter;
    pTrack->getTrackMetadata(&trackMetadataAfter);
    auto coverInfoAfter = pTrack->getCoverInfo();

    // Updated
    EXPECT_TRUE(pTrack->isMetadataSynchronized());
    EXPECT_TRUE(pTrack->isDirty());
    EXPECT_NE(trackMetadataBefore, trackMetadataAfter);
    EXPECT_NE(coverInfoBefore, coverInfoAfter);
}

TEST_F(TrackUpdateTest, parseModifiedDirtyAgain) {
    auto pTrack = newTestTrackParsedModified();

    mixxx::TrackMetadata trackMetadataBefore;
    pTrack->getTrackMetadata(&trackMetadataBefore);
    auto coverInfoBefore = pTrack->getCoverInfo();

    SoundSourceProxy(pTrack).updateTrackFromSource(
            SoundSourceProxy::ImportTrackMetadataMode::Again);

    mixxx::TrackMetadata trackMetadataAfter;
    pTrack->getTrackMetadata(&trackMetadataAfter);
    auto coverInfoAfter = pTrack->getCoverInfo();

    // Updated
    EXPECT_TRUE(pTrack->isMetadataSynchronized());
    EXPECT_TRUE(pTrack->isDirty());
    EXPECT_NE(trackMetadataBefore, trackMetadataAfter);
    EXPECT_EQ(coverInfoBefore, coverInfoAfter);
}
