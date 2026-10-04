#include "dialog/songsuggesterutils.h"

#include <algorithm>
#include <cmath>

#include "track/track.h"

namespace mixxx {

SongSuggesterEnergyLevel songSuggesterEnergyLevelForRatio(double ratio) {
    if (ratio < kSongSuggesterHighEnergyRatioMax) {
        return SongSuggesterEnergyLevel::High;
    }
    if (ratio > kSongSuggesterLowEnergyRatioMin) {
        return SongSuggesterEnergyLevel::Low;
    }
    return SongSuggesterEnergyLevel::Medium;
}

SongSuggesterFameLevel songSuggesterFameLevelForPlays(int plays) {
    if (plays >= kSongSuggesterPopularPlays) {
        return SongSuggesterFameLevel::Popular;
    }
    if (plays >= kSongSuggesterKnownPlays) {
        return SongSuggesterFameLevel::Known;
    }
    return SongSuggesterFameLevel::Underground;
}

QString songSuggesterTrackLanguage(const TrackPointer& pTrack) {
    if (!pTrack) {
        return QString();
    }
#if defined(__EXTRA_METADATA__)
    return pTrack->getMetadata().getTrackInfo().getLanguage();
#else
    // The language property of TrackInfo is not available in this build.
    Q_UNUSED(pTrack);
    return QString();
#endif // __EXTRA_METADATA__
}

QList<TrackPointer> filterAndRankSongSuggesterMatches(
        const QList<TrackPointer>& candidates,
        const TrackPointer& pReferenceTrack,
        const SongSuggesterCriteria& criteria) {
    QList<TrackPointer> matches;
    const TrackId referenceTrackId = pReferenceTrack ? pReferenceTrack->getId() : TrackId();

    for (const TrackPointer& pTrack : candidates) {
        if (!pTrack) {
            continue;
        }
        if (referenceTrackId.isValid() && pTrack->getId() == referenceTrackId) {
            continue;
        }
        if (std::abs(pTrack->getBpm() - criteria.targetBpm) > criteria.bpmTolerance) {
            continue;
        }
        if (criteria.energy != SongSuggesterEnergyLevel::Any) {
            const auto replayGain = pTrack->getReplayGain();
            if (!replayGain.hasRatio() ||
                    songSuggesterEnergyLevelForRatio(replayGain.getRatio()) !=
                            criteria.energy) {
                continue;
            }
        }
        if (!criteria.language.isEmpty() &&
                !songSuggesterTrackLanguage(pTrack).contains(
                        criteria.language, Qt::CaseInsensitive)) {
            continue;
        }
        if (criteria.fame != SongSuggesterFameLevel::Any &&
                songSuggesterFameLevelForPlays(pTrack->getTimesPlayed()) !=
                        criteria.fame) {
            continue;
        }
        matches.append(pTrack);
    }

    std::stable_sort(
            matches.begin(),
            matches.end(),
            [&criteria](const TrackPointer& a, const TrackPointer& b) {
                const double distanceA = std::abs(a->getBpm() - criteria.targetBpm);
                const double distanceB = std::abs(b->getBpm() - criteria.targetBpm);
                if (distanceA != distanceB) {
                    return distanceA < distanceB;
                }
                return a->getTimesPlayed() > b->getTimesPlayed();
            });

    if (matches.size() > kSongSuggesterMaxResults) {
        matches.resize(kSongSuggesterMaxResults);
    }
    return matches;
}

} // namespace mixxx
