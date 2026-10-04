#include <gtest/gtest.h>

#include <cmath>

#include <QList>
#include <QString>
#include <QVariant>

#include "audio/types.h"
#include "dialog/autorecommendationengine.h"
#include "track/keyfactory.h"
#include "track/track.h"
#include "util/duration.h"

namespace {

class AutoRecommendationEngineTest : public testing::Test {
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
            int timesPlayed,
            mixxx::track::io::key::ChromaticKey key =
                    mixxx::track::io::key::INVALID,
            double durationSec = 180.0) {
        auto pTrack = Track::newDummy(trackPath(filePath), trackId);
        // Valid audio properties are required so that setting a BPM
        // can create a constant-tempo beat grid. The duration is also
        // read back by the fade/transition sequencing.
        pTrack->setAudioProperties(
                mixxx::audio::ChannelCount(2),
                mixxx::audio::SampleRate(44100),
                mixxx::audio::Bitrate(),
                mixxx::Duration::fromSeconds(durationSec));
        // Bpm values are only accepted within the valid range.
        if (bpm > 0.0) {
            EXPECT_TRUE(pTrack->trySetBpm(bpm));
        }
        if (replayGainRatio > 0.0) {
            mixxx::ReplayGain replayGain;
            replayGain.setRatio(replayGainRatio);
            pTrack->setReplayGain(replayGain);
        }
        pTrack->resetPlayCounter(timesPlayed);
        if (key != mixxx::track::io::key::INVALID) {
            pTrack->setKeys(KeyFactory::makeBasicKeys(
                    key, mixxx::track::io::key::FILE_METADATA));
        }
        return pTrack;
    }

    static mixxx::AutoRecommendationReference referenceFrom(
            const TrackPointer& pTrack) {
        mixxx::AutoRecommendationReference reference;
        if (pTrack) {
            reference.bpm = pTrack->getBpm();
            const auto replayGain = pTrack->getReplayGain();
            reference.replayGainRatio = replayGain.hasRatio()
                    ? replayGain.getRatio()
                    : -1.0;
            reference.timesPlayed = pTrack->getTimesPlayed();
            reference.keyValue = static_cast<int>(pTrack->getKey());
        }
        return reference;
    }
};

TEST_F(AutoRecommendationEngineTest, TempoDistance) {
    mixxx::AutoRecommendationReference reference;
    reference.bpm = 120.0;

    // Unknown or invalid tempos do not produce a distance.
    EXPECT_DOUBLE_EQ(-1.0, mixxx::autoRecommendationTempoDistance(0.0, reference));
    EXPECT_DOUBLE_EQ(-1.0, mixxx::autoRecommendationTempoDistance(120.0, {}));
    reference.bpm = 0.0;
    EXPECT_DOUBLE_EQ(-1.0, mixxx::autoRecommendationTempoDistance(120.0, reference));
    reference.bpm = 120.0;

    // A direct match has no distance.
    EXPECT_DOUBLE_EQ(0.0, mixxx::autoRecommendationTempoDistance(120.0, reference));
    // Half-time and double-time matches are as good as a direct match.
    EXPECT_DOUBLE_EQ(0.0, mixxx::autoRecommendationTempoDistance(60.0, reference));
    EXPECT_DOUBLE_EQ(0.0, mixxx::autoRecommendationTempoDistance(240.0, reference));
    // Relative distance from the closest tempo match.
    EXPECT_NEAR(0.05, mixxx::autoRecommendationTempoDistance(126.0, reference), 1e-9);
    EXPECT_NEAR(0.05, mixxx::autoRecommendationTempoDistance(114.0, reference), 1e-9);
    // A double-time match that is slightly off.
    EXPECT_NEAR(0.025, mixxx::autoRecommendationTempoDistance(123.0, reference), 1e-9);
}

