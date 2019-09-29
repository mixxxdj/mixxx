#include <gtest/gtest.h>

#include "track/trackmetadata.h"

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
