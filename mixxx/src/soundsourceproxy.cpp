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

#include <Q3ValueList>
#include <QLibrary>
#include <QDebug>

//M4A Plugin filename
#define PLUGIN_M4A "libsoundsourcem4a"

//Static memory allocation
QMap<QString, QLibrary*> SoundSourceProxy::m_plugins;

SoundSourceProxy::SoundSourceProxy(QString qFilename)
	: SoundSource(qFilename),
	  m_pSoundSource(NULL) {
    initialize(qFilename);
}

SoundSourceProxy::SoundSourceProxy(TrackInfoObject * pTrack)
	: SoundSource(pTrack->getLocation()),
	  m_pSoundSource(NULL) {
    initialize(pTrack->getLocation());

    // Set the track duration in seconds
    if(getSrate())
	pTrack->setDuration(length()/(2*getSrate()));
    else
	pTrack->setDuration(0);
}

void SoundSourceProxy::initialize(QString qFilename) {

#ifdef __FFMPEGFILE__
    m_pSoundSource = new SoundSourceFFmpeg(qFilename);
    return;
#endif

    if (qFilename.toLower().endsWith(".mp3"))
	m_pSoundSource = new SoundSourceMp3(qFilename);
    else if (qFilename.toLower().endsWith(".ogg"))
	m_pSoundSource = new SoundSourceOggVorbis(qFilename);

    else if (qFilename.toLower().endsWith(".m4a") ||
	     qFilename.toLower().endsWith(".mp4")) 
    {
        initPlugin(PLUGIN_M4A, qFilename);
    }
	
#ifdef __SNDFILE__
    else if (qFilename.toLower().endsWith(".wav") ||
	     qFilename.toLower().endsWith(".aif") ||
	     qFilename.toLower().endsWith(".aiff") ||
	     qFilename.toLower().endsWith(".flac"))
	m_pSoundSource = new SoundSourceSndFile(qFilename);
#endif
}

SoundSourceProxy::~SoundSourceProxy()
{
    delete m_pSoundSource;
}

void SoundSourceProxy::initPlugin(QString lib_filename, QString track_filename) {
    typedef SoundSource* (*getSoundSource)(QString filename);
    
    QLibrary* plugin = getPlugin(lib_filename);
    
    getSoundSource getter = (getSoundSource)plugin->resolve("getSoundSource");
    if (getter)
    {
        m_pSoundSource = getter(track_filename);
    }
}

QLibrary* SoundSourceProxy::getPlugin(QString lib_filename)
{
    QLibrary* plugin;
    if (m_plugins.contains(lib_filename))
    	plugin = m_plugins.value(lib_filename);
    else {
    	plugin = new QLibrary(lib_filename);
	if (!plugin->load())
 	    qDebug() << "Failed to dynamically load" << lib_filename;
	else
	    qDebug() << "Dynamically loaded" << lib_filename;        
    	m_plugins.insert(lib_filename, plugin);
    }
    return plugin;
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


unsigned int SoundSourceProxy::getSrate()
{
    if (!m_pSoundSource) {
	return 0;
    }
    return m_pSoundSource->getSrate();
}

Q3ValueList<long> * SoundSourceProxy::getCuePoints()
{
    if (!m_pSoundSource) {
	return NULL;
    }
    return m_pSoundSource->getCuePoints();
}

QString SoundSourceProxy::getFilename()
{
    if (!m_pSoundSource) {
	return "";
    }
    return m_pSoundSource->getFilename();
}
