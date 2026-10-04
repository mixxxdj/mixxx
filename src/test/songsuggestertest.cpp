#include <gtest/gtest.h>

#include <QDateTime>
#include <QList>
#include <QString>
#include <QVariant>

#include "audio/types.h"
#include "dialog/songsuggesterutils.h"
#include "track/track.h"
#include "util/duration.h"
#if defined(__EXTRA_METADATA__)
#include "track/trackmetadata.h"
#endif // __EXTRA_METADATA__

namespace {

class SongSuggesterTest : public testing::Test {
  protected:
    static QString trackPath(const QString& name) {
        // newDummy requires an absolute file path with a location.
        return QStringLiteral("C:/mixxx-test/") + name
                + QStringLiteral(".mp3");
    }

    static TrackPointer newTrack(
            const QString& filePath,
            TrackId trackId,
            double bpm,
            double replayGainRatio,
            int timesPlayed) {
        auto pTrack = Track::newDummy(trackPath(filePath), trackId);
        // Valid audio properties are required so that setting a BPM
        // can create a constant-tempo beat grid.
        pTrack->setAudioProperties(
                mixxx::audio::ChannelCount(2),
                mixxx::audio::SampleRate(44100),
                mixxx::audio::Bitrate(),
                mixxx::Duration::fromSeconds(180));
        // Bpm values are only accepted within the valid range.
        EXPECT_TRUE(pTrack->trySetBpm(bpm));
        if (replayGainRatio > 0.0) {
            mixxx::ReplayGain replayGain;
            replayGain.setRatio(replayGainRatio);
            pTrack->setReplayGain(replayGain);
        }
        pTrack->resetPlayCounter(timesPlayed);
        return pTrack;
    }

    static TrackPointer newTrack(
            const QString& filePath,
            TrackId trackId,
            double bpm) {
        return newTrack(filePath, trackId, bpm, 1.0, 0);
    }

