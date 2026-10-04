#include "dialog/autorecommendationengine.h"

#include <algorithm>
#include <cmath>

#include "track/keyutils.h"
#include "track/track.h"

namespace {

// Tempo distances above this threshold are considered dissimilar and
// capped, so a single criterion cannot dominate the overall score.
constexpr double kMaxTempoDistance = 0.15;

// Loudness distances (in dB) above this threshold are capped.
constexpr double kMaxLoudnessDistanceDb = 9.0;

// Fame distances (log10 of the play count ratio) above this threshold
// are capped.
constexpr double kMaxFameDistance = 1.0;

// Relative tempo distances between consecutive queue entries above
// this threshold are considered too dissimilar for a smooth fade.
constexpr double kMaxTransitionTempoDistance = 0.10;

// Minimum track duration in seconds that leaves a usable fade window:
// durations below this score proportionally, so very short tracks are
// sequenced towards the end of the queue.
constexpr double kMinFadeDurationSec = 60.0;

double clamp01(double value) {
    return std::clamp(value, 0.0, 1.0);
}

// Converts a raw distance into a similarity score in [0.0, 1.0],
// capping distances above maxDistance at zero similarity. Returns a
// negative value if the distance is unknown.
double similarityFromDistance(double distance, double maxDistance) {
    if (distance < 0.0) {
        return -1.0;
    }
    return clamp01(1.0 - distance / maxDistance);
}

} // anonymous namespace

