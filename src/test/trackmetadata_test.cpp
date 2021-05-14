#include <gtest/gtest.h>

#include "track/trackrecord.h"

class TrackMetadataTest : public testing::Test {
};

TEST_F(TrackMetadataTest, parseArtistTitleFromFileName) {
    {
        mixxx::TrackInfo trackInfo;
        trackInfo.parseArtistTitleFromFileName(" only - title ", false);
        EXPECT_EQ(QString(), trackInfo.getArtist());
        EXPECT_EQ("only - title", trackInfo.getTitle());
    }
    {
        mixxx::TrackInfo trackInfo;
        trackInfo.parseArtistTitleFromFileName(" only-title ", true);
        EXPECT_EQ(QString(), trackInfo.getArtist());
        EXPECT_EQ("only-title", trackInfo.getTitle());
    }
    {
        mixxx::TrackInfo trackInfo;
        trackInfo.parseArtistTitleFromFileName(" only -_title ", true);
        EXPECT_EQ(QString(), trackInfo.getArtist());
        EXPECT_EQ("only -_title", trackInfo.getTitle());
    }
    {
        mixxx::TrackInfo trackInfo;
        trackInfo.parseArtistTitleFromFileName(" only  -  title ", false);
        EXPECT_EQ(QString(), trackInfo.getArtist());
        EXPECT_EQ("only  -  title", trackInfo.getTitle());
    }
    {
        mixxx::TrackInfo trackInfo;
        trackInfo.parseArtistTitleFromFileName(" artist  -  title ", true);
        EXPECT_EQ("artist", trackInfo.getArtist());
        EXPECT_EQ("title", trackInfo.getTitle());
    }
    {
        mixxx::TrackInfo trackInfo;
        trackInfo.parseArtistTitleFromFileName(" only -\ttitle\t", true);
        EXPECT_EQ(QString(), trackInfo.getArtist());
        EXPECT_EQ("only -\ttitle", trackInfo.getTitle());
    }
    {
        mixxx::TrackInfo trackInfo;
        trackInfo.parseArtistTitleFromFileName(" - artist__-__title - ", true);
        EXPECT_EQ("- artist_", trackInfo.getArtist());
        EXPECT_EQ("_title -", trackInfo.getTitle());
    }
    {
        mixxx::TrackInfo trackInfo;
        trackInfo.parseArtistTitleFromFileName(" - only__-__title_-_", true);
        EXPECT_EQ(QString(), trackInfo.getArtist());
        EXPECT_EQ("- only__-__title_-_", trackInfo.getTitle());
    }
    {
        mixxx::TrackInfo trackInfo;
        trackInfo.parseArtistTitleFromFileName(" again - only_-_title _ ", true);
        EXPECT_EQ(QString(), trackInfo.getArtist());
        EXPECT_EQ("again - only_-_title _", trackInfo.getTitle());
    }
}

