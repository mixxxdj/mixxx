/***************************************************************************
                          visualbuffersignal.cpp  -  description
                             -------------------
    begin                : Fri Jun 13 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#include "visualbuffersignal.h"
#include "../readerextract.h"

VisualBufferSignal::VisualBufferSignal(ReaderExtract *pReaderExtract, ControlPotmeter *pPlaypos) : VisualBuffer(pReaderExtract, pPlaypos)
{
    // Find resampling factor, and length of own buffer
    CSAMPLE fSignalRate = (CSAMPLE)pReaderExtract->getRate();
    CSAMPLE fFactor = 1.;
    if (DISPLAYRATE<fSignalRate)
        fFactor = DISPLAYRATE/fSignalRate;
    m_iLen = fFactor*m_iSourceLen;

    // Should match update boundary used in EngineBuffer when calling Reader::wake()
    m_iDisplayLen = m_iLen-(2*m_iLen/READCHUNK_NO);

    // Determine resampling and positioning factor
    m_fResampleFactor = ((GLfloat)m_iSourceLen/(GLfloat)m_iLen);

    // Allocate buffer in video memory
    m_pBuffer = allocate(3*m_iLen);


    // Reset buffer
    GLfloat *p = m_pBuffer;
    for (int i=0; i<m_iLen; i++)
    {
        *p++ = (float)i;
        *p++ = 0.; //(float)cos((i/(1.0f*len))*6.28*5+1.5707963267);
        *p++ = 0.;
    }
}

VisualBufferSignal::~VisualBufferSignal()
{
}

void VisualBufferSignal::update(int iPos, int iLen)
{
    int iCpos = (int)((CSAMPLE)iPos/(CSAMPLE)m_fPositionFactor);
    int iClen = (int)((CSAMPLE)iLen/(CSAMPLE)m_fPositionFactor);

    CSAMPLE *pSource = &m_pSource[iCpos];
    GLfloat *pDest = &m_pBuffer[(int)(iCpos/m_fResampleFactor)*3];

    for (int i=0; i<m_iSourceLen/READCHUNK_NO; i+=m_fResampleFactor)
    {
        GLfloat fVal = 0;
        for (int j=i; j<i+m_fResampleFactor; j++)
        {
            fVal += pSource[j]*(1./32768.);
        }
        *pDest++;
        *pDest++ = fVal/m_fResampleFactor;
        *pDest++;
    }
}

void VisualBufferSignal::draw(GLfloat *p, int iLen)
{
    glVertexPointer(3, GL_FLOAT, 0, p);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP,0,iLen);
}
