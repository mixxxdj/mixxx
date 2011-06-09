/**
 * \file soundsourcemediafoundation.h
 * \class SoundSourceMediaFoundation
 * \brief Decodes MPEG4/AAC audio using the SourceReader interface of the
 * Media Foundation framework included in Windows 7.
 * \author Bill Good <bkgood at gmail dot com>
 * \author Albert Santoni <alberts at mixxx dot org>
 * \date Jan 10, 2011
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDSOURCEMEDIAFOUNDATION_H
#define SOUNDSOURCEMEDIAFOUNDATION_H

#include <QFile>
#include <QString>

#include <windows.h>
class IMFSourceReader;
class IMFMediaType;
class IMFMediaSource;

#include "defs.h"
#include "soundsource.h"

class SoundSourceMediaFoundation : public Mixxx::SoundSource {
public:
    SoundSourceMediaFoundation(QString filename);
    ~SoundSourceMediaFoundation();
    int open();
    long seek(long filepos);
    unsigned read(unsigned long size, const SAMPLE *buffer);
    inline long unsigned length();
    int parseHeader();
    static QList<QString> supportedFileExtensions();

private:
    bool ConfigureAudioStream();
    bool ReadProperties();
    void CopyFrames(qint16 *dest, size_t *destFrames, const qint16 *src,
        size_t srcFrames);
    static inline qreal secondsFromMF(qint64 mf);
    static inline qint64 mfFromSeconds(qreal sec);
    static inline qint64 frameFromMF(qint64 mf);
    static inline qint64 mfFromFrame(qint64 frame);
    QFile m_file;
    IMFSourceReader *m_pReader;
    IMFMediaType *m_pAudioType;
    wchar_t* m_wcFilename;
    int m_nextFrame;
    qint16 *m_leftoverBuffer;
    size_t m_leftoverBufferSize;
    size_t m_leftoverBufferLength;
    int m_leftoverBufferPosition;
    qint64 m_mfDuration;
    bool m_dead;
    bool m_seeking;
};

#endif // ifndef SOUNDSOURCEMEDIAFOUNDATION_H
