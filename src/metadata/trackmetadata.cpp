#include "metadata/trackmetadata.h"

#include "util/math.h"

namespace Mixxx {

namespace {
const float BPM_ZERO = 0.0f;
const float BPM_MAX = 300.0f;

float parseBpmString(const QString& sBpm) {
    float bpm = sBpm.toFloat();
    while (bpm > BPM_MAX) {
        bpm /= 10.0f;
    }
    return bpm;
}

float parseReplayGainString(QString sReplayGain) {
    QString ReplayGainstring = sReplayGain.remove(" dB");
    float fReplayGain = db2ratio(ReplayGainstring.toFloat());
    // I found some mp3s of mine with replaygain tag set to 0dB even if not normalized.
    // This is because of Rapid Evolution 3, I suppose. I prefer to rescan them by setting value to 0 (i.e. rescan via analyserrg)
    if (fReplayGain == 1.0f) {
        fReplayGain = 0.0f;
    }
    return fReplayGain;
}

}

TrackMetadata::TrackMetadata()
        : m_channels(0), m_sampleRate(0), m_bitrate(0), m_duration(0), m_replayGain(0.0f), m_bpm(BPM_ZERO) {
}

void TrackMetadata::setBpmString(QString sBpm) {
    if (!sBpm.isEmpty()) {
        float fBpm = parseBpmString(sBpm);
        if (BPM_ZERO < fBpm) {
            setBpm(fBpm);
        }
    }
}

void TrackMetadata::setReplayGainString(QString sReplayGain) {
    setReplayGain(parseReplayGainString(sReplayGain));
}

} //namespace Mixxx
