#include <gtest/gtest.h>

#include <QElapsedTimer>
#include <QSet>
#include <QVariant>
#include <algorithm>
#include <limits>
#include <random>

#include "library/autodj/smart/bridgefinder.h"
#include "library/autodj/smart/mixscorer.h"
#include "library/autodj/smart/smartsequencer.h"
#include "library/autodj/smart/trackfeatures.h"

namespace {

TrackFeatures makeTrack(int id, int camelot, bool minor, double bpm, double energy = 0.0) {
    TrackFeatures f;
    f.energyIsManual = energy > 0.0; // tests rate energy by hand unless stated
    f.id = TrackId(QVariant(id));
    f.camelotNumber = camelot;
    f.camelotMinor = minor;
    f.bpm = bpm;
    f.energy = energy;
    return f;
}

double orderCost(const MixScorer& scorer,
        const QVector<TrackFeatures>& tracks,
        const QList<TrackId>& order) {
    auto find = [&tracks](const TrackId& id) -> const TrackFeatures& {
        for (const auto& t : tracks) {
            if (t.id == id) {
                return t;
            }
        }
        return tracks.front();
    };
    double sum = scorer.startCost(find(order.front())) + scorer.endCost(find(order.back()));
    for (int k = 1; k < order.size(); ++k) {
        sum += scorer.score(find(order[k - 1]), find(order[k])).total;
    }
    return sum;
}

// Exact answer by trying every order. Only for tiny inputs.
double bruteForceBest(const MixScorer& scorer, QVector<TrackFeatures> tracks) {
    std::sort(tracks.begin(), tracks.end(), [](const auto& a, const auto& b) {
        return a.id.toString().toInt() < b.id.toString().toInt();
    });
    double best = std::numeric_limits<double>::max();
    do {
        double sum = scorer.startCost(tracks.front()) + scorer.endCost(tracks.back());
        for (int k = 1; k < tracks.size(); ++k) {
            sum += scorer.score(tracks[k - 1], tracks[k]).total;
        }
        best = std::min(best, sum);
    } while (std::next_permutation(tracks.begin(), tracks.end(), [](const auto& a, const auto& b) {
        return a.id.toString().toInt() < b.id.toString().toInt();
    }));
    return best;
}

} // namespace

// ---------- TrackFeatures ----------

TEST(TrackFeaturesTest, OpenKeyToCamelot) {
    EXPECT_EQ(8, TrackFeatures::camelotFromOpenKey(1));  // C major / A minor
    EXPECT_EQ(9, TrackFeatures::camelotFromOpenKey(2));  // G major / E minor
    EXPECT_EQ(1, TrackFeatures::camelotFromOpenKey(6));  // B major / G# minor
    EXPECT_EQ(7, TrackFeatures::camelotFromOpenKey(12)); // F major / D minor
    EXPECT_EQ(0, TrackFeatures::camelotFromOpenKey(0));
    EXPECT_EQ(0, TrackFeatures::camelotFromOpenKey(13));
}

TEST(TrackFeaturesTest, CamelotText) {
    EXPECT_EQ(QStringLiteral("8A"), makeTrack(1, 8, true, 120).camelotText());
    EXPECT_EQ(QStringLiteral("12B"), makeTrack(1, 12, false, 120).camelotText());
    EXPECT_EQ(QStringLiteral("?"), makeTrack(1, 0, false, 120).camelotText());
}

// ---------- MixScorer: keys ----------

TEST(MixScorerTest, CamelotDistanceWrapsRoundTheWheel) {
    EXPECT_EQ(0, MixScorer::camelotDistance(8, 8));
    EXPECT_EQ(1, MixScorer::camelotDistance(12, 1));
    EXPECT_EQ(6, MixScorer::camelotDistance(1, 7));
    EXPECT_EQ(2, MixScorer::camelotStep(8, 10));
    EXPECT_EQ(-5, MixScorer::camelotStep(8, 3)); // +7
    EXPECT_EQ(-1, MixScorer::camelotStep(1, 12));
}

TEST(MixScorerTest, CamelotRulesAreOrderedAsDjsExpect) {
    const auto a8 = makeTrack(1, 8, true, 124);
    const double same = MixScorer::camelotCost(a8, makeTrack(2, 8, true, 124), 0, true);
    const double relative = MixScorer::camelotCost(a8, makeTrack(2, 8, false, 124), 0, true);
    const double adjacentUp = MixScorer::camelotCost(a8, makeTrack(2, 9, true, 124), 0, true);
    const double adjacentDown = MixScorer::camelotCost(a8, makeTrack(2, 7, true, 124), 0, true);
    const double diagonal = MixScorer::camelotCost(a8, makeTrack(2, 9, false, 124), 0, true);
    const double twoSteps = MixScorer::camelotCost(a8, makeTrack(2, 10, true, 124), 0, true);
    const double tritone = MixScorer::camelotCost(a8, makeTrack(2, 2, true, 124), 0, true);

    EXPECT_DOUBLE_EQ(0.0, same);
    EXPECT_LT(same, relative);
    EXPECT_LT(relative, adjacentUp);
    EXPECT_DOUBLE_EQ(adjacentUp, adjacentDown);
    EXPECT_LT(adjacentUp, diagonal);
    EXPECT_LT(diagonal, twoSteps);
    EXPECT_LT(twoSteps, tritone);
    EXPECT_GE(twoSteps, MixScorer::kClashKeyCost);
}

TEST(MixScorerTest, EnergyBoostOnlyExcusedWhenEnergyRises) {
    const auto a8 = makeTrack(1, 8, true, 124);
    const auto a10 = makeTrack(2, 10, true, 124); // +2 steps
    const auto a3 = makeTrack(3, 3, true, 124);   // +7 steps
    EXPECT_DOUBLE_EQ(2.0, MixScorer::camelotCost(a8, a10, 1.5, true));
    EXPECT_DOUBLE_EQ(2.0, MixScorer::camelotCost(a8, a3, 1.0, true));
    EXPECT_GE(MixScorer::camelotCost(a8, a10, 0.0, true), MixScorer::kClashKeyCost);
    EXPECT_GE(MixScorer::camelotCost(a8, a10, 2.0, false), MixScorer::kClashKeyCost);
    // Going DOWN two steps is not a boost move.
    EXPECT_GE(MixScorer::camelotCost(a10, a8, 2.0, true), MixScorer::kClashKeyCost);
}

