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

#include "sources/soundsourceplugin.h"

#include <windows.h>

#ifdef Q_OS_WIN
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif

class IMFSourceReader;
class IMFMediaType;
class IMFMediaSource;

class SoundSourceMediaFoundation : public Mixxx::SoundSourcePlugin {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceMediaFoundation(QUrl url);
    ~SoundSourceMediaFoundation();

    void close() /*override*/;

    SINT seekSampleFrame(SINT frameIndex) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames, CSAMPLE* sampleBuffer) /*override*/;

private:
    Result tryOpen(SINT channelCountHint) /*override*/;

    bool configureAudioStream(SINT channelCountHint);

    void copyFrames(CSAMPLE *dest, SINT *destFrames, const CSAMPLE *src,
            SINT srcFrames);

    HRESULT m_hrCoInitialize;
    HRESULT m_hrMFStartup;
    IMFSourceReader *m_pReader;
    IMFMediaType *m_pAudioType;
    wchar_t *m_wcFilename;
    int m_nextFrame;
    CSAMPLE *m_leftoverBuffer;
    SINT m_leftoverBufferSize;
    SINT m_leftoverBufferLength;
    int m_leftoverBufferPosition;
    qint64 m_mfDuration;
    long m_iCurrentPosition;
    bool m_dead;
    bool m_seeking;
};

extern "C" MY_EXPORT const char* getMixxxVersion();
extern "C" MY_EXPORT int getSoundSourceAPIVersion();
extern "C" MY_EXPORT Mixxx::SoundSource* getSoundSource(QString fileName);
extern "C" MY_EXPORT char** supportedFileExtensions();
extern "C" MY_EXPORT void freeFileExtensions(char** fileExtensions);

#endif // SOUNDSOURCEMEDIAFOUNDATION_H