    static mixxx::SongSuggesterCriteria defaultCriteria() {
        mixxx::SongSuggesterCriteria criteria;
        criteria.targetBpm = 120.0;
        criteria.bpmTolerance = 5.0;
        return criteria;
    }
};

TEST_F(SongSuggesterTest, EnergyLevelForRatio) {
    // A louder master requires negative gain for normalization
    // (ratio < 1.0) and is classified as high energy.
    EXPECT_EQ(mixxx::SongSuggesterEnergyLevel::High,
            mixxx::songSuggesterEnergyLevelForRatio(0.5));
    EXPECT_EQ(mixxx::SongSuggesterEnergyLevel::Medium,
            mixxx::songSuggesterEnergyLevelForRatio(
                    mixxx::kSongSuggesterHighEnergyRatioMax));
    EXPECT_EQ(mixxx::SongSuggesterEnergyLevel::High,
            mixxx::songSuggesterEnergyLevelForRatio(
                    mixxx::kSongSuggesterHighEnergyRatioMax - 0.0001));
    EXPECT_EQ(mixxx::SongSuggesterEnergyLevel::Medium,
            mixxx::songSuggesterEnergyLevelForRatio(
                    mixxx::kSongSuggesterHighEnergyRatioMax + 0.0001));
    EXPECT_EQ(mixxx::SongSuggesterEnergyLevel::Medium,
            mixxx::songSuggesterEnergyLevelForRatio(1.0)); // 0 dB
    EXPECT_EQ(mixxx::SongSuggesterEnergyLevel::Medium,
            mixxx::songSuggesterEnergyLevelForRatio(
                    mixxx::kSongSuggesterLowEnergyRatioMin));
    EXPECT_EQ(mixxx::SongSuggesterEnergyLevel::Low,
            mixxx::songSuggesterEnergyLevelForRatio(
                    mixxx::kSongSuggesterLowEnergyRatioMin + 0.0001));
    EXPECT_EQ(mixxx::SongSuggesterEnergyLevel::Low,
            mixxx::songSuggesterEnergyLevelForRatio(2.0));
}

TEST_F(SongSuggesterTest, FameLevelForPlays) {
    EXPECT_EQ(mixxx::SongSuggesterFameLevel::Underground,
            mixxx::songSuggesterFameLevelForPlays(0));
    EXPECT_EQ(mixxx::SongSuggesterFameLevel::Underground,
            mixxx::songSuggesterFameLevelForPlays(
                    mixxx::kSongSuggesterKnownPlays - 1));
    EXPECT_EQ(mixxx::SongSuggesterFameLevel::Known,
            mixxx::songSuggesterFameLevelForPlays(
                    mixxx::kSongSuggesterKnownPlays));
    EXPECT_EQ(mixxx::SongSuggesterFameLevel::Known,
            mixxx::songSuggesterFameLevelForPlays(49));
    EXPECT_EQ(mixxx::SongSuggesterFameLevel::Popular,
            mixxx::songSuggesterFameLevelForPlays(
                    mixxx::kSongSuggesterPopularPlays));
    EXPECT_EQ(mixxx::SongSuggesterFameLevel::Popular,
            mixxx::songSuggesterFameLevelForPlays(500));
}

TEST_F(SongSuggesterTest, FilterMatchesByBpmTolerance) {
    const auto pReferenceTrack =
            newTrack(QStringLiteral("reference"), TrackId(QVariant(1)), 120.0);
    QList<TrackPointer> candidates;
    // Exactly at the target tempo.
    candidates.append(
            newTrack(QStringLiteral("exact"), TrackId(QVariant(2)), 120.0));
    // At the boundaries of the tolerance (inclusive).
    candidates.append(
            newTrack(QStringLiteral("lower"), TrackId(QVariant(3)), 115.0));
    candidates.append(
            newTrack(QStringLiteral("upper"), TrackId(QVariant(4)), 125.0));
    // Just outside the tolerance.
    candidates.append(
            newTrack(QStringLiteral("below"), TrackId(QVariant(5)), 114.9));
    candidates.append(
            newTrack(QStringLiteral("above"), TrackId(QVariant(6)), 125.1));

    const auto matches = mixxx::filterAndRankSongSuggesterMatches(
            candidates, pReferenceTrack, defaultCriteria());
    ASSERT_EQ(3, matches.size());
    EXPECT_EQ(trackPath(QStringLiteral("exact")), matches.at(0)->getLocation());
    EXPECT_EQ(trackPath(QStringLiteral("lower")), matches.at(1)->getLocation());
    EXPECT_EQ(trackPath(QStringLiteral("upper")), matches.at(2)->getLocation());
}

TEST_F(SongSuggesterTest, FilterExcludesReferenceTrack) {
    const auto pReferenceTrack =
            newTrack(QStringLiteral("reference"), TrackId(QVariant(1)), 120.0);
    QList<TrackPointer> candidates;
    // Same ID as the reference track: must never match itself.
    candidates.append(newTrack(
            QStringLiteral("duplicate"),
            pReferenceTrack->getId(),
            120.0));
    candidates.append(
            newTrack(QStringLiteral("other"), TrackId(QVariant(2)), 120.0));
    // A reference track without a valid ID cannot be excluded by ID.
    const auto pAnonymousReference =
            newTrack(QStringLiteral("anonymous"), TrackId(), 120.0);

    const auto matches = mixxx::filterAndRankSongSuggesterMatches(
            candidates, pReferenceTrack, defaultCriteria());
    ASSERT_EQ(1, matches.size());
    EXPECT_EQ(trackPath(QStringLiteral("other")), matches.at(0)->getLocation());

    const auto anonymousMatches = mixxx::filterAndRankSongSuggesterMatches(
            candidates, pAnonymousReference, defaultCriteria());
    ASSERT_EQ(2, anonymousMatches.size());
}

TEST_F(SongSuggesterTest, FilterMatchesByEnergy) {
    const auto pReferenceTrack =
            newTrack(QStringLiteral("reference"), TrackId(QVariant(1)), 120.0);
    QList<TrackPointer> candidates;
    // No ReplayGain ratio: never matches a specific energy level.
    candidates.append(newTrack(
            QStringLiteral("unanalyzed"), TrackId(QVariant(2)), 120.0, 0.0, 0));
    candidates.append(
            newTrack(QStringLiteral("loud"), TrackId(QVariant(3)), 120.0, 0.7, 0));
    candidates.append(newTrack(
            QStringLiteral("quiet"), TrackId(QVariant(4)), 120.0, 1.3, 0));
    candidates.append(newTrack(
            QStringLiteral("medium"), TrackId(QVariant(5)), 120.0, 1.0, 0));

    mixxx::SongSuggesterCriteria criteria = defaultCriteria();
    criteria.energy = mixxx::SongSuggesterEnergyLevel::High;
    auto matches = mixxx::filterAndRankSongSuggesterMatches(
            candidates, pReferenceTrack, criteria);
    ASSERT_EQ(1, matches.size());
    EXPECT_EQ(trackPath(QStringLiteral("loud")), matches.at(0)->getLocation());

    criteria.energy = mixxx::SongSuggesterEnergyLevel::Medium;
    matches = mixxx::filterAndRankSongSuggesterMatches(
            candidates, pReferenceTrack, criteria);
    ASSERT_EQ(1, matches.size());
    EXPECT_EQ(trackPath(QStringLiteral("medium")), matches.at(0)->getLocation());

    criteria.energy = mixxx::SongSuggesterEnergyLevel::Low;
    matches = mixxx::filterAndRankSongSuggesterMatches(
            candidates, pReferenceTrack, criteria);
    ASSERT_EQ(1, matches.size());
    EXPECT_EQ(trackPath(QStringLiteral("quiet")), matches.at(0)->getLocation());
}

TEST_F(SongSuggesterTest, FilterMatchesByFame) {
    const auto pReferenceTrack =
            newTrack(QStringLiteral("reference"), TrackId(QVariant(1)), 120.0);
    QList<TrackPointer> candidates;
    candidates.append(newTrack(
            QStringLiteral("underground"), TrackId(QVariant(2)), 120.0, 1.0, 4));
    candidates.append(newTrack(
            QStringLiteral("known"), TrackId(QVariant(3)), 120.0, 1.0, 5));
    candidates.append(newTrack(
            QStringLiteral("popular"), TrackId(QVariant(4)), 120.0, 1.0, 50));

    mixxx::SongSuggesterCriteria criteria = defaultCriteria();
    criteria.fame = mixxx::SongSuggesterFameLevel::Underground;
    auto matches = mixxx::filterAndRankSongSuggesterMatches(
            candidates, pReferenceTrack, criteria);
    ASSERT_EQ(1, matches.size());
    EXPECT_EQ(
            trackPath(QStringLiteral("underground")),
            matches.at(0)->getLocation());

    criteria.fame = mixxx::SongSuggesterFameLevel::Known;
    matches = mixxx::filterAndRankSongSuggesterMatches(
            candidates, pReferenceTrack, criteria);
    ASSERT_EQ(1, matches.size());
    EXPECT_EQ(
            trackPath(QStringLiteral("known")),
            matches.at(0)->getLocation());

    criteria.fame = mixxx::SongSuggesterFameLevel::Popular;
    matches = mixxx::filterAndRankSongSuggesterMatches(
            candidates, pReferenceTrack, criteria);
    ASSERT_EQ(1, matches.size());
    EXPECT_EQ(
            trackPath(QStringLiteral("popular")),
            matches.at(0)->getLocation());
}

TEST_F(SongSuggesterTest, RankByBpmDistanceAndPlayCount) {
    const auto pReferenceTrack =
            newTrack(QStringLiteral("reference"), TrackId(QVariant(1)), 120.0);
    QList<TrackPointer> candidates;
    // Same BPM distance: ties are broken by play count (descending).
    candidates.append(newTrack(
            QStringLiteral("tie-low"), TrackId(QVariant(2)), 119.0, 1.0, 3));
    candidates.append(newTrack(
            QStringLiteral("tie-high"), TrackId(QVariant(3)), 121.0, 1.0, 10));
    // Farther away from the target tempo: comes last.
    candidates.append(newTrack(
            QStringLiteral("far"), TrackId(QVariant(4)), 122.0, 1.0, 50));
    // Closest to the target tempo: comes first.
    candidates.append(newTrack(
            QStringLiteral("near"), TrackId(QVariant(5)), 120.5, 1.0, 0));
    // On the other side of the target tempo, same distance as "far".
    candidates.append(newTrack(
            QStringLiteral("far2"), TrackId(QVariant(6)), 118.0, 1.0, 50));

    const auto matches = mixxx::filterAndRankSongSuggesterMatches(
            candidates, pReferenceTrack, defaultCriteria());
    ASSERT_EQ(5, matches.size());
    EXPECT_EQ(trackPath(QStringLiteral("near")), matches.at(0)->getLocation());
    // |119 - 120| == |121 - 120| == 1.0, but "tie-high" has more plays.
    EXPECT_EQ(
            trackPath(QStringLiteral("tie-high")),
            matches.at(1)->getLocation());
    EXPECT_EQ(
            trackPath(QStringLiteral("tie-low")),
            matches.at(2)->getLocation());
    // |122 - 120| == |118 - 120| == 2.0 with equal play counts:
    // the original order is preserved (stable sort).
    EXPECT_EQ(trackPath(QStringLiteral("far")), matches.at(3)->getLocation());
    EXPECT_EQ(trackPath(QStringLiteral("far2")), matches.at(4)->getLocation());
}

TEST_F(SongSuggesterTest, RankIsLimitedToMaxResults) {
    const auto pReferenceTrack =
            newTrack(QStringLiteral("reference"), TrackId(QVariant(1)), 120.0);
    QList<TrackPointer> candidates;
    for (int i = 0; i < mixxx::kSongSuggesterMaxResults + 10; ++i) {
        // Decreasing play counts and increasing BPM distance, so the
        // truncated tail is predictable.
        candidates.append(newTrack(
                QStringLiteral("track%1").arg(i),
                TrackId(QVariant(i + 2)),
                120.0 + 0.01 * i,
                1.0,
                1000 - i));
    }

    const auto matches = mixxx::filterAndRankSongSuggesterMatches(
            candidates, pReferenceTrack, defaultCriteria());
    EXPECT_EQ(mixxx::kSongSuggesterMaxResults, matches.size());
    // The first candidate is closest to the target tempo and must not
    // be dropped.
    EXPECT_EQ(
            trackPath(QStringLiteral("track0")), matches.at(0)->getLocation());
    // The last candidates are farthest away and must have been cut.
    EXPECT_EQ(
            trackPath(QStringLiteral("track99")),
            matches.at(matches.size() - 1)->getLocation());
}

TEST_F(SongSuggesterTest, FilterIgnoresNullCandidates) {
    const auto pReferenceTrack =
            newTrack(QStringLiteral("reference"), TrackId(QVariant(1)), 120.0);
    QList<TrackPointer> candidates;
    candidates.append(TrackPointer());
    candidates.append(
            newTrack(QStringLiteral("valid"), TrackId(QVariant(2)), 120.0));

    const auto matches = mixxx::filterAndRankSongSuggesterMatches(
            candidates, pReferenceTrack, defaultCriteria());
    ASSERT_EQ(1, matches.size());
    EXPECT_EQ(trackPath(QStringLiteral("valid")), matches.at(0)->getLocation());
}

TEST_F(SongSuggesterTest, TrackLanguageWithoutExtraMetadata) {
    const auto pTrack =
            newTrack(QStringLiteral("lang"), TrackId(QVariant(1)), 120.0);
#if defined(__EXTRA_METADATA__)
    mixxx::TrackMetadata trackMetadata;
    trackMetadata.refTrackInfo().setLanguage(QStringLiteral("Deutsch"));
    pTrack->replaceMetadataFromSource(
            trackMetadata,
            QDateTime::currentDateTimeUtc());
    EXPECT_EQ(QStringLiteral("Deutsch"), mixxx::songSuggesterTrackLanguage(pTrack));
#else
    // Without __EXTRA_METADATA__ the language property of TrackInfo is
    // not available and the empty string is reported instead.
    EXPECT_TRUE(mixxx::songSuggesterTrackLanguage(pTrack).isEmpty());
#endif // __EXTRA_METADATA__
    EXPECT_TRUE(mixxx::songSuggesterTrackLanguage(TrackPointer()).isEmpty());
}

} // anonymous namespace
