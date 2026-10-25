#include "library/autodj/smart/smartsequencer.h"

#include <QSet>

#include "library/autodj/smart/bridgefinder.h"

#include <QElapsedTimer>
#include <algorithm>
#include <limits>
#include <random>
#include <vector>

namespace {
constexpr double kEps = 1e-9;
// Stop kicking after this many rounds in a row without improvement.
constexpr int kMaxStaleKicks = 300;
} // namespace

SmartSequencer::SmartSequencer(const MixScorer& scorer)
        : m_scorer(scorer) {
}

// Cost of every ordered pair (not symmetric: A->B can differ from B->A
// because the energy direction matters), plus the set-shape cost of each
// track as the opener and as the closer.
SmartSequencer::Costs SmartSequencer::buildCosts(
        const QVector<TrackFeatures>& tracks) const {
    const int n = static_cast<int>(tracks.size());
    Costs c;
    c.pair = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    c.start.resize(n);
    c.end.resize(n);
    for (int i = 0; i < n; ++i) {
        c.start[i] = m_scorer.startCost(tracks[i]);
        c.end[i] = m_scorer.endCost(tracks[i]);
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                c.pair[i][j] = m_scorer.score(tracks[i], tracks[j]).total;
            }
        }
    }
    return c;
}

// static
double SmartSequencer::pathCost(const Costs& c, const QVector<int>& path) {
    if (path.isEmpty()) {
        return 0.0;
    }
    double sum = c.start[path.front()] + c.end[path.back()];
    for (int k = 1; k < path.size(); ++k) {
        sum += c.pair[path[k - 1]][path[k]];
    }
    return sum;
}

// static
// Always go to the cheapest unused track next.
QVector<int> SmartSequencer::greedyPath(const Costs& c, int start) {
    const int n = static_cast<int>(c.pair.size());
    QVector<bool> used(n, false);
    QVector<int> path;
    path.reserve(n);
    path.push_back(start);
    used[start] = true;
    while (path.size() < n) {
        const int last = path.back();
        int best = -1;
        double bestCost = std::numeric_limits<double>::max();
        for (int j = 0; j < n; ++j) {
            if (!used[j] && c.pair[last][j] < bestCost) {
                bestCost = c.pair[last][j];
                best = j;
            }
        }
        used[best] = true;
        path.push_back(best);
    }
    return path;
}

// static
// 2-opt: try reversing a stretch of the list. Costs are not symmetric, so
// the reversed stretch is priced with prefix sums; each test is O(1).
// Reversing a stretch that touches either end also changes which track
// opens or closes the set, so the start/end costs are included.
bool SmartSequencer::improveTwoOpt(const Costs& c, QVector<int>* pPath, bool lockFirst) {
    QVector<int>& p = *pPath;
    const int n = static_cast<int>(p.size());
    if (n < 2) {
        return false;
    }
    QVector<double> fwd(n, 0.0); // edges walked forwards
    QVector<double> rev(n, 0.0); // the same edges walked backwards
    for (int k = 1; k < n; ++k) {
        fwd[k] = fwd[k - 1] + c.pair[p[k - 1]][p[k]];
        rev[k] = rev[k - 1] + c.pair[p[k]][p[k - 1]];
    }
    for (int i = lockFirst ? 1 : 0; i < n - 1; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double before = fwd[j] - fwd[i];
            double after = rev[j] - rev[i];
            if (i > 0) {
                before += c.pair[p[i - 1]][p[i]];
                after += c.pair[p[i - 1]][p[j]];
            } else {
                before += c.start[p[i]];
                after += c.start[p[j]];
            }
            if (j < n - 1) {
                before += c.pair[p[j]][p[j + 1]];
                after += c.pair[p[i]][p[j + 1]];
            } else {
                before += c.end[p[j]];
                after += c.end[p[i]];
            }
            if (after + kEps < before) {
                std::reverse(p.begin() + i, p.begin() + j + 1);
                return true;
            }
        }
    }
    return false;
}

