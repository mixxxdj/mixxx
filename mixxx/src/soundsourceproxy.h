/***************************************************************************
                          soundsourceproxy.h  -  description
                             -------------------
    begin                : Wed Oct 13 2004
    copyright            : (C) 2004 by Tue Haste Andersen
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

#ifndef SOUNDSOURCEPROXY_H
#define SOUNDSOURCEPROXY_H

#include <QMap>
#include <QMutex>
#include <QString>
#include <QLibrary>
#include <QRegExp>

#include "soundsource.h"
#include "trackinfoobject.h"

class QLibrary;

/**
  *@author Tue Haste Andersen
  */


/*
  Base class for sound sources.
*/
class SoundSourceProxy : public Mixxx::SoundSource
{
public:
    SoundSourceProxy(QString qFilename);
    SoundSourceProxy(TrackPointer pTrack);
    ~SoundSourceProxy();
    static void loadPlugins();
    int open();
    long seek(long);
    unsigned read(unsigned long size, const SAMPLE*);
    long unsigned length();
    int parseHeader();
    static int ParseHeader(TrackInfoObject* p);
    unsigned int getSampleRate();
    /** Returns filename */
    QString getFilename();
    static QStringList supportedFileExtensions();
    static QStringList supportedFileExtensionsByPlugins();
    static QString supportedFileExtensionsString();
    static QString supportedFileExtensionsRegex();
    static bool isFilenameSupported(QString filename);
private:
    static SoundSource* initialize(QString qFilename);
    //void initPlugin(QString lib_filename, QString track_filename);
    static QLibrary* getPlugin(QString lib_filename);

    SoundSource *m_pSoundSource;
    TrackPointer m_pTrack;

    static QRegExp m_supportedFileRegex;
    static QMap<QString, QLibrary*> m_plugins;
    static QMap<QString, getSoundSourceFunc> m_extensionsSupportedByPlugins;
    static QMutex m_extensionsMutex;
};

#endif
