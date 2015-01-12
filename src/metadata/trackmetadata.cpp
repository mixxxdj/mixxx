#include "metadata/trackmetadata.h"

#include "util/math.h"

namespace Mixxx {

/*static*/ const double TrackMetadata::BPM_UNDEFINED = 0.0;
/*static*/ const double TrackMetadata::BPM_MIN = 0.0; // exclusive lower bound
/*static*/ const double TrackMetadata::BPM_MAX = 300.0; // inclusive upper bound

/*static*/ const double TrackMetadata::REPLAYGAIN_UNDEFINED = 0.0f;
/*static*/ const double TrackMetadata::REPLAYGAIN_MIN = 0.0f; // exclusive lower bound
/*static*/ const double TrackMetadata::REPLAYGAIN_0DB = 1.0f;

TrackMetadata::TrackMetadata()
        : m_channels(0), m_sampleRate(0), m_bitrate(0), m_duration(0), m_bpm(BPM_UNDEFINED), m_replayGain(REPLAYGAIN_UNDEFINED) {
}

double TrackMetadata::parseBpmString(const QString& sBpm) {
    if (sBpm.trimmed().isEmpty()) {
        return BPM_UNDEFINED;
    }
    bool bpmValid = false;
    double bpm = sBpm.toDouble(&bpmValid);
    if ((!bpmValid) || (BPM_MIN > bpm)) {
        qDebug() << "Failed to parse BPM:" << sBpm;
        return BPM_UNDEFINED;
    }
    while (bpm > BPM_MAX) {
        bpm /= 10.0;
    }
    return bpm;
}

bool TrackMetadata::setBpmString(const QString& sBpm) {
    const double bpm = parseBpmString(sBpm);
    if (BPM_UNDEFINED != bpm) {
        setBpm(parseBpmString(sBpm));
        return true;
    } else {
        return false;
    }
}

float TrackMetadata::parseReplayGainDbString(QString sReplayGainDb) {
    sReplayGainDb.remove("dB");
    bool replayGainDbValid = false;
    const double replayGainDb = sReplayGainDb.toDouble(&replayGainDbValid);
    if (!replayGainDbValid) {
        return REPLAYGAIN_UNDEFINED;
    }
    const float replayGain = db2ratio(replayGainDb);
    if (REPLAYGAIN_MIN > replayGain) {
        return REPLAYGAIN_UNDEFINED;
    }
    // I found some mp3s of mine with replaygain tag set to 0dB even if not normalized.
    // This is because of Rapid Evolution 3, I suppose. I prefer to rescan them by setting value to 0 (i.e. rescan via analyserrg)
    if (REPLAYGAIN_0DB == replayGain) {
        return REPLAYGAIN_UNDEFINED;
    }
    return replayGain;
}

bool TrackMetadata::setReplayGainDbString(QString sReplayGainDb) {
    const float replayGain = parseReplayGainDbString(sReplayGainDb);
    if (REPLAYGAIN_UNDEFINED != replayGain) {
        setReplayGain(replayGain);
        return true;
    } else {
        return false;
    }
}

} //namespace Mixxx
