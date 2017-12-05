#include "track/trackmetadata.h"

namespace mixxx {

/*static*/ constexpr int TrackMetadata::kCalendarYearInvalid;

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

bool operator==(const TrackMetadata& lhs, const TrackMetadata& rhs) {
    return (lhs.getAlbumInfo() == rhs.getAlbumInfo()) &&
            (lhs.getTrackInfo() == rhs.getTrackInfo()) &&
            (lhs.getBitrate() == rhs.getBitrate()) &&
            (lhs.getChannels() == rhs.getChannels()) &&
            (lhs.getDuration() == rhs.getDuration()) &&
            (lhs.getSampleRate() == rhs.getSampleRate());
}

QDebug operator<<(QDebug dbg, const TrackMetadata& arg) {
    dbg << '{';
    arg.dbgTrackInfo(dbg);
    arg.dbgAlbumInfo(dbg);
    arg.dbgBitrate(dbg);
    arg.dbgChannels(dbg);
    arg.dbgDuration(dbg);
    arg.dbgSampleRate(dbg);
    dbg << '}';
    return dbg;
}

} //namespace mixxx