TEST(MixScorerTest, UnknownKeyIsMildPenalty) {
    const double c = MixScorer::camelotCost(makeTrack(1, 0, true, 124),
            makeTrack(2, 8, true, 124),
            0,
            true);
    EXPECT_GT(c, 1.0);
    EXPECT_LT(c, MixScorer::kClashKeyCost);
}

// ---------- MixScorer: tempo ----------

TEST(MixScorerTest, TempoCost) {
    EXPECT_DOUBLE_EQ(0.0, MixScorer::tempoCost(124, 124, 5, true));
    EXPECT_NEAR(1.0, MixScorer::tempoCost(100, 105, 5, true), 0.03);
    EXPECT_LT(MixScorer::tempoCost(124, 126, 5, true), 1.0);
    EXPECT_NEAR(0.0, MixScorer::tempoCost(87, 174, 5, true), 1e-9);  // double time
    EXPECT_GE(MixScorer::tempoCost(87, 174, 5, false), MixScorer::kClashTempoCost);
    EXPECT_GE(MixScorer::tempoCost(100, 120, 5, true), MixScorer::kClashTempoCost);
    EXPECT_DOUBLE_EQ(2.0, MixScorer::tempoCost(0, 124, 5, true)); // unknown
}

// ---------- MixScorer: energy ----------

TEST(MixScorerTest, TempoCostIsSymmetricAndSmooth) {
    // Speeding up costs the same as slowing down by the same ratio.
    EXPECT_NEAR(MixScorer::tempoCost(118, 128, 5, true),
            MixScorer::tempoCost(128, 118, 5, true),
            1e-9);
    EXPECT_NEAR(MixScorer::tempoCost(135, 149.4, 5, true),
            MixScorer::tempoCost(149.4, 135, 5, true),
            1e-9);
    // Just inside the tolerance mixes; just past it is a clash.
    EXPECT_LT(MixScorer::tempoCost(100, 100 * std::exp(0.049), 5, true), 1.0);
    EXPECT_GE(MixScorer::tempoCost(100, 100 * std::exp(0.051), 5, true),
            MixScorer::kClashTempoCost);
    // The Promise (118.1) -> Rock The Casbah (129.5): about 9%, a clash.
    EXPECT_GE(MixScorer::tempoCost(118.1, 129.5, 5, true), MixScorer::kClashTempoCost);
}

TEST(MixScorerTest, EnergyBuildPenalisesDrops) {
    MixScoreWeights w;
    w.direction = MixScoreWeights::EnergyDirection::Build;
    MixScorer scorer(w);
    EXPECT_DOUBLE_EQ(0.0, scorer.energyCost(5, 6));
    EXPECT_GT(scorer.energyCost(6, 5), 0.0);
    EXPECT_GT(scorer.energyCost(3, 8), 0.0); // too big a jump
}

TEST(MixScorerTest, EnergyWaveAllowsSmallDips) {
    MixScoreWeights w;
    w.direction = MixScoreWeights::EnergyDirection::Wave;
    MixScorer scorer(w);
    EXPECT_DOUBLE_EQ(0.0, scorer.energyCost(6, 5));
    EXPECT_GT(scorer.energyCost(8, 4), 0.0);
}

TEST(MixScorerTest, UnknownEnergyShowsQuestionMark) {
    MixScorer scorer;
    const MixScore s = scorer.score(makeTrack(1, 8, true, 124), makeTrack(2, 9, true, 125));
    EXPECT_TRUE(s.reason.contains(QStringLiteral("energy ?"))) << s.reason.toStdString();
}

TEST(MixScorerTest, ClashLabel) {
    MixScorer scorer;
    EXPECT_TRUE(MixScorer::clashLabel(
            scorer.score(makeTrack(1, 8, true, 124), makeTrack(2, 9, true, 124)))
                        .isEmpty());
    EXPECT_EQ(QStringLiteral("key clash"),
            MixScorer::clashLabel(
                    scorer.score(makeTrack(1, 8, true, 124), makeTrack(2, 2, true, 124))));
    EXPECT_EQ(QStringLiteral("tempo clash"),
            MixScorer::clashLabel(
                    scorer.score(makeTrack(1, 8, true, 100), makeTrack(2, 8, true, 125))));
    EXPECT_EQ(QStringLiteral("key + tempo clash"),
            MixScorer::clashLabel(
                    scorer.score(makeTrack(1, 8, true, 100), makeTrack(2, 2, true, 125))));
}

TEST(MixScorerTest, MeasuredEnergyCountsLessThanRated) {
    MixScorer scorer;
    TrackFeatures a = makeTrack(1, 8, true, 124, 8);
    TrackFeatures b = makeTrack(2, 8, true, 124, 4); // big energy drop
    const double rated = scorer.score(a, b).energyCost;
    a.energyIsManual = false;
    const double measured = scorer.score(a, b).energyCost;
    EXPECT_GT(rated, 0.0);
    EXPECT_NEAR(measured, rated * scorer.weights().measuredEnergyTrust, 1e-9);
    EXPECT_TRUE(scorer.score(a, b).reason.contains(QStringLiteral("(measured)")));
}

TEST(MixScorerTest, OnlyRatedEnergyExcusesKeyJump) {
    MixScorer scorer;
    TrackFeatures a = makeTrack(1, 8, true, 124, 5);
    TrackFeatures b = makeTrack(2, 10, true, 124, 7); // +2 steps, energy +2
    EXPECT_LT(scorer.score(a, b).keyCost, MixScorer::kClashKeyCost);
    b.energyIsManual = false;
    EXPECT_GE(scorer.score(a, b).keyCost, MixScorer::kClashKeyCost);
}

