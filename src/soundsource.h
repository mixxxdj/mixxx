/***************************************************************************
 soundsource.h  -  description
 -------------------
 begin                : Wed Feb 20 2002
 copyright            : (C) 2002 by Tue and Ken Haste Andersen
 email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDSOURCE_H
#define SOUNDSOURCE_H

#define MIXXX_SOUNDSOURCE_API_VERSION 7
/** @note SoundSource API Version history:
 1 - Mixxx 1.8.0 Beta 2
 2 - Mixxx 1.9.0 Pre (added key code)
 3 - Mixxx 1.10.0 Pre (added freeing function for extensions)
 4 - Mixxx 1.11.0 Pre (added composer field to SoundSource)
 5 - Mixxx 1.12.0 Pre (added album artist and grouping fields to SoundSource)
 6 - Mixxx 1.12.0 Pre (added cover art suppport)
 7 - Mixxx 1.13.0 New AudioSource API
 */

#include "audiosource.h"
#include "util/defs.h"

#include <QImage>
#include <QString>

/** Getter function to be declared by all SoundSource plugins */
namespace Mixxx {
class SoundSource;
}

typedef Mixxx::SoundSource* (*getSoundSourceFunc)(QString filename);
typedef char** (*getSupportedFileExtensionsFunc)();
typedef int (*getSoundSourceAPIVersionFunc)();
/* New in version 3 */
typedef void (*freeFileExtensionsFunc)(char** exts);

namespace Mixxx {

/*
 Base class for sound sources.
 */
class SoundSource: public AudioSource {
public:
    virtual ~SoundSource();

    inline const QString& getFilename() const {
        return m_sFilename;
    }
    inline const QString& getType() const {
        return m_sType;
    }

    // Parses metadata before opening the SoundSource for reading.
    //
    // Only metadata that is quickly readable should be read.
    // The implementation is free to set inaccurate estimated
    // values here that are overwritten when the AudioSource is
    // actually opened for reading.
    virtual Result parseHeader() = 0;

    // Returns the first cover art image embedded within the file (if any).
    virtual QImage parseCoverArt() = 0;

    inline const QString& getArtist() const {
        return m_sArtist;
    }
    inline const QString& getTitle() const {
        return m_sTitle;
    }
    inline const QString& getAlbum() const {
        return m_sAlbum;
    }
    inline const QString& getAlbumArtist() const {
        return m_sAlbumArtist;
    }
    inline const QString& getComment() const {
        return m_sComment;
    }
    inline const QString& getYear() const {
        return m_sYear;
    }
    inline const QString& getGenre() const {
        return m_sGenre;
    }
    inline const QString& getComposer() const {
        return m_sComposer;
    }
    inline const QString& getGrouping() const {
        return m_sGrouping;
    }
    inline const QString& getTrackNumber() const {
        return m_sTrackNumber;
    }
    inline float getReplayGain() const {
        return m_fReplayGain;
    }
    inline const QString& getKey() const {
        return m_sKey;
    }
    inline float getBPM() const {
        return m_fBpm;
    }
    inline int getBitrate() const {
        return m_iBitrate;
    }
    int getChannels() const;
    int getSampleRate() const;
    int getDuration() const;

    inline void setArtist(QString artist) {
        m_sArtist = artist;
    }
    inline void setTitle(QString title) {
        m_sTitle = title;
    }
    inline void setAlbum(QString album) {
        m_sAlbum = album;
    }
    inline void setAlbumArtist(QString albumArtist) {
        m_sAlbumArtist = albumArtist;
    }
    inline void setComment(QString comment) {
        m_sComment = comment;
    }
    inline void setYear(QString year) {
        m_sYear = year;
    }
    inline void setGenre(QString genre) {
        m_sGenre = genre;
    }
    inline void setComposer(QString composer) {
        m_sComposer = composer;
    }
    inline void setGrouping(QString grouping) {
        m_sGrouping = grouping;
    }
    inline void setTrackNumber(QString trackNumber) {
        m_sTrackNumber = trackNumber;
    }
    inline void setKey(QString key) {
        m_sKey = key;
    }
    inline void setBpm(float bpm) {
        m_fBpm = bpm;
    }
    void setBpmString(QString sBpm);
    inline void setReplayGain(float replayGain) {
        m_fReplayGain = replayGain;
    }
    void setReplayGainString(QString sReplayGain);
    inline void setChannels(int channels) {
        m_iChannels = channels;
    }
    inline void setSampleRate(int sampleRate) {
        m_iSampleRate = sampleRate;
    }
    inline void setBitrate(int bitrate) {
        m_iBitrate = bitrate;
    }
    inline void setDuration(int duration) {
        m_iDuration = duration;
    }

    /**
     * Opens the SoundSource for reading audio data.
     *
     * When opening a SoundSource the corresponding
     * AudioSource must be initialized.
     */
    virtual Result open() = 0;

protected:
    explicit SoundSource(QString sFilename);
    SoundSource(QString sFilename, QString sType);

private:
    const QString m_sFilename;
    const QString m_sType;

    QString m_sArtist;
    QString m_sTitle;
    QString m_sAlbum;
    QString m_sAlbumArtist;
    QString m_sComment;
    QString m_sYear;
    QString m_sGenre;
    QString m_sComposer;
    QString m_sGrouping;
    QString m_sTrackNumber;
    QString m_sKey;

    // The following members need to be initialized
    // explicitly in the constructor! Otherwise their
    // value is undefined.
    float m_fReplayGain;
    float m_fBpm; // beats / minute

    // Audio properties (from metadata)
    int m_iChannels; // #channels
    int m_iSampleRate; // Hz
    int m_iBitrate; // kbit / s
    int m_iDuration; // #seconds
};

typedef QSharedPointer<SoundSource> SoundSourcePointer;

} //namespace Mixxx

#endif
