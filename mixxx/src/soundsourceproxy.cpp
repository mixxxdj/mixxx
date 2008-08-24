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
#ifdef __M4A__
	#include "soundsourcem4a.h"
#endif
#include "soundsourceoggvorbis.h"
#ifdef __SNDFILE__
#include "soundsourcesndfile.h"
#endif
#ifdef __AUDIOFILE__
#include "soundsourceaudiofile.h"
#endif
#ifdef __FFMPEGFILE__
#include "soundsourceffmpeg.h"
#endif

//Added by qt3to4:
#include <Q3ValueList>


SoundSourceProxy::SoundSourceProxy(QString qFilename) : SoundSource(qFilename)
{
#ifdef __FFMPEGFILE__
    m_pSoundSource = new SoundSourceFFmpeg(qFilename);
    return;
#endif
    if (qFilename.toLower().endsWith(".mp3"))
        m_pSoundSource = new SoundSourceMp3(qFilename);
#ifdef __M4A__
    else if (qFilename.toLower().endsWith(".m4a"))
        m_pSoundSource = new SoundSourceM4A(qFilename);
#endif
    else if (qFilename.toLower().endsWith(".ogg"))
        m_pSoundSource = new SoundSourceOggVorbis(qFilename);
    else
#ifdef __SNDFILE__
        m_pSoundSource = new SoundSourceSndFile(qFilename);
#endif
#ifdef __AUDIOFILE__
        m_pSoundSource = new SoundSourceAudioFile(qFilename);
#endif
}

SoundSourceProxy::SoundSourceProxy(TrackInfoObject * pTrack) : SoundSource(pTrack->getLocation())
{
    QString qFilename = pTrack->getLocation();

#ifdef __FFMPEGFILE__
    m_pSoundSource = new SoundSourceFFmpeg(qFilename);
    return;
#endif

    if (qFilename.toLower().endsWith(".mp3"))
	m_pSoundSource = new SoundSourceMp3(qFilename);
#ifdef __M4A__
    else if (qFilename.toLower().endsWith(".m4a"))
	m_pSoundSource = new SoundSourceM4A(qFilename);
#endif
    else if (qFilename.toLower().endsWith(".ogg"))
	m_pSoundSource = new SoundSourceOggVorbis(qFilename);
    else
    {
#ifdef __SNDFILE__
	m_pSoundSource = new SoundSourceSndFile(qFilename);
#endif
#ifdef __AUDIOFILE__
	m_pSoundSource = new SoundSourceAudioFile(qFilename);
#endif
    }

    if(getSrate()) pTrack->setDuration(length()/(2*getSrate()));
    else pTrack->setDuration(0);
}

SoundSourceProxy::~SoundSourceProxy()
{
    delete m_pSoundSource;
}

long SoundSourceProxy::seek(long l)
{
    return m_pSoundSource->seek(l);
}

unsigned SoundSourceProxy::read(unsigned long size, const SAMPLE * p)
{
    return m_pSoundSource->read(size, p);
}

long unsigned SoundSourceProxy::length()
{
    return m_pSoundSource->length();
}

int SoundSourceProxy::ParseHeader(TrackInfoObject * p)
{
    QString qFilename = p->getFilename();
#ifdef __FFMPEGFILE__
    return SoundSourceFFmpeg::ParseHeader(p);
#endif
    if (qFilename.toLower().endsWith(".mp3"))
	return SoundSourceMp3::ParseHeader(p);
#ifdef __M4A__
    else if (qFilename.toLower().endsWith(".m4a"))
	return SoundSourceM4A::ParseHeader(p);
#endif
    else if (qFilename.toLower().endsWith(".ogg"))
	return SoundSourceOggVorbis::ParseHeader(p);
    else if (qFilename.toLower().endsWith(".wav") || qFilename.toLower().endsWith(".aif") ||
	     qFilename.toLower().endsWith(".aiff") || qFilename.toLower().endsWith(".flac"))
#ifdef __SNDFILE__
	return SoundSourceSndFile::ParseHeader(p);
#endif
#ifdef __AUDIOFILE__
    return SoundSourceAudioFile::ParseHeader(p);
#endif

    return ERR;
}


int SoundSourceProxy::getSrate()
{
    return m_pSoundSource->getSrate();
}

Q3ValueList<long> * SoundSourceProxy::getCuePoints()
{
    return m_pSoundSource->getCuePoints();
}

QString SoundSourceProxy::getFilename()
{
    return m_pSoundSource->getFilename();
}