TEST(MixScorerTest, ShiftKeyMovesSevenStepsPerSemitone) {
    const TrackFeatures t = makeTrack(1, 8, true, 120);
    EXPECT_EQ(3, MixScorer::shiftKey(t, 1).camelotNumber);  // 8A up = 3A
    EXPECT_EQ(1, MixScorer::shiftKey(t, -1).camelotNumber); // 8A down = 1A
    EXPECT_EQ(10, MixScorer::shiftKey(t, 2).camelotNumber); // 2 up = +2 steps
    EXPECT_EQ(8, MixScorer::shiftKey(t, 12).camelotNumber); // an octave
    EXPECT_TRUE(MixScorer::shiftKey(t, 1).camelotMinor);    // letter stays
    EXPECT_EQ(0, MixScorer::shiftKey(makeTrack(2, 0, true, 120), 1).camelotNumber);
}

TEST(MixScorerTest, KeyMorphOnlyWhenTheKeysClash) {
    const TrackFeatures from = makeTrack(1, 8, true, 120);
    // Already fits: same key, neighbour, relative.
    EXPECT_EQ(0, MixScorer::keyMorphSemitones(from, makeTrack(2, 8, true, 120), 1));
    EXPECT_EQ(0, MixScorer::keyMorphSemitones(from, makeTrack(2, 9, true, 120), 1));
    EXPECT_EQ(0, MixScorer::keyMorphSemitones(from, makeTrack(2, 8, false, 120), 1));
    // Unknown key: leave it.
    EXPECT_EQ(0, MixScorer::keyMorphSemitones(from, makeTrack(2, 0, true, 120), 1));
    EXPECT_EQ(0, MixScorer::keyMorphSemitones(makeTrack(1, 0, true, 120), from, 1));
}

TEST(MixScorerTest, KeyMorphFindsTheSmallestShift) {
    const TrackFeatures from = makeTrack(1, 8, true, 120);
    // 3A is one semitone above 8A: pitch it down one.
    EXPECT_EQ(-1, MixScorer::keyMorphSemitones(from, makeTrack(2, 3, true, 120), 1));
    // 1A is one semitone below 8A: pitch it up one.
    EXPECT_EQ(1, MixScorer::keyMorphSemitones(from, makeTrack(2, 1, true, 120), 1));
    // Every result really fits.
    for (int camelot = 1; camelot <= 12; ++camelot) {
        for (bool minor : {true, false}) {
            const TrackFeatures to = makeTrack(2, camelot, minor, 120);
            const int shift = MixScorer::keyMorphSemitones(from, to, 2);
            if (shift != 0) {
                EXPECT_LE(std::abs(shift), 2);
                EXPECT_LT(MixScorer::camelotCost(from, MixScorer::shiftKey(to, shift), 0, false),
                        MixScorer::kClashKeyCost)
                        << to.camelotText().toStdString();
            }
        }
    }
    // 10A (two steps) needs two semitones: not with a limit of one.
    EXPECT_EQ(0, MixScorer::keyMorphSemitones(from, makeTrack(2, 10, true, 120), 1));
    EXPECT_EQ(-2, MixScorer::keyMorphSemitones(from, makeTrack(2, 10, true, 120), 2));
}

TEST(MixScorerTest, ScoreHasReadableReason) {
    MixScorer scorer;
    const MixScore s = scorer.score(makeTrack(1, 8, true, 124, 5), makeTrack(2, 9, true, 125, 6));
    EXPECT_TRUE(s.reason.contains(QStringLiteral("8A -> 9A")));
    EXPECT_TRUE(s.reason.contains(QStringLiteral("energy +1.0 (rated)"))) << s.reason.toStdString();
    EXPECT_GT(s.total, 0.0);
}

// ---------- SmartSequencer ----------

TEST(SmartSequencerTest, EmptyAndSingle) {
    SmartSequencer seq{MixScorer()};
    EXPECT_TRUE(seq.solve({}).order.isEmpty());
    const auto r = seq.solve({makeTrack(1, 8, true, 124)});
    ASSERT_EQ(1, r.order.size());
    EXPECT_EQ(0, r.clashCount);
}

TEST(SmartSequencerTest, SortsAHarmonicChainWithoutClashes) {
    // 8A..12A shuffled. The only clash-free orders walk the wheel in steps.
    QVector<TrackFeatures> tracks = {
            makeTrack(1, 11, true, 124),
            makeTrack(2, 8, true, 124),
            makeTrack(3, 12, true, 124),
            makeTrack(4, 10, true, 124),
            makeTrack(5, 9, true, 124),
    };
    SmartSequencer seq{MixScorer()};
    const auto r = seq.solve(tracks);
    ASSERT_EQ(5, r.order.size());
    EXPECT_EQ(0, r.clashCount) << r.warnings.join('\n').toStdString();
}

TEST(SmartSequencerTest, KeepsFixedFirstTrack) {
    QVector<TrackFeatures> tracks = {
            makeTrack(1, 8, true, 124),
            makeTrack(2, 3, true, 124),
            makeTrack(3, 9, true, 124),
            makeTrack(4, 4, true, 124),
    };
    SmartSequencer seq{MixScorer()};
    const auto r = seq.solve(tracks, TrackId(QVariant(2)));
    ASSERT_EQ(4, r.order.size());
    EXPECT_EQ(TrackId(QVariant(2)), r.order.front());
}

TEST(SmartSequencerTest, ReportsClashesItCannotAvoid) {
    // 8A and 2A are opposite on the wheel: no smooth path exists.
    QVector<TrackFeatures> tracks = {
            makeTrack(1, 8, true, 124),
            makeTrack(2, 2, true, 124),
    };
    SmartSequencer seq{MixScorer()};
    const auto r = seq.solve(tracks);
    EXPECT_EQ(1, r.clashCount);
    ASSERT_EQ(1, r.warnings.size());
    EXPECT_TRUE(r.warnings[0].contains(QStringLiteral("key clash")));
}

