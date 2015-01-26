#ifndef TRACKMETADATA_H
#define TRACKMETADATA_H

#include <QString>

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

    // year, date or date/time
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

    // beats / minute
    static const double BPM_UNDEFINED;
    static const double BPM_MIN; // lower bound (exclusive)
    static const double BPM_MAX; // upper bound (inclusive)
    inline double getBpm() const {
        return m_bpm;
    }
    inline static bool isBpmValid(double bpm) {
        return (BPM_MIN < bpm) && (BPM_MAX >= bpm);
    }
    inline bool isBpmValid() const {
        return isBpmValid(getBpm());
    }
    inline void setBpm(double bpm) {
        m_bpm = bpm;
    }
    inline void resetBpm() {
        m_bpm = BPM_UNDEFINED;
    }
    bool setBpmString(const QString& sBpm);

    static const float REPLAYGAIN_UNDEFINED;
    static const float REPLAYGAIN_MIN; // lower bound (exclusive)
    static const float REPLAYGAIN_0DB;
    inline float getReplayGain() const {
        return m_replayGain;
    }
    inline static bool isReplayGainValid(float replayGain) {
        return REPLAYGAIN_MIN < replayGain;
    }
    inline bool isReplayGainValid() const {
        return isReplayGainValid(getReplayGain());
    }
    inline void setReplayGain(float replayGain) {
        m_replayGain = replayGain;
    }
    inline void resetReplayGain() {
        m_replayGain = REPLAYGAIN_UNDEFINED;
    }
    bool setReplayGainDbString(QString sReplayGainDb); // in dB

private:
    QString m_artist;
    QString m_title;
    QString m_album;
    QString m_albumArtist;
    QString m_genre;
    QString m_comment;
    QString m_year;
    QString m_trackNumber;
    QString m_composer;
    QString m_grouping;
    QString m_key;

    // The following members need to be initialized
    // explicitly in the constructor! Otherwise their
    // value is undefined.
    int m_channels;
    int m_sampleRate;
    int m_bitrate;
    int m_duration;
    double m_bpm;
    float m_replayGain;
};

}

#endif
