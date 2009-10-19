/***************************************************************************
                          enginebufferscalereal.cpp  -  description
                            -------------------
    begin                : Fri Feb 25 2005
    copyright            : (C) 2005 by Tue Haste Andersen
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

#include "enginebufferscalereal.h"
#include "mathstuff.h"
#include "engineobject.h"
#include "controlobject.h"

EngineBufferScaleReal::EngineBufferScaleReal() : EngineBufferScale()
{
    m_pControlObjectSampleRate = ControlObject::getControl(ConfigKey("[Master]","samplerate"));
    m_pFragmentBuffer = new float[(int)(96000.*2.*kfRealSearchFragmentLength)];
    m_dBaseRate = 1.;
    m_dTempo = 1.;
    clear();
}

EngineBufferScaleReal::~EngineBufferScaleReal()
{
    delete [] m_pFragmentBuffer;
}

double EngineBufferScaleReal::setTempo(double dTempo)
{
    m_dTempo = dTempo;

    Q_ASSERT(fabs(m_dTempo)>1.);

    // Determine playback direction
    if (m_dTempo<0.)
        m_bBackwards = true;
    else
        m_bBackwards = false;

    return m_dTempo;
}

void EngineBufferScaleReal::setBaseRate(double dBaseRate)
{
    m_dBaseRate = dBaseRate;
}


void EngineBufferScaleReal::clear()
{
    m_iFragmentPlaypos = 0;
    m_iFragmentLength  = 0;
}

CSAMPLE* EngineBufferScaleReal::scale(double playpos, int buf_size,
                                      CSAMPLE* pBase, int iBaseLength) {
    if (m_dTempo<1.)
        return buffer;

    // Copy samples to fragment buffer if necessary
    int iWantedFragmentLength = (int)(m_pControlObjectSampleRate->get()*kfRealSearchFragmentLength)*2;

    //qDebug() << "len " << m_iFragmentLength << ", want " << iWantedFragmentLength << ", pos " << m_iFragmentPlaypos << ", tempo " << m_dTempo;

    if (m_iFragmentLength<iWantedFragmentLength)
    {
        int iTo = (int)math_min(iWantedFragmentLength,m_iFragmentLength+buf_size*m_dTempo*m_dBaseRate);
        for (int i=m_iFragmentLength; i<iTo; ++i)
            m_pFragmentBuffer[i] = pBase[((int)playpos+i)%iBaseLength];
        m_iFragmentLength = iTo;
    }

    // Copy samples to output buffer
    int i = 0;
    while (m_iFragmentPlaypos < m_iFragmentLength && i<buf_size)
        buffer[++i] = m_pFragmentBuffer[++m_iFragmentPlaypos];

    // Start copying new bunch of samples to fragment buffer
    if (i<buf_size)
    {
        clear();

/*
        int iTo = (int)math_min(iWantedFragmentLength,m_iFragmentLength+buf_size*m_dTempo*m_dBaseRate);
        double new_playpos = playpos+(double)i*m_dTempo;
        for (int j=m_iFragmentLength; j<iTo; ++i)
            m_pFragmentBuffer[j] = pBase[((int)new_playpos+j)%iBaseLength];
        m_iFragmentLength = iTo;

        // And again, copy samples to output buffer
        while (m_iFragmentPlaypos < m_iFragmentLength && i<buf_size)
            buffer[++i] = m_pFragmentBuffer[++m_iFragmentPlaypos];
 */
    }

    new_playpos = playpos + buf_size*m_dTempo;

    return buffer;
}