TEST(SmartSequencerTest, BuildStartsCalmAndEndsAtPeak) {
    // Same key and tempo, so only energy decides the order.
    QVector<TrackFeatures> tracks = {
            makeTrack(1, 8, true, 124, 9),
            makeTrack(2, 8, true, 124, 3),
            makeTrack(3, 8, true, 124, 6),
            makeTrack(4, 8, true, 124, 7),
    };
    SmartSequencer seq{MixScorer()};
    const auto r = seq.solve(tracks);
    ASSERT_EQ(4, r.order.size());
    EXPECT_EQ(TrackId(QVariant(2)), r.order.front()); // energy 3 first
    EXPECT_EQ(TrackId(QVariant(1)), r.order.back());  // energy 9 last
}

TEST(SmartSequencerTest, HeuristicAlsoBuildsEnergy) {
    // Above kExactLimit: 20 tracks, same key/tempo, energies 1..10 twice.
    QVector<TrackFeatures> tracks;
    std::mt19937 rng(11);
    for (int i = 1; i <= 20; ++i) {
        tracks.push_back(makeTrack(i, 8, true, 124, 1 + (i % 10)));
    }
    std::shuffle(tracks.begin(), tracks.end(), rng);
    SmartSequencer seq{MixScorer()};
    const auto r = seq.solve(tracks);
    ASSERT_EQ(20, r.order.size());
    auto energyOf = [&](const TrackId& id) {
        for (const auto& t : tracks) {
            if (t.id == id) {
                return t.energy;
            }
        }
        return 0.0;
    };
    for (int k = 1; k < r.order.size(); ++k) {
        EXPECT_LE(energyOf(r.order[k - 1]), energyOf(r.order[k])) << "at " << k;
    }
}

TEST(SmartSequencerTest, OrderLinesListEveryTrackAndClash) {
    QVector<TrackFeatures> tracks = {
            makeTrack(1, 8, true, 124),
            makeTrack(2, 2, true, 124), // clash with 8A either way
    };
    tracks[0].displayName = QStringLiteral("Artist - One");
    SmartSequencer seq{MixScorer()};
    const auto r = seq.solve(tracks);
    ASSERT_EQ(3, r.orderLines.size()); // track, clash note, track
    EXPECT_TRUE(r.orderLines[1].contains(QStringLiteral("key clash")));
    EXPECT_TRUE(r.orderLines.join('\n').contains(QStringLiteral("Artist - One")));
}

TEST(SmartSequencerTest, ExactSolverMatchesBruteForce) {
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> key(1, 12);
    std::uniform_int_distribution<int> letter(0, 1);
    std::uniform_real_distribution<double> bpm(118.0, 132.0);
    std::uniform_real_distribution<double> energy(1.0, 10.0);
    MixScorer scorer;
    SmartSequencer seq(scorer);
    for (int round = 0; round < 20; ++round) {
        QVector<TrackFeatures> tracks;
        for (int i = 1; i <= 7; ++i) {
            tracks.push_back(makeTrack(i, key(rng), letter(rng) == 1, bpm(rng), energy(rng)));
        }
        const double best = bruteForceBest(scorer, tracks);
        const auto r = seq.solve(tracks);
        ASSERT_EQ(7, r.order.size());
        EXPECT_NEAR(best, orderCost(scorer, tracks, r.order), 1e-6);
        EXPECT_NEAR(best, r.totalCost, 1e-6);
    }
}

TEST(SmartSequencerTest, LargeSetFindsClashFreeWheelWalk) {
    // All 24 keys, shuffled. A clash-free order exists
    // (1A..12A, 12B..1B), and is above kExactLimit, so this tests the
    // heuristic path.
    QVector<TrackFeatures> tracks;
    int id = 1;
    for (int k = 1; k <= 12; ++k) {
        tracks.push_back(makeTrack(id++, k, true, 124));
        tracks.push_back(makeTrack(id++, k, false, 124));
    }
    std::mt19937 rng(7);
    std::shuffle(tracks.begin(), tracks.end(), rng);
    ASSERT_GT(tracks.size(), SmartSequencer::kExactLimit);

    SmartSequencer seq{MixScorer()};
    QElapsedTimer timer;
    timer.start();
    const auto r = seq.solve(tracks, std::nullopt, 2000);
    EXPECT_LT(timer.elapsed(), 3000);
    ASSERT_EQ(24, r.order.size());
    QSet<int> seen;
    for (const auto& t : r.order) {
        seen.insert(t.toString().toInt());
    }
    EXPECT_EQ(24, seen.size()); // every track exactly once
    EXPECT_EQ(0, r.clashCount) << r.warnings.join('\n').toStdString();
}

// ---------- BridgeFinder ----------

TEST(BridgeFinderTest, FindsTrackThatFixesATempoGap) {
    // The Promise (8B, 118.1) -> Rock The Casbah (8A, 129.5): ~9% apart.
    const TrackFeatures from = makeTrack(1, 8, false, 118.1);
    const TrackFeatures to = makeTrack(2, 8, true, 129.5);
    const QVector<TrackFeatures> library = {
            makeTrack(10, 8, false, 123.5), // 8B, halfway: fixes both sides
            makeTrack(11, 2, true, 123.5),  // right tempo, wrong key
            makeTrack(12, 8, false, 100.0), // right key, wrong tempo
            makeTrack(13, 8, true, 124.0),  // 8A, also fixes both sides
    };
    BridgeFinder finder{MixScorer()};
    const auto found = finder.find(from, to, library, {}, {});
    ASSERT_EQ(2, found.size());
    for (const auto& b : found) {
        EXPECT_TRUE(b.track.id == TrackId(QVariant(10)) || b.track.id == TrackId(QVariant(13)));
        EXPECT_TRUE(MixScorer::clashLabel(b.in).isEmpty());
        EXPECT_TRUE(MixScorer::clashLabel(b.out).isEmpty());
    }
    EXPECT_LE(found[0].cost, found[1].cost);
}

