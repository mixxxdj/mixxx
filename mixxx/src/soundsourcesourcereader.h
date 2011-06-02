/**
 * \file soundsourcecoreaudio.h
 * \class SoundSourceCoreAudio
 * \brief Decodes M4As (etc) using the AudioToolbox framework included as
 *        part of Core Audio on OS X (and iOS).
 * \author Albert Santoni <alberts at mixxx dot org>
 * \date Dec 12, 2010
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDSOURCESOURCEREADER_H
#define SOUNDSOURCESOURCEREADER_H

#include <QFile>
#include <QString>

#include <windows.h>
class IMFSourceReader;
class IMFMediaType;
class IMFMediaSource;

#include "defs.h"
#include "soundsource.h"

class SoundSourceSourceReader : public Mixxx::SoundSource {
public:
    SoundSourceSourceReader(QString filename);
    ~SoundSourceSourceReader();
    int open();
    long seek(long filepos);
    unsigned read(unsigned long size, const SAMPLE *buffer);
    inline long unsigned length();
    int parseHeader();
    static QList<QString> supportedFileExtensions();

private:
    HRESULT ConfigureAudioStream(
        IMFSourceReader *pReader,   // Pointer to the source reader.
        IMFMediaType **ppPCMAudio   // Receives the audio format.
        );
    HRESULT CreateMediaSource(PCWSTR sURL, IMFMediaSource **ppSource);

    QFile m_file;
    unsigned int m_samples; // total number of samples
    IMFSourceReader *m_pReader;
    IMFMediaType *m_pAudioType;
    HANDLE m_hFile;
    wchar_t* m_wcFilename;

};

#endif // ifndef SOUNDSOURCESOURCEREADER_H
