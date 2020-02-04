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
    mixxx::TrackMetadata& oldTrackMetadata = oldTrackRecord.refMetadata();
    oldTrackMetadata.setBitrate(mixxx::AudioSource::Bitrate(100));
    oldTrackMetadata.setChannels(mixxx::AudioSignal::ChannelCount(1));
    oldTrackMetadata.setDuration(mixxx::Duration::fromSeconds(60));
    oldTrackMetadata.setSampleRate(mixxx::AudioSignal::SampleRate(10000));
    mixxx::TrackInfo& oldTrackInfo = oldTrackMetadata.refTrackInfo();
    oldTrackInfo.setArtist("old artist");
    oldTrackInfo.setBpm(mixxx::Bpm(100));
    oldTrackInfo.setComment("old comment");
    oldTrackInfo.setComposer("old composer");
    oldTrackInfo.setGenre("old genre");
    oldTrackInfo.setGrouping("old grouping");
    oldTrackInfo.setKey("1A");
    oldTrackInfo.setReplayGain(mixxx::ReplayGain(0.1, 1));
    oldTrackInfo.setTitle("old title");
    oldTrackInfo.setTrackNumber("1");
    oldTrackInfo.setTrackTotal("10");
    oldTrackInfo.setYear("2001-01-01");
    mixxx::AlbumInfo& oldAlbumInfo = oldTrackMetadata.refAlbumInfo();
    oldAlbumInfo.setArtist("old album artist");
    oldAlbumInfo.setTitle("old album title");

    // Imported track metadata (from file tags) with extra properties
    mixxx::TrackMetadata newTrackMetadata;
    newTrackMetadata.setBitrate(mixxx::AudioSource::Bitrate(200));
    newTrackMetadata.setChannels(mixxx::AudioSignal::ChannelCount(2));
    newTrackMetadata.setDuration(mixxx::Duration::fromSeconds(120));
    newTrackMetadata.setSampleRate(mixxx::AudioSignal::SampleRate(20000));
    mixxx::TrackInfo& newTrackInfo = newTrackMetadata.refTrackInfo();
    newTrackInfo.setArtist("new artist");
    newTrackInfo.setBpm(mixxx::Bpm(200));
    newTrackInfo.setComment("new comment");
    newTrackInfo.setComposer("new composer");
#if defined(__EXTRA_METADATA__)
    newTrackInfo.setConductor("new conductor");
    newTrackInfo.setDiscNumber("1");
    newTrackInfo.setDiscTotal("2");
    newTrackInfo.setEncoder("encoder");
    newTrackInfo.setEncoderSettings("encoder settings");
#endif // __EXTRA_METADATA__
    newTrackInfo.setGenre("new genre");
    newTrackInfo.setGrouping("new grouping");
#if defined(__EXTRA_METADATA__)
    newTrackInfo.setISRC("isrc");
#endif // __EXTRA_METADATA__
    newTrackInfo.setKey("1A");
#if defined(__EXTRA_METADATA__)
    newTrackInfo.setLanguage("language");
    newTrackInfo.setLyricist("lyricist");
    newTrackInfo.setMood("mood");
    newTrackInfo.setMovement("movement");
    newTrackInfo.setMusicBrainzArtistId(QUuid("11111111-1111-1111-1111-111111111111"));
    newTrackInfo.setMusicBrainzRecordingId(QUuid("22222222-2222-2222-2222-222222222222"));
    newTrackInfo.setMusicBrainzReleaseId(QUuid("33333333-3333-3333-3333-333333333333"));
    newTrackInfo.setMusicBrainzWorkId(QUuid("44444444-4444-4444-4444-444444444444"));
    newTrackInfo.setRemixer("remixer");
#endif // __EXTRA_METADATA__
    newTrackInfo.setReplayGain(mixxx::ReplayGain(0.2, 2));
    newTrackInfo.setTitle("new title");
    newTrackInfo.setTrackNumber("2");
    newTrackInfo.setTrackTotal("20");
#if defined(__EXTRA_METADATA__)
    newTrackInfo.setWork("work");
