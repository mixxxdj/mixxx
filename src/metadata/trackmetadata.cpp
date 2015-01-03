#include "metadata/trackmetadata.h"

#include "util/math.h"

namespace Mixxx {

TrackMetadata::TrackMetadata()
        : m_channels(0), m_sampleRate(0), m_bitrate(0), m_duration(0), m_bpm(BPM_UNDEFINED), m_replayGain(REPLAYGAIN_UNDEFINED) {
}

double TrackMetadata::parseBpmString(const QString& sBpm) {
    bool bpmValid = false;
    double bpm = sBpm.toDouble(&bpmValid);
    if ((!bpmValid) || (BPM_MIN > bpm)) {
        return BPM_UNDEFINED;
    }
    while (bpm > BPM_MAX) {
        bpm /= 10.0;
    }
    return bpm;
}

float TrackMetadata::parseReplayGainDbString(QString sReplayGainDb) {
    sReplayGainDb.remove("dB");
    bool replayGainDbValid = false;
    const double replayGainDb = sReplayGainDb.toDouble(&replayGainDbValid);
    if (!replayGainDbValid) {
        return REPLAYGAIN_UNDEFINED;
    }
    float replayGain = db2ratio(replayGainDb);
    if (REPLAYGAIN_MIN > replayGain) {
        return REPLAYGAIN_UNDEFINED;
    }
    // I found some mp3s of mine with replaygain tag set to 0dB even if not normalized.
    // This is because of Rapid Evolution 3, I suppose. I prefer to rescan them by setting value to 0 (i.e. rescan via analyserrg)
    if (replayGain == REPLAYGAIN_0DB) {
        replayGain = REPLAYGAIN_UNDEFINED;
    }
    return replayGain;
}

} //namespace Mixxx
