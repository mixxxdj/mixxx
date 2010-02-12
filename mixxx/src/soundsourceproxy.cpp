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
    QString filename = qFilename.toLower();
    if (filename.endsWith(".mp3"))
        m_pSoundSource = new SoundSourceMp3(qFilename);
    else if (filename.endsWith(".ogg") || filename.endsWith(".oga"))
        m_pSoundSource = new SoundSourceOggVorbis(qFilename);
#ifdef __M4A__
    else if (filename.endsWith(".m4a"))
        m_pSoundSource = new SoundSourceM4A(qFilename);
#endif
#ifdef __SNDFILE__
    else if (filename.endsWith(".wav") ||
             filename.endsWith(".aif") ||
             filename.endsWith(".aiff") ||
             filename.endsWith(".flac"))
        m_pSoundSource = new SoundSourceSndFile(qFilename);
#endif
}

SoundSourceProxy::SoundSourceProxy(TrackInfoObject * pTrack) : SoundSource(pTrack->getLocation())
{
    QString qFilename = pTrack->getLocation();

#ifdef __FFMPEGFILE__
    m_pSoundSource = new SoundSourceFFmpeg(qFilename);
    return;
#endif
    QString filename = qFilename.toLower();
    if (filename.endsWith(".mp3"))
        m_pSoundSource = new SoundSourceMp3(qFilename);
    else if (filename.endsWith(".ogg") || filename.endsWith(".oga"))
        m_pSoundSource = new SoundSourceOggVorbis(qFilename);
#ifdef __M4A__
    else if (filename.endsWith(".m4a"))
        m_pSoundSource = new SoundSourceM4A(qFilename);
#endif
#ifdef __SNDFILE__
    else if (filename.endsWith(".wav") ||
             filename.endsWith(".aif") ||
             filename.endsWith(".aiff") ||
             filename.endsWith(".flac"))
        m_pSoundSource = new SoundSourceSndFile(qFilename);
#endif

    if(getSrate()) pTrack->setDuration(length()/(2*getSrate()));
    else pTrack->setDuration(0);
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

int SoundSourceProxy::ParseHeader(TrackInfoObject * p)
{
    QString qFilename = p->getFilename();
#ifdef __FFMPEGFILE__
    return SoundSourceFFmpeg::ParseHeader(p);
#endif
    QString filename = qFilename.toLower();
    if (filename.endsWith(".mp3"))
        return SoundSourceMp3::ParseHeader(p);
    else if (filename.endsWith(".ogg") || filename.endsWith(".oga"))
        return SoundSourceOggVorbis::ParseHeader(p);
#ifdef __M4A__
    else if (filename.endsWith(".m4a"))
        return SoundSourceM4A::ParseHeader(p);
#endif
#ifdef __SNDFILE__
    else if (filename.endsWith(".wav") ||
             filename.endsWith(".aif") ||
             filename.endsWith(".aiff") ||
             filename.endsWith(".flac"))
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
