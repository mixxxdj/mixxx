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

#include "trackinfoobject.h"
#include "soundsourceproxy.h"
#include "soundsourcemp3.h"
#include "soundsourceoggvorbis.h"
#ifdef __SNDFILE__
#include "soundsourcesndfile.h"
#endif
#ifdef __FFMPEGFILE__
#include "soundsourceffmpeg.h"
#endif

#include <QLibrary>
#include <QMutexLocker>
#include <QMutex>
#include <QDebug>

//M4A Plugin filename
#define PLUGIN_M4A "libsoundsourcem4a"

//Static memory allocation
QMap<QString, QLibrary*> SoundSourceProxy::m_plugins;
QMap<QString, getSoundSourceFunc> SoundSourceProxy::m_extensionsSupportedByPlugins;
QMutex SoundSourceProxy::m_extensionsMutex;


//Constructor
SoundSourceProxy::SoundSourceProxy(QString qFilename)
	: SoundSource(qFilename),
	  m_pSoundSource(NULL) {
    m_pSoundSource = initialize(qFilename);
   
}

//Other constructor
SoundSourceProxy::SoundSourceProxy(TrackInfoObject * pTrack)
	: SoundSource(pTrack->getLocation()),
	  m_pSoundSource(NULL) {

    m_pSoundSource = initialize(pTrack->getLocation());

    // Set the track duration in seconds
    if(getSampleRate())
	    pTrack->setDuration(length()/(2*getSampleRate()));
    else
	    pTrack->setDuration(0);
}

void SoundSourceProxy::loadPlugins()
{
   /** TODO: Scan for and initialize all plugins in a static function here,
             and call it from mixxx.cpp */
   getPlugin(PLUGIN_M4A);
}

