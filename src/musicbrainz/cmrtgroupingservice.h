#pragma once

#include "library/dao/trackfingerprintdao.h"
#include "musicbrainz/fingerprintmatcher.h"

namespace mixxx {

/// Runs the MBID -> SimHash -> overlap -> quality check -> group-assignment
/// pipeline for a single track, after its MBID has been written by
/// AcoustIdWorker.
///
/// Thread-safety: all methods must be called from the AcoustIdWorker thread.
/// m_fingerprintDao is the worker's thread-local DB connection, passed in by
/// reference -- TrackDAO is intentionally absent here, since it belongs to
/// the main thread and calling it from this thread would be the exact data
/// race that AcoustIdWorker::acoustidResultReady uses Qt::QueuedConnection
/// to avoid.
class CmrtGroupingService {
  public:
    explicit CmrtGroupingService(TrackFingerprintDao& fingerprintDao);

    void processTrack(TrackId trackId, const QString& musicbrainzRecordingId);

  private:
    void assignToExistingGroup(TrackId trackId,
            int groupId,
            double offsetSeconds,
            double qualityScore,
            double matchScore);

    void createNewGroup(TrackId trackId, const FingerprintMetadata& fpRow, double qualityScore);

    void handleMatchedCandidate(TrackId newTrackId,
            const MbidGroupCandidate& candidate,
            const FingerprintMatcher::MatchResult& matchResult,
            double newTrackQualityScore);

    void replaceCanonical(int groupId,
            TrackId oldCanonicalId,
            TrackId newCanonicalId,
            double offsetOfNewFromOld,
            double newCanonicalQualityScore,
            double matchScoreOfNewFromOld);

    TrackFingerprintDao& m_fingerprintDao; // not owned; lives on worker thread
};

} // namespace mixxx