TEST_F(AutoRecommendationEngineTest, LoudnessDistance) {
    mixxx::AutoRecommendationReference reference;
    reference.replayGainRatio = 1.0;

    // Unknown loudness values do not produce a distance.
    EXPECT_DOUBLE_EQ(-1.0, mixxx::autoRecommendationLoudnessDistance(-1.0, reference));
    EXPECT_DOUBLE_EQ(-1.0, mixxx::autoRecommendationLoudnessDistance(0.0, reference));
    reference.replayGainRatio = -1.0;
    EXPECT_DOUBLE_EQ(-1.0, mixxx::autoRecommendationLoudnessDistance(1.0, reference));
    reference.replayGainRatio = 1.0;

    // Identical ratios have no distance.
    EXPECT_DOUBLE_EQ(0.0, mixxx::autoRecommendationLoudnessDistance(1.0, reference));
    // Doubling the ratio means +6 dB.
    EXPECT_NEAR(20.0 * std::log10(2.0),
            mixxx::autoRecommendationLoudnessDistance(2.0, reference),
            1e-9);
}

TEST_F(AutoRecommendationEngineTest, FameDistance) {
    mixxx::AutoRecommendationReference reference;
    reference.timesPlayed = 0;

    // Identical play counts have no distance, including zero plays.
    EXPECT_DOUBLE_EQ(0.0, mixxx::autoRecommendationFameDistance(0, reference));
    // log10 of the ratio of the play counts, offset by 1.
    reference.timesPlayed = 9;
    EXPECT_NEAR(1.0, mixxx::autoRecommendationFameDistance(99, reference), 1e-9);
    EXPECT_NEAR(1.0, mixxx::autoRecommendationFameDistance(0, reference), 1e-9);
}

TEST_F(AutoRecommendationEngineTest, ScorePerfectMatch) {
    const auto pReference = newTrack(
            QStringLiteral("ref"), TrackId(QVariant(1)), 120.0, 1.0, 3);
    const auto pCandidate = newTrack(
            QStringLiteral("cand"), TrackId(QVariant(2)), 120.0, 1.0, 3);
    const auto reference = referenceFrom(pReference);

    EXPECT_DOUBLE_EQ(
            1.0, mixxx::autoRecommendationScore(pCandidate, reference, {}));
    // Null candidates score 0.0.
    EXPECT_DOUBLE_EQ(
            0.0,
            mixxx::autoRecommendationScore(TrackPointer(), reference, {}));
}

TEST_F(AutoRecommendationEngineTest, ScoreSkipsUnknownValues) {
    const auto pReference = newTrack(
            QStringLiteral("ref"), TrackId(QVariant(1)), 120.0, 1.0, 0);
    // The candidate has a tempo but no ReplayGain ratio and no key.
    const auto pCandidate = newTrack(
            QStringLiteral("cand"), TrackId(QVariant(2)), 120.0, 0.0, 0);
    const auto reference = referenceFrom(pReference);

    mixxx::AutoRecommendationWeights weights;
    weights.tempo = 1.0;
    weights.energy = 1.0;
    weights.key = 1.0;
    weights.fame = 1.0;
    // Only the tempo and fame criteria are known and both are a
    // perfect match; the unknown criteria are skipped.
    EXPECT_DOUBLE_EQ(
            1.0, mixxx::autoRecommendationScore(pCandidate, reference, weights));

    // With all weights disabled the score is 0.0.
    weights.tempo = 0.0;
    weights.energy = 0.0;
    weights.key = 0.0;
    weights.fame = 0.0;
    EXPECT_DOUBLE_EQ(
            0.0, mixxx::autoRecommendationScore(pCandidate, reference, weights));
}

