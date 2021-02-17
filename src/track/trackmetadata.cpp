#include "track/trackmetadata.h"

#include "audio/streaminfo.h"
#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("TrackMetadata");

} // anonymous namespace

/*static*/ constexpr int TrackMetadata::kCalendarYearInvalid;

bool TrackMetadata::updateStreamInfoFromSource(
        const audio::StreamInfo& streamInfo) {
    if (getStreamInfo() == streamInfo) {
        return false;
    }
    const auto streamChannelCount =
            streamInfo.getSignalInfo().getChannelCount();
    if (streamChannelCount.isValid() &&
            streamChannelCount != getStreamInfo().getSignalInfo().getChannelCount() &&
            getStreamInfo().getSignalInfo().getChannelCount().isValid()) {
        kLogger.debug()
                << "Modifying channel count:"
                << getStreamInfo().getSignalInfo().getChannelCount()
                << "->"
                << streamChannelCount;
    }
    const auto streamSampleRate =
            streamInfo.getSignalInfo().getSampleRate();
    if (streamSampleRate.isValid() &&
            streamSampleRate != getStreamInfo().getSignalInfo().getSampleRate() &&
            getStreamInfo().getSignalInfo().getSampleRate().isValid()) {
        kLogger.debug()
                << "Modifying sample rate:"
                << getStreamInfo().getSignalInfo().getSampleRate()
                << "->"
                << streamSampleRate;
    }
    const auto streamBitrate =
            streamInfo.getBitrate();
    if (streamBitrate.isValid() &&
            streamBitrate != getStreamInfo().getBitrate() &&
            getStreamInfo().getSignalInfo().isValid()) {
        kLogger.debug()
                << "Modifying bitrate:"
                << getStreamInfo().getSignalInfo()
                << "->"
                << streamBitrate;
    }
    const auto streamDuration =
            streamInfo.getDuration();
    if (streamDuration > Duration::empty() &&
            streamDuration != getStreamInfo().getDuration() &&
            getStreamInfo().getDuration() > Duration::empty()) {
        kLogger.debug()
                << "Modifying duration:"
                << getStreamInfo().getDuration()
                << "->"
                << streamDuration;
    }
    setStreamInfo(streamInfo);
    return true;
}

QString TrackMetadata::getBitrateText() const {
    if (!getStreamInfo().getBitrate().isValid()) {
        return QString();
    }
    return QString::number(getStreamInfo().getBitrate()) +
            QChar(' ') +
            audio::Bitrate::unit();
}

QString TrackMetadata::getDurationText(
        Duration::Precision precision) const {
    double durationSeconds;
    if (precision == Duration::Precision::SECONDS) {
        // Round to full seconds before formatting for consistency
        // getDurationText() should always display the same number
        // as getDurationSecondsRounded()
        durationSeconds = getDurationSecondsRounded();
    } else {
        durationSeconds = getStreamInfo().getDuration().toDoubleSeconds();
    }
    return Duration::formatTime(durationSeconds, precision);
}

int TrackMetadata::parseCalendarYear(const QString& year, bool* pValid) {
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

QString TrackMetadata::formatCalendarYear(const QString& year, bool* pValid) {
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

QString TrackMetadata::reformatYear(const QString& year) {
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

void TrackMetadata::normalizeBeforeExport() {
    m_albumInfo.normalizeBeforeExport();
    m_trackInfo.normalizeBeforeExport();
}

bool TrackMetadata::anyFileTagsModified(
        const TrackMetadata& importedFromFile,
        Bpm::Comparison cmpBpm) const {
    // NOTE(uklotzde): The read-only audio properties that are stored
    // directly as members of this class might differ after they have
    // been updated while decoding audio data. They are read-only and
    // must not be considered when exporting metadata!
    return getAlbumInfo() != importedFromFile.getAlbumInfo() ||
            !getTrackInfo().compareEq(importedFromFile.getTrackInfo(), cmpBpm);
}

bool operator==(const TrackMetadata& lhs, const TrackMetadata& rhs) {
    return lhs.getStreamInfo() == rhs.getStreamInfo() &&
            lhs.getAlbumInfo() == rhs.getAlbumInfo() &&
            lhs.getTrackInfo() == rhs.getTrackInfo();
}

QDebug operator<<(QDebug dbg, const TrackMetadata& arg) {
    dbg << "TrackMetadata{";
    arg.dbgStreamInfo(dbg);
    arg.dbgTrackInfo(dbg);
    arg.dbgAlbumInfo(dbg);
    dbg << '}';
    return dbg;
}

} // namespace mixxx
