#pragma once

#include <QDateTime>

#include "track/albuminfo.h"
#include "track/trackinfo.h"


namespace mixxx {

class TrackMetadata final {
    PROPERTY_SET_BYVAL_GET_BYREF(AlbumInfo, albumInfo, AlbumInfo)
    PROPERTY_SET_BYVAL_GET_BYREF(TrackInfo, trackInfo, TrackInfo)

public:
    TrackMetadata() = default;
    TrackMetadata(TrackMetadata&&) = default;
    TrackMetadata(const TrackMetadata&) = default;
    /*non-virtual*/ ~TrackMetadata() = default;

    TrackMetadata& operator=(TrackMetadata&&) = default;
    TrackMetadata& operator=(const TrackMetadata&) = default;

    // Parse an format date/time values according to ISO 8601
    static QDate parseDate(QString str) {
        return QDate::fromString(str.trimmed().replace(" ", ""), Qt::ISODate);
    }
    static QDateTime parseDateTime(QString str) {
        return QDateTime::fromString(str.trimmed().replace(" ", ""), Qt::ISODate);
    }
    static QString formatDate(QDate date) {
        return date.toString(Qt::ISODate);
    }
    static QString formatDateTime(QDateTime dateTime) {
        return dateTime.toString(Qt::ISODate);
    }

    // Parse and format the calendar year (for simplified display)
    static constexpr int kCalendarYearInvalid = 0;
    static int parseCalendarYear(QString year, bool* pValid = nullptr);
    static QString formatCalendarYear(QString year, bool* pValid = nullptr);

    static QString reformatYear(QString year);
};

inline
bool operator==(const TrackMetadata& lhs, const TrackMetadata& rhs) {
    return (lhs.getAlbumInfo() == rhs.getAlbumInfo()) &&
            (lhs.getTrackInfo() == rhs.getTrackInfo());
}

inline
bool operator!=(const TrackMetadata& lhs, const TrackMetadata& rhs) {
    return !(lhs == rhs);
}

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::TrackMetadata)
