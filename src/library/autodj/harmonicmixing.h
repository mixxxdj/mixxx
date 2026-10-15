#pragma once

#include <QList>

#include "proto/keys.pb.h"

namespace mixxx {

/// Decides whether a candidate track mixes harmonically with a reference track.
///
/// Key compatibility follows the Circle of Fifths rules implemented by KeyUtils
/// (same key, relative major/minor and the perfect fourth/fifth). Tempo
/// compatibility is a relative tolerance around the reference tempo, so that
/// Auto DJ can pick the next track from the tracks that need only a small pitch
/// adjustment to be beat-matched.
class HarmonicMixing {
  public:
    /// Tempo tolerance used when the user has not configured one.
    static constexpr double kDefaultBpmTolerancePercent = 6.0;

    /// Absolute tempo window around a reference tempo.
    struct BpmRange {
        double min = 0.0;
        double max = 0.0;

        bool isValid() const {
            return min > 0.0 && max >= min;
        }
    };

    /// Returns the tempo windows around referenceBpm for a relative tolerance in
    /// percent. Returns an invalid range if referenceBpm or the tolerance is not
    /// positive.
    static BpmRange bpmRange(double referenceBpm, double tolerancePercent);

    /// Returns the tempo windows to try, in order, when looking for a track that
    /// mixes harmonically: the window for the configured tolerance first and a
    /// wider one after it. The last entry is invalid, which means "do not
    /// constrain the tempo at all", so that the caller can still mix by key when
    /// nothing fits the tempo window. Returns an empty list if the reference
    /// tempo is unknown, because then no window can be determined.
    static QList<BpmRange> bpmFallbackRanges(
            double referenceBpm, double tolerancePercent);

    /// Returns the keys that mix harmonically with referenceKey, including
    /// referenceKey itself. Returns an empty list for an invalid referenceKey.
    static QList<mixxx::track::io::key::ChromaticKey> compatibleKeys(
            mixxx::track::io::key::ChromaticKey referenceKey);

    /// Returns true if candidateBpm lies inside the tempo window around
    /// referenceBpm. Returns false if the window cannot be determined, so an
    /// unknown reference tempo never constrains the selection.
    static bool isBpmCompatible(
            double referenceBpm,
            double candidateBpm,
            double tolerancePercent);

    /// Returns true if candidateKey mixes harmonically with referenceKey.
    static bool isKeyCompatible(
            mixxx::track::io::key::ChromaticKey referenceKey,
            mixxx::track::io::key::ChromaticKey candidateKey);
};

} // namespace mixxx
