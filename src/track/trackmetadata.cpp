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

TrackMetadata::TrackMetadata()
    : m_duration(0.0),
      m_bitrate(0),
      m_channels(0),
      m_sampleRate(0) {
}

bool operator==(const TrackMetadata& lhs, const TrackMetadata& rhs) {
    // Compare the integer and double fields 1st for maximum efficiency
    return (lhs.getBitrate() == rhs.getBitrate()) &&
            (lhs.getChannels() == rhs.getChannels()) &&
            (lhs.getSampleRate() == rhs.getSampleRate()) &&
            (lhs.getDuration() == rhs.getDuration()) &&
            (lhs.getArtist() == rhs.getArtist()) &&
            (lhs.getTitle() == rhs.getTitle()) &&
            (lhs.getAlbum() == rhs.getAlbum()) &&
            (lhs.getAlbumArtist() == rhs.getAlbumArtist()) &&
            (lhs.getGenre() == rhs.getGenre()) &&
            (lhs.getComment() == rhs.getComment()) &&
            (lhs.getYear() == rhs.getYear()) &&
            (lhs.getTrackNumber() == rhs.getTrackNumber()) &&
            (lhs.getTrackTotal() == rhs.getTrackTotal()) &&
            (lhs.getComposer() == rhs.getComposer()) &&
            (lhs.getGrouping() == rhs.getGrouping()) &&
            (lhs.getKey() == rhs.getKey()) &&
            (lhs.getBpm() == rhs.getBpm()) &&
            (lhs.getReplayGain() == rhs.getReplayGain());
}

} //namespace mixxx
