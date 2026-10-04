#pragma once

#include <QList>
#include <QSet>
#include <QString>

#include "track/track_decl.h"
#include "track/trackid.h"

namespace mixxx {

// Score weighting for the individual criteria of the auto
// recommendation algorithm. All weights are non-negative and the
// resulting track score is normalized to the range [0.0, 1.0].
struct AutoRecommendationWeights final {
    double tempo = 1.0;
    double energy = 1.0;
    double key = 1.0;
    double fame = 0.5;
};

// Reference values for scoring candidate tracks.
struct AutoRecommendationReference final {
    // Reference tempo in BPM.
    double bpm = 0.0;
    // Linear ReplayGain ratio of the reference track (1.0 == 0 dB),
    // or a value below 0 if unknown.
    double replayGainRatio = -1.0;
    // Number of times the reference track has been played.
    int timesPlayed = 0;
    // Numerical key value of the reference track, or 0 if unknown.
    int keyValue = 0;
};

// Returns the tempo distance between bpm and the reference tempo,
// considering half-time and double-time matches of the reference
// tempo. This prefers tracks that are mixable by tempo
// synchronization without ignoring tracks at half or double speed.
double autoRecommendationTempoDistance(
        double bpm,
        const AutoRecommendationReference& reference);

// Returns the loudness distance between a linear ReplayGain ratio and
// the reference ratio, or -1.0 if one of both is unknown.
double autoRecommendationLoudnessDistance(
        double replayGainRatio,
        const AutoRecommendationReference& reference);

// Returns the fame distance between two play counts. Famous tracks are
// usually less interesting for discovery than tracks from the same
// niche, so distances are compared on a logarithmic scale.
double autoRecommendationFameDistance(
        int timesPlayed,
        const AutoRecommendationReference& reference);

// Returns the relative tempo distance of bpmB from the anchor tempo
// bpmA for fade/transition sequencing, considering half-time and
// double-time matches (60 <-> 120 <-> 240 BPM are equally good
// transitions), or -1.0 if either tempo is unknown.
double autoRecommendationTransitionTempoDistance(double bpmA, double bpmB);

// Returns how well a track duration in seconds is suited for a fade
// transition in the range [0.0, 1.0]: durations up to the minimum
// fade window score proportionally, longer tracks score 1.0. Unknown
// durations (<= 0) return -1.0.
double autoRecommendationFadeScore(double durationSec);

// Returns the fade/transition quality in the range [0.0, 1.0] of
// placing candidateBpm/candidateDurationSec right after a track with
// previousBpm: blends the BPM compatibility of both tracks with the
// fade-window adequacy of the candidate's duration. Unknown criteria
// are skipped and the rest is re-normalized; returns -1.0 if nothing
// is known.
double autoRecommendationTransitionScore(
        double previousBpm,
        double candidateBpm,
        double candidateDurationSec);

// Returns a weighted overall score in the range [0.0, 1.0] for a
// candidate track. Higher is better. Unknown values are skipped and
// the remaining criteria are re-normalized, so tracks with missing
// analysis are not penalized.
double autoRecommendationScore(
        const TrackPointer& pTrack,
        const AutoRecommendationReference& reference,
        const AutoRecommendationWeights& weights);

// Selects the best matching tracks from candidates, excluding tracks
// that are already in the queue or in excludeTrackIds, the reference
// track itself, and null or ID-less candidates. The returned track IDs
// are ordered from best to worst score. maxResults limits the number
// of returned track IDs.
//
// With transitionAware enabled (fade/transition awareness mode) the
// selected set is re-sequenced for smooth fades: each entry is picked
// greedily by blending its match score with its transition quality to
// the previous entry (BPM compatibility including half/double time,
// plus the fade-window adequacy of its duration), starting from the
// reference track. The set of selected tracks does not change, only
// its order. Ties keep the higher-scoring, earlier candidate.
QList<TrackId> autoRecommendationSelect(
        const QList<TrackPointer>& candidates,
        const TrackPointer& pReferenceTrack,
        const QSet<TrackId>& excludeTrackIds,
        const AutoRecommendationWeights& weights,
        int maxResults,
        bool transitionAware = false);

} // namespace mixxx