TEST_F(AutoRecommendationEngineTest, ScorePrefersSameKey) {
    const auto pReference = newTrack(
            QStringLiteral("ref"),
            TrackId(QVariant(1)),
            120.0,
            1.0,
            0,
            mixxx::track::io::key::C_MAJOR);
    const auto reference = referenceFrom(pReference);

    // Both candidates match perfectly except for their key.
    const auto pSameKey = newTrack(
            QStringLiteral("same"),
            TrackId(QVariant(2)),
            120.0,
            1.0,
            0,
            mixxx::track::io::key::C_MAJOR);
    const auto pOtherKey = newTrack(
            QStringLiteral("other"),
            TrackId(QVariant(3)),
            120.0,
            1.0,
            0,
            mixxx::track::io::key::F_SHARP_MAJOR);

    // Sanity checks: all three tracks must expose a valid key, and the
    // reference values must carry it, otherwise the key criterion is
    // silently skipped for every candidate.
    ASSERT_GT(reference.keyValue, 0);
    ASSERT_NE(mixxx::track::io::key::INVALID, pSameKey->getKey());
    ASSERT_NE(mixxx::track::io::key::INVALID, pOtherKey->getKey());

    mixxx::AutoRecommendationWeights weights;
    EXPECT_DOUBLE_EQ(
            1.0, mixxx::autoRecommendationScore(pSameKey, reference, weights));
    EXPECT_LT(
            mixxx::autoRecommendationScore(pOtherKey, reference, weights),
            1.0);
}

TEST_F(AutoRecommendationEngineTest, SelectOrdersByScore) {
    const auto pReference = newTrack(
            QStringLiteral("ref"), TrackId(QVariant(1)), 120.0, 1.0, 0);
    QList<TrackPointer> candidates;
    // Same tempo and loudness as the reference, differing fame
    // distance: fewer plays score higher.
    candidates.append(newTrack(
            QStringLiteral("fame0"), TrackId(QVariant(2)), 120.0, 1.0, 0));
    candidates.append(newTrack(
            QStringLiteral("fame9"), TrackId(QVariant(3)), 120.0, 1.0, 9));
    candidates.append(newTrack(
            QStringLiteral("fame99"), TrackId(QVariant(4)), 120.0, 1.0, 99));
    // Farther away from the reference tempo: scores lowest.
    candidates.append(newTrack(
            QStringLiteral("slow"), TrackId(QVariant(5)), 90.0, 1.0, 0));

    const auto selected = mixxx::autoRecommendationSelect(
            candidates, pReference, {}, {}, 10);
    ASSERT_EQ(4, selected.size());
    EXPECT_EQ(TrackId(QVariant(2)), selected.at(0));
    EXPECT_EQ(TrackId(QVariant(3)), selected.at(1));
    EXPECT_EQ(TrackId(QVariant(4)), selected.at(2));
    EXPECT_EQ(TrackId(QVariant(5)), selected.at(3));
}

TEST_F(AutoRecommendationEngineTest, SelectExcludesAndLimits) {
    const auto pReference = newTrack(
            QStringLiteral("ref"), TrackId(QVariant(1)), 120.0, 1.0, 0);
    QList<TrackPointer> candidates;
    // A copy of the reference track (same ID) must never be selected.
    candidates.append(pReference);
    // Null and ID-less candidates are skipped.
    candidates.append(TrackPointer());
    candidates.append(newTrack(
            QStringLiteral("noid"), TrackId(), 120.0, 1.0, 0));
    // A track that is already queued.
    const auto pQueued = newTrack(
            QStringLiteral("queued"), TrackId(QVariant(2)), 120.0, 1.0, 0);
    candidates.append(pQueued);
    candidates.append(newTrack(
            QStringLiteral("new1"), TrackId(QVariant(3)), 120.0, 1.0, 0));
    candidates.append(newTrack(
            QStringLiteral("new2"), TrackId(QVariant(4)), 120.0, 1.0, 0));

    QSet<TrackId> excluded;
    excluded.insert(pQueued->getId());

    // A limit of 1 only returns the best match.
    const auto limited = mixxx::autoRecommendationSelect(
            candidates, pReference, excluded, {}, 1);
    ASSERT_EQ(1, limited.size());
    EXPECT_EQ(TrackId(QVariant(3)), limited.at(0));

    const auto selected = mixxx::autoRecommendationSelect(
            candidates, pReference, excluded, {}, 10);
    ASSERT_EQ(2, selected.size());
    EXPECT_EQ(TrackId(QVariant(3)), selected.at(0));
    EXPECT_EQ(TrackId(QVariant(4)), selected.at(1));

    // A non-positive limit selects nothing.
    EXPECT_TRUE(mixxx::autoRecommendationSelect(
            candidates, pReference, excluded, {}, 0)
                        .isEmpty());
}

