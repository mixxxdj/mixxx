#include "metadata/trackmetadata.h"

#include "util/time.h"

namespace Mixxx {

/*static*/ const double TrackMetadata::kBpmUndefined = 0.0;
/*static*/ const double TrackMetadata::kBpmMin = 0.0; // lower bound (exclusive)
/*static*/ const double TrackMetadata::kBpmMax = 300.0; // upper bound (inclusive)

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
        if (isBpmValid(bpm)) {
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
    if (isBpmValid(bpm)) {
        return QString::number(bpm);
    } else {
        return QString();
    }
}

QString TrackMetadata::formatBpm(int bpm) {
    if (isBpmValid(bpm)) {
        return QString::number(bpm);
    } else {
        return QString();
    }
}

double TrackMetadata::normalizeBpm(double bpm) {
    if (isBpmValid(bpm)) {
        const double normalizedBpm = parseBpm(formatBpm(bpm));
        // NOTE(uklotzde): Subsequently formatting and parsing the
        // normalized value should not alter it anymore!
        DEBUG_ASSERT(normalizedBpm == parseBpm(formatBpm(normalizedBpm)));
        return normalizedBpm;
    } else {
        return bpm;
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

QString TrackMetadata::formatDuration(int duration) {
    return Time::formatSeconds(duration, false);
}

TrackMetadata::TrackMetadata()
    : m_bpm(kBpmUndefined),
      m_bitrate(0),
      m_channels(0),
      m_duration(0),
      m_sampleRate(0) {
}

bool operator==(const TrackMetadata& lhs, const TrackMetadata& rhs) {
    return (lhs.getArtist() == rhs.getArtist()) &&
            (lhs.getTitle() == rhs.getTitle()) &&
            (lhs.getAlbum() == rhs.getAlbum()) &&
            (lhs.getAlbumArtist() == rhs.getAlbumArtist()) &&
            (lhs.getGenre() == rhs.getGenre()) &&
            (lhs.getComment() == rhs.getComment()) &&
            (lhs.getYear() == rhs.getYear()) &&
            (lhs.getTrackNumber() == rhs.getTrackNumber()) &&
            (lhs.getComposer() == rhs.getComposer()) &&
            (lhs.getGrouping() == rhs.getGrouping()) &&
            (lhs.getKey() == rhs.getKey()) &&
            (lhs.getChannels() == rhs.getChannels()) &&
            (lhs.getSampleRate() == rhs.getSampleRate()) &&
            (lhs.getBitrate() == rhs.getBitrate()) &&
            (lhs.getDuration() == rhs.getDuration()) &&
            (lhs.getBpm() == rhs.getBpm()) &&
            (lhs.getReplayGain() == rhs.getReplayGain());
}

} //namespace Mixxx