TEST(BridgeFinderTest, SkipsQueuedTracksAndCopiesOfThem) {
    const TrackFeatures from = makeTrack(1, 8, false, 118.1);
    const TrackFeatures to = makeTrack(2, 8, true, 129.5);
    TrackFeatures queued = makeTrack(10, 8, false, 123.5);
    TrackFeatures copy = makeTrack(11, 8, false, 123.6);
    queued.displayName = QStringLiteral("Visage - Fade To Grey");
    copy.displayName = QStringLiteral("visage - fade to grey ");
    BridgeFinder finder{MixScorer()};
    const auto found = finder.find(from,
            to,
            {queued, copy},
            {queued.id},
            {BridgeFinder::nameKey(queued)});
    EXPECT_TRUE(found.isEmpty());
}

TEST(SmartSequencerTest, SuggestsBridgesForRemainingClashes) {
    QVector<TrackFeatures> tracks = {
            makeTrack(1, 8, false, 118.1),
            makeTrack(2, 8, true, 129.5),
    };
    const QVector<TrackFeatures> library = {makeTrack(10, 8, false, 123.5)};
    SmartSequencer seq{MixScorer()};
    const auto r = seq.solve(tracks, std::nullopt, 2000, library);
    ASSERT_EQ(1, r.clashCount);
    ASSERT_EQ(1, r.bestBridges.size());
    EXPECT_EQ(1, r.bestBridges[0].first);
    ASSERT_EQ(1, r.bestBridges[0].second.size());
    EXPECT_EQ(TrackId(QVariant(10)), r.bestBridges[0].second.first());
    EXPECT_TRUE(r.orderLines.join('\n').contains(QStringLiteral("add:")));
}

TEST(SmartSequencerTest, SaysSoWhenNoBridgeExists) {
    QVector<TrackFeatures> tracks = {
            makeTrack(1, 8, true, 124),
            makeTrack(2, 2, true, 124),
    };
    const QVector<TrackFeatures> library = {makeTrack(10, 5, false, 90)};
    SmartSequencer seq{MixScorer()};
    const auto r = seq.solve(tracks, std::nullopt, 2000, library);
    EXPECT_TRUE(r.bestBridges.isEmpty());
    EXPECT_TRUE(r.orderLines.join('\n').contains(QStringLiteral("no track (or pair of tracks)")));
}

TEST(SmartSequencerTest, ListsEveryBridgeOptionPerGap) {
    // One gap with two possible bridges, one gap with none.
    QVector<TrackFeatures> tracks = {
            makeTrack(1, 8, false, 118.1),
            makeTrack(2, 8, true, 129.5),
    };
    const QVector<TrackFeatures> library = {
            makeTrack(10, 8, false, 123.5),
            makeTrack(13, 8, true, 124.0),
    };
    SmartSequencer seq{MixScorer()};
    const auto r = seq.solve(tracks, std::nullopt, 2000, library);
    ASSERT_EQ(1, r.gaps.size());
    EXPECT_EQ(1, r.gaps[0].k);
    ASSERT_EQ(2, r.gaps[0].options.size());
    EXPECT_EQ(2, r.gaps[0].optionTexts.size());
    EXPECT_EQ(r.bestBridges[0].second, r.gaps[0].options[0]); // best first
}

TEST(BridgeFinderTest, SmartFillChainsSmoothMixes) {
    // From 8A at 120: the library holds a smooth path 8A 121 -> 9A 122 ->
    // 9A 123, plus tracks that clash (wrong key or far tempo) and one queued.
    const TrackFeatures last = makeTrack(1, 8, true, 120);
    const QVector<TrackFeatures> library = {
            makeTrack(10, 8, true, 121),
            makeTrack(11, 9, true, 122),
            makeTrack(12, 9, true, 123),
            makeTrack(13, 2, false, 121), // key clash
            makeTrack(14, 8, true, 140),  // tempo clash
            makeTrack(15, 8, true, 120),  // already queued
    };
    BridgeFinder finder{MixScorer()};
    const auto chain = finder.extend(last, library, {TrackId(QVariant(15))}, {}, 5);
    ASSERT_EQ(3, chain.size()); // stops when nothing mixes smoothly
    QSet<TrackId> seen;
    TrackFeatures prev = last;
    for (const auto& t : chain) {
        EXPECT_FALSE(seen.contains(t.id));
        seen.insert(t.id);
        EXPECT_TRUE(MixScorer::clashLabel(MixScorer().score(prev, t)).isEmpty());
        prev = t;
    }
    EXPECT_FALSE(seen.contains(TrackId(QVariant(13))));
    EXPECT_FALSE(seen.contains(TrackId(QVariant(14))));
}

namespace {
TrackFeatures song(int id, const char* artist, const char* title) {
    TrackFeatures f = makeTrack(id, 8, true, 120);
    f.artist = QString::fromUtf8(artist);
    f.title = QString::fromUtf8(title);
    f.displayName = f.artist + QStringLiteral(" - ") + f.title;
    return f;
}
} // namespace