TEST_F(AutoRecommendationEngineTest, TransitionTempoDistance) {
    // Unknown tempos do not produce a distance.
    EXPECT_DOUBLE_EQ(-1.0,
            mixxx::autoRecommendationTransitionTempoDistance(0.0, 120.0));
    EXPECT_DOUBLE_EQ(-1.0,
            mixxx::autoRecommendationTransitionTempoDistance(120.0, 0.0));
    // Direct, half-time and double-time matches are perfect
    // transitions.
    EXPECT_DOUBLE_EQ(0.0,
            mixxx::autoRecommendationTransitionTempoDistance(120.0, 120.0));
    EXPECT_DOUBLE_EQ(0.0,
            mixxx::autoRecommendationTransitionTempoDistance(120.0, 60.0));
    EXPECT_DOUBLE_EQ(0.0,
            mixxx::autoRecommendationTransitionTempoDistance(120.0, 240.0));
    // Relative distance from the closest tempo match.
    EXPECT_NEAR(0.05,
            mixxx::autoRecommendationTransitionTempoDistance(120.0, 126.0),
            1e-9);
}

TEST_F(AutoRecommendationEngineTest, FadeScore) {
    // Unknown durations do not produce a score.
    EXPECT_DOUBLE_EQ(-1.0, mixxx::autoRecommendationFadeScore(0.0));
    EXPECT_DOUBLE_EQ(-1.0, mixxx::autoRecommendationFadeScore(-30.0));
    // Tracks at or above the minimum fade window score fully.
    EXPECT_DOUBLE_EQ(1.0, mixxx::autoRecommendationFadeScore(60.0));
    EXPECT_DOUBLE_EQ(1.0, mixxx::autoRecommendationFadeScore(600.0));
    // Shorter tracks score proportionally, so they are sequenced
    // towards the end of the queue.
    EXPECT_NEAR(0.5, mixxx::autoRecommendationFadeScore(30.0), 1e-9);
    EXPECT_NEAR(1.0 / 3.0, mixxx::autoRecommendationFadeScore(20.0), 1e-9);
}

TEST_F(AutoRecommendationEngineTest, TransitionScore) {
    // A perfect tempo match with a usable fade window is a perfect
    // transition, including half-time.
    EXPECT_DOUBLE_EQ(
            1.0, mixxx::autoRecommendationTransitionScore(120.0, 120.0, 180.0));
    EXPECT_DOUBLE_EQ(
            1.0, mixxx::autoRecommendationTransitionScore(120.0, 60.0, 180.0));
    // 132 vs 120 BPM is 10% off: the tempo criterion scores 0 and only
    // the fade window remains.
    EXPECT_NEAR(
            0.5, mixxx::autoRecommendationTransitionScore(120.0, 132.0, 180.0),
            1e-9);
    // Unknown tempos fall back to the duration criterion.
    EXPECT_DOUBLE_EQ(
            1.0, mixxx::autoRecommendationTransitionScore(0.0, 120.0, 180.0));
    // If nothing is known the transition is unknown.
    EXPECT_DOUBLE_EQ(
            -1.0, mixxx::autoRecommendationTransitionScore(0.0, 0.0, 0.0));
}

