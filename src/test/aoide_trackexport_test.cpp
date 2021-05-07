#include <gtest/gtest.h>

#include <QtDebug>

#include "aoide/trackexport.h"
#include "tagging/taggingconfig.h"
#include "track/track.h"

namespace {

const QDir kTestDir(QDir::current().absoluteFilePath("src/test/id3-test-data"));

}

namespace aoide {

class TrackExportTest : public testing::Test {
  protected:
    void exportDateTimeOrYear(const QString& input, const QJsonValue& expectedOutput) {
        const auto actualOutput =
                QJsonValue::fromVariant(json::exportDateTimeOrYear(input));
        if (actualOutput != expectedOutput) {
            qWarning() << "expected =" << expectedOutput << ", actual =" << actualOutput;
            EXPECT_EQ(expectedOutput, actualOutput);
        }
    }
};

TEST_F(TrackExportTest, exportDateTimeOrYear) {
    // Unmodified
    exportDateTimeOrYear("2018-01-01T01:02:03.457Z", "2018-01-01T01:02:03.457Z");
    exportDateTimeOrYear("2018-01-01T01:02:03.457+02:00", "2018-01-01T01:02:03.457+02:00");

    // Round to milliseconds
    exportDateTimeOrYear("2018-01-01T01:02:03.45678Z", "2018-01-01T01:02:03.457Z");
    exportDateTimeOrYear("2018-01-01T01:02:03.45678+02:00", "2018-01-01T01:02:03.457+02:00");

    // Strip zero milliseconds
    exportDateTimeOrYear("2018-04-27T07:00:00.000Z", "2018-04-27T07:00:00Z");
    exportDateTimeOrYear("2018-04-27T07:00:00.000-06:00", "2018-04-27T07:00:00-06:00");

    // Without milliseconds
    exportDateTimeOrYear("2018-04-27T07:00:00Z", "2018-04-27T07:00:00Z");
    exportDateTimeOrYear("2018-04-27T07:00:00-06:00", "2018-04-27T07:00:00-06:00");

    // Missing time zone or spec -> assume UTC
    exportDateTimeOrYear("2018-04-27T07:00:00.123", "2018-04-27T07:00:00.123Z");
    exportDateTimeOrYear("2018-04-27T07:00:00", "2018-04-27T07:00:00Z");

    // Missing time zone or spec and missing seconds -> assume UTC
    exportDateTimeOrYear("2018-04-27T07:00", "2018-04-27T07:00:00Z");

    // Space-separated and missing time zone or spec -> assume UTC
    exportDateTimeOrYear("2018-12-08 04:28:16", "2018-12-08T04:28:16Z");
    exportDateTimeOrYear("2018-12-21 05:59", "2018-12-21T05:59:00Z");

    // Only a date without a time
    exportDateTimeOrYear("\t2007-11-16", 20071116);
    exportDateTimeOrYear("1996-01-01\n", 19960101);
    exportDateTimeOrYear("1989- 3- 9", 19890309);

    // Only a year + month
    exportDateTimeOrYear("2007-11 ", 20071100);
    exportDateTimeOrYear(" 2007- 4", 20070400);

    // Only a year
    exportDateTimeOrYear(" 2007 ", 2007);
}

TEST_F(TrackExportTest, ExportTrack) {
    const mixxx::TaggingConfig taggingConfig;

    TrackPointer trackPtr = Track::newTemporary(
            mixxx::FileAccess(mixxx::FileInfo("test.flac")));
    trackPtr->setAudioProperties(
            mixxx::audio::ChannelCount(2),
            mixxx::audio::SampleRate(44100),
            mixxx::audio::Bitrate(500),
            mixxx::Duration::fromMillis(1000));
    trackPtr->setTitle("Track Title");
    trackPtr->setArtist("Track Artist");
    trackPtr->setAlbum("Album Title");
    trackPtr->setAlbumArtist("Album Artist");
    trackPtr->updateGenre(
            taggingConfig,
            "Genre1 ;Genre2");
    trackPtr->setGrouping("Grouping");
    trackPtr->setComment("Comment");
    trackPtr->setRating(3);
    const auto tag = mixxx::Tag(
            mixxx::TagLabel("A tag"),
            mixxx::TagScore(0.125));
    trackPtr->replaceCustomTag(taggingConfig, tag);

    const QString collectionUid = "collection1";
    trackPtr->setDateAdded(QDateTime::currentDateTime());

    json::Track jsonTrack = exportTrack(
            json::MediaSourceConfig::forLocalFiles(),
            trackPtr->getFileInfo(),
            trackPtr->getRecord(),
            trackPtr->getCuePoints());

    EXPECT_EQ(1, jsonTrack.allTitles().size());
    EXPECT_EQ(1, jsonTrack.titles().size());
    EXPECT_EQ(trackPtr->getTitle(), jsonTrack.titles().first().name());
    EXPECT_EQ(1, jsonTrack.allActors().size());
    EXPECT_EQ(1, jsonTrack.actors(json::Actor::kRoleArtist).size());
    EXPECT_EQ(trackPtr->getArtist(), jsonTrack.actors(json::Actor::kRoleArtist).first().name());
    EXPECT_EQ(1, jsonTrack.album().allTitles().size());
    EXPECT_EQ(1, jsonTrack.album().titles().size());
    EXPECT_EQ(trackPtr->getAlbum(), jsonTrack.album().titles().first().name());
    EXPECT_EQ(1, jsonTrack.album().allActors().size());
    EXPECT_EQ(1, jsonTrack.album().actors(json::Actor::kRoleArtist).size());
    EXPECT_EQ(
            trackPtr->getAlbumArtist(),
            jsonTrack.album().actors(json::Actor::kRoleArtist).first().name());

    // Tags
    const auto facets = jsonTrack.tags().toFacets().value_or(mixxx::Facets());
    EXPECT_EQ(5, facets.countFacets());
    EXPECT_EQ(2, facets.countTags(mixxx::library::tags::kFacetGenre));
    EXPECT_EQ(
            mixxx::TagLabel("Genre1;Genre2"),
            facets.joinTagsLabel(";", mixxx::library::tags::kFacetGenre));
    EXPECT_EQ(1.0,
            facets.collectTagsOrdered(
                          mixxx::Facets::ScoreOrdering::Descending,
                          mixxx::library::tags::kFacetGenre)
                    .first()
                    .getScore());
    EXPECT_EQ(
            1, facets.countTags(mixxx::library::tags::kFacetComment));
    EXPECT_EQ(trackPtr->getComment(),
            facets.getSingleTagLabel(mixxx::library::tags::kFacetComment));
    EXPECT_EQ(1.0,
            facets.collectTagsOrdered(
                          mixxx::Facets::ScoreOrdering::Descending,
                          mixxx::library::tags::kFacetComment)
                    .first()
                    .getScore());
    EXPECT_EQ(
            1, facets.countTags(mixxx::library::tags::kFacetGrouping));
    EXPECT_EQ(trackPtr->getGrouping(),
            facets.getSingleTagLabel(mixxx::library::tags::kFacetGrouping));
    EXPECT_EQ(1.0,
            facets.collectTagsOrdered(
                          mixxx::Facets::ScoreOrdering::Descending,
                          mixxx::library::tags::kFacetGrouping)
                    .first()
                    .getScore());
    EXPECT_EQ(1,
            facets.countTagsLabeled(
                    mixxx::library::tags::kLabelOrgMixxx,
                    mixxx::library::tags::kFacetRating));
    EXPECT_NEAR(trackPtr->getRating() / 5.0,
            *facets.getTagScore(
                    mixxx::library::tags::kLabelOrgMixxx,
                    mixxx::library::tags::kFacetRating),
            1e-6);
    EXPECT_EQ(1, facets.countTags());
    EXPECT_EQ(tag, facets.collectTags().first());

    jsonTrack.removeTitles(json::Title::kKindMain);
    json::Title trackTitle;
    trackTitle.setName("New Track Title");
    jsonTrack.addTitles({trackTitle});
    EXPECT_EQ(1, jsonTrack.titles().size());
    EXPECT_EQ(json::Title::kKindMain, jsonTrack.titles().first().kind());
    EXPECT_EQ(trackTitle.name(), jsonTrack.titles().first().name());

    jsonTrack.removeActors(json::Actor::kRoleArtist);
    json::Actor trackArtist;
    trackArtist.setRole(json::Actor::kRoleArtist);
    trackArtist.setName("New Track Artist");
    jsonTrack.addActors({trackArtist});
    EXPECT_EQ(1, jsonTrack.actors(json::Actor::kRoleArtist).size());
    EXPECT_EQ(
            json::Actor::kKindSummary,
            jsonTrack.actors(json::Actor::kRoleArtist).first().kind());
    EXPECT_EQ(trackArtist.name(), jsonTrack.actors(json::Actor::kRoleArtist).first().name());

    json::Album album = jsonTrack.album();

    album.removeTitles(json::Title::kKindMain);
    json::Title albumTitle;
    albumTitle.setName("New Album Title");
    album.addTitles({albumTitle});
    EXPECT_EQ(1, album.titles().size());
    EXPECT_EQ(json::Title::kKindMain, album.titles().first().kind());
    EXPECT_EQ(albumTitle.name(), album.titles().first().name());

    album.removeActors(json::Actor::kRoleArtist);
    json::Actor albumArtist;
    albumArtist.setRole(json::Actor::kRoleArtist);
    albumArtist.setName("New Album Artist");
    album.addActors({albumArtist});
    EXPECT_EQ(1, album.actors(json::Actor::kRoleArtist).size());
    EXPECT_EQ(
            json::Actor::kKindSummary,
            album.actors(json::Actor::kRoleArtist).first().kind());
    EXPECT_EQ(albumArtist.name(), album.actors(json::Actor::kRoleArtist).first().name());
}

} // namespace aoide
