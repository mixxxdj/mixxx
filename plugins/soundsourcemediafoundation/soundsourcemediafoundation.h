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

#include "util/defs.h"
#include "defs_version.h"
#include "soundsource.h"

#ifdef Q_OS_WIN
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif

class IMFSourceReader;
class IMFMediaType;
class IMFMediaSource;

class SoundSourceMediaFoundation : public Mixxx::SoundSource {
    typedef SoundSource Super;

public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceMediaFoundation(QString filename);
    ~SoundSourceMediaFoundation();

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) /*override*/;
    QImage parseCoverArt() /*override*/;

    Result open();

    diff_type seekFrame(diff_type frameIndex) /*override*/;
    size_type readFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer) /*override*/;

private:
    bool configureAudioStream();

    void copyFrames(sample_type *dest, size_t *destFrames, const sample_type *src,
            size_t srcFrames);

    IMFSourceReader *m_pReader;
    IMFMediaType *m_pAudioType;
    wchar_t *m_wcFilename;
    int m_nextFrame;
    sample_type *m_leftoverBuffer;
    size_t m_leftoverBufferSize;
    size_t m_leftoverBufferLength;
    int m_leftoverBufferPosition;
    qint64 m_mfDuration;
    long m_iCurrentPosition;
    bool m_dead;
    bool m_seeking;
};

extern "C" MY_EXPORT const char* getMixxxVersion() {
    return VERSION;
}

extern "C" MY_EXPORT int getSoundSourceAPIVersion() {
    return MIXXX_SOUNDSOURCE_API_VERSION;
}

extern "C" MY_EXPORT Mixxx::SoundSource* getSoundSource(QString filename) {
    return new SoundSourceMediaFoundation(filename);
}

extern "C" MY_EXPORT char** supportedFileExtensions() {
    QList<QString> exts = SoundSourceMediaFoundation::supportedFileExtensions();
    //Convert to C string array.
    char** c_exts = (char**) malloc((exts.count() + 1) * sizeof(char*));
    for (int i = 0; i < exts.count(); i++) {
        QByteArray qba = exts[i].toUtf8();
        c_exts[i] = strdup(qba.constData());
        qDebug() << c_exts[i];
    }
    c_exts[exts.count()] = NULL; //NULL terminate the list

    return c_exts;
}

extern "C" MY_EXPORT void freeFileExtensions(char **exts) {
    for (int i(0); exts[i]; ++i)
        free(exts[i]);
    free(exts);
}

#endif // ifndef SOUNDSOURCEMEDIAFOUNDATION_H