// static
// Or-opt: lift out 1-3 tracks in a row and drop them somewhere cheaper.
bool SmartSequencer::improveOrOpt(const Costs& c, QVector<int>* pPath, bool lockFirst) {
    QVector<int>& p = *pPath;
    const int n = static_cast<int>(p.size());
    // Edge cost; -1 means "no track here" (start or end of the list), where
    // the set-shape cost applies instead. Both -1 cannot happen here.
    auto edge = [&c](int a, int b) {
        if (a < 0) {
            return b < 0 ? 0.0 : c.start[b];
        }
        if (b < 0) {
            return c.end[a];
        }
        return c.pair[a][b];
    };
    const int firstMovable = lockFirst ? 1 : 0;
    for (int len = 1; len <= 3; ++len) {
        for (int i = firstMovable; i + len <= n; ++i) {
            if (len == n) {
                continue; // nothing to move it next to
            }
            const int prev = i > 0 ? p[i - 1] : -1;
            const int next = i + len < n ? p[i + len] : -1;
            const int s0 = p[i];
            const int s1 = p[i + len - 1];
            const double saved = edge(prev, s0) + edge(s1, next) - edge(prev, next);
            // k = the gap just before p[k]; k == n means the very end.
            for (int k = firstMovable; k <= n; ++k) {
                if (k >= i && k <= i + len) {
                    continue; // same place
                }
                const int x = k > 0 ? p[k - 1] : -1;
                const int y = k < n ? p[k] : -1;
                const double added = edge(x, s0) + edge(s1, y) - edge(x, y);
                if (added + kEps < saved) {
                    QVector<int> out;
                    out.reserve(n);
                    for (int t = 0; t <= n; ++t) {
                        if (t == k) {
                            out += p.mid(i, len);
                        }
                        if (t < n && (t < i || t >= i + len)) {
                            out.push_back(p[t]);
                        }
                    }
                    p = out;
                    return true;
                }
            }
        }
    }
    return false;
}

