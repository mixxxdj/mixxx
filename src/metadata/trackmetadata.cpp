#include "metadata/trackmetadata.h"

#include "util/math.h"

namespace Mixxx {

/*static*/ const double TrackMetadata::BPM_UNDEFINED = 0.0;
/*static*/ const double TrackMetadata::BPM_MIN = 0.0; // lower bound (inclusive)
/*static*/ const double TrackMetadata::BPM_MAX = 300.0; // lower bound (inclusive)

/*static*/ const float TrackMetadata::REPLAYGAIN_UNDEFINED = 0.0f;
/*static*/ const float TrackMetadata::REPLAYGAIN_MIN = 0.0f; // lower bound (inclusive)
/*static*/ const float TrackMetadata::REPLAYGAIN_0DB = 1.0f;

namespace {

double parseBpmString(const QString& sBpm, bool* pValid = 0) {
    if (pValid) {
        *pValid = false;
    }
    if (sBpm.trimmed().isEmpty()) {
        return TrackMetadata::BPM_UNDEFINED;
    }
    bool bpmValid = false;
    double bpm = sBpm.toDouble(&bpmValid);
    if (bpmValid) {
        if (TrackMetadata::BPM_UNDEFINED == bpm) {
            // special case
            if (pValid) {
                *pValid = true;
            }
            return bpm;
        }
        while (TrackMetadata::BPM_MAX < bpm) {
            // TODO(XXX): Why?
            qDebug() << "Scaling BPM:" << bpm;
            bpm /= 10.0;
        }
        if (TrackMetadata::isBpmValid(bpm)) {
            if (pValid) {
                *pValid = true;
            }
            return bpm;
        } else {
            qDebug() << "BPM out of range:" << bpm;
        }
    } else {
        qDebug() << "Failed to parse BPM:" << sBpm;
    }
    return TrackMetadata::BPM_UNDEFINED;
}

float parseReplayGainDbString(QString sReplayGainDb, bool* pValid = 0) {
    if (pValid) {
        *pValid = false;
    }
    sReplayGainDb.remove("dB"); // TODO(XXX): Why?
    if (sReplayGainDb.trimmed().isEmpty()) {
        return TrackMetadata::REPLAYGAIN_UNDEFINED;
    }
    bool replayGainDbValid = false;
    const float replayGainDb = sReplayGainDb.toFloat(&replayGainDbValid);
    if (replayGainDbValid) {
        if (TrackMetadata::REPLAYGAIN_UNDEFINED == replayGainDb) {
            // special case
            if (pValid) {
                *pValid = true;
            }
            return replayGainDb;
        }
        // I found some mp3s of mine with replaygain tag set to 0dB even if not normalized.
        // This is because of Rapid Evolution 3, I suppose. I prefer to rescan them by
        // setting value to 0 (i.e. rescan via analyserrg)
        if (TrackMetadata::REPLAYGAIN_0DB == replayGainDb) {
            qDebug() << "Ignoring 0dB replay gain:" << replayGainDb;
            return TrackMetadata::REPLAYGAIN_UNDEFINED;
        }
        if (TrackMetadata::isReplayGainValid(replayGainDb)) {
            if (pValid) {
                *pValid = true;
            }
            return replayGainDb;
        } else {
            qDebug() << "Replay gain out of range:" << replayGainDb;
        }
    } else {
        qDebug() << "Failed to parse replay gain:" << sReplayGainDb;
    }
    return TrackMetadata::REPLAYGAIN_UNDEFINED;
}

}

TrackMetadata::TrackMetadata() :
        m_channels(0),
        m_sampleRate(0),
        m_bitrate(0),
        m_duration(0),
        m_bpm(BPM_UNDEFINED),
        m_replayGain(REPLAYGAIN_UNDEFINED) {
}

bool TrackMetadata::setBpmString(const QString& sBpm) {
    bool bpmValid;
    const double bpm = parseBpmString(sBpm, &bpmValid);
    if (bpmValid) {
        setBpm(bpm);
    }
    return bpmValid;
}

bool TrackMetadata::setReplayGainDbString(QString sReplayGainDb) {
    bool replayGainValid;
    const float replayGain = parseReplayGainDbString(sReplayGainDb, &replayGainValid);
    if (replayGainValid) {
        setReplayGain(replayGain);
    }
    return replayGainValid;
}

} //namespace Mixxx
