/**
 * \file soundsourcemediafoundation.h
 * \class SoundSourceMediaFoundation
 * \brief Decodes MPEG4/AAC audio using the SourceReader interface of the
 * Media Foundation framework included in Windows 7.
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
    HRESULT ConfigureAudioStream(
        IMFSourceReader *pReader,   // Pointer to the source reader.
        IMFMediaType **ppPCMAudio   // Receives the audio format.
        );
    QFile m_file;
    unsigned int m_samples; // total number of samples
    IMFSourceReader *m_pReader;
    IMFMediaType *m_pAudioType;
    HANDLE m_hFile;
    wchar_t* m_wcFilename;

};

#endif // ifndef SOUNDSOURCEMEDIAFOUNDATION_H
