#pragma once

#include <optional>

#include "track/track_decl.h"

/// A schedulable track with additional options for analysis.
class AnalyzerTrack {
  public:
    struct Options {
        /// If set, overrides whether the analysis should assume constant BPM.
        std::optional<bool> useFixedTempo;
    };

    AnalyzerTrack(TrackPointer track, Options options);

    /// Fetches the track to be analyzed.
    const TrackPointer getTrack() const;

    /// Fetches the additional options.
    const Options getOptions() const;

  private:
    /// The track to be analyzed.
    const TrackPointer track;
    /// The additional options.
    const Options options;
};