TEST(BridgeFinderTest, NameKeyMatchesOtherVersionsOfTheSameSong) {
    const auto key = [](const char* a, const char* t) {
        return BridgeFinder::nameKey(song(1, a, t));
    };
    EXPECT_EQ(key("The Ward Brothers", "Cross That Bridge (Extended)"),
            key("Ward Brothers", "Cross That Bridge"));
    EXPECT_EQ(key("Killing Joke", "Love Like Blood (12\" Version)"),
            key("KILLING JOKE", "Love Like Blood"));
    EXPECT_EQ(key("New Order", "Blue Monday - 1988 Remix"), key("New Order", "Blue Monday"));
    EXPECT_EQ(key("Big Pig", "Big Pig - Breakaway! (Extended)"), key("Big Pig", "Breakaway"));
    EXPECT_EQ(key("Ultravox", "White China [Razormaid]"), key("Ultravox", "White China"));
    EXPECT_EQ(key("Echo & The Bunnymen", "The Killing Moon"),
            key("Echo and the Bunnymen", "Killing Moon"));
    // Different songs, or the same title by someone else, stay different.
    EXPECT_NE(key("Ultravox", "White China"), key("Ultravox", "Vienna"));
    // Covers and other artists' versions count as the same song.
    EXPECT_EQ(key("Dio", "Rainbow in the Dark"), key("Computer Club", "Rainbow in the Dark"));
    EXPECT_EQ(key("DJ Snake & Lil Jon", "Turn Down For What (Remix) (Dirty)"),
            key("DJ Snake feat. Lil Jon", "Turn Down For What"));
    EXPECT_EQ(key("George Michael", "Freedom! '90 (Back to Reality Mix)"),
            key("Michael, George", "Freedom (remix)"));
    EXPECT_EQ(key("Sisters of Mercy", "Lucretia My Reflection"),
            key("", "Lucretia My Reflection by Destroid"));
    EXPECT_EQ(key("Juvenile", "Back That Azz Up feat. Mannie Fresh"),
            key("Juvenile", "Back That Azz Up"));
    // A title that is not a version tag after " - " is kept.
    EXPECT_NE(key("Band", "Song - Part One"), key("Band", "Song - Part Two"));
}

TEST(BridgeFinderTest, SmartFillSkipsOtherVersionsAndTheSameArtistInARow) {
    TrackFeatures last = song(1, "Xymox", "Obsession");
    last.bpm = 120;
    QVector<TrackFeatures> library = {
            song(10, "Xymox", "Obsession (Extended)"), // another version: never
            song(11, "Xymox", "Evelyn"),               // same artist as the last
            song(12, "Cyberaktif", "Nothing Stays"),
    };
    library[0].bpm = library[1].bpm = 120.5;
    library[2].bpm = 121;
    BridgeFinder finder{MixScorer()};
    const auto withRule = finder.extend(last, library, {}, {}, 1, true);
    ASSERT_EQ(1, withRule.size());
    EXPECT_EQ(TrackId(QVariant(12)), withRule[0].id);
    const auto chain = finder.extend(last, library, {}, {}, 5, false);
    for (const auto& t : chain) {
        EXPECT_FALSE(t.id == TrackId(QVariant(10)));
    }
}

TEST(BridgeFinderTest, SmartFillFollowsTheEnergyDirection) {
    // From energy 5: one candidate at 4 (down), one at 6 (up), same key/tempo.
    TrackFeatures last = makeTrack(1, 8, true, 120, 5);
    const QVector<TrackFeatures> library = {
            makeTrack(10, 8, true, 120, 4),
            makeTrack(11, 8, true, 120, 6),
    };
    MixScoreWeights build;
    build.direction = MixScoreWeights::EnergyDirection::Build;
    const auto up = BridgeFinder(MixScorer(build)).extend(last, library, {}, {}, 1);
    ASSERT_EQ(1, up.size());
    EXPECT_EQ(TrackId(QVariant(11)), up[0].id);
    // Keep level: a step of 1 either way costs the same, so both are fine;
    // a jump of 3 is worse than a step of 1.
    MixScoreWeights hold;
    hold.direction = MixScoreWeights::EnergyDirection::Hold;
    const QVector<TrackFeatures> levels = {
            makeTrack(20, 8, true, 120, 8),
            makeTrack(21, 8, true, 120, 5.5),
    };
    const auto level = BridgeFinder(MixScorer(hold)).extend(last, levels, {}, {}, 1);
    ASSERT_EQ(1, level.size());
    EXPECT_EQ(TrackId(QVariant(21)), level[0].id);
}

TEST(BridgeFinderTest, SmartFillPicksACalmOpenerWhenThereIsNothingToStartFrom) {
    QVector<TrackFeatures> library;
    for (int i = 1; i <= 8; ++i) {
        library.append(makeTrack(i, 8, true, 120, i)); // energy 1..8
    }
    library.append(makeTrack(99, 0, true, 120, 1)); // no key: never
    // Calmest quarter of the 8 rated tracks = energy 1 and 2.
    for (quint32 r = 0; r < 20; ++r) {
        const auto start = BridgeFinder::pickStart(library, {}, {}, true, r);
        ASSERT_TRUE(start.has_value());
        EXPECT_LE(start->energy, 2.0);
    }
    // Queued tracks are skipped; with nothing usable: nullopt.
    EXPECT_FALSE(BridgeFinder::pickStart({library.last()}, {}, {}, true, 0).has_value());
    const auto any = BridgeFinder::pickStart(library, {TrackId(QVariant(1))}, {}, false, 0);
    ASSERT_TRUE(any.has_value());
    EXPECT_FALSE(any->id == TrackId(QVariant(1)));
}

TEST(BridgeFinderTest, SmartFillMovesAroundTheKeysAndVaries) {
    // Plenty of 8A tracks, plus neighbours 9A, 7A and 8B, all at 120 BPM.
    QVector<TrackFeatures> library;
    int id = 100;
    for (int i = 0; i < 10; ++i) {
        library.append(makeTrack(id++, 8, true, 120));
    }
    for (int i = 0; i < 4; ++i) {
        library.append(makeTrack(id++, 9, true, 120));
        library.append(makeTrack(id++, 7, true, 120));
        library.append(makeTrack(id++, 8, false, 120));
    }
    const TrackFeatures last = makeTrack(1, 8, true, 120);
    BridgeFinder finder{MixScorer()};
    const auto noLongRuns = [&last](const QList<TrackFeatures>& chain) {
        TrackFeatures prev2 = last;
        TrackFeatures prev = last;
        int run = 1;
        for (const auto& t : chain) {
            const bool same = t.camelotNumber == prev.camelotNumber &&
                    t.camelotMinor == prev.camelotMinor;
            run = same ? run + 1 : 1;
            if (run > 2) {
                return false;
            }
            prev2 = prev;
            prev = t;
        }
        return true;
    };
    const auto fixed = finder.extend(last, library, {}, {}, 10, false, 0);
    ASSERT_EQ(10, fixed.size());
    EXPECT_TRUE(noLongRuns(fixed));
    // Random picks: different fills, still no long runs in one key.
    QSet<QString> fills;
    for (quint32 seed = 1; seed <= 10; ++seed) {
        const auto chain = finder.extend(last, library, {}, {}, 10, false, seed);
        ASSERT_EQ(10, chain.size());
        EXPECT_TRUE(noLongRuns(chain));
        QString ids;
        for (const auto& t : chain) {
            ids += t.id.toString() + QChar(',');
        }
        fills.insert(ids);
    }
    EXPECT_GT(fills.size(), 5);
}