// static
// Exact answer by dynamic programming (Held-Karp), for small sets.
// dp[mask][j] = cheapest way to play exactly the tracks in `mask`, ending on j.
QVector<int> SmartSequencer::exactPath(const Costs& c, int fixedStart) {
    const int n = static_cast<int>(c.pair.size());
    const int full = (1 << n) - 1;
    const double inf = std::numeric_limits<double>::max();
    const auto at = [n](int mask, int j) {
        return static_cast<std::size_t>(mask) * n + j;
    };
    std::vector<double> dp(static_cast<std::size_t>(full + 1) * n, inf);
    std::vector<int> parent(static_cast<std::size_t>(full + 1) * n, -1);
    for (int s = 0; s < n; ++s) {
        if (fixedStart < 0 || s == fixedStart) {
            dp[at(1 << s, s)] = c.start[s];
        }
    }
    for (int mask = 1; mask <= full; ++mask) {
        for (int j = 0; j < n; ++j) {
            const double cur = dp[at(mask, j)];
            if (!(mask & (1 << j)) || cur == inf) {
                continue;
            }
            for (int k = 0; k < n; ++k) {
                if (mask & (1 << k)) {
                    continue;
                }
                const int next = mask | (1 << k);
                const double v = cur + c.pair[j][k];
                if (v < dp[at(next, k)]) {
                    dp[at(next, k)] = v;
                    parent[at(next, k)] = j;
                }
            }
        }
    }
    int bestEnd = -1;
    double bestTotal = inf;
    for (int j = 0; j < n; ++j) {
        const double v = dp[at(full, j)];
        if (v != inf && v + c.end[j] < bestTotal) {
            bestTotal = v + c.end[j];
            bestEnd = j;
        }
    }
    QVector<int> path;
    path.reserve(n);
    int mask = full;
    for (int j = bestEnd; j != -1;) {
        path.push_back(j);
        const int prev = parent[at(mask, j)];
        mask &= ~(1 << j);
        j = prev;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

// static
// Polish with 2-opt and Or-opt until nothing improves or time runs out.
void SmartSequencer::localSearch(const Costs& c,
        QVector<int>* pPath,
        bool lockFirst,
        const QElapsedTimer& timer,
        int timeBudgetMs) {
    bool improved = true;
    while (improved && timer.elapsed() < timeBudgetMs) {
        improved = improveTwoOpt(c, pPath, lockFirst);
        improved = improveOrOpt(c, pPath, lockFirst) || improved;
    }
}

// static
// For large sets: greedy start, polish, then repeatedly "kick" the order
// (move a random block) and re-polish, keeping any improvement.
QVector<int> SmartSequencer::heuristicPath(const Costs& c, int fixedStart, int timeBudgetMs) {
    QElapsedTimer timer;
    timer.start();
    const int n = static_cast<int>(c.pair.size());
    const bool lockFirst = fixedStart >= 0;

    // Step 1: greedy from every possible first track; keep the best.
    QVector<int> best;
    double bestCost = std::numeric_limits<double>::max();
    for (int s = 0; s < n; ++s) {
        if (lockFirst && s != fixedStart) {
            continue;
        }
        const QVector<int> path = greedyPath(c, s);
        const double cost = pathCost(c, path);
        if (cost < bestCost) {
            bestCost = cost;
            best = path;
        }
        if (timer.elapsed() > timeBudgetMs / 4) {
            break; // leave time for polishing
        }
    }

    // Step 2: polish.
    localSearch(c, &best, lockFirst, timer, timeBudgetMs);
    bestCost = pathCost(c, best);

    // Step 3: kick and re-polish. Fixed seed: the same crate always gives
    // the same order, which makes results repeatable and testable.
    std::mt19937 rng(20260925u);
    const int firstMovable = lockFirst ? 1 : 0;
    const int movable = n - firstMovable;
    int staleKicks = 0;
    while (movable >= 4 && staleKicks < kMaxStaleKicks && timer.elapsed() < timeBudgetMs) {
        QVector<int> cand = best;
        const int maxLen = std::min(8, movable - 1);
        const int len = std::uniform_int_distribution<int>(1, maxLen)(rng);
        const int from = std::uniform_int_distribution<int>(firstMovable, n - len)(rng);
        QVector<int> block = cand.mid(from, len);
        cand.remove(from, len);
        if (rng() & 1u) {
            std::reverse(block.begin(), block.end());
        }
        const int to = std::uniform_int_distribution<int>(
                firstMovable, static_cast<int>(cand.size()))(rng);
        for (int b = 0; b < len; ++b) {
            cand.insert(to + b, block[b]);
        }
        localSearch(c, &cand, lockFirst, timer, timeBudgetMs);
        const double cost = pathCost(c, cand);
        if (cost + kEps < bestCost) {
            best = cand;
            bestCost = cost;
            staleKicks = 0;
        } else {
            ++staleKicks;
        }
    }
    return best;
}

SequenceResult SmartSequencer::solve(const QVector<TrackFeatures>& tracks,
        std::optional<TrackId> startId,
        int timeBudgetMs,
        const QVector<TrackFeatures>& bridgeCandidates) const {
    SequenceResult result;
    const int n = static_cast<int>(tracks.size());
    if (n == 0) {
        return result;
    }
    const Costs c = buildCosts(tracks);

    int fixedStart = -1;
    if (startId) {
        for (int i = 0; i < n; ++i) {
            if (tracks[i].id == *startId) {
                fixedStart = i;
                break;
            }
        }
    }

    const QVector<int> best = n <= kExactLimit
            ? exactPath(c, fixedStart)
            : heuristicPath(c, fixedStart, timeBudgetMs);

    // Bridges must not repeat a queued song (or another copy of it).
    const BridgeFinder bridgeFinder(m_scorer);
    QSet<TrackId> usedIds;
    QSet<QString> usedNames;
    for (const TrackFeatures& t : tracks) {
        usedIds.insert(t.id);
        if (!t.displayName.isEmpty()) {
            usedNames.insert(BridgeFinder::nameKey(t));
        }
    }
    const QSet<TrackId> queuedIds = usedIds;
    const QSet<QString> queuedNames = usedNames;

    // Report: the whole running order, with the clashes that could not be
    // avoided marked between the two tracks involved.
    result.totalCost = pathCost(c, best);
    for (int k = 0; k < n; ++k) {
        const TrackFeatures& to = tracks[best[k]];
        result.order.push_back(to.id);
        if (k > 0) {
            const TrackFeatures& from = tracks[best[k - 1]];
            const MixScore s = m_scorer.score(from, to);
            const QString label = MixScorer::clashLabel(s);
            if (!label.isEmpty()) {
                ++result.clashCount;
                // k is 0-based, so the pair is tracks k and k + 1 counted from 1.
                QString warning = QStringLiteral("Tracks %1 and %2 (%3): %4")
                                          .arg(QString::number(k),
                                                  QString::number(k + 1),
                                                  label,
                                                  s.reason);
                if (!from.displayName.isEmpty() || !to.displayName.isEmpty()) {
                    warning += QStringLiteral("\n    %1\n    -> %2")
                                       .arg(from.displayName, to.displayName);
                }
                result.warnings << warning;
                result.orderLines << QStringLiteral("      !! %1: %2").arg(label, s.reason);
                if (!bridgeCandidates.isEmpty()) {
                    // One track if any bridges the gap, else two in a row.
                    const auto findBridges = [&](const QSet<TrackId>& ids,
                                                     const QSet<QString>& names,
                                                     int maxResults) {
                        QList<BridgeSuggestion> found = bridgeFinder.find(
                                from, to, bridgeCandidates, ids, names, maxResults);
                        if (found.isEmpty()) {
                            found = bridgeFinder.findPairs(
                                    from, to, bridgeCandidates, ids, names, maxResults);
                        }
                        return found;
                    };
                    const auto idsOf = [](const BridgeSuggestion& b) {
                        QList<TrackId> ids{b.track.id};
                        if (b.isPair) {
                            ids.append(b.second.id);
                        }
                        return ids;
                    };
                    const auto textOf = [](const BridgeSuggestion& b) {
                        return b.isPair ? QStringLiteral("%1  then  %2")
                                                  .arg(MixScorer::trackText(b.track),
                                                          MixScorer::trackText(b.second))
                                        : MixScorer::trackText(b.track);
                    };
                    BridgeGap gap;
                    gap.k = k;
                    gap.fromText = MixScorer::trackText(from);
                    gap.toText = MixScorer::trackText(to);
                    for (const BridgeSuggestion& option :
                            findBridges(queuedIds, queuedNames, SequenceResult::kMaxBridgeOptions)) {
                        gap.options.append(idsOf(option));
                        gap.optionTexts.append(textOf(option));
                    }
                    result.gaps.append(gap);
                    const QList<BridgeSuggestion> bridges = findBridges(usedIds, usedNames, 3);
                    if (bridges.isEmpty()) {
                        result.orderLines << QStringLiteral(
                                "         no track (or pair of tracks) in your library bridges "
                                "this gap");
                    } else {
                        for (int b = 0; b < bridges.size(); ++b) {
                            result.orderLines << QStringLiteral("         %1 %2")
                                                         .arg(b == 0 ? QStringLiteral("add:")
                                                                     : QStringLiteral("or: "),
                                                                 textOf(bridges[b]));
                        }
                        // The best one is reserved, so no track is suggested twice.
                        const BridgeSuggestion& pick = bridges.first();
                        result.bestBridges.append(std::make_pair(k, idsOf(pick)));
                        for (const TrackFeatures* pTrack : {&pick.track, &pick.second}) {
                            if (pTrack == &pick.second && !pick.isPair) {
                                continue;
                            }
                            usedIds.insert(pTrack->id);
                            if (!pTrack->displayName.isEmpty()) {
                                usedNames.insert(BridgeFinder::nameKey(*pTrack));
                            }
                        }
                    }
                }
            } else if (from.gridUnsteady || to.gridUnsteady) {
                // Mixes in key and tempo, but a beat grid drifts: Auto DJ
                // cannot beatmatch it and switches quickly instead.
                const TrackFeatures& unsteady = from.gridUnsteady ? from : to;
                const QString note = QStringLiteral(
                        "no beatmatch: the beat grid of %1 drifts off the beat (quick switch)")
                                             .arg(unsteady.displayName.isEmpty()
                                                             ? MixScorer::trackText(unsteady)
                                                             : unsteady.displayName);
                result.warnings << QStringLiteral("Tracks %1 and %2: %3")
                                           .arg(QString::number(k), QString::number(k + 1), note);
                result.orderLines << QStringLiteral("      ~~ %1").arg(note);
            }
        }
        result.orderLines << MixScorer::trackLine(k + 1, to);
    }
    return result;
}
