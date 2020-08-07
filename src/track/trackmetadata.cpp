#include "track/trackmetadata.h"

#include "audio/streaminfo.h"
#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("TrackMetadata");

} // anonymous namespace

/*static*/ constexpr int TrackMetadata::kCalendarYearInvalid;

bool TrackMetadata::updateAudioPropertiesFromStream(
        const audio::StreamInfo& streamInfo) {
    bool changed = false;
    const auto streamChannelCount =
            streamInfo.getSignalInfo().getChannelCount();
    if (streamChannelCount.isValid() &&
        streamChannelCount != getChannelCount()) {
        if (getChannelCount().isValid()) {
            kLogger.debug()
                    << "Modifying channel count:"
                    << getChannelCount()
                    << "->"
                    << streamChannelCount;
        }
        setChannelCount(streamChannelCount);
        changed = true;
    }
    const auto streamSampleRate =
            streamInfo.getSignalInfo().getSampleRate();
    if (streamSampleRate.isValid() &&
        streamSampleRate != getSampleRate()) {
        if (getSampleRate().isValid()) {
            kLogger.debug()
                    << "Modifying sample rate:"
                    << getSampleRate()
                    << "->"
                    << streamSampleRate;
        }
        setSampleRate(streamSampleRate);
        changed = true;
    }
    const auto streamBitrate =
            streamInfo.getBitrate();
    if (streamBitrate.isValid() &&
        streamBitrate != getBitrate()) {
        if (getBitrate().isValid()) {
            kLogger.debug()
                    << "Modifying bitrate:"
                    << getBitrate()
                    << "->"
                    << streamBitrate;
        }
        setBitrate(streamBitrate);
        changed = true;
    }
    const auto streamDuration =
            streamInfo.getDuration();
    if (streamDuration > Duration::empty() &&
        streamDuration != getDuration()) {
        if (getDuration() > Duration::empty()) {
            kLogger.debug()
                    << "Modifying duration:"
                    << getDuration()
                    << "->"
                    << streamDuration;
        }
        setDuration(streamDuration);
        changed = true;
    }
    return changed;
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
    return lhs.getAlbumInfo() == rhs.getAlbumInfo() &&
            lhs.getTrackInfo() == rhs.getTrackInfo() &&
            lhs.getChannelCount() == rhs.getChannelCount() &&
            lhs.getSampleRate() == rhs.getSampleRate() &&
            lhs.getBitrate() == rhs.getBitrate() &&
            lhs.getDuration() == rhs.getDuration();
}

QDebug operator<<(QDebug dbg, const TrackMetadata& arg) {
    dbg << "TrackMetadata{";
    arg.dbgTrackInfo(dbg);
    arg.dbgAlbumInfo(dbg);
    arg.dbgBitrate(dbg);
    arg.dbgChannelCount(dbg);
    arg.dbgDuration(dbg);
    arg.dbgSampleRate(dbg);
    dbg << '}';
    return dbg;
}

} // namespace mixxx
