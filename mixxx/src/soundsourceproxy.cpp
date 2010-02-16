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

#include <QtDebug>

#include "trackinfoobject.h"
#include "soundsourceproxy.h"
#include "soundsourcemp3.h"
#ifdef __M4A__
	#include "soundsourcem4a.h"
#endif
#include "soundsourceoggvorbis.h"
#ifdef __SNDFILE__
#include "soundsourcesndfile.h"
#endif
#ifdef __FFMPEGFILE__
#include "soundsourceffmpeg.h"
#endif

//Added by qt3to4:
#include <Q3ValueList>


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
#ifdef __M4A__
    else if (qFilename.toLower().endsWith(".m4a") ||
	     qFilename.toLower().endsWith(".mp4"))
	m_pSoundSource = new SoundSourceM4A(qFilename);
#endif
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

int SoundSourceProxy::ParseHeader(TrackInfoObject* p)
{
    if (p == NULL)
	return ERR;

    QString qFilename = p->getFilename();
    qDebug() << "ParseHeader(" << qFilename << ")";

#ifdef __FFMPEGFILE__
    return SoundSourceFFmpeg::ParseHeader(p);
#endif
    if (qFilename.toLower().endsWith(".mp3"))
	return SoundSourceMp3::ParseHeader(p);
    else if (qFilename.toLower().endsWith(".ogg"))
	return SoundSourceOggVorbis::ParseHeader(p);
#ifdef __M4A__
    else if (qFilename.toLower().endsWith(".m4a") ||
	     qFilename.toLower().endsWith(".mp4"))
	return SoundSourceM4A::ParseHeader(p);
#endif
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