namespace mixxx {

double autoRecommendationTempoDistance(
        double bpm,
        const AutoRecommendationReference& reference) {
    if (bpm <= 0.0 || reference.bpm <= 0.0) {
        return -1.0;
    }
    // Consider the direct match and the half-time and double-time
    // matches of the reference tempo, and use the smallest relative
    // distance.
    double bestDistance = -1.0;
    for (const double factor : {1.0, 2.0, 0.5}) {
        const double referenceBpm = reference.bpm * factor;
        const double distance = std::abs(bpm - referenceBpm) / referenceBpm;
        if (bestDistance < 0.0 || distance < bestDistance) {
            bestDistance = distance;
        }
    }
    return bestDistance;
}

double autoRecommendationLoudnessDistance(
        double replayGainRatio,
        const AutoRecommendationReference& reference) {
    if (replayGainRatio <= 0.0 || reference.replayGainRatio <= 0.0) {
        return -1.0;
    }
    // A ratio of 1.0 corresponds to 0 dB, so the loudness difference
    // in dB is 20 * log10 of the ratio of both values. The dB distance
    // is clamped to avoid extreme values for very quiet tracks.
    const double ratio = replayGainRatio / reference.replayGainRatio;
    const double distanceDb = std::abs(20.0 * std::log10(ratio));
    return std::min(distanceDb, 2.0 * kMaxLoudnessDistanceDb);
}

double autoRecommendationFameDistance(
        int timesPlayed,
        const AutoRecommendationReference& reference) {
    if (timesPlayed < 0 || reference.timesPlayed < 0) {
        return -1.0;
    }
    // Compare on a log scale with an offset of 1 to support play
    // counts of 0.
    const double logA = std::log10(1.0 + timesPlayed);
    const double logB = std::log10(1.0 + reference.timesPlayed);
    return std::abs(logA - logB);
}

double autoRecommendationTransitionTempoDistance(double bpmA, double bpmB) {
    if (bpmA <= 0.0 || bpmB <= 0.0) {
        return -1.0;
    }
    // Half- and double-time matches make equally good transitions, so
    // reuse the reference-based tempo distance.
    AutoRecommendationReference reference;
    reference.bpm = bpmA;
    return autoRecommendationTempoDistance(bpmB, reference);
}

double autoRecommendationFadeScore(double durationSec) {
    if (durationSec <= 0.0) {
        return -1.0;
    }
    return clamp01(durationSec / kMinFadeDurationSec);
}

double autoRecommendationTransitionScore(
        double previousBpm,
        double candidateBpm,
        double candidateDurationSec) {
    // Blend the BPM compatibility toward the previous queue entry
    // with the fade-window adequacy of the candidate's own duration.
    // Unknown criteria are skipped and the rest is re-normalized, so
    // tracks with missing analysis are not penalized.
    double sum = 0.0;
    double criteria = 0.0;

    const double tempoDistance =
            autoRecommendationTransitionTempoDistance(previousBpm, candidateBpm);
    if (tempoDistance >= 0.0) {
        sum += similarityFromDistance(
                tempoDistance, kMaxTransitionTempoDistance);
        criteria += 1.0;
    }

    const double fadeScore = autoRecommendationFadeScore(candidateDurationSec);
    if (fadeScore >= 0.0) {
        sum += fadeScore;
        criteria += 1.0;
    }

    if (criteria <= 0.0) {
        return -1.0;
    }
    return sum / criteria;
}

double autoRecommendationScore(
        const TrackPointer& pTrack,
        const AutoRecommendationReference& reference,
        const AutoRecommendationWeights& weights) {
    if (!pTrack) {
        return 0.0;
    }

    // Score each criterion on a similarity scale from 0.0 (dissimilar)
    // to 1.0 (identical). Unknown values are skipped and the remaining
    // criteria are re-normalized, so tracks with missing analysis are
    // not penalized.
    double weightedSum = 0.0;
    double weightSum = 0.0;

    const double tempoSimilarity = similarityFromDistance(
            autoRecommendationTempoDistance(pTrack->getBpm(), reference),
            kMaxTempoDistance);
    if (tempoSimilarity >= 0.0 && weights.tempo > 0.0) {
        weightedSum += weights.tempo * tempoSimilarity;
        weightSum += weights.tempo;
    }

    const auto replayGain = pTrack->getReplayGain();
    const double loudnessSimilarity = similarityFromDistance(
            autoRecommendationLoudnessDistance(
                    replayGain.hasRatio() ? replayGain.getRatio() : -1.0,
                    reference),
            kMaxLoudnessDistanceDb);
    if (loudnessSimilarity >= 0.0 && weights.energy > 0.0) {
        weightedSum += weights.energy * loudnessSimilarity;
        weightSum += weights.energy;
    }

    // Key similarity from the circle-of-fifths distance between the
    // candidate and the reference key. Only evaluated if the reference
    // has a valid key.
    if (weights.key > 0.0 && reference.keyValue > 0) {
        const auto referenceKey =
                static_cast<mixxx::track::io::key::ChromaticKey>(
                        reference.keyValue);
        const auto candidateKey = pTrack->getKey();
        if (candidateKey != mixxx::track::io::key::INVALID) {
            // Note: shortestStepsToCompatibleKey() is only defined for
            // steps in the range -5 .. +5 (compatible keys on the
            // circle of fifths). For maximally distant keys it can
            // return -6, so only the magnitude is meaningful here:
            // 0 steps is a perfect match, 6 steps is the worst case.
            const int steps = std::abs(KeyUtils::shortestStepsToCompatibleKey(
                    candidateKey, referenceKey));
            const double keySimilarity = clamp01(1.0 - steps / 6.0);
            weightedSum += weights.key * keySimilarity;
            weightSum += weights.key;
        }
    }

    const double fameSimilarity = similarityFromDistance(
            autoRecommendationFameDistance(
                    pTrack->getTimesPlayed(), reference),
            kMaxFameDistance);
    if (fameSimilarity >= 0.0 && weights.fame > 0.0) {
        weightedSum += weights.fame * fameSimilarity;
        weightSum += weights.fame;
    }

    if (weightSum <= 0.0) {
        return 0.0;
    }
    return clamp01(weightedSum / weightSum);
}

QList<TrackId> autoRecommendationSelect(
        const QList<TrackPointer>& candidates,
        const TrackPointer& pReferenceTrack,
        const QSet<TrackId>& excludeTrackIds,
        const AutoRecommendationWeights& weights,
        int maxResults,
        bool transitionAware) {
    QList<TrackId> selectedTrackIds;
    if (maxResults <= 0) {
        return selectedTrackIds;
    }

    AutoRecommendationReference reference;
    if (pReferenceTrack) {
        reference.bpm = pReferenceTrack->getBpm();
        const auto referenceReplayGain = pReferenceTrack->getReplayGain();
        reference.replayGainRatio = referenceReplayGain.hasRatio()
                ? referenceReplayGain.getRatio()
                : -1.0;
        reference.timesPlayed = pReferenceTrack->getTimesPlayed();
        reference.keyValue = static_cast<int>(pReferenceTrack->getKey());
    }

    struct ScoredCandidate {
        TrackId trackId;
        TrackPointer pTrack;
        double score;
    };
    QList<ScoredCandidate> scoredCandidates;

    for (const TrackPointer& pTrack : candidates) {
        if (!pTrack) {
            continue;
        }
        const TrackId trackId = pTrack->getId();
        if (!trackId.isValid()) {
            continue;
        }
        if (pReferenceTrack && pReferenceTrack->getId().isValid() &&
                trackId == pReferenceTrack->getId()) {
            continue;
        }
        if (excludeTrackIds.contains(trackId)) {
            continue;
        }
        scoredCandidates.append(
                {trackId, pTrack, autoRecommendationScore(pTrack, reference, weights)});
    }

    // Sort from best to worst score. The sort is stable to keep the
    // order of equally scored candidates deterministic.
    std::stable_sort(
            scoredCandidates.begin(),
            scoredCandidates.end(),
            [](const ScoredCandidate& a, const ScoredCandidate& b) {
                return a.score > b.score;
            });

    QList<ScoredCandidate> selected;
    selected.reserve(static_cast<qsizetype>(maxResults));
    for (const ScoredCandidate& scoredCandidate : scoredCandidates) {
        if (selected.size() >= maxResults) {
            break;
        }
        selected.append(scoredCandidate);
    }

    if (transitionAware && selected.size() > 1) {
        // Fade/transition awareness mode: re-sequence the selected
        // tracks so consecutive queue entries transition smoothly.
        // Starting from the reference track, greedily pick the
        // remaining candidate whose blend of match score and
        // transition quality (BPM compatibility with the previous
        // entry, fade-window adequacy of its own duration) is highest.
        // The selected set does not change, only its order. Ties keep
        // the higher-scoring, earlier candidate, so the result stays
        // deterministic.
        QList<ScoredCandidate> remaining = selected;
        selected.clear();
        selected.reserve(remaining.size());
        double currentBpm = pReferenceTrack ? pReferenceTrack->getBpm() : 0.0;
        while (!remaining.isEmpty()) {
            qsizetype bestIndex = 0;
            double bestComposite = -2.0;
            for (qsizetype i = 0; i < remaining.size(); ++i) {
                const ScoredCandidate& candidate = remaining.at(i);
                const double transitionScore =
                        autoRecommendationTransitionScore(currentBpm,
                                candidate.pTrack->getBpm(),
                                candidate.pTrack->getDuration());
                // Without transition information the match score alone
                // ranks the candidate.
                const double composite = transitionScore < 0.0
                        ? candidate.score
                        : (candidate.score + transitionScore) / 2.0;
                if (composite > bestComposite) {
                    bestComposite = composite;
                    bestIndex = i;
                }
            }
            const ScoredCandidate best = remaining.takeAt(bestIndex);
            // Unknown tempos keep the last known tempo as the anchor
            // for the following transition.
            if (best.pTrack->getBpm() > 0.0) {
                currentBpm = best.pTrack->getBpm();
            }
            selected.append(best);
        }
    }

    selectedTrackIds.reserve(selected.size());
    for (const ScoredCandidate& scoredCandidate : selected) {
        selectedTrackIds.append(scoredCandidate.trackId);
    }
    return selectedTrackIds;
}

} // namespace mixxx
