#include "metadata/trackmetadata.h"

#include "util/math.h"

namespace Mixxx {

/*static*/ const double TrackMetadata::kBpmUndefined = 0.0;
/*static*/ const double TrackMetadata::kBpmMin = 0.0; // lower bound (exclusive)
/*static*/ const double TrackMetadata::kBpmMax = 300.0; // upper bound (inclusive)

/*static*/ const double TrackMetadata::kReplayGainUndefined = 0.0;
/*static*/ const double TrackMetadata::kReplayGainMin = 0.0; // lower bound (inclusive)
/*static*/ const double TrackMetadata::kReplayGain0dB = 1.0;

/*static*/ const int TrackMetadata::kCalendarYearInvalid = 0;

double TrackMetadata::parseBpm(const QString& sBpm, bool* pValid) {
    if (pValid) {
        *pValid = false;
    }
    if (sBpm.trimmed().isEmpty()) {
        return kBpmUndefined;
    }
    bool bpmValid = false;
    double bpm = sBpm.toDouble(&bpmValid);
    if (bpmValid) {
        if (kBpmUndefined == bpm) {
            // special case
            if (pValid) {
                *pValid = true;
            }
            return bpm;
        }
        while (kBpmMax < bpm) {
            // Some applications might store the BPM as an
            // integer scaled by a factor of 10 or 100 to
            // preserve fractional digits.
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
    return kBpmUndefined;
}

QString TrackMetadata::formatBpm(double bpm) {
    if (TrackMetadata::isBpmValid(bpm)) {
        return QString::number(bpm);
    } else {
        return QString();
    }
}

QString TrackMetadata::formatBpm(int bpm) {
    if (TrackMetadata::isBpmValid(bpm)) {
        return QString::number(bpm);
    } else {
        return QString();
    }
}

namespace {

const QString kReplayGainUnit("dB");
const QString kReplayGainSuffix(" " + kReplayGainUnit);

} // anonymous namespace

double TrackMetadata::parseReplayGain(QString sReplayGain, bool* pValid) {
    if (pValid) {
        *pValid = false;
    }
    QString normalizedReplayGain(sReplayGain.trimmed());
    const int plusIndex = normalizedReplayGain.indexOf('+');
    if (0 == plusIndex) {
        // strip leading "+"
        normalizedReplayGain = normalizedReplayGain.mid(plusIndex + 1).trimmed();
    }
    const int unitIndex = normalizedReplayGain.lastIndexOf(kReplayGainUnit, -1, Qt::CaseInsensitive);
    if ((0 <= unitIndex) && ((normalizedReplayGain.length() - 2) == unitIndex)) {
        // strip trailing unit suffix
        normalizedReplayGain = normalizedReplayGain.left(unitIndex).trimmed();
    }
    if (normalizedReplayGain.isEmpty()) {
        return kReplayGainUndefined;
    }
    bool replayGainValid = false;
    const double replayGainDb = normalizedReplayGain.toDouble(&replayGainValid);
    if (replayGainValid) {
        const double replayGain = db2ratio(replayGainDb);
        DEBUG_ASSERT(kReplayGainUndefined != replayGain);
        // Some applications (e.g. Rapid Evolution 3) write a replay gain
        // of 0 dB even if the replay gain is undefined. To be safe we
        // ignore this special value and instead prefer to recalculate
        // the replay gain.
        if (kReplayGain0dB == replayGain) {
            // special case
            qDebug() << "Ignoring possibly undefined replay gain:" << formatReplayGain(replayGain);
            if (pValid) {
                *pValid = true;
            }
            return kReplayGainUndefined;
        }
        if (TrackMetadata::isReplayGainValid(replayGain)) {
            if (pValid) {
                *pValid = true;
            }
            return replayGain;
        } else {
            qDebug() << "Invalid replay gain value:" << sReplayGain << " -> "<< replayGain;
        }
    } else {
        qDebug() << "Failed to parse replay gain:" << sReplayGain;
    }
    return kReplayGainUndefined;
}

QString TrackMetadata::formatReplayGain(double replayGain) {
    if (isReplayGainValid(replayGain)) {
        return QString::number(ratio2db(replayGain)) + kReplayGainSuffix;
    } else {
        return QString();
    }
}

int TrackMetadata::parseCalendarYear(QString year, bool* pValid) {
    const QDateTime dateTime(parseDateTime(year));
    if (0 < dateTime.date().year()) {
        if (pValid) {
            *pValid = true;
        }
        return dateTime.date().year();
    } else {
        bool calendarYearValid = false;
        // Ignore everything beginning with the first dash '-'
        // to successfully parse the calendar year of incomplete
        // dates like yyyy-MM or 2015-W07.
        const QString calendarYearSection(year.section('-', 0, 0).trimmed());
        const int calendarYear = calendarYearSection.toInt(&calendarYearValid);
        if (calendarYearValid) {
            calendarYearValid = 0 < calendarYear;
        }
        if (pValid) {
            *pValid = calendarYearValid;
        }
        if (calendarYearValid) {
            return calendarYear;
        } else {
            return kCalendarYearInvalid;
        }
    }
}

QString TrackMetadata::formatCalendarYear(QString year, bool* pValid) {
    bool calendarYearValid = false;
    int calendarYear = parseCalendarYear(year, &calendarYearValid);
    if (pValid) {
        *pValid = calendarYearValid;
    }
    if (calendarYearValid) {
        return QString::number(calendarYear);
    } else {
        return QString(); // empty string
    }
}

QString TrackMetadata::reformatYear(QString year) {
    const QDateTime dateTime(parseDateTime(year));
    if (dateTime.isValid()) {
        // date/time
        return formatDateTime(dateTime);
    }
    const QDate date(dateTime.date());
    if (date.isValid()) {
        // only date
        return formatDate(date);
    }
    bool calendarYearValid = false;
    const QString calendarYear(formatCalendarYear(year, &calendarYearValid));
    if (calendarYearValid) {
        // only calendar year
        return calendarYear;
    }
    // just trim and simplify whitespaces
    return year.simplified();
}

TrackMetadata::TrackMetadata() :
        m_channels(0),
        m_sampleRate(0),
        m_bitrate(0),
        m_duration(0),
        m_bpm(kBpmUndefined),
        m_replayGain(kReplayGainUndefined) {
}

} //namespace Mixxx