TEST(BridgeFinderTest, GenreCostGroupsFamilies) {
    const auto cost = [](const char* a, const char* b) {
        return BridgeFinder::genreCost(QString::fromUtf8(a), QString::fromUtf8(b));
    };
    EXPECT_DOUBLE_EQ(0.0, cost("Goth", "Gothic Rock"));
    EXPECT_DOUBLE_EQ(0.0, cost("Gothic Darkwave Rock", "Darkwave"));
    EXPECT_DOUBLE_EQ(0.0, cost("80's", "New Wave"));
    EXPECT_DOUBLE_EQ(0.0, cost("EBM ", "Industrial"));
    EXPECT_DOUBLE_EQ(0.0, cost("Rock/Pop", "Pop"));
    EXPECT_DOUBLE_EQ(BridgeFinder::kRelatedGenreCost, cost("Goth", "Industrial"));
    EXPECT_DOUBLE_EQ(BridgeFinder::kRelatedGenreCost, cost("Synthpop", "Dance"));
    EXPECT_DOUBLE_EQ(BridgeFinder::kOtherGenreCost, cost("Gothic Rock", "Hip Hop"));
    EXPECT_DOUBLE_EQ(BridgeFinder::kOtherGenreCost, cost("Industrial", "Pop"));
    EXPECT_DOUBLE_EQ(BridgeFinder::kUnknownGenreCost, cost("", "Goth"));
    EXPECT_DOUBLE_EQ(BridgeFinder::kUnknownGenreCost, cost("Other", "Dance"));
    EXPECT_DOUBLE_EQ(0.0, cost("General Post-Punk", "Post punk"));
    EXPECT_DOUBLE_EQ(0.0, cost("World", "World"));
}

TEST(BridgeFinderTest, SmartFillPrefersTheSameStyle) {
    // Same key and tempo; only the genre differs.
    TrackFeatures last = makeTrack(1, 8, true, 120);
    last.genre = QStringLiteral("Gothic Rock");
    QVector<TrackFeatures> library = {
            makeTrack(10, 8, true, 120),
            makeTrack(11, 8, true, 120),
    };
    library[0].genre = QStringLiteral("Hip Hop");
    library[1].genre = QStringLiteral("Darkwave");
    BridgeFinder finder{MixScorer()};
    const auto chain = finder.extend(last, library, {}, {}, 1, false, 0);
    ASSERT_EQ(1, chain.size());
    EXPECT_EQ(TrackId(QVariant(11)), chain[0].id);
}

TEST(BridgeFinderTest, LiveAssistantRanksTheBestNextTracks) {
    TrackFeatures now = makeTrack(1, 8, true, 128, 5);
    now.displayName = QStringLiteral("A - Now");
    QVector<TrackFeatures> library = {
            makeTrack(2, 8, true, 128, 6),  // same key, same tempo: best
            makeTrack(3, 9, true, 127, 6),  // next key, close tempo
            makeTrack(4, 2, true, 128, 6),  // key clash (opposite side): never
            makeTrack(5, 8, true, 100, 6),  // tempo clash: never
            makeTrack(6, 8, false, 129, 6), // relative key
            makeTrack(7, 0, true, 128, 6),  // no key: cannot judge
    };
    const QStringList names = {"B - One", "C - Two", "D - Three", "E - Four", "F - Five", "G - Six"};
    for (int i = 0; i < library.size(); ++i) {
        library[i].displayName = names[i];
    }
    const BridgeFinder finder{MixScorer(MixScoreWeights())};
    const auto list = finder.suggestNext(now, library, {}, {}, 10, false);
    ASSERT_EQ(3, list.size());
    EXPECT_TRUE(list[0].track.id == TrackId(QVariant(2)));
    for (const auto& s : list) {
        EXPECT_FALSE(s.track.id == TrackId(QVariant(4)));
        EXPECT_FALSE(s.track.id == TrackId(QVariant(5)));
        EXPECT_FALSE(s.track.id == TrackId(QVariant(7)));
        EXPECT_TRUE(MixScorer::clashLabel(s.score).isEmpty());
    }
    for (int i = 1; i < list.size(); ++i) {
        EXPECT_LE(list[i - 1].cost, list[i].cost);
    }
    // Limited to `count`.
    EXPECT_EQ(1, finder.suggestNext(now, library, {}, {}, 1, false).size());
}

