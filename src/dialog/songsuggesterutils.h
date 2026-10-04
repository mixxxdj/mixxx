#pragma once

#include <QList>
#include <QString>

#include "track/track_decl.h"
#include "track/trackid.h"

namespace mixxx {

enum class SongSuggesterEnergyLevel {
    Any,
    Low,
    Medium,
    High,
};

// Energy is estimated from ReplayGain loudness, which is stored as a
// linear ratio where 1.0 == 0 dB. A louder master requires negative gain
// (ratio < 1.0) for normalization and is treated as high energy.
inline constexpr double kSongSuggesterHighEnergyRatioMax = 0.8; // about -2 dB
inline constexpr double kSongSuggesterLowEnergyRatioMin = 1.25; // about +2 dB

SongSuggesterEnergyLevel songSuggesterEnergyLevelForRatio(double ratio);

enum class SongSuggesterFameLevel {
    Any,
    Underground,
    Known,
    Popular,
};

// Fame is approximated by the number of times a track has been played.
inline constexpr int kSongSuggesterKnownPlays = 5;
inline constexpr int kSongSuggesterPopularPlays = 50;

SongSuggesterFameLevel songSuggesterFameLevelForPlays(int plays);

// Maximum number of suggested tracks returned by
// filterAndRankSongSuggesterMatches().
inline constexpr int kSongSuggesterMaxResults = 100;

// Criteria for matching tracks against a reference track.
struct SongSuggesterCriteria final {
    // Target tempo in BPM.
    double targetBpm = 0.0;
    // Maximum permitted distance between targetBpm and the tempo of
    // a matching track (inclusive).
    double bpmTolerance = 0.0;
    // If not empty, only tracks whose language tag contains this text
    // (case-insensitive) match. Language tags are only available in
    // builds with __EXTRA_METADATA__; otherwise this criterion is
    // silently ignored.
    QString language;
    SongSuggesterEnergyLevel energy = SongSuggesterEnergyLevel::Any;
    SongSuggesterFameLevel fame = SongSuggesterFameLevel::Any;
};

// Returns the language tag of a track or an empty string if the track
// is null or if this build does not include __EXTRA_METADATA__ (the
// language property of TrackInfo is only available then).
QString songSuggesterTrackLanguage(const TrackPointer& pTrack);

// Returns the tracks from candidates that match the criteria, excluding
// the reference track (by track ID). The matches are stable-sorted by
// their distance from the target tempo with ties broken by play count
// (descending), and truncated to a maximum of kSongSuggesterMaxResults
// tracks.
QList<TrackPointer> filterAndRankSongSuggesterMatches(
        const QList<TrackPointer>& candidates,
        const TrackPointer& pReferenceTrack,
        const SongSuggesterCriteria& criteria);

} // namespace mixxx
