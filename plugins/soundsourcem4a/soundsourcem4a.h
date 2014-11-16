/***************************************************************************
                          soundsourcem4a.h  -  mp4/m4a decoder
                             -------------------
    copyright            : (C) 2008 by Garth Dahlstrom
    email                : ironstorm@users.sf.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDSOURCEM4A_H
#define SOUNDSOURCEM4A_H

#include "soundsource.h"

#include "m4a/ip.h"

#include "defs_version.h"

#ifdef __MP4V2__
    #include <mp4v2/mp4v2.h>
#else
    #include <mp4.h>
#endif

#include <neaacdec.h>

#include <QString>
#include <QDebug>

#include <QtDebug>

//As per QLibrary docs: http://doc.trolltech.com/4.6/qlibrary.html#resolve
#ifdef Q_OS_WIN
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif


namespace Mixxx {

class SoundSourceM4A : public SoundSource {
    typedef SoundSource Super;

    public:
        static QList<QString> supportedFileExtensions();

        explicit SoundSourceM4A(QString qFileName);
        ~SoundSourceM4A();

        Result parseHeader() /*override*/;

        QImage parseCoverArt() /*override*/;

        Result open() /*override*/;
        void close() /*override*/;

        diff_type seekFrame(diff_type frameIndex) /*override*/;
        size_type readFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer) /*override*/;

    private:
        Result initializeDecoder();

        /*non-virtual*/ void closeThis();

        input_plugin_data ipd;
        int trackId;
};

extern "C" MY_EXPORT const char* getMixxxVersion()
{
    return VERSION;
}

extern "C" MY_EXPORT int getSoundSourceAPIVersion()
{
    return MIXXX_SOUNDSOURCE_API_VERSION;
}

extern "C" MY_EXPORT SoundSource* getSoundSource(QString filename)
{
    return new SoundSourceM4A(filename);
}

extern "C" MY_EXPORT char** supportedFileExtensions()
{
    QList<QString> exts = SoundSourceM4A::supportedFileExtensions();
    //Convert to C string array.
    char** c_exts = (char**)malloc((exts.count() + 1) * sizeof(char*));
    for (int i = 0; i < exts.count(); i++)
    {
        QByteArray qba = exts[i].toUtf8();
        c_exts[i] = strdup(qba.constData());
        qDebug() << c_exts[i];
    }
    c_exts[exts.count()] = NULL; //NULL terminate the list

    return c_exts;
}

extern "C" MY_EXPORT void freeFileExtensions(char **exts)
{
    for (int i(0); exts[i]; ++i) free(exts[i]);
    free(exts);
}

} // namespace Mixxx

#endif
