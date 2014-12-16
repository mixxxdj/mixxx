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
#include "util/sandbox.h"

/**
  *@author Tue Haste Andersen
  */


/**
 * Creates sound sources for filenames or tracks.
 */
class SoundSourceProxy
{
public:
    static void loadPlugins();

    static QStringList supportedFileExtensions();
    static QStringList supportedFileExtensionsByPlugins();
    static QString supportedFileExtensionsString();
    static QString supportedFileExtensionsRegex();
    static bool isFilenameSupported(QString filename);

    SoundSourceProxy(QString qFilename, SecurityTokenPointer pToken);
    explicit SoundSourceProxy(TrackPointer pTrack);

    const Mixxx::SoundSourcePointer& getSoundSource() const {
        return m_pSoundSource;
    }

    // Opens the audio data through the proxy will
    // update the some metadata of the track object.
    // Returns a null pointer on failure.
    Mixxx::SoundSourcePointer open() const;

private:
    static QRegExp m_supportedFileRegex;
    static QMap<QString, QLibrary*> m_plugins;
    static QMap<QString, getSoundSourceFunc> m_extensionsSupportedByPlugins;
    static QMutex m_extensionsMutex;

    static QLibrary* getPlugin(QString lib_filename);

    static Mixxx::SoundSourcePointer initialize(const QString& qFilename);

    const TrackPointer m_pTrack;
    const SecurityTokenPointer m_pSecurityToken;

    const Mixxx::SoundSourcePointer m_pSoundSource;
};

#endif
