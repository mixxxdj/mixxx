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



#include "readerextractwave.h"

#include "mathstuff.h"

#include "engineobject.h"

#include "enginebufferscalest.h"

#include "SoundTouch.h"

#include "controlobject.h"




using namespace soundtouch;



EngineBufferScaleST::EngineBufferScaleST(ReaderExtractWave *wave) : EngineBufferScale(wave)

{

    m_pSoundTouch = new soundtouch::SoundTouch(); 

    m_bPitchIndpTimeStretch = false;

    m_dBaseRate = 1.;

    m_dTempo = 1.;

    m_pSoundTouch->setChannels(2);

    m_pSoundTouch->setRate(m_dBaseRate);

    m_pSoundTouch->setTempo(m_dTempo);

    

    slotSetSamplerate(44100.);

    ControlObject *p = ControlObject::getControl(ConfigKey("[Master]","samplerate"));

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

    if (m_bPitchIndpTimeStretch)

        m_pSoundTouch->setRate(1.);

    else

        m_pSoundTouch->setTempo(1.);

}

bool EngineBufferScaleST::getPitchIndpTimeStretch(void)
{
    return m_bPitchIndpTimeStretch;
}



void EngineBufferScaleST::setBaseRate(double dBaseRate)

{

    m_dBaseRate = dBaseRate;

    

    if (m_bPitchIndpTimeStretch)

        m_pSoundTouch->setRate(m_dBaseRate);

    else

        m_pSoundTouch->setRate(m_dBaseRate*m_dTempo);

}



void EngineBufferScaleST::clear()

{

    m_pSoundTouch->clear();

    m_bClear = true;

}



void EngineBufferScaleST::slotSetSamplerate(double dSampleRate)

{

    int iSrate = (int)dSampleRate;

    if (iSrate>0)

        m_pSoundTouch->setSampleRate(iSrate);

    else

        m_pSoundTouch->setSampleRate(44100);

}

    

double EngineBufferScaleST::setTempo(double dTempo)

{

    double dTempoOld = m_dTempo;

    m_dTempo = fabs(dTempo);

    if (m_dTempo>MAX_SEEK_SPEED)

        m_dTempo = MAX_SEEK_SPEED;

        

    if (dTempoOld != m_dTempo)

    {

        if (m_bPitchIndpTimeStretch)

            m_pSoundTouch->setTempo(m_dTempo);

        else

            m_pSoundTouch->setRate(m_dBaseRate*m_dTempo);

    }



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



CSAMPLE *EngineBufferScaleST::scale(double playpos, int buf_size, float *pBase, int iBaseLength)

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

        while (m_pSoundTouch->numSamples()<(unsigned int)(buf_size/2))

        {

            int iLen = math_min(kiSoundTouchReadAheadLength, m_iReadAheadPos/2);

            int j=0;

            for (int i=m_iReadAheadPos-1; i>=m_iReadAheadPos-iLen*2; --i)

            {

                buffer_back[j] = pBase[i];

                j++;

            }

            m_pSoundTouch->putSamples((const SAMPLETYPE *)buffer_back, iLen);

            m_iReadAheadPos = (m_iReadAheadPos-iLen*2+iBaseLength)%iBaseLength;

            if (m_iReadAheadPos==0)

                m_iReadAheadPos = iBaseLength;

        }

        

    }

    else

    {

        while (m_pSoundTouch->numSamples()<(unsigned int)(buf_size/2))

        {

            int iLen = math_min(kiSoundTouchReadAheadLength,(iBaseLength-m_iReadAheadPos)/2);

            m_pSoundTouch->putSamples((const SAMPLETYPE *)&pBase[m_iReadAheadPos], iLen);

            m_iReadAheadPos = (m_iReadAheadPos+iLen*2)%iBaseLength;

            if (m_iReadAheadPos==iBaseLength)

                m_iReadAheadPos = 0;

        }

    }

        

    // Calculate new playpos

    double dFrames = (double)m_pSoundTouch->receiveSamples((SAMPLETYPE *)buffer,(unsigned int)(buf_size/2));

    if (m_bBackwards)

        new_playpos = playpos - m_dTempo*m_dBaseRate*dFrames*2.;

    else

        new_playpos = playpos + m_dTempo*m_dBaseRate*dFrames*2.;

    

    return buffer;

}

