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

#include <QImage>
#include <QString>
#include <QSharedPointer>

#include "util/types.h"
#include "util/defs.h"

#define MIXXX_SOUNDSOURCE_API_VERSION 6
/** @note SoundSource API Version history:
           1 - Mixxx 1.8.0 Beta 2
           2 - Mixxx 1.9.0 Pre (added key code)
           3 - Mixxx 1.10.0 Pre (added freeing function for extensions)
           4 - Mixxx 1.11.0 Pre (added composer field to SoundSource)
           5 - Mixxx 1.12.0 Pre (added album artist and grouping fields to SoundSource)
           6 - Mixxx 1.13.0 (added cover art suppport)
  */

/** Getter function to be declared by all SoundSource plugins */
namespace Mixxx {
    class SoundSource;
}
typedef Mixxx::SoundSource* (*getSoundSourceFunc)(QString filename);
typedef char** (*getSupportedFileExtensionsFunc)();
typedef int (*getSoundSourceAPIVersionFunc)();
/* New in version 3 */
typedef void (*freeFileExtensionsFunc)(char** exts);


/*
  Base class for sound sources.
*/
namespace Mixxx
{
class SoundSource
{
public:
    virtual ~SoundSource();

    // Parses the metadata before actually opening the file for reading the audio stream.
    virtual Result parseHeader() = 0;

    // Returns the first cover art image embedded within the file (if any).
    virtual QImage parseCoverArt() = 0;

    // Functions for reading the audio data of the file.
    virtual Result open() = 0;
    virtual long unsigned length() = 0;
    virtual long seek(long) = 0;
    virtual unsigned read(unsigned long size, const SAMPLE*) = 0;

    const QString& getType() const {
        return m_sType;
    }
    const QString& getFilename() const {
        return m_qFilename;
    }
    const QString& getArtist() const {
        return m_sArtist;
    }
    const QString& getTitle() const {
        return m_sTitle;
    }
    const QString& getAlbum() const {
        return m_sAlbum;
    }
    const QString& getAlbumArtist() const {
        return m_sAlbumArtist;
    }
    const QString& getComment() const {
        return m_sComment;
    }
    const QString& getYear() const {
        return m_sYear;
    }
    const QString& getGenre() const {
        return m_sGenre;
    }
    const QString& getComposer() const {
        return m_sComposer;
    }
    const QString& getGrouping() const {
        return m_sGrouping;
    }
    const QString& getTrackNumber() const {
        return m_sTrackNumber;
    }
    float getReplayGain() const {
        return m_fReplayGain;
    }
    const QString& getKey() const {
        return m_sKey;
    }
    float getBPM() const {
        return m_fBpm;
    }
    int getChannels() const {
        return m_iChannels;
    }
    int getBitrate() const {
        return m_iBitrate;
    }
    int getSampleRate() const {
        return m_iSampleRate;
    }
    int getDuration() const {
        return m_iDuration;
    }

    void setArtist(QString artist) {
        m_sArtist = artist;
    }
    void setTitle(QString title) {
        m_sTitle = title;
    }
    void setAlbum(QString album) {
        m_sAlbum = album;
    }
    void setAlbumArtist(QString albumArtist) {
        m_sAlbumArtist = albumArtist;
    }
    void setComment(QString comment) {
        m_sComment = comment;
    }
    void setYear(QString year) {
        m_sYear = year;
    }
    void setGenre(QString genre) {
        m_sGenre = genre;
    }
    void setComposer(QString composer) {
        m_sComposer = composer;
    }
    void setGrouping(QString grouping) {
        m_sGrouping = grouping;
    }
    void setTrackNumber(QString trackNumber) {
        m_sTrackNumber = trackNumber;
    }
    void setKey(QString key){
        m_sKey = key;
    }
    void setBpm(float bpm) {
        m_fBpm = bpm;
    }
    void setBpmString(const QString& sBpm);
    void setReplayGain(float replayGain) {
        m_fReplayGain = replayGain;
    }
    void setReplayGainString(QString sReplayGain);

    void setChannels(int channels) {
        m_iChannels = channels;
    }
    void setSampleRate(int sampleRate) {
        m_iSampleRate = sampleRate;
    }
    void setBitrate(int bitrate) {
        m_iBitrate = bitrate;
    }
    void setDuration(int duration) {
        m_iDuration = duration;
    }

protected:
    explicit SoundSource(QString qFilename);

    void setType(QString type) {
        m_sType = type;
    }

private:
    const QString m_qFilename;

    QString m_sType;
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

    //Dontcha be forgettin' to initialize these variables.... arr
    float m_fReplayGain;
    float m_fBpm;
    int m_iChannels;
    int m_iBitrate;
    unsigned int m_iSampleRate;
    int m_iDuration;
};

typedef QSharedPointer<SoundSource> SoundSourcePointer;

} //namespace Mixxx

#endif
