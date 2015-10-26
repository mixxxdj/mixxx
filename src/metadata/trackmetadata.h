#ifndef MIXXX_TRACKMETADATA_H
#define MIXXX_TRACKMETADATA_H

#include <QDateTime>

#include "util/replaygain.h"

namespace Mixxx {

// DTO for track metadata properties. Must not be subclassed (no virtual destructor)!
class TrackMetadata {
public:
    TrackMetadata();

    inline const QString& getArtist() const {
        return m_artist;
    }
    inline void setArtist(QString artist) {
        m_artist = artist;
    }

    inline const QString& getTitle() const {
        return m_title;
    }
    inline void setTitle(QString title) {
        m_title = title;
    }

    inline const QString& getAlbum() const {
        return m_album;
    }
    inline void setAlbum(QString album) {
        m_album = album;
    }

    inline const QString& getAlbumArtist() const {
        return m_albumArtist;
    }
    inline void setAlbumArtist(QString albumArtist) {
        m_albumArtist = albumArtist;
    }

    inline const QString& getGenre() const {
        return m_genre;
    }
    inline void setGenre(QString genre) {
        m_genre = genre;
    }

    inline const QString& getComment() const {
        return m_comment;
    }
    inline void setComment(QString comment) {
        m_comment = comment;
    }

    // year, date or date/time formatted according to ISO 8601
    inline const QString& getYear() const {
        return m_year;
    }
    inline void setYear(QString year) {
        m_year = year;
    }

    inline const QString& getTrackNumber() const {
        return m_trackNumber;
    }
    inline void setTrackNumber(QString trackNumber) {
        m_trackNumber = trackNumber;
    }

    inline const QString& getComposer() const {
        return m_composer;
    }
    inline void setComposer(QString composer) {
        m_composer = composer;
    }

    inline const QString& getGrouping() const {
        return m_grouping;
    }
    inline void setGrouping(QString grouping) {
        m_grouping = grouping;
    }

    inline const QString& getKey() const {
        return m_key;
    }
    inline void setKey(QString key) {
        m_key = key;
    }

    // #channels
    inline int getChannels() const {
        return m_channels;
    }
    inline void setChannels(int channels) {
        m_channels = channels;
    }

    // Hz
    inline int getSampleRate() const {
        return m_sampleRate;
    }
    inline void setSampleRate(int sampleRate) {
        m_sampleRate = sampleRate;
    }

    // kbit / s
    inline int getBitrate() const {
        return m_bitrate;
    }
    inline void setBitrate(int bitrate) {
        m_bitrate = bitrate;
    }

    // #seconds
    inline int getDuration() const {
        return m_duration;
    }
    inline void setDuration(int duration) {
        m_duration = duration;
    }
    // Returns the duration as a string: H:MM:SS
    static QString formatDuration(int duration);

    // beats / minute
    static const double kBpmUndefined;
    static const double kBpmMin; // lower bound (exclusive)
    static const double kBpmMax; // upper bound (inclusive)
    inline double getBpm() const {
        return m_bpm;
    }
    inline int getBpmAsInteger() const {
        return round(getBpm());
    }
    inline static bool isBpmValid(double bpm) {
        return (kBpmMin < bpm) && (kBpmMax >= bpm);
    }
    inline bool isBpmValid() const {
        return isBpmValid(getBpm());
    }
    inline void setBpm(double bpm) {
        m_bpm = bpm;
    }
    inline void resetBpm() {
        m_bpm = kBpmUndefined;
    }

    const ReplayGain& getReplayGain() const {
        return m_replayGain;
    }
    void setReplayGain(const ReplayGain& replayGain) {
        m_replayGain = replayGain;
    }
    void resetReplayGain() {
        m_replayGain = ReplayGain();
    }

    // Parse and format BPM metadata
    static double parseBpm(const QString& sBpm, bool* pValid = 0);
    static QString formatBpm(double bpm);
    static QString formatBpm(int bpm);
    static double normalizeBpm(double bpm);

    // Parse an format date/time values according to ISO 8601
    inline static QDate parseDate(QString str) {
        return QDate::fromString(str.trimmed().replace(" ", ""), Qt::ISODate);
    }
    inline static QDateTime parseDateTime(QString str) {
        return QDateTime::fromString(str.trimmed().replace(" ", ""), Qt::ISODate);
    }
    inline static QString formatDate(QDate date) {
        return date.toString(Qt::ISODate);
    }
    inline static QString formatDateTime(QDateTime dateTime) {
        return dateTime.toString(Qt::ISODate);
    }

    // Parse and format the calendar year (for simplified display)
    static const int kCalendarYearInvalid;
    static int parseCalendarYear(QString year, bool* pValid = 0);
    static QString formatCalendarYear(QString year, bool* pValid = 0);

    static QString reformatYear(QString year);

private:
    // String fields (in alphabetical order)
    QString m_album;
    QString m_albumArtist;
    QString m_artist;
    QString m_comment;
    QString m_composer;
    QString m_genre;
    QString m_grouping;
    QString m_key;
    QString m_title;
    QString m_trackNumber;
    QString m_year;

    ReplayGain m_replayGain;

    // Floating-point fields (in alphabetical order)
    double m_bpm;

    // Integer fields (in alphabetical order)
    int m_bitrate; // kbit/s
    int m_channels;
    int m_duration; // seconds
    int m_sampleRate; // Hz
};

bool operator==(const TrackMetadata& lhs, const TrackMetadata& rhs);

inline
bool operator!=(const TrackMetadata& lhs, const TrackMetadata& rhs) {
    return !(lhs == rhs);
}

}

#endif // MIXXX_TRACKMETADATA_H
