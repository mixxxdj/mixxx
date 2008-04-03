/***************************************************************************
                          enginebufferscalest.h  -  description
                             -------------------
    begin                : November 2004
    copyright            : (C) 2004 by Tue Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QMutex>
#include "readerextractwave.h"
#include "mathstuff.h"
#include "engineobject.h"
#include "enginebufferscalest.h"
#include "SoundTouch.h"
#include "controlobject.h"

using namespace soundtouch;

EngineBufferScaleST::EngineBufferScaleST(ReaderExtractWave * wave) : EngineBufferScale(wave)
{
    m_qMutex.lock();
    m_pSoundTouch = new soundtouch::SoundTouch();
    m_bPitchIndpTimeStretch = false;
    m_dBaseRate = 1.;
    m_dTempo = 1.;

    m_pSoundTouch->setChannels(2);
    m_pSoundTouch->setRate(m_dBaseRate);
    m_pSoundTouch->setTempo(m_dTempo);
    m_qMutex.unlock();

    slotSetSamplerate(44100.);
    ControlObject * p = ControlObject::getControl(ConfigKey("[Master]","samplerate"));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotSetSamplerate(double)));

    buffer_back = new CSAMPLE[kiSoundTouchReadAheadLength*2];
    m_bBackwards = false;
    m_bClear = true;
}

EngineBufferScaleST::~EngineBufferScaleST()
{
    delete m_pSoundTouch;
    delete [] buffer_back;
}

void EngineBufferScaleST::setPitchIndpTimeStretch(bool b)
{
    m_bPitchIndpTimeStretch = b;
    m_qMutex.lock();
    if (m_bPitchIndpTimeStretch)
        m_pSoundTouch->setRate(1.);
    else
        m_pSoundTouch->setTempo(1.);
    m_qMutex.unlock();
}

bool EngineBufferScaleST::getPitchIndpTimeStretch(void)
{
    return m_bPitchIndpTimeStretch;
}


void EngineBufferScaleST::setBaseRate(double dBaseRate)
{
    m_dBaseRate = dBaseRate;

    m_qMutex.lock();
    if (m_bPitchIndpTimeStretch)
        m_pSoundTouch->setRate(m_dBaseRate);
    //It's an error to pass a rate or tempo smaller than MIN_SEEK_SPEED to SoundTouch.
    else if(m_dTempo >= MIN_SEEK_SPEED)
        m_pSoundTouch->setRate(m_dBaseRate*m_dTempo);
    m_qMutex.unlock();
}


void EngineBufferScaleST::clear()
{
    m_qMutex.lock();
    m_pSoundTouch->clear();
    m_bClear = true;
    m_qMutex.unlock();
}

void EngineBufferScaleST::slotSetSamplerate(double dSampleRate)
{
    int iSrate = (int)dSampleRate;

    m_qMutex.lock();
    if (iSrate>0)
        m_pSoundTouch->setSampleRate(iSrate);
    else
        m_pSoundTouch->setSampleRate(44100);
    m_qMutex.unlock();
}

double EngineBufferScaleST::setTempo(double dTempo)
{
    double dTempoOld = m_dTempo;
    m_dTempo = fabs(dTempo);

    if (m_dTempo>MAX_SEEK_SPEED)
        m_dTempo = MAX_SEEK_SPEED;
    else if (m_dTempo<MIN_SEEK_SPEED)
        m_dTempo = 0.0;

    m_qMutex.lock();
    //It's an error to pass a rate or tempo smaller than MIN_SEEK_SPEED to SoundTouch.
    if (dTempoOld != m_dTempo && m_dTempo != 0.0)
    {
        if (m_bPitchIndpTimeStretch)
            m_pSoundTouch->setTempo(m_dTempo);
        else
            m_pSoundTouch->setRate(m_dBaseRate*m_dTempo);
    }
    m_qMutex.unlock();


    if (dTempo<0.)
    {
        if (!m_bBackwards)
            clear();

        m_bBackwards = true;
        return -m_dTempo;
    }

    else
    {
        if (m_bBackwards)
            clear();

        m_bBackwards = false;
        return m_dTempo;
    }
}

CSAMPLE * EngineBufferScaleST::scale(double playpos, int buf_size, float * pBase, int iBaseLength)
{
    if (!pBase)
    {
        pBase = wavebuffer;
        iBaseLength = READBUFFERSIZE;
    }

    if (m_bClear)
    {
        m_iReadAheadPos = (int)playpos;
        if (!even(m_iReadAheadPos))
            m_iReadAheadPos = (m_iReadAheadPos+1)%iBaseLength;
        m_bClear = false;
    }

    // Invert wavebuffer is backwards playback
    if (m_bBackwards)
    {
        m_qMutex.lock();
        while (m_pSoundTouch->numSamples()<(unsigned int)(buf_size/2))
        {
            int iLen = math_min(kiSoundTouchReadAheadLength, m_iReadAheadPos/2);
            int i = m_iReadAheadPos;
            for(int j=0; j < (iLen * 2); j = j + 2)
            {
            	buffer_back[j] = pBase[i]; //Left channel.
            	buffer_back[j+1] = pBase[i+1]; //Right channel.
            	i = i - 2;
            }
            m_pSoundTouch->putSamples((const SAMPLETYPE *)buffer_back, iLen);
            m_iReadAheadPos = (m_iReadAheadPos-iLen*2+iBaseLength)%iBaseLength;
            if (m_iReadAheadPos==0)
                m_iReadAheadPos = iBaseLength;
        }
        m_qMutex.unlock();
    }

    else
    {
        m_qMutex.lock();
        while (m_pSoundTouch->numSamples()<(unsigned int)(buf_size/2))
        {
            int iLen = math_min(kiSoundTouchReadAheadLength,(iBaseLength-m_iReadAheadPos)/2);
            m_pSoundTouch->putSamples((const SAMPLETYPE *)&pBase[m_iReadAheadPos], iLen);
            m_iReadAheadPos = (m_iReadAheadPos+iLen*2)%iBaseLength;
            if (m_iReadAheadPos==iBaseLength)
                m_iReadAheadPos = 0;
        }
        m_qMutex.unlock();       
    }

    // Calculate new playpos
    m_qMutex.lock();
    double dFrames = (double)m_pSoundTouch->receiveSamples((SAMPLETYPE *)buffer,(unsigned int)(buf_size/2));
    m_qMutex.unlock();
    if (m_bBackwards)
        new_playpos = playpos - m_dTempo*m_dBaseRate*dFrames*2.;
    else
        new_playpos = playpos + m_dTempo*m_dBaseRate*dFrames*2.;

    return buffer;

}

