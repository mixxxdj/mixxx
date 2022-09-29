#pragma once

#include <optional>

#include "analyzer/analyzertrack.h"
#include "track/track_decl.h"
#include "track/trackid.h"

/// A track to be scheduled for analysis with additional options.
class AnalyzerScheduledTrack {
  public:
    AnalyzerScheduledTrack(TrackId trackId,
            AnalyzerTrack::Options options = AnalyzerTrack::Options());

    /// Fetches the id of the track to be analyzed.
    const TrackId getTrackId() const;

    /// Fetches the additional options.
    const AnalyzerTrack::Options getOptions() const;

  private:
    /// The id of the track to be analyzed.
    TrackId trackId;
    /// The additional options.
    AnalyzerTrack::Options options;
};
