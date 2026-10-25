#pragma once

#include <QList>
#include <QSet>
#include <QString>
#include <QVector>
#include <optional>

#include "library/autodj/smart/mixscorer.h"

/// A library track (or two) that could be played between two tracks that
/// clash.
struct BridgeSuggestion {
    TrackFeatures track;  ///< the bridge (the first of two for a pair)
    TrackFeatures second; ///< the second track of a two-track bridge
    bool isPair = false;
    MixScore in;  ///< from -> bridge
    MixScore out; ///< bridge -> to (for a pair: second -> to)
    double cost = 0.0;
};

/// Auto DJ 2.0 plus Video Mixing: finds "bridge" tracks. For a clashing pair A -> B it looks
/// for library tracks X where both A -> X and X -> B mix smoothly (no key or
/// tempo clash), best first. Pure logic, safe on a worker thread.
/// Live Assistant: one suggested next track.
struct NextSuggestion {
    TrackFeatures track;
    MixScore score; ///< now -> this track
    double cost = 0.0; ///< score + genre cost, lower is better
};

class BridgeFinder {
  public:
    explicit BridgeFinder(const MixScorer& scorer);

    /// @param excludeIds tracks that must not be suggested (already queued)
    /// @param excludeNames lower-case "artist - title" to skip, so another
    ///        copy of a queued song is not suggested either
    QList<BridgeSuggestion> find(const TrackFeatures& from,
            const TrackFeatures& to,
            const QVector<TrackFeatures>& candidates,
            const QSet<TrackId>& excludeIds,
            const QSet<QString>& excludeNames,
            int maxResults = 3) const;

    /// Two-track bridges, for gaps no single track can bridge (e.g. a tempo
    /// jump of more than about 10%: each step may be at most 5%). Every one
    /// of the three mixes (from -> first -> second -> to) must be smooth.
    QList<BridgeSuggestion> findPairs(const TrackFeatures& from,
            const TrackFeatures& to,
            const QVector<TrackFeatures>& candidates,
            const QSet<TrackId>& excludeIds,
            const QSet<QString>& excludeNames,
            int maxResults = 3) const;

    /// Extra cost for a bridge that is not a music video next to one that
    /// is (in a video set the screen would show cover art instead).
    static constexpr double kNoVideoBridgeCost = 2.0;
    /// Extra cost for a track whose beat grid drifts (cannot be beatmatched:
    /// a quick switch instead of a mix) in Smart Fill and the Live
    /// Assistant. Such tracks are never suggested as bridges.
    static constexpr double kUnsteadyGridCost = 1.5;

    /// Smart Fill: a chain of up to `count` library tracks to play after
    /// `last`, each the smoothest next mix from the one before (no key or
    /// tempo clash). Stops early when nothing mixes smoothly any more.
    /// @param avoidSameArtist never two tracks by the same artist in a row
    ///        (the energy direction comes from the scorer's weights)
    /// @param randomSeed 0 = always the smoothest next track (tests); any
    ///        other value = a random pick among the few that mix nearly as
    ///        well, so every fill is different
    /// Staying in one key for more than two tracks costs extra, so the set
    /// moves around the Camelot wheel instead of sitting in one key.
    QList<TrackFeatures> extend(const TrackFeatures& last,
            const QVector<TrackFeatures>& candidates,
            QSet<TrackId> excludeIds,
            QSet<QString> excludeNames,
            int count,
            bool avoidSameArtist = false,
            quint32 randomSeed = 0) const;

    /// Live Assistant: the best `count` tracks to play after `now`, best
    /// first. Only smooth mixes (no key or tempo clash); ranked by the mix
    /// score (key, tempo, energy direction) plus the genre cost. No
    /// randomness: the same situation gives the same list.
    QList<NextSuggestion> suggestNext(const TrackFeatures& now,
            const QVector<TrackFeatures>& candidates,
            const QSet<TrackId>& excludeIds,
            const QSet<QString>& excludeNames,
            int count,
            bool avoidSameArtist) const;

    /// How well two genres go together, as an extra mixing cost:
    /// 0 = same genre or family (e.g. "Goth" and "Gothic Rock"),
    /// kRelatedGenreCost = neighbouring families (e.g. Goth and Industrial),
    /// kUnknownGenreCost = one of them untagged ("", "Other", "Unknown"),
    /// kOtherGenreCost = unrelated (e.g. Goth and Hip Hop).
    static double genreCost(const QString& genreA, const QString& genreB);
    static constexpr double kRelatedGenreCost = 0.75;
    static constexpr double kUnknownGenreCost = 0.5;
    static constexpr double kOtherGenreCost = 2.0;

    /// Extra cost for a third track in a row in the same key.
    static constexpr double kSameKeyRunCost = 1.5;
    /// Random picks come from tracks within this cost of the best one...
    static constexpr double kRandomCostMargin = 1.0;
    /// ...and at most this many of them.
    static constexpr int kRandomPoolSize = 6;

    /// Smart Fill with nothing to start from: an opening track. To build
    /// energy, one of the calmest quarter (by known energy) so the set has
    /// room to rise; otherwise any track. `randomValue` picks among them
    /// (pass a random number; a fixed one gives a fixed answer in tests).
    /// Only tracks with key and BPM. nullopt if there is none.
    static std::optional<TrackFeatures> pickStart(const QVector<TrackFeatures>& candidates,
            const QSet<TrackId>& excludeIds,
            const QSet<QString>& excludeNames,
            bool calm,
            quint32 randomValue);

    /// The same SONG gives the same key, whatever the version and whoever
    /// performs it (covers count as the same song, the DJ's choice): the
    /// TITLE in lower case, without a leading "The", punctuation, bracketed
    /// tags ("(12\" Version)", "[Remastered]"), a " - ... Remix / Edit /
    /// Version" ending, "feat. X", a year tag ("'90") or the artist's own
    /// name in front. "Dio - Rainbow in the Dark" and "Computer Club -
    /// Rainbow in the Dark (Extended)" match. Falls back to the display name
    /// when the title is not tagged ("Artist - Title", or "Title by Artist").
    static QString nameKey(const TrackFeatures& track);
    /// The artist, tidied the same way; empty if unknown.
    static QString artistKey(const TrackFeatures& track);
    static QString normalizeArtist(const QString& artist);
    static QString normalizeTitle(const QString& title, const QString& normalizedArtist);

  private:
    MixScorer m_scorer;
};
