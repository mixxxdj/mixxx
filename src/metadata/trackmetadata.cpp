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

QString formatBpm(double bpm) {
    if (TrackMetadata::isBpmValid(bpm)) {
        return QString::number(bpm, 'f');
    } else {
        return QString();
    }
}

double parseBpm(const QString& sBpm, bool* pValid = 0) {
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
            qDebug() << "Scaling BPM value:" << bpm;
            bpm /= 10.0;
        }
        if (TrackMetadata::isBpmValid(bpm)) {
            if (pValid) {
                *pValid = true;
            }
            return bpm;
        } else {
            qDebug() << "Invalid BPM value:" << sBpm << "->" << bpm;
        }
    } else {
        qDebug() << "Failed to parse BPM:" << sBpm;
    }
    return TrackMetadata::BPM_UNDEFINED;
}

const QString REPLAYGAIN_UNIT("dB");
const QString REPLAYGAIN_SUFFIX(" " + REPLAYGAIN_UNIT);

QString formatReplayGainDb(double replayGain) {
    if (TrackMetadata::isReplayGainValid(replayGain)) {
        return QString::number(ratio2db(replayGain), 'f') + REPLAYGAIN_SUFFIX;
    } else {
        return QString();
    }
}

double parseReplayGainDb(QString sReplayGainDb, bool* pValid = 0) {
    if (pValid) {
        *pValid = false;
    }
    QString normalizedReplayGainDb(sReplayGainDb.trimmed());
    const int plusIndex = normalizedReplayGainDb.indexOf('+');
    if (0 == plusIndex) {
        // strip leading "+"
        normalizedReplayGainDb = normalizedReplayGainDb.mid(plusIndex + 1).trimmed();
    }
    const int unitIndex = normalizedReplayGainDb.lastIndexOf(REPLAYGAIN_UNIT, -1, Qt::CaseInsensitive);
    if ((0 <= unitIndex) && ((normalizedReplayGainDb.length() - 2) == unitIndex)) {
        // strip trailing unit suffix
        normalizedReplayGainDb = normalizedReplayGainDb.left(unitIndex).trimmed();
    }
    if (normalizedReplayGainDb.isEmpty()) {
        return TrackMetadata::REPLAYGAIN_UNDEFINED;
    }
    bool replayGainDbValid = false;
    const double replayGainDb = normalizedReplayGainDb.toDouble(&replayGainDbValid);
    if (replayGainDbValid) {
        const double replayGain = db2ratio(replayGainDb);
        DEBUG_ASSERT(TrackMetadata::REPLAYGAIN_UNDEFINED != replayGain); // impossible
        // I found some mp3s of mine with replaygain tag set to 0dB even if not normalized.
        // This is because of Rapid Evolution 3, I suppose. I prefer to rescan them by
        // setting value to 0 (i.e. rescan via analyserrg)
        if (TrackMetadata::REPLAYGAIN_0DB == replayGain) {
            // special case
            qDebug() << "Ignoring replay gain:" << formatReplayGainDb(replayGain);
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
            qDebug() << "Invalid replay gain value:" << sReplayGainDb << " -> "<< replayGain;
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
    const double bpm = parseBpm(sBpm, &bpmValid);
    if (bpmValid) {
        setBpm(bpm);
    }
    return bpmValid;
}

QString TrackMetadata::getBpmString() const {
    return formatBpm(getBpm());
}

bool TrackMetadata::setReplayGainDbString(QString sReplayGainDb) {
    bool replayGainValid;
    const float replayGain = parseReplayGainDb(sReplayGainDb, &replayGainValid);
    if (replayGainValid) {
        setReplayGain(replayGain);
    }
    return replayGainValid;
}

QString TrackMetadata::getReplayGainDbString() const {
    return formatReplayGainDb(getReplayGain());
}

} //namespace Mixxx