TEST(BridgeFinderTest, LiveAssistantSkipsUsedSongsAndSameArtist) {
    TrackFeatures now = makeTrack(1, 8, true, 128, 5);
    now.displayName = QStringLiteral("Mesh - Born To Lie");
    now.artist = QStringLiteral("Mesh");
    now.title = QStringLiteral("Born To Lie");
    TrackFeatures otherVersion = makeTrack(2, 8, true, 128, 5);
    otherVersion.displayName = QStringLiteral("Mesh - Born To Lie (Club Version)");
    otherVersion.artist = QStringLiteral("Mesh");
    otherVersion.title = QStringLiteral("Born To Lie (Club Version)");
    TrackFeatures sameArtist = makeTrack(3, 8, true, 128, 5);
    sameArtist.displayName = QStringLiteral("Mesh - Crash");
    sameArtist.artist = QStringLiteral("Mesh");
    sameArtist.title = QStringLiteral("Crash");
    TrackFeatures played = makeTrack(4, 8, true, 128, 5);
    played.displayName = QStringLiteral("And One - Military Fashion Show");
    TrackFeatures fine = makeTrack(5, 8, true, 128, 5);
    fine.displayName = QStringLiteral("VNV Nation - Chrome");
    const QVector<TrackFeatures> library = {otherVersion, sameArtist, played, fine};
    const BridgeFinder finder{MixScorer(MixScoreWeights())};

    // The same song in another version never; a played track never.
    auto list = finder.suggestNext(now, library, {played.id}, {}, 10, false);
    ASSERT_EQ(2, list.size());
    for (const auto& s : list) {
        EXPECT_FALSE(s.track.id == otherVersion.id);
        EXPECT_FALSE(s.track.id == played.id);
    }
    // Same artist only when allowed.
    list = finder.suggestNext(now, library, {played.id}, {}, 10, true);
    ASSERT_EQ(1, list.size());
    EXPECT_TRUE(list[0].track.id == fine.id);
}

TEST(SmartSequencerTest, TwoTrackBridgeForABigTempoJump) {
    // 111.8 -> 125.0 BPM is 12% apart: no single track is within 5% of
    // both, but 117 then 121.5 closes the gap in three smooth steps.
    QVector<TrackFeatures> tracks = {
            makeTrack(1, 8, true, 111.8),
            makeTrack(2, 8, true, 125.0),
    };
    QVector<TrackFeatures> library = {
            makeTrack(10, 8, true, 117.0),
            makeTrack(11, 8, true, 121.5),
            makeTrack(12, 2, true, 118.0), // wrong key
    };
    library[0].displayName = QStringLiteral("A - First Step");
    library[1].displayName = QStringLiteral("B - Second Step");
    library[2].displayName = QStringLiteral("C - Clash");
    SmartSequencer seq{MixScorer()};
    // Keep the slow track first, so the jump is upwards.
    const auto r = seq.solve(tracks, TrackId(QVariant(1)), 2000, library);
    ASSERT_EQ(1, r.gaps.size());
    ASSERT_FALSE(r.gaps[0].options.isEmpty());
    const QList<TrackId> best = r.gaps[0].options[0];
    ASSERT_EQ(2, best.size());
    EXPECT_TRUE(best[0] == TrackId(QVariant(10)));
    EXPECT_TRUE(best[1] == TrackId(QVariant(11)));
    EXPECT_TRUE(r.gaps[0].optionTexts[0].contains(QStringLiteral("then")));
    ASSERT_EQ(1, r.bestBridges.size());
    EXPECT_EQ(2, r.bestBridges[0].second.size());
}

TEST(BridgeFinderTest, SingleBridgesComeBeforePairs) {
    // A gap one track can bridge never gets a two-track bridge.
    const TrackFeatures from = makeTrack(1, 8, false, 118.1);
    const TrackFeatures to = makeTrack(2, 8, true, 129.5);
    QVector<TrackFeatures> tracks = {from, to};
    QVector<TrackFeatures> library = {
            makeTrack(10, 8, false, 123.5),
            makeTrack(11, 8, false, 121.0),
            makeTrack(12, 8, true, 126.0),
    };
    SmartSequencer seq{MixScorer()};
    const auto r = seq.solve(tracks, std::nullopt, 2000, library);
    ASSERT_EQ(1, r.gaps.size());
    for (const auto& option : r.gaps[0].options) {
        EXPECT_EQ(1, option.size());
    }
}

TEST(BridgeFinderTest, VideoBridgesBetweenVideos) {
    TrackFeatures from = makeTrack(1, 8, false, 118.1);
    TrackFeatures to = makeTrack(2, 8, true, 129.5);
    from.isVideo = true;
    to.isVideo = true;
    TrackFeatures audio = makeTrack(10, 8, false, 123.5); // the smoother mix
    TrackFeatures video = makeTrack(11, 8, true, 124.0);
    video.isVideo = true;
    const BridgeFinder finder{MixScorer()};
    auto found = finder.find(from, to, {audio, video}, {}, {});
    ASSERT_EQ(2, found.size());
    EXPECT_TRUE(found[0].track.id == video.id);
    // Between audio tracks the smoother mix wins as before.
    from.isVideo = false;
    to.isVideo = false;
    found = finder.find(from, to, {audio, video}, {}, {});
    ASSERT_EQ(2, found.size());
    EXPECT_TRUE(found[0].track.id == audio.id);
}

TEST(BridgeFinderTest, DriftingGridIsNeverABridge) {
    const TrackFeatures from = makeTrack(1, 8, false, 118.1);
    const TrackFeatures to = makeTrack(2, 8, true, 129.5);
    TrackFeatures drifting = makeTrack(10, 8, false, 123.5);
    drifting.gridUnsteady = true;
    const BridgeFinder finder{MixScorer()};
    EXPECT_TRUE(finder.find(from, to, {drifting}, {}, {}).isEmpty());
    EXPECT_TRUE(finder.findPairs(from, to, {drifting}, {}, {}).isEmpty());
}

TEST(SmartSequencerTest, ReportsTracksThatCannotBeBeatmatched) {
    QVector<TrackFeatures> tracks = {
            makeTrack(1, 8, true, 124.0),
            makeTrack(2, 8, true, 124.5),
    };
    tracks[1].gridUnsteady = true;
    tracks[1].displayName = QStringLiteral("Erasure - Chains of Love");
    SmartSequencer seq{MixScorer()};
    const auto r = seq.solve(tracks, std::nullopt, 2000, {});
    EXPECT_EQ(0, r.clashCount); // key and tempo are fine
    ASSERT_EQ(1, r.warnings.size());
    EXPECT_TRUE(r.warnings[0].contains(QStringLiteral("no beatmatch")));
    EXPECT_TRUE(r.warnings[0].contains(QStringLiteral("Erasure")));
}