#endif // __EXTRA_METADATA__
    newTrackInfo.setYear("2002-02-02");
    mixxx::AlbumInfo& newAlbumInfo = newTrackMetadata.refAlbumInfo();
    newAlbumInfo.setArtist("new album artist");
#if defined(__EXTRA_METADATA__)
    newAlbumInfo.setCopyright("copyright");
    newAlbumInfo.setLicense("license");
    newAlbumInfo.setMusicBrainzArtistId(QUuid("55555555-5555-5555-5555-555555555555"));
    newAlbumInfo.setMusicBrainzReleaseGroupId(QUuid("66666666-6666-6666-6666-666666666666"));
    newAlbumInfo.setMusicBrainzReleaseId(QUuid("77777777-7777-7777-7777-777777777777"));
    newAlbumInfo.setRecordLabel("copyright");
    newAlbumInfo.setReplayGain(mixxx::ReplayGain(0.3, 3));
#endif // __EXTRA_METADATA__
    newAlbumInfo.setTitle("new album title");

    mixxx::TrackRecord mergedTrackRecord = oldTrackRecord;
    ASSERT_EQ(mergedTrackRecord.getMetadata(), oldTrackRecord.getMetadata());
    ASSERT_NE(newTrackMetadata, oldTrackMetadata);
    mergedTrackRecord.mergeImportedMetadata(newTrackMetadata);

    mixxx::TrackMetadata& mergedTrackMetadata = mergedTrackRecord.refMetadata();
    EXPECT_EQ(oldTrackMetadata.getBitrate(), mergedTrackMetadata.getBitrate());
    EXPECT_EQ(oldTrackMetadata.getChannels(), mergedTrackMetadata.getChannels());
    EXPECT_EQ(oldTrackMetadata.getDuration(), mergedTrackMetadata.getDuration());
    EXPECT_EQ(oldTrackMetadata.getSampleRate(), mergedTrackMetadata.getSampleRate());
    mixxx::TrackInfo& mergedTrackInfo = mergedTrackMetadata.refTrackInfo();
    EXPECT_EQ(oldTrackInfo.getArtist(), mergedTrackInfo.getArtist());
    EXPECT_EQ(oldTrackInfo.getBpm(), mergedTrackInfo.getBpm());
    EXPECT_EQ(oldTrackInfo.getComment(), mergedTrackInfo.getComment());
    EXPECT_EQ(oldTrackInfo.getComposer(), mergedTrackInfo.getComposer());
#if defined(__EXTRA_METADATA__)
    EXPECT_EQ(newTrackInfo.getConductor(), mergedTrackInfo.getConductor());
    EXPECT_EQ(newTrackInfo.getDiscNumber(), mergedTrackInfo.getDiscNumber());
    EXPECT_EQ(newTrackInfo.getDiscTotal(), mergedTrackInfo.getDiscTotal());
    EXPECT_EQ(newTrackInfo.getEncoder(), mergedTrackInfo.getEncoder());
    EXPECT_EQ(newTrackInfo.getEncoderSettings(), mergedTrackInfo.getEncoderSettings());
#endif // __EXTRA_METADATA__
    EXPECT_EQ(oldTrackInfo.getGenre(), mergedTrackInfo.getGenre());
    EXPECT_EQ(oldTrackInfo.getGrouping(), mergedTrackInfo.getGrouping());
#if defined(__EXTRA_METADATA__)
    EXPECT_EQ(newTrackInfo.getISRC(), mergedTrackInfo.getISRC());
#endif // __EXTRA_METADATA__
    EXPECT_EQ(oldTrackInfo.getKey(), mergedTrackInfo.getKey());
#if defined(__EXTRA_METADATA__)
    EXPECT_EQ(newTrackInfo.getLanguage(), mergedTrackInfo.getLanguage());
    EXPECT_EQ(newTrackInfo.getLyricist(), mergedTrackInfo.getLyricist());
    EXPECT_EQ(newTrackInfo.getMood(), mergedTrackInfo.getMood());
    EXPECT_EQ(newTrackInfo.getMovement(), mergedTrackInfo.getMovement());
    EXPECT_EQ(newTrackInfo.getMusicBrainzArtistId(), mergedTrackInfo.getMusicBrainzArtistId());
    EXPECT_EQ(newTrackInfo.getMusicBrainzRecordingId(), mergedTrackInfo.getMusicBrainzRecordingId());
    EXPECT_EQ(newTrackInfo.getMusicBrainzReleaseId(), mergedTrackInfo.getMusicBrainzReleaseId());
    EXPECT_EQ(newTrackInfo.getMusicBrainzWorkId(), mergedTrackInfo.getMusicBrainzWorkId());
    EXPECT_EQ(newTrackInfo.getRemixer(), mergedTrackInfo.getRemixer());
