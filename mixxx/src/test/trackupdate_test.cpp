#include <QtDebug>

#include "library/coverart.h"
#include "sources/soundsourceproxy.h"
#include "test/mixxxtest.h"
#include "test/soundsourceproviderregistration.h"
#include "track/track.h"

// Test for updating track metadata and cover art from files.
class TrackUpdateTest : public MixxxTest, SoundSourceProviderRegistration {
  protected:
    static bool hasTrackMetadata(const TrackPointer& pTrack) {
        return !pTrack->getArtist().isEmpty();
    }

    static bool hasCoverArt(const TrackPointer& pTrack) {
        return pTrack->getCoverInfo().type != CoverInfo::NONE;
    }

    static TrackPointer newTestTrack() {
        return Track::newTemporary(
                QDir(MixxxTest::getOrInitTestDir().filePath(QStringLiteral("id3-test-data"))),
                "TOAL_TPE2.mp3");
    }

    TrackPointer newTestTrackParsed() const {
        auto pTrack = newTestTrack();
        EXPECT_EQ(
                SoundSourceProxy::UpdateTrackFromSourceResult::MetadataImportedAndUpdated,
                SoundSourceProxy(pTrack).updateTrackFromSource(
                        SoundSourceProxy::UpdateTrackFromSourceMode::Once,
                        SyncTrackMetadataParams{}));
        EXPECT_TRUE(pTrack->checkSourceSynchronized());
        EXPECT_TRUE(hasTrackMetadata(pTrack));
        EXPECT_TRUE(hasCoverArt(pTrack));
        pTrack->markClean();
        EXPECT_FALSE(pTrack->isDirty());
        return pTrack;
    }

    TrackPointer newTestTrackParsedModified() const {
        auto pTrack = newTestTrackParsed();
        pTrack->setArtist(pTrack->getArtist() + pTrack->getArtist());
        auto coverInfo = pTrack->getCoverInfo();
        coverInfo.type = CoverInfo::FILE;
        coverInfo.source = CoverInfo::USER_SELECTED;
        coverInfo.setImageDigest(QImage(1, 1, QImage::Format_Mono));
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
    ASSERT_EQ(
            SoundSourceProxy::UpdateTrackFromSourceResult::NotUpdated,
            SoundSourceProxy(pTrack).updateTrackFromSource(
                    SoundSourceProxy::UpdateTrackFromSourceMode::Once,
                    SyncTrackMetadataParams{}));

    const auto trackMetadataAfter = pTrack->getMetadata();
    const auto coverInfoAfter = pTrack->getCoverInfo();

    // Verify that the track has not been modified
    ASSERT_TRUE(pTrack->checkSourceSynchronized());
    ASSERT_FALSE(pTrack->isDirty());
    ASSERT_EQ(trackMetadataBefore, trackMetadataAfter);
    ASSERT_EQ(coverInfoBefore, coverInfoAfter);
}

TEST_F(TrackUpdateTest, parseModifiedCleanAgainSkipCover) {
    auto pTrack = newTestTrackParsedModified();
    pTrack->markClean();

    const auto trackMetadataBefore = pTrack->getMetadata();
    const auto coverInfoBefore = pTrack->getCoverInfo();

    EXPECT_EQ(
            SoundSourceProxy::UpdateTrackFromSourceResult::MetadataImportedAndUpdated,
            SoundSourceProxy(pTrack).updateTrackFromSource(
                    SoundSourceProxy::UpdateTrackFromSourceMode::Always,
                    SyncTrackMetadataParams{}));

    const auto trackMetadataAfter = pTrack->getMetadata();
    const auto coverInfoAfter = pTrack->getCoverInfo();

    // Updated
    EXPECT_TRUE(pTrack->checkSourceSynchronized());
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

    EXPECT_EQ(
            SoundSourceProxy::UpdateTrackFromSourceResult::MetadataImportedAndUpdated,
            SoundSourceProxy(pTrack).updateTrackFromSource(
                    SoundSourceProxy::UpdateTrackFromSourceMode::Always,
                    SyncTrackMetadataParams{}));

    const auto trackMetadataAfter = pTrack->getMetadata();
    const auto coverInfoAfter = pTrack->getCoverInfo();

    // Updated
    EXPECT_TRUE(pTrack->checkSourceSynchronized());
    EXPECT_TRUE(pTrack->isDirty());
    EXPECT_NE(trackMetadataBefore, trackMetadataAfter);
    EXPECT_NE(coverInfoBefore, coverInfoAfter);
}

TEST_F(TrackUpdateTest, parseModifiedDirtyAgain) {
    auto pTrack = newTestTrackParsedModified();

    const auto trackMetadataBefore = pTrack->getMetadata();
    const auto coverInfoBefore = pTrack->getCoverInfo();

    EXPECT_EQ(
            SoundSourceProxy::UpdateTrackFromSourceResult::MetadataImportedAndUpdated,
            SoundSourceProxy(pTrack).updateTrackFromSource(
                    SoundSourceProxy::UpdateTrackFromSourceMode::Always,
                    SyncTrackMetadataParams{}));

    const auto trackMetadataAfter = pTrack->getMetadata();
    const auto coverInfoAfter = pTrack->getCoverInfo();

    // Updated
    EXPECT_TRUE(pTrack->checkSourceSynchronized());
    EXPECT_TRUE(pTrack->isDirty());
    EXPECT_NE(trackMetadataBefore, trackMetadataAfter);
    EXPECT_EQ(coverInfoBefore, coverInfoAfter);
}

// TODO: Add tests for SoundSourceProxy::UpdateTrackFromSourceMode::Newer