TEST_F(TrackMetadataTest, mergeImportedMetadata) {
    // Existing track metadata (stored in the database) without extra properties
    mixxx::TrackRecord oldTrackRecord;
    mixxx::TrackMetadata* pOldTrackMetadata = oldTrackRecord.ptrMetadata();
    pOldTrackMetadata->setStreamInfo(
            mixxx::audio::StreamInfo{
                    mixxx::audio::SignalInfo{
                            mixxx::audio::ChannelCount(1),
                            mixxx::audio::SampleRate(10000),
                    },
                    mixxx::audio::Bitrate(100),
                    mixxx::Duration::fromSeconds(60),
            });
    mixxx::TrackInfo* pOldTrackInfo = pOldTrackMetadata->ptrTrackInfo();
    pOldTrackInfo->setArtist("old artist");
    pOldTrackInfo->setBpm(mixxx::Bpm(100));
    pOldTrackInfo->setComment("old comment");
    pOldTrackInfo->setComposer("old composer");
    pOldTrackInfo->setGenre("old genre");
    pOldTrackInfo->setGrouping("old grouping");
    pOldTrackInfo->setKey("1A");
    pOldTrackInfo->setReplayGain(mixxx::ReplayGain(0.1, 1));
    pOldTrackInfo->setTitle("old title");
    pOldTrackInfo->setTrackNumber("1");
    pOldTrackInfo->setTrackTotal("10");
    pOldTrackInfo->setYear("2001-01-01");
    mixxx::AlbumInfo* pOldAlbumInfo = pOldTrackMetadata->ptrAlbumInfo();
    pOldAlbumInfo->setArtist("old album artist");
    pOldAlbumInfo->setTitle("old album title");

    // Imported track metadata (from file tags) with extra properties
    mixxx::TrackMetadata newTrackMetadata;
    newTrackMetadata.setStreamInfo(
            mixxx::audio::StreamInfo{
                    mixxx::audio::SignalInfo{
                            mixxx::audio::ChannelCount(2),
                            mixxx::audio::SampleRate(20000),
                    },
                    mixxx::audio::Bitrate(200),
                    mixxx::Duration::fromSeconds(120),
            });
    mixxx::TrackInfo* pNewTrackInfo = newTrackMetadata.ptrTrackInfo();
    pNewTrackInfo->setArtist("new artist");
    pNewTrackInfo->setBpm(mixxx::Bpm(200));
    pNewTrackInfo->setComment("new comment");
    pNewTrackInfo->setComposer("new composer");
#if defined(__EXTRA_METADATA__)
    pNewTrackInfo->setConductor("new conductor");
    pNewTrackInfo->setDiscNumber("1");
    pNewTrackInfo->setDiscTotal("2");
    pNewTrackInfo->setEncoder("encoder");
    pNewTrackInfo->setEncoderSettings("encoder settings");
#endif // __EXTRA_METADATA__
    pNewTrackInfo->setGenre("new genre");
    pNewTrackInfo->setGrouping("new grouping");
#if defined(__EXTRA_METADATA__)
    pNewTrackInfo->setISRC("isrc");
#endif // __EXTRA_METADATA__
    pNewTrackInfo->setKey("1A");
#if defined(__EXTRA_METADATA__)
    pNewTrackInfo->setLanguage("language");
    pNewTrackInfo->setLyricist("lyricist");
    pNewTrackInfo->setMood("mood");
    pNewTrackInfo->setMovement("movement");
    pNewTrackInfo->setMusicBrainzArtistId(QUuid("11111111-1111-1111-1111-111111111111"));
    pNewTrackInfo->setMusicBrainzRecordingId(QUuid("22222222-2222-2222-2222-222222222222"));
    pNewTrackInfo->setMusicBrainzReleaseId(QUuid("33333333-3333-3333-3333-333333333333"));
    pNewTrackInfo->setMusicBrainzWorkId(QUuid("44444444-4444-4444-4444-444444444444"));
    pNewTrackInfo->setRemixer("remixer");
#endif // __EXTRA_METADATA__
    pNewTrackInfo->setReplayGain(mixxx::ReplayGain(0.2, 2));
    pNewTrackInfo->setTitle("new title");
    pNewTrackInfo->setTrackNumber("2");
    pNewTrackInfo->setTrackTotal("20");
#if defined(__EXTRA_METADATA__)
    pNewTrackInfo->setWork("work");
#endif // __EXTRA_METADATA__
    pNewTrackInfo->setYear("2002-02-02");
    mixxx::AlbumInfo* pNewAlbumInfo = newTrackMetadata.ptrAlbumInfo();
    pNewAlbumInfo->setArtist("new album artist");
#if defined(__EXTRA_METADATA__)
    pNewAlbumInfo->setCopyright("copyright");
    pNewAlbumInfo->setLicense("license");
    pNewAlbumInfo->setMusicBrainzArtistId(QUuid("55555555-5555-5555-5555-555555555555"));
    pNewAlbumInfo->setMusicBrainzReleaseGroupId(QUuid("66666666-6666-6666-6666-666666666666"));
    pNewAlbumInfo->setMusicBrainzReleaseId(QUuid("77777777-7777-7777-7777-777777777777"));
    pNewAlbumInfo->setRecordLabel("copyright");
    pNewAlbumInfo->setReplayGain(mixxx::ReplayGain(0.3, 3));
#endif // __EXTRA_METADATA__
    pNewAlbumInfo->setTitle("new album title");

    mixxx::TrackRecord mergedTrackRecord = oldTrackRecord;
    ASSERT_EQ(mergedTrackRecord.getMetadata(), oldTrackRecord.getMetadata());
    ASSERT_NE(newTrackMetadata, *pOldTrackMetadata);
    mergedTrackRecord.mergeImportedMetadata(newTrackMetadata);

    mixxx::TrackMetadata* pMergedTrackMetadata = mergedTrackRecord.ptrMetadata();
    EXPECT_EQ(pOldTrackMetadata->getStreamInfo(), pMergedTrackMetadata->getStreamInfo());
    mixxx::TrackInfo* pMergedTrackInfo = pMergedTrackMetadata->ptrTrackInfo();
    EXPECT_EQ(pOldTrackInfo->getArtist(), pMergedTrackInfo->getArtist());
    EXPECT_EQ(pOldTrackInfo->getBpm(), pMergedTrackInfo->getBpm());
    EXPECT_EQ(pOldTrackInfo->getComment(), pMergedTrackInfo->getComment());
    EXPECT_EQ(pOldTrackInfo->getComposer(), pMergedTrackInfo->getComposer());
#if defined(__EXTRA_METADATA__)
    EXPECT_EQ(pNewTrackInfo->getConductor(), pMergedTrackInfo->getConductor());
    EXPECT_EQ(pNewTrackInfo->getDiscNumber(), pMergedTrackInfo->getDiscNumber());
    EXPECT_EQ(pNewTrackInfo->getDiscTotal(), pMergedTrackInfo->getDiscTotal());
    EXPECT_EQ(pNewTrackInfo->getEncoder(), pMergedTrackInfo->getEncoder());
    EXPECT_EQ(pNewTrackInfo->getEncoderSettings(), pMergedTrackInfo->getEncoderSettings());
#endif // __EXTRA_METADATA__
    EXPECT_EQ(pOldTrackInfo->getGenre(), pMergedTrackInfo->getGenre());
    EXPECT_EQ(pOldTrackInfo->getGrouping(), pMergedTrackInfo->getGrouping());
#if defined(__EXTRA_METADATA__)
    EXPECT_EQ(pNewTrackInfo->getISRC(), pMergedTrackInfo->getISRC());
#endif // __EXTRA_METADATA__
    EXPECT_EQ(pOldTrackInfo->getKey(), pMergedTrackInfo->getKey());
#if defined(__EXTRA_METADATA__)
    EXPECT_EQ(pNewTrackInfo->getLanguage(), pMergedTrackInfo->getLanguage());
    EXPECT_EQ(pNewTrackInfo->getLyricist(), pMergedTrackInfo->getLyricist());
    EXPECT_EQ(pNewTrackInfo->getMood(), pMergedTrackInfo->getMood());
    EXPECT_EQ(pNewTrackInfo->getMovement(), pMergedTrackInfo->getMovement());
    EXPECT_EQ(pNewTrackInfo->getMusicBrainzArtistId(), pMergedTrackInfo->getMusicBrainzArtistId());
    EXPECT_EQ(pNewTrackInfo->getMusicBrainzRecordingId(),
            pMergedTrackInfo->getMusicBrainzRecordingId());
    EXPECT_EQ(pNewTrackInfo->getMusicBrainzReleaseId(),
            pMergedTrackInfo->getMusicBrainzReleaseId());
    EXPECT_EQ(pNewTrackInfo->getMusicBrainzWorkId(),
            pMergedTrackInfo->getMusicBrainzWorkId());
    EXPECT_EQ(pNewTrackInfo->getRemixer(), pMergedTrackInfo->getRemixer());
#endif // __EXTRA_METADATA__
    EXPECT_EQ(
            pOldTrackInfo->getReplayGain(), pMergedTrackInfo->getReplayGain());
    EXPECT_EQ(pOldTrackInfo->getTitle(), pMergedTrackInfo->getTitle());
    EXPECT_EQ(pOldTrackInfo->getTrackNumber(),
            pMergedTrackInfo->getTrackNumber());
    EXPECT_EQ(
            pOldTrackInfo->getTrackTotal(), pMergedTrackInfo->getTrackTotal());
#if defined(__EXTRA_METADATA__)
    EXPECT_EQ(pNewTrackInfo->getWork(), pMergedTrackInfo->getWork());
#endif // __EXTRA_METADATA__
    EXPECT_EQ(pOldTrackInfo->getYear(), pMergedTrackInfo->getYear());
    mixxx::AlbumInfo* pMergedAlbumInfo =
            pMergedTrackMetadata->ptrAlbumInfo();
    EXPECT_EQ(pOldAlbumInfo->getArtist(), pMergedAlbumInfo->getArtist());
#if defined(__EXTRA_METADATA__)
    EXPECT_EQ(pNewAlbumInfo->getCopyright(), pMergedAlbumInfo->getCopyright());
    EXPECT_EQ(pNewAlbumInfo->getLicense(), pMergedAlbumInfo->getLicense());
    EXPECT_EQ(pNewAlbumInfo->getMusicBrainzArtistId(),
            pMergedAlbumInfo->getMusicBrainzArtistId());
    EXPECT_EQ(pNewAlbumInfo->getMusicBrainzReleaseGroupId(),
            pMergedAlbumInfo->getMusicBrainzReleaseGroupId());
    EXPECT_EQ(pNewAlbumInfo->getMusicBrainzReleaseId(),
            pMergedAlbumInfo->getMusicBrainzReleaseId());
    EXPECT_EQ(pNewAlbumInfo->getRecordLabel(), pMergedAlbumInfo->getRecordLabel());
    EXPECT_EQ(pNewAlbumInfo->getReplayGain(), pMergedAlbumInfo->getReplayGain());
#endif // __EXTRA_METADATA__
    EXPECT_EQ(pOldAlbumInfo->getTitle(), pMergedAlbumInfo->getTitle());

    // Check that all existing properties are preserved, even if empty or missing
    pMergedTrackInfo->setArtist("");
    pMergedTrackInfo->setTitle(QString());
    pMergedAlbumInfo->setArtist("");
    pMergedAlbumInfo->setTitle(QString());
    mergedTrackRecord.mergeImportedMetadata(newTrackMetadata);
    EXPECT_EQ(QString(""), pMergedTrackInfo->getArtist());
    EXPECT_EQ(QString(), pMergedTrackInfo->getTitle());
    EXPECT_EQ(QString(""), pMergedAlbumInfo->getArtist());
    EXPECT_EQ(QString(), pMergedAlbumInfo->getTitle());

    // Check that the placeholder for track total is replaced with the actual property
    ASSERT_NE(mixxx::TrackRecord::kTrackTotalPlaceholder, pNewTrackInfo->getTrackTotal());
    pMergedTrackInfo->setTrackTotal(mixxx::TrackRecord::kTrackTotalPlaceholder);
    mergedTrackRecord.mergeImportedMetadata(newTrackMetadata);
    EXPECT_EQ(pNewTrackInfo->getTrackTotal(), pMergedTrackInfo->getTrackTotal());
    // ...but if track total is missing entirely it should be preserved
    ASSERT_NE(QString(), pNewTrackInfo->getTrackTotal());
    pMergedTrackInfo->setTrackTotal(QString());
    mergedTrackRecord.mergeImportedMetadata(newTrackMetadata);
    EXPECT_EQ(QString(), pMergedTrackInfo->getTrackTotal());
}
