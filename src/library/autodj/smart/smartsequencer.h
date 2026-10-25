#pragma once

#include <QElapsedTimer>
#include <QList>
#include <QStringList>
#include <QVector>
#include <optional>

#include <utility>

#include "library/autodj/smart/mixscorer.h"

/// A clash in the sorted order and the library tracks that could bridge it.
struct BridgeGap {
    int k = 0;              ///< the bridge goes right after the k-th track (from 1)
    QString fromText;       ///< the two tracks either side, for the DJ
    QString toText;
    /// Best first; empty = nothing bridges this gap. Each option is one
    /// track, or two (played in that order) when no single track bridges.
    QList<QList<TrackId>> options;
    QStringList optionTexts;
};

/// The outcome of SmartSequencer::solve().
struct SequenceResult {
    QList<TrackId> order;
    double totalCost = 0.0;
    /// Transitions the solver could not make smooth (key or tempo clash).
    int clashCount = 0;
    /// One human-readable line per clash, so the DJ knows where a bridge
    /// track is needed.
    QStringList warnings;
    /// The whole running order, one entry per track, with clash notes and
    /// bridge-track suggestions.
    QStringList orderLines;
    /// Best bridge per clash, as (k, tracks): play `tracks` (one or two)
    /// right after the k-th track of `order` (k counted from 1). Each track
    /// used once.
    QList<std::pair<int, QList<TrackId>>> bestBridges;
    /// Every clash with ALL its bridge options (up to kMaxBridgeOptions),
    /// so the DJ can choose. Options only exclude tracks already queued, so
    /// one track may be offered for two gaps (the DJ picks where it goes).
    QList<BridgeGap> gaps;
    static constexpr int kMaxBridgeOptions = 6;
};

/// Sorts a set of tracks into the lowest-cost mixing order.
///
/// This is an open "travelling salesperson path" problem.
/// Besides the cost of each pair of neighbours, the first and last track
/// carry a "set shape" cost (see MixScorer::startCost/endCost), e.g. so an
/// energy-building set starts calm and ends at the peak.
/// - Up to kExactLimit tracks: exact answer (Held-Karp dynamic programming).
/// - Larger sets: multi-start greedy, then 2-opt / Or-opt polishing and
///   random "kick and re-polish" rounds until the time budget runs out.
/// Pure logic: no Track objects, no database, safe on a worker thread.
class SmartSequencer {
  public:
    /// Largest set solved exactly. 2^16 * 16 * 16 = ~17M steps, ~12 MB memory.
    static constexpr int kExactLimit = 16;

    explicit SmartSequencer(const MixScorer& scorer);

    /// @param startId optional track that must stay first (e.g. playing now)
    /// @param timeBudgetMs stop improving after this long, return best so far
    /// @param bridgeCandidates library tracks to suggest as bridges for
    ///        clashes that remain; empty = no suggestions
    SequenceResult solve(const QVector<TrackFeatures>& tracks,
            std::optional<TrackId> startId = std::nullopt,
            int timeBudgetMs = 2000,
            const QVector<TrackFeatures>& bridgeCandidates = {}) const;

  private:
    struct Costs {
        QVector<QVector<double>> pair; ///< pair[a][b]: play b right after a
        QVector<double> start;         ///< extra cost if a track opens the set
        QVector<double> end;           ///< extra cost if a track closes the set
    };

    Costs buildCosts(const QVector<TrackFeatures>& tracks) const;
    static QVector<int> exactPath(const Costs& c, int fixedStart);
    static QVector<int> heuristicPath(const Costs& c, int fixedStart, int timeBudgetMs);
    static void localSearch(const Costs& c,
            QVector<int>* pPath,
            bool lockFirst,
            const QElapsedTimer& timer,
            int timeBudgetMs);
    static QVector<int> greedyPath(const Costs& c, int start);
    static bool improveTwoOpt(const Costs& c, QVector<int>* pPath, bool lockFirst);
    static bool improveOrOpt(const Costs& c, QVector<int>* pPath, bool lockFirst);
    static double pathCost(const Costs& c, const QVector<int>& path);

    MixScorer m_scorer;
};