SoundSource* SoundSourceProxy::initialize(QString qFilename) {

    SoundSource* sndsrc = NULL;
    QString extension = qFilename;
    extension.remove(0, (qFilename.lastIndexOf(".")+1));
    extension = extension.toLower();

#ifdef __FFMPEGFILE__
    return new SoundSourceFFmpeg(qFilename);
#endif

    if (SoundSourceMp3::supportedFileExtensions().contains(extension)) {
	    return new SoundSourceMp3(qFilename);
    } else if (SoundSourceOggVorbis::supportedFileExtensions().contains(extension)) {
	    return new SoundSourceOggVorbis(qFilename);
    } else if (m_extensionsSupportedByPlugins.contains(extension)) {
    	//QLibrary* plugin = getPlugin(PLUGIN_M4A);
        //getSoundSourceFunc getter = (getSoundSourceFunc)plugin->resolve("getSoundSource");
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

SoundSourceProxy::~SoundSourceProxy()
{
    delete m_pSoundSource;
}
/*
void SoundSourceProxy::initPlugin(QString lib_filename, QString track_filename) {
    //typedef QString (*getMixxxVersionFunc)();
    
    QLibrary* plugin = getPlugin(lib_filename);
    
    getSoundSourceFunc getter = (getSoundSourceFunc)plugin->resolve("getSoundSource");
    //getMixxxVersionFunc getMixxxVersion = (getMixxxVersionFunc)plugin->resolve("getMixxxVersion");
    if (getter)
    {
        m_pSoundSource = getter(track_filename);
    }
}
*/

QLibrary* SoundSourceProxy::getPlugin(QString lib_filename)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    QLibrary* plugin;
    if (m_plugins.contains(lib_filename))
    	plugin = m_plugins.value(lib_filename);
    else {
    	plugin = new QLibrary(lib_filename);
        if (!plugin->load()) {
            qDebug() << "Failed to dynamically load" << lib_filename << plugin->errorString();
        } else {
            qDebug() << "Dynamically loaded" << lib_filename;        
            //Add the plugin to our list of loaded QLibraries/plugins
            m_plugins.insert(lib_filename, plugin);

            //Map the file extensions this plugin supports onto a function
            //pointer to the "getter" function that gets a SoundSourceBlah.
            getSoundSourceFunc getter = (getSoundSourceFunc)plugin->resolve("getSoundSource");
            Q_ASSERT(getter); //Getter function not found. 
                              //Did you export it properly in your plugin?
            getSupportedFileExtensionsFunc getFileExts = (getSupportedFileExtensionsFunc)plugin->resolve("supportedFileExtensions");
            Q_ASSERT(getFileExts);
            QList<QString> supportedFileExtensions = getFileExts();
            for (int i = 0; i < supportedFileExtensions.count(); i++)
            {
                m_extensionsSupportedByPlugins.insert(supportedFileExtensions[i], getter);
            }
            //So now we have a list of file extensions (eg. "m4a", "mp4", etc)            //that map onto the getter function for this plugin (eg. the 
            //function that returns a SoundSourceM4A object) 
        }
    }
    return plugin;
}


int SoundSourceProxy::open()
{
    if (!m_pSoundSource) {
	return 0;
    }
    return m_pSoundSource->open();
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

int SoundSourceProxy::parseHeader()
{
    //TODO: Reorganize code so that the static ParseHeader isn't needed?
    return 0;
}

int SoundSourceProxy::ParseHeader(TrackInfoObject * p)
{
    
    QString qFilename = p->getLocation();
    SoundSource* sndsrc = initialize(qFilename);
    if (sndsrc == NULL)
        return ERR;

    if (sndsrc->parseHeader() == OK) {
        //Dump the metadata from the soundsource into the TIO
        qDebug() << "Album:" << sndsrc->getAlbum();
        p->setArtist(sndsrc->getArtist());
        p->setTitle(sndsrc->getTitle());
        p->setAlbum(sndsrc->getAlbum());
        p->setType(sndsrc->getType());
        p->setYear(sndsrc->getYear());
        p->setGenre(sndsrc->getGenre());
        p->setTrackNumber(sndsrc->getTrackNumber());
        p->setBpm(sndsrc->getBPM());
        p->setDuration(sndsrc->getDuration());
        p->setBitrate(sndsrc->getBitrate());
        p->setSampleRate(sndsrc->getSampleRate());
        p->setChannels(sndsrc->getChannels());
        p->setHeaderParsed(true);
    }
    else
    {
        p->setHeaderParsed(false);
    }
    delete sndsrc;

    return 0;
}

QList<QString> SoundSourceProxy::supportedFileExtensions()
{
    //TODO: Fill me in...
    qDebug() << "STUB: SSP::supportedFileExtensions()" << __FILE__ <<__LINE__;

    QMutexLocker locker(&m_extensionsMutex);
    QList<QString> supportedFileExtensions;
    supportedFileExtensions.append(SoundSourceMp3::supportedFileExtensions());
    supportedFileExtensions.append(SoundSourceOggVorbis::supportedFileExtensions());
    supportedFileExtensions.append(SoundSourceSndFile::supportedFileExtensions());
    supportedFileExtensions.append(m_extensionsSupportedByPlugins.keys());
    
    return supportedFileExtensions;
}

QString SoundSourceProxy::supportedFileExtensionsString()
{
    QList<QString> supportedFileExtList = SoundSourceProxy::supportedFileExtensions();
    QString supportedFileExtString;

    //Turn the list into a "*.mp3 *.wav *.etc" style string
    QString ext;
    QListIterator<QString> it(supportedFileExtList);
    while (it.hasNext())
    {
        ext = it.next();
        supportedFileExtString.append(QString("*.%1").arg(ext));
        if (it.hasNext())
            supportedFileExtString.append(" ");
    }
    
    return supportedFileExtString;
}

QString SoundSourceProxy::supportedFileExtensionsRegex()
{
    QList<QString> supportedFileExtList = SoundSourceProxy::supportedFileExtensions();
    QString supportedFileExtRegex = "\\.(";

    //Turn the list into a "\\.(mp3|wav|etc)" style regex string
    QString ext;
    QListIterator<QString> it(supportedFileExtList);
    while (it.hasNext())
    {
        ext = it.next();
        supportedFileExtRegex.append(QString("%1").arg(ext));
        if (it.hasNext())
            supportedFileExtRegex.append("|");
    }
    supportedFileExtRegex.append(")");
    
    return supportedFileExtRegex;
}

/*
int SoundSourceProxy::ParseHeader(TrackInfoObject * p)
{
    typedef int (*ParseHeaderFunction)(TrackInfoObject*);
    
    QString qFilename = p->getFilename();
#ifdef __FFMPEGFILE__
    return SoundSourceFFmpeg::ParseHeader(p);
#endif
    if (qFilename.toLower().endsWith(".mp3"))
	return SoundSourceMp3::ParseHeader(p);
    else if (qFilename.toLower().endsWith(".ogg"))
	return SoundSourceOggVorbis::ParseHeader(p);

    else if (qFilename.toLower().endsWith(".m4a") ||
	     qFilename.toLower().endsWith(".mp4"))
    {
    	QLibrary* plugin = getPlugin(PLUGIN_M4A);
    	ParseHeaderFunction parseFn = (ParseHeaderFunction)plugin->resolve("getParseHeader");
    	if (parseFn)
        {
            return parseFn(p);
        }
    }

#ifdef __SNDFILE__
    else if (qFilename.toLower().endsWith(".wav") ||
	     qFilename.toLower().endsWith(".aif") ||
	     qFilename.toLower().endsWith(".aiff") ||
	     qFilename.toLower().endsWith(".flac"))
	return SoundSourceSndFile::ParseHeader(p);
#endif

    return ERR;
}
*/


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
