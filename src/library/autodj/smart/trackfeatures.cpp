#include "library/autodj/smart/trackfeatures.h"

#include <cmath>

#include "track/keyutils.h"
#include "track/replaygain.h"
#include "track/track.h"

namespace {
// AnalyzerEbur128 stores ReplayGain 2.0 as: gain_dB = -18 LUFS - measured LUFS.
constexpr double kReplayGain2ReferenceLUFS = -18.0;
} // namespace

// static
int TrackFeatures::camelotFromOpenKey(int openKeyNumber) {
    if (openKeyNumber < 1 || openKeyNumber > 12) {
        return 0;
    }
    return ((openKeyNumber + 6) % 12) + 1;
}

// static
TrackFeatures TrackFeatures::fromTrack(const TrackPointer& pTrack) {
    TrackFeatures f;
    if (!pTrack) {
        return f;
    }
    f.id = pTrack->getId();
    f.bpm = pTrack->getBpm();
    f.durationSec = pTrack->getDuration();
    f.displayName = pTrack->getInfo();
    f.artist = pTrack->getArtist();
    f.title = pTrack->getTitle();
    f.genre = pTrack->getGenre();
    f.isVideo = isVideoFile(pTrack->getLocation());

    const auto key = pTrack->getKey();
    if (key != mixxx::track::io::key::INVALID &&
            mixxx::track::io::key::ChromaticKey_IsValid(key)) {
        f.camelotNumber = camelotFromOpenKey(KeyUtils::keyToOpenKeyNumber(key));
        f.camelotMinor = !KeyUtils::keyIsMajor(key);
    }

    const mixxx::ReplayGain replayGain = pTrack->getReplayGain();
    if (replayGain.hasRatio() && replayGain.getRatio() > 0.0) {
        const double gainDb = 20.0 * std::log10(replayGain.getRatio());
        f.loudnessLufs = kReplayGain2ReferenceLUFS - gainDb;
    }

#ifdef __STEM__
    f.isStem = !pTrack->getStemInfo().isEmpty();
#endif

    // TODO(autodj-2): read energy once AnalyzerEnergy stores it on Track.
    return f;
}
