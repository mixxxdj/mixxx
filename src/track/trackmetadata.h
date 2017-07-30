#ifndef MIXXX_TRACKMETADATA_H
#define MIXXX_TRACKMETADATA_H

#include <QDateTime>

#include "track/bpm.h"
#include "track/replaygain.h"

namespace mixxx {

// DTO for track metadata properties. Must not be subclassed (no virtual destructor)!
class TrackMetadata final {
public:
    TrackMetadata();

    const QString& getArtist() const {
        return m_artist;
    }
    void setArtist(QString artist) {
        m_artist = artist;
    }

    const QString& getTitle() const {
        return m_title;
    }
    void setTitle(QString title) {
        m_title = title;
    }

    const QString& getAlbum() const {
        return m_album;
    }
    void setAlbum(QString album) {
        m_album = album;
    }

    const QString& getAlbumArtist() const {
        return m_albumArtist;
    }
    void setAlbumArtist(QString albumArtist) {
        m_albumArtist = albumArtist;
    }

    const QString& getGenre() const {
        return m_genre;
    }
    void setGenre(QString genre) {
        m_genre = genre;
    }

    const QString& getComment() const {
        return m_comment;
    }
    void setComment(QString comment) {
        m_comment = comment;
    }

    // year, date or date/time formatted according to ISO 8601
    const QString& getYear() const {
        return m_year;
    }
    void setYear(QString year) {
        m_year = year;
    }

    const QString& getTrackNumber() const {
        return m_trackNumber;
    }
    void setTrackNumber(QString trackNumber) {
        m_trackNumber = trackNumber;
    }
    const QString& getTrackTotal() const {
        return m_trackTotal;
    }
    void setTrackTotal(QString trackTotal) {
        m_trackTotal = trackTotal;
    }

    const QString& getComposer() const {
        return m_composer;
    }
    void setComposer(QString composer) {
        m_composer = composer;
    }

    const QString& getGrouping() const {
        return m_grouping;
    }
    void setGrouping(QString grouping) {
        m_grouping = grouping;
    }

    const QString& getKey() const {
        return m_key;
    }
    void setKey(QString key) {
        m_key = key;
    }

    // #channels
    int getChannels() const {
        return m_channels;
    }
    void setChannels(int channels) {
        m_channels = channels;
    }

    // Hz
    int getSampleRate() const {
        return m_sampleRate;
    }
    void setSampleRate(int sampleRate) {
        m_sampleRate = sampleRate;
    }

    // kbit / s
    int getBitrate() const {
        return m_bitrate;
    }
    void setBitrate(int bitrate) {
        m_bitrate = bitrate;
    }

    // #seconds
    double getDuration() const {
        return m_duration;
    }
    void setDuration(double duration) {
        m_duration = duration;
    }

    // beats / minute
    const Bpm& getBpm() const {
        return m_bpm;
    }
    void setBpm(Bpm bpm) {
        m_bpm = bpm;
    }
    void resetBpm() {
        m_bpm.resetValue();
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
    QString m_trackTotal;
    QString m_year;

    Bpm m_bpm;
    ReplayGain m_replayGain;

    // Floating-point fields (in alphabetical order)
    double m_duration; // seconds

    // Integer fields (in alphabetical order)
    int m_bitrate; // kbit/s
    int m_channels;
    int m_sampleRate; // Hz
};

bool operator==(const TrackMetadata& lhs, const TrackMetadata& rhs);

inline
bool operator!=(const TrackMetadata& lhs, const TrackMetadata& rhs) {
    return !(lhs == rhs);
}

}

#endif // MIXXX_TRACKMETADATA_H
