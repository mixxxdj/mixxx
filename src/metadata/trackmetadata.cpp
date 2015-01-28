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

const QString REPLAYGAIN_UNIT("dB");
const QString REPLAYGAIN_SUFFIX(" " + REPLAYGAIN_UNIT);

float parseReplayGainDbString(QString sReplayGainDb, bool* pValid = 0) {
    if (pValid) {
        *pValid = false;
    }
    QString normalizedReplayGainDb(sReplayGainDb.trimmed());
    const int plusIndex = normalizedReplayGainDb.indexOf('+');
    if (0 == plusIndex) {
        // strip leading "+"
        normalizedReplayGainDb = normalizedReplayGainDb.mid(plusIndex + 1).trimmed();
    }
    const int dbIndex = normalizedReplayGainDb.indexOf(REPLAYGAIN_UNIT, Qt::CaseInsensitive);
    if ((0 <= dbIndex) && ((normalizedReplayGainDb.length() - 2) == dbIndex)) {
        // strip trailing "db"
        normalizedReplayGainDb = normalizedReplayGainDb.left(dbIndex).trimmed();
    }
    if (normalizedReplayGainDb.isEmpty()) {
        return TrackMetadata::REPLAYGAIN_UNDEFINED;
    }
    bool replayGainDbValid = false;
    const double replayGainDb = normalizedReplayGainDb.toDouble(&replayGainDbValid);
    if (replayGainDbValid) {
        const float replayGain = db2ratio(replayGainDb);
        DEBUG_ASSERT(TrackMetadata::REPLAYGAIN_UNDEFINED != replayGain); // impossible
        // I found some mp3s of mine with replaygain tag set to 0dB even if not normalized.
        // This is because of Rapid Evolution 3, I suppose. I prefer to rescan them by
        // setting value to 0 (i.e. rescan via analyserrg)
        if (TrackMetadata::REPLAYGAIN_0DB == replayGain) {
            // special case
            qDebug() << "Ignoring 0dB replay gain:" << replayGainDb;
            if (pValid) {
                *pValid = true;
            }
            return TrackMetadata::REPLAYGAIN_UNDEFINED;
        }
        if (TrackMetadata::isReplayGainValid(replayGain)) {
            if (pValid) {
                *pValid = true;
            }
            return replayGain;
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

QString TrackMetadata::getBpmString() const {
    if (isBpmValid()) {
        return QString::number(getBpm(), 'f');
    } else {
        return QString();
    }
}

bool TrackMetadata::setReplayGainDbString(QString sReplayGainDb) {
    bool replayGainValid;
    const float replayGain = parseReplayGainDbString(sReplayGainDb, &replayGainValid);
    if (replayGainValid) {
        setReplayGain(replayGain);
    }
    return replayGainValid;
}

QString TrackMetadata::getReplayGainDbString() const {
    if (isReplayGainValid()) {
        return QString::number(ratio2db(getReplayGain()), 'f') + REPLAYGAIN_SUFFIX;
    } else {
        return QString();
    }
}

} //namespace Mixxx
