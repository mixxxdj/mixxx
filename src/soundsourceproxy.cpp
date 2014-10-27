/* -*- mode:C++; indent-tabs-mode:t; tab-width:8; c-basic-offset:4; -*- */
/***************************************************************************
                          soundsourceproxy.cpp  -  description
                             -------------------
    begin                : Wed Oct 13 2004
    copyright            : (C) 2004 Tue Haste Andersen
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

#include <QLibrary>
#include <QMutexLocker>
#include <QMutex>
#include <QtDebug>
#include <QDir>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QApplication>

#include "trackinfoobject.h"
#include "soundsourceproxy.h"
#ifdef __MAD__
#include "soundsourcemp3.h"
#endif
#include "soundsourceoggvorbis.h"
#ifdef __OPUS__
#include "soundsourceopus.h"
#endif
#ifdef __COREAUDIO__
#include "soundsourcecoreaudio.h"
#endif
#ifdef __SNDFILE__
#include "soundsourcesndfile.h"
#endif
#ifdef __FFMPEGFILE__
#include "soundsourceffmpeg.h"
#endif
#ifdef __MODPLUG__
#include "soundsourcemodplug.h"
#endif
#include "soundsourceflac.h"
#include "util/cmdlineargs.h"
#include "util/regex.h"

//Static memory allocation
QRegExp SoundSourceProxy::m_supportedFileRegex;
QMap<QString, QLibrary*> SoundSourceProxy::m_plugins;
QMap<QString, getSoundSourceFunc> SoundSourceProxy::m_extensionsSupportedByPlugins;
QMutex SoundSourceProxy::m_extensionsMutex;

//Constructor
SoundSourceProxy::SoundSourceProxy(QString qFilename, SecurityTokenPointer pToken)
    : Mixxx::SoundSource(qFilename),
      m_pSoundSource(NULL),
      m_pSecurityToken(pToken) {
    if (pToken.isNull()) {
        // Open a security token for the file if we are in a sandbox.
        QFileInfo info(m_qFilename);
        m_pSecurityToken = Sandbox::openSecurityToken(info, true);
    }

    // Create the underlying SoundSource.
    m_pSoundSource = initialize(qFilename);
}

//Other constructor
SoundSourceProxy::SoundSourceProxy(TrackPointer pTrack)
    : SoundSource(pTrack->getLocation()),
      m_pSoundSource(NULL),
      m_pTrack(pTrack),
      m_pSecurityToken(pTrack->getSecurityToken()) {
    if (m_pSecurityToken.isNull()) {
        // Open a security token for the file if we are in a sandbox.
        QFileInfo info(m_qFilename);
        m_pSecurityToken = Sandbox::openSecurityToken(info, true);
    }

    m_pSoundSource = initialize(pTrack->getLocation());
}

SoundSourceProxy::~SoundSourceProxy() {
    delete m_pSoundSource;
}

// static
void SoundSourceProxy::loadPlugins() {
    // Scan for and initialize all plugins.
    QList<QDir> pluginDirs;
    QStringList nameFilters;

    const QString& pluginPath = CmdlineArgs::Instance().getPluginPath();
    if (!pluginPath.isEmpty()) {
        qDebug() << "Adding plugin path from commandline arg:" << pluginPath;
        pluginDirs << QDir(pluginPath);
    }

    const QString dataLocation = QDesktopServices::storageLocation(
            QDesktopServices::DataLocation);
    const QString applicationPath = QCoreApplication::applicationDirPath();

#ifdef __LINUX__
    // TODO(rryan): Why can't we use applicationDirPath() and assume it's in the
    // 'bin' folder of $PREFIX, so we just traverse
    // ../lib/mixxx/plugins/soundsource.
    QDir libPluginDir(UNIX_LIB_PATH);
    if (libPluginDir.cd("plugins") && libPluginDir.cd("soundsource")) {
        pluginDirs << libPluginDir;
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("plugins") && dataPluginDir.cd("soundsource")) {
        pluginDirs << dataPluginDir;
    }

    // For people who build from source.
    QDir developer32Root(applicationPath);
    if (developer32Root.cd("lin32_build") && developer32Root.cd("plugins")) {
        pluginDirs << developer32Root.absolutePath();
    }
    QDir developer64Root(applicationPath);
    if (developer64Root.cd("lin64_build") && developer64Root.cd("plugins")) {
        pluginDirs << developer64Root.absolutePath();
    }
#elif __WINDOWS__
    QDir appPluginDir(applicationPath);
    if (appPluginDir.cd("plugins") && appPluginDir.cd("soundsource")) {
        pluginDirs << appPluginDir;
    }
#elif __APPLE__
    // blah/Mixxx.app/Contents/MacOS/../PlugIns/
    // TODO(XXX): Our SCons bundle target doesn't handle plugin subdirectories
    // :( so we can't do:
    //blah/Mixxx.app/Contents/PlugIns/soundsource
    QDir bundlePluginDir(applicationPath);
    if (bundlePluginDir.cdUp() && bundlePluginDir.cd("PlugIns")) {
        pluginDirs << bundlePluginDir;
    }

    // For people who build from source.
    QDir developer32Root(applicationPath);
    if (developer32Root.cd("osx32_build") && developer32Root.cd("plugins")) {
        pluginDirs << developer32Root.absolutePath();
    }
    QDir developer64Root(applicationPath);
    if (developer64Root.cd("osx64_build") && developer64Root.cd("plugins")) {
        pluginDirs << developer64Root.absolutePath();
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("Plugins") && dataPluginDir.cd("soundsource")) {
        pluginDirs << dataPluginDir;
    }

    nameFilters << "libsoundsource*";
#endif

    foreach(QDir dir, pluginDirs) {
        QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);
        foreach (const QString& file, files) {
            getPlugin(dir.filePath(file));
        }
    }
}

// static
Mixxx::SoundSource* SoundSourceProxy::initialize(QString qFilename) {
    QString extension = qFilename;
    extension.remove(0, (qFilename.lastIndexOf(".")+1));
    extension = extension.toLower();

#ifdef __FFMPEGFILE__
    return new SoundSourceFFmpeg(qFilename);
#endif
    if (SoundSourceOggVorbis::supportedFileExtensions().contains(extension)) {
        return new SoundSourceOggVorbis(qFilename);
#ifdef __OPUS__
    } else if (SoundSourceOpus::supportedFileExtensions().contains(extension)) {
        return new SoundSourceOpus(qFilename);
#endif
#ifdef __MAD__
    } else if (SoundSourceMp3::supportedFileExtensions().contains(extension)) {
        return new SoundSourceMp3(qFilename);
#endif
    } else if (SoundSourceFLAC::supportedFileExtensions().contains(extension)) {
        return new SoundSourceFLAC(qFilename);
#ifdef __COREAUDIO__
    } else if (SoundSourceCoreAudio::supportedFileExtensions().contains(extension)) {
        return new SoundSourceCoreAudio(qFilename);
#endif
#ifdef __MODPLUG__
    } else if (SoundSourceModPlug::supportedFileExtensions().contains(extension)) {
        return new SoundSourceModPlug(qFilename);
#endif
    } else if (m_extensionsSupportedByPlugins.contains(extension)) {
        getSoundSourceFunc getter = m_extensionsSupportedByPlugins.value(extension);
        if (getter)
        {
            qDebug() << "Getting SoundSource plugin object for" << extension;
            return getter(qFilename);
        }
        else {
            qDebug() << "Failed to resolve getSoundSource in plugin for" <<
                        extension;
            return NULL; //Failed to load plugin
        }
#ifdef __SNDFILE__
    } else if (SoundSourceSndFile::supportedFileExtensions().contains(extension)) {
        return new SoundSourceSndFile(qFilename);
#endif
    } else { //Unsupported filetype
        return NULL;
    }
}

// static
QLibrary* SoundSourceProxy::getPlugin(QString lib_filename)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    if (m_plugins.contains(lib_filename)) {
        return m_plugins.value(lib_filename);
    }
    QScopedPointer<QLibrary> plugin(new QLibrary(lib_filename));
    if (!plugin->load()) {
    qDebug() << "Failed to dynamically load" << lib_filename << plugin->errorString();
    return NULL;
    }
    qDebug() << "Dynamically loaded" << lib_filename;

    bool incompatible = false;
    //Plugin API version check
    getSoundSourceAPIVersionFunc getver = (getSoundSourceAPIVersionFunc)
        plugin->resolve("getSoundSourceAPIVersion");
    if (getver) {
    int pluginAPIVersion = getver();
    if (pluginAPIVersion != MIXXX_SOUNDSOURCE_API_VERSION) {
        //SoundSource API version mismatch
        incompatible = true;
    }
    } else {
    //Missing getSoundSourceAPIVersion symbol
    incompatible = true;
    }
    if (incompatible)
    {
    //Plugin is using an older/incompatible version of the
    //plugin API!
    qDebug() << "Plugin" << lib_filename << "is incompatible with your version of Mixxx!";
    return NULL;
    }

    //Map the file extensions this plugin supports onto a function
    //pointer to the "getter" function that gets a SoundSourceBlah.
    getSoundSourceFunc getter = (getSoundSourceFunc)
        plugin->resolve("getSoundSource");
    // Getter function not found.
    if (getter == NULL) {
    qDebug() << "ERROR: Couldn't resolve getter function. Plugin"
         << lib_filename << "corrupt.";
    return NULL;
    }

    // Did you export it properly in your plugin?
    getSupportedFileExtensionsFunc getFileExts = (getSupportedFileExtensionsFunc)
        plugin->resolve("supportedFileExtensions");
    if (getFileExts == NULL) {
    qDebug() << "ERROR: Couldn't resolve getFileExts function. Plugin"
         << lib_filename << "corrupt.";
    return NULL;
    }

    freeFileExtensionsFunc freeFileExts =
        reinterpret_cast<freeFileExtensionsFunc>(
        plugin->resolve("freeFileExtensions"));
    if (freeFileExts == NULL) {
    qDebug() << "ERROR: Couldn't resolve freeFileExts function. Plugin"
         << lib_filename << "corrupt.";
    return NULL;
    }

    char** supportedFileExtensions = getFileExts();
    int i = 0;
    while (supportedFileExtensions[i] != NULL) {
    qDebug() << "Plugin supports:" << supportedFileExtensions[i];
    m_extensionsSupportedByPlugins.insert(QString(supportedFileExtensions[i]), getter);
    i++;
    }
    freeFileExts(supportedFileExtensions);

    QLibrary* pPlugin = plugin.take();
    // Add the plugin to our list of loaded QLibraries/plugins and take
    // ownership of the QLibrary from its QScopedPointer so it is not deleted.
    m_plugins.insert(lib_filename, pPlugin);

    // So now we have a list of file extensions (eg. "m4a", "mp4", etc) that map
    // onto the getter function for this plugin (eg. the function that returns a
    // SoundSourceM4A object)
    return pPlugin;
}


Result SoundSourceProxy::open() {
    if (!m_pSoundSource) {
        return ERR;
    }
    Result retVal = m_pSoundSource->open();

    //Update some metadata (currently only the duration)
    //after a song is open()'d. Eg. We don't know the length
    //of VBR MP3s until we've seeked through and counted all
    //the frames. We don't do that in ParseHeader() to keep
    //library scanning fast.
    // .... but only do this if the song doesn't already
    //      have a duration parsed. (Some SoundSources don't
    //      parse metadata on open(), so they won't have the
    //      duration.)
    // SSMP3 will set duration to -1 on VBR files,
    //  so we must look for that here too
    if (m_pTrack && m_pTrack->getDuration() <= 0) {
        m_pTrack->setDuration(m_pSoundSource->getDuration());
    }

    return retVal;
}

long SoundSourceProxy::seek(long l)
{
    if (!m_pSoundSource) {
	return 0;
    }
    return m_pSoundSource->seek(l);
}

unsigned SoundSourceProxy::read(unsigned long size, const SAMPLE * p)
{
    if (!m_pSoundSource) {
    return 0;
    }
    return m_pSoundSource->read(size, p);
}

long unsigned SoundSourceProxy::length()
{
    if (!m_pSoundSource) {
        return 0;
    }
    return m_pSoundSource->length();
}

Result SoundSourceProxy::parseHeader() {
    return m_pSoundSource ? m_pSoundSource->parseHeader() : ERR;
}

QImage SoundSourceProxy::parseCoverArt() {
    return m_pSoundSource ? m_pSoundSource->parseCoverArt() : QImage();
}

// static
QStringList SoundSourceProxy::supportedFileExtensions()
{
    QMutexLocker locker(&m_extensionsMutex);
    QList<QString> supportedFileExtensions;
#ifdef __FFMPEGFILE__
    supportedFileExtensions.append(SoundSourceFFmpeg::supportedFileExtensions());
#endif
#ifdef __MAD__
    supportedFileExtensions.append(SoundSourceMp3::supportedFileExtensions());
#endif
    supportedFileExtensions.append(SoundSourceOggVorbis::supportedFileExtensions());
#ifdef __OPUS__
    supportedFileExtensions.append(SoundSourceOpus::supportedFileExtensions());
#endif
#ifdef __SNDFILE__
    supportedFileExtensions.append(SoundSourceSndFile::supportedFileExtensions());
#endif
#ifdef __COREAUDIO__
    supportedFileExtensions.append(SoundSourceCoreAudio::supportedFileExtensions());
#endif
#ifdef __MODPLUG__
    supportedFileExtensions.append(SoundSourceModPlug::supportedFileExtensions());
#endif
    supportedFileExtensions.append(m_extensionsSupportedByPlugins.keys());

    return supportedFileExtensions;
}

// static
QStringList SoundSourceProxy::supportedFileExtensionsByPlugins() {
    QMutexLocker locker(&m_extensionsMutex);
    QList<QString> supportedFileExtensions;
    supportedFileExtensions.append(m_extensionsSupportedByPlugins.keys());
    return supportedFileExtensions;
}

// static
QString SoundSourceProxy::supportedFileExtensionsString() {
    QStringList supportedFileExtList = SoundSourceProxy::supportedFileExtensions();
    // Turn the list into a "*.mp3 *.wav *.etc" style string
    for (int i = 0; i < supportedFileExtList.size(); ++i) {
    supportedFileExtList[i] = QString("*.%1").arg(supportedFileExtList[i]);
    }
    return supportedFileExtList.join(" ");
}

// static
QString SoundSourceProxy::supportedFileExtensionsRegex() {
    QStringList supportedFileExtList = SoundSourceProxy::supportedFileExtensions();
    return RegexUtils::fileExtensionsRegex(supportedFileExtList);
}

// static
bool SoundSourceProxy::isFilenameSupported(QString fileName) {
    if (m_supportedFileRegex.isValid()) {
        QString regex = SoundSourceProxy::supportedFileExtensionsRegex();
        m_supportedFileRegex = QRegExp(regex, Qt::CaseInsensitive);
    }
    return fileName.contains(m_supportedFileRegex);
}


unsigned int SoundSourceProxy::getSampleRate()
{
    if (!m_pSoundSource) {
    return 0;
    }
    return m_pSoundSource->getSampleRate();
}


QString SoundSourceProxy::getFilename()
{
    if (!m_pSoundSource) {
    return "";
    }
    return m_pSoundSource->getFilename();
}