#endif // __EXTRA_METADATA__
    EXPECT_EQ(oldTrackInfo.getReplayGain(), mergedTrackInfo.getReplayGain());
    EXPECT_EQ(oldTrackInfo.getTitle(), mergedTrackInfo.getTitle());
    EXPECT_EQ(oldTrackInfo.getTrackNumber(), mergedTrackInfo.getTrackNumber());
    EXPECT_EQ(oldTrackInfo.getTrackTotal(), mergedTrackInfo.getTrackTotal());
#if defined(__EXTRA_METADATA__)
    EXPECT_EQ(newTrackInfo.getWork(), mergedTrackInfo.getWork());
#endif // __EXTRA_METADATA__
    EXPECT_EQ(oldTrackInfo.getYear(), mergedTrackInfo.getYear());
    mixxx::AlbumInfo& mergedAlbumInfo = mergedTrackMetadata.refAlbumInfo();
    EXPECT_EQ(oldAlbumInfo.getArtist(), mergedAlbumInfo.getArtist());
#if defined(__EXTRA_METADATA__)
    EXPECT_EQ(newAlbumInfo.getCopyright(), mergedAlbumInfo.getCopyright());
    EXPECT_EQ(newAlbumInfo.getLicense(), mergedAlbumInfo.getLicense());
    EXPECT_EQ(newAlbumInfo.getMusicBrainzArtistId(), mergedAlbumInfo.getMusicBrainzArtistId());
    EXPECT_EQ(newAlbumInfo.getMusicBrainzReleaseGroupId(), mergedAlbumInfo.getMusicBrainzReleaseGroupId());
    EXPECT_EQ(newAlbumInfo.getMusicBrainzReleaseId(), mergedAlbumInfo.getMusicBrainzReleaseId());
    EXPECT_EQ(newAlbumInfo.getRecordLabel(), mergedAlbumInfo.getRecordLabel());
    EXPECT_EQ(newAlbumInfo.getReplayGain(), mergedAlbumInfo.getReplayGain());
#endif // __EXTRA_METADATA__
    EXPECT_EQ(oldAlbumInfo.getTitle(), mergedAlbumInfo.getTitle());

    // Check that all existing properties are preserved, even if empty or missing
    mergedTrackInfo.setArtist("");
    mergedTrackInfo.setTitle(QString());
    mergedAlbumInfo.setArtist("");
    mergedAlbumInfo.setTitle(QString());
    mergedTrackRecord.mergeImportedMetadata(newTrackMetadata);
    EXPECT_EQ(QString(""), mergedTrackInfo.getArtist());
    EXPECT_EQ(QString(), mergedTrackInfo.getTitle());
    EXPECT_EQ(QString(""), mergedAlbumInfo.getArtist());
    EXPECT_EQ(QString(), mergedAlbumInfo.getTitle());

    // Check that the placeholder for track total is replaced with the actual property
    ASSERT_NE(mixxx::TrackRecord::kTrackTotalPlaceholder, newTrackInfo.getTrackTotal());
    mergedTrackInfo.setTrackTotal(mixxx::TrackRecord::kTrackTotalPlaceholder);
    mergedTrackRecord.mergeImportedMetadata(newTrackMetadata);
    EXPECT_EQ(newTrackInfo.getTrackTotal(), mergedTrackInfo.getTrackTotal());
    // ...but if track total is missing entirely it should be preserved
    ASSERT_NE(QString(), newTrackInfo.getTrackTotal());
    mergedTrackInfo.setTrackTotal(QString());
    mergedTrackRecord.mergeImportedMetadata(newTrackMetadata);
    EXPECT_EQ(QString(), mergedTrackInfo.getTrackTotal());
}