TEST_F(AutoRecommendationEngineTest, SelectTransitionAwareReordersByTempo) {
    const auto pReference = newTrack(
            QStringLiteral("ref"), TrackId(QVariant(1)), 120.0, 1.0, 0);
    // Perfect tempo match to the reference, but a fame distance.
    const auto pOnSync = newTrack(
            QStringLiteral("onsync"), TrackId(QVariant(2)), 120.0, 1.0, 99);
    // 6% off the reference tempo (fame is a perfect match), so it
    // scores higher on relevance alone.
    const auto pOffTempo = newTrack(
            QStringLiteral("offtempo"), TrackId(QVariant(3)), 126.0, 1.0, 0);
    const QList<TrackPointer> candidates = {pOnSync, pOffTempo};

    // Score-only mode queues the better-scoring off-tempo track first.
    const auto byScore = mixxx::autoRecommendationSelect(
            candidates, pReference, {}, {}, 10);
    ASSERT_EQ(2, byScore.size());
    EXPECT_EQ(pOffTempo->getId(), byScore.at(0));
    EXPECT_EQ(pOnSync->getId(), byScore.at(1));

    // Fade/transition awareness keeps the same set but sequences the
    // tempo-compatible track first for a smooth fade.
    const auto byTransition = mixxx::autoRecommendationSelect(
            candidates, pReference, {}, {}, 10, true);
    ASSERT_EQ(2, byTransition.size());
    EXPECT_EQ(pOnSync->getId(), byTransition.at(0));
    EXPECT_EQ(pOffTempo->getId(), byTransition.at(1));
}

TEST_F(AutoRecommendationEngineTest, SelectTransitionAwareReordersByDuration) {
    const auto pReference = newTrack(
            QStringLiteral("ref"), TrackId(QVariant(1)), 120.0, 1.0, 0);
    // A perfect match, but far too short for a proper fade.
    const auto pStinger = newTrack(
            QStringLiteral("stinger"), TrackId(QVariant(2)), 120.0, 1.0, 0,
            mixxx::track::io::key::INVALID,
            10.0);
    // A weaker match (fame distance) with a full-length fade window.
    const auto pFull = newTrack(
            QStringLiteral("full"), TrackId(QVariant(3)), 120.0, 1.0, 9);
    const QList<TrackPointer> candidates = {pStinger, pFull};

    // Score-only mode queues the perfect-scoring stinger first.
    const auto byScore = mixxx::autoRecommendationSelect(
            candidates, pReference, {}, {}, 10);
    ASSERT_EQ(2, byScore.size());
    EXPECT_EQ(pStinger->getId(), byScore.at(0));
    EXPECT_EQ(pFull->getId(), byScore.at(1));

    // Fade/transition awareness sequences the full-length track first
    // so the transition has a usable fade window.
    const auto byTransition = mixxx::autoRecommendationSelect(
            candidates, pReference, {}, {}, 10, true);
    ASSERT_EQ(2, byTransition.size());
    EXPECT_EQ(pFull->getId(), byTransition.at(0));
    EXPECT_EQ(pStinger->getId(), byTransition.at(1));
}

TEST_F(AutoRecommendationEngineTest,
        SelectTransitionAwareKeepsScoreOrderWithoutTempo) {
    const auto pReference = newTrack(
            QStringLiteral("ref"), TrackId(QVariant(1)), 120.0, 1.0, 0);
    // Candidates without a BPM: the tempo criterion is unknown, and
    // with equal durations the transition quality is identical for
    // all candidates, so the score order is preserved.
    const auto pNoBpmA = newTrack(
            QStringLiteral("nobpm-a"), TrackId(QVariant(2)), 0.0, 1.0, 0);
    const auto pNoBpmB = newTrack(
            QStringLiteral("nobpm-b"), TrackId(QVariant(3)), 0.0, 1.0, 9);
    const QList<TrackPointer> candidates = {pNoBpmB, pNoBpmA};

    const auto byScore = mixxx::autoRecommendationSelect(
            candidates, pReference, {}, {}, 10);
    const auto byTransition = mixxx::autoRecommendationSelect(
            candidates, pReference, {}, {}, 10, true);
    ASSERT_EQ(2, byScore.size());
    ASSERT_EQ(2, byTransition.size());
    EXPECT_EQ(pNoBpmA->getId(), byScore.at(0));
    EXPECT_EQ(pNoBpmB->getId(), byScore.at(1));
    EXPECT_EQ(byScore, byTransition);
}

} // anonymous namespace
