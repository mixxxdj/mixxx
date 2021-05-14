#pragma once

#include <QDateTime>

#include "audio/streaminfo.h"
#include "track/albuminfo.h"
#include "track/trackinfo.h"

namespace mixxx {

class TrackMetadata final {
    // Audio properties
    //  - read-only
    //  - stored in file tags
    //  - adjusted when opening the audio stream (if available)
    MIXXX_DECL_PROPERTY(audio::StreamInfo, streamInfo, StreamInfo)

    // Track properties
    //   - read-write
    //   - stored in file tags
    MIXXX_DECL_PROPERTY(AlbumInfo, albumInfo, AlbumInfo)
    MIXXX_DECL_PROPERTY(TrackInfo, trackInfo, TrackInfo)

  public:
    TrackMetadata() = default;
    TrackMetadata(TrackMetadata&&) = default;
    TrackMetadata(const TrackMetadata&) = default;
    /*non-virtual*/ ~TrackMetadata() = default;

    TrackMetadata& operator=(TrackMetadata&&) = default;
    TrackMetadata& operator=(const TrackMetadata&) = default;

    bool updateStreamInfoFromSource(
            const audio::StreamInfo& streamInfo);

    // Adjusts floating-point values to match their string representation
    // in file tags to account for rounding errors.
    void normalizeBeforeExport();

    // Returns true if the current metadata differs from the imported metadata
    // and needs to be exported. A result of false indicates that no export
    // is needed.
    // NOTE: Some tag formats like ID3v1/v2 only support integer precision
    // for storing bpm values. To avoid re-exporting unmodified track metadata
    // with fractional bpm values over and over again the comparison of bpm
    // values should be restricted to integer.
    bool anyFileTagsModified(
            const TrackMetadata& importedFromFile,
            Bpm::Comparison cmpBpm = Bpm::Comparison::Default) const;

    QString getBitrateText() const;

    double getDurationSecondsRounded() const {
        return std::round(getStreamInfo().getDuration().toDoubleSeconds());
    }
    QString getDurationText(
            Duration::Precision precision) const;

    // Parse an format date/time values according to ISO 8601
    static QDate parseDate(const QString& str) {
        return QDate::fromString(str.trimmed().replace(" ", ""), Qt::ISODate);
    }
    static QDateTime parseDateTime(const QString& str) {
        return QDateTime::fromString(str.trimmed().replace(" ", ""), Qt::ISODate);
    }
    static QString formatDate(QDate date) {
        return date.toString(Qt::ISODate);
    }
    static QString formatDateTime(const QDateTime& dateTime) {
        return dateTime.toString(Qt::ISODate);
    }

    // Parse and format the calendar year (for simplified display)
    static constexpr int kCalendarYearInvalid = 0;
    static int parseCalendarYear(const QString& year, bool* pValid = nullptr);
    static QString formatCalendarYear(const QString& year, bool* pValid = nullptr);

    static QString reformatYear(const QString& year);
};

bool operator==(const TrackMetadata& lhs, const TrackMetadata& rhs);

inline bool operator!=(const TrackMetadata& lhs, const TrackMetadata& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const TrackMetadata& arg);

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::TrackMetadata)
