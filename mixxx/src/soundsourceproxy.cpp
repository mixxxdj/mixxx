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

#include "soundsourceproxy.h"
#include "soundsourcemp3.h"
#include "soundsourceoggvorbis.h"
#ifdef __SNDFILE__
#include "soundsourcesndfile.h"
#endif
#ifdef __AUDIOFILE__
#include "soundsourceaudiofile.h"
#endif

SoundSourceProxy::SoundSourceProxy(QString qFilename) : SoundSource(qFilename)
{
    if (qFilename.lower().endsWith(".mp3"))
        m_pSoundSource = new SoundSourceMp3(qFilename);
    else if (qFilename.lower().endsWith(".ogg"))
        m_pSoundSource = new SoundSourceOggVorbis(qFilename);
    else
#ifdef __SNDFILE__
        m_pSoundSource = new SoundSourceSndFile(qFilename);
#endif
#ifdef __AUDIOFILE__
        m_pSoundSource = new SoundSourceAudioFile(qFilename);
#endif
}

SoundSourceProxy::~SoundSourceProxy()
{
    delete m_pSoundSource;
}

long SoundSourceProxy::seek(long l)
{
    return m_pSoundSource->seek(l);
}

unsigned SoundSourceProxy::read(unsigned long size, const SAMPLE*p)
{
    return m_pSoundSource->read(size, p);
}

long unsigned SoundSourceProxy::length()
{
    return m_pSoundSource->length();
}

int SoundSourceProxy::ParseHeader(TrackInfoObject *p)
{
    QString qFilename = p->getFilename();
    if (qFilename.lower().endsWith(".mp3"))
        return SoundSourceMp3::ParseHeader(p);
    else if (qFilename.lower().endsWith(".ogg"))
        return SoundSourceOggVorbis::ParseHeader(p);
    else if (qFilename.lower().endsWith(".wav") || qFilename.lower().endsWith(".aif") ||
             qFilename.lower().endsWith(".aiff"))
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

QString SoundSourceProxy::getFilename()
{
    return m_pSoundSource->getFilename();
}
