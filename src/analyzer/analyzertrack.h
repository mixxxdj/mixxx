#pragma once

#include <optional>

#include "track/track_decl.h"

/// A scheduled not-null track with additional options for analysis.
class AnalyzerTrack {
  public:
    struct Options {
        Options()
                : fingerprintOnly(false) {
        }
        /// If set, overrides whether the analysis should assume constant BPM.
        std::optional<bool> useFixedTempo;
        /// If true, run only fingerprint analysis — skip beats, waveform, and
        /// all other analyzers. Useful for back-filling fingerprints on a
        /// library that was already fully analyzed without this step.
        bool fingerprintOnly;
    };

    explicit AnalyzerTrack(TrackPointer track, Options options = Options());

    /// Fetches the (not-null) track to be analyzed.
    const TrackPointer& getTrack() const;

    /// Fetches the additional options.
    const Options& getOptions() const;

  private:
    /// The (not-null) track to be analyzed.
    TrackPointer m_track;
    /// The additional options.
    Options m_options;
};
