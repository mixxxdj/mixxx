/***************************************************************************
                          soundsourcemodplug.h  -  description
                             -------------------
    copyright            : (C) 2012 by Stefan Nuernberger
    email                : kabelfrickler@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDSOURCEMODPLUG_H
#define SOUNDSOURCEMODPLUG_H

#include "soundsource.h"
#include "defs_version.h"
#include <QByteArray>
#include <QList>
#include <QString>
#include <QtDebug>

namespace ModPlug {
#include "libmodplug/modplug.h"
}

//As per QLibrary docs: http://doc.trolltech.com/4.6/qlibrary.html#resolve
#ifdef Q_WS_WIN
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif

namespace Mixxx {
/*
    Class for reading tracker files using libmodplug
    The whole file is decoded at once and saved
    in RAM to allow seeking and smooth operation in Mixxx.
 */
class SoundSourceModPlug : public Mixxx::SoundSource
{
public:
    SoundSourceModPlug(QString qFilename);
    ~SoundSourceModPlug();
    int open();
    long seek(long);
    unsigned read(unsigned long size, const SAMPLE*);
    inline long unsigned length();
    int parseHeader();
    static QList<QString> supportedFileExtensions();

private:
    bool opened;
    unsigned long filelength;
    unsigned int seekpos;
    ModPlug::ModPlugFile *file; ///< pointer to ModPlugFile struct
    ModPlug::_ModPlug_Settings settings; ///< struct of parameters
    QByteArray filebuf; ///< original module file data
    QByteArray smplbuf; ///< 16bit stereo samples, 44.1kHz
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
    return new SoundSourceModPlug(filename);
}

extern "C" MY_EXPORT char** supportedFileExtensions()
{
    QList<QString> exts = SoundSourceModPlug::supportedFileExtensions();
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

} // ns Mixxx

#endif
