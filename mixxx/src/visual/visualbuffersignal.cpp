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
#include "../mathstuff.h"
#include "../controlobjectthreadmain.h"
#include "../controlobject.h"
#include "../enginebuffer.h"
#include "../configobject.h"
#include <qgl.h>

VisualBufferSignal::VisualBufferSignal(ReaderExtract *pReaderExtract, EngineBuffer *pEngineBuffer, const char *group) : VisualBuffer(pReaderExtract, pEngineBuffer, group)
{
//    qDebug("signal: resampleFactor %f, displayRate %f, displayFactor %f, readerExtractFactor %f", m_fResampleFactor, m_fDisplayRate,m_fDisplayFactor, m_fReaderExtractFactor);

    // Ensure a horizontal line is visible
    int i;
    for (i=0; i<m_iLen; i+=2)
        m_pBuffer[i*3+1]=0.05f;

    // Initialize wrap buffer
    m_fWrapBuffer[ 0] = -2;
    m_fWrapBuffer[ 1] = 0;
    m_fWrapBuffer[ 2] = 0;
    m_fWrapBuffer[ 3] = -1;
    m_fWrapBuffer[ 4] = 0;
    m_fWrapBuffer[ 5] = 0;
    m_fWrapBuffer[ 6] = 0;
    m_fWrapBuffer[ 7] = 0;
    m_fWrapBuffer[ 8] = 0;
    m_fWrapBuffer[ 9] = 1;
    m_fWrapBuffer[10] = 0;
    m_fWrapBuffer[11] = 0; 
}

VisualBufferSignal::~VisualBufferSignal()
{
}

void VisualBufferSignal::update(int iPos, int iLen, long int, int)
{
    // Update resample factor
//    m_fResampleFactor = (float)m_pReaderExtract->getRate()/(float)MAXDISPLAYRATE;

    int iStart = (int)floorf((float)iPos/m_fResampleFactor);
    int iEnd   = min((int)ceilf((float)(iPos+iLen)/m_fResampleFactor), m_iLen-1);

    Q_ASSERT(iStart>=0);
    Q_ASSERT(iStart<m_iLen);
    Q_ASSERT(iEnd>=0);
    Q_ASSERT(iEnd<m_iLen);

    float fPositive = 0.;
    float fNegative = 0.;
    if (even(iStart))
    {
        fPositive = m_pBuffer[((iStart-2+m_iLen)%m_iLen)*3+1]*32768.;
        fNegative = m_pBuffer[((iStart-1+m_iLen)%m_iLen)*3+1]*32768.;
    }
    else
    {
        fNegative = m_pBuffer[((iStart-2+m_iLen)%m_iLen)*3+1]*32768.;
        fPositive = m_pBuffer[((iStart-1+m_iLen)%m_iLen)*3+1]*32768.;
    }

    for (int i=iStart; i<=iEnd; ++i)
    {
        int iVisualResampleFactor = m_pReaderExtract->getRate()/kiVisualResampleRate;
        for (int j=i*(int)m_fResampleFactor; j<(i+1)*(int)m_fResampleFactor; j+=iVisualResampleFactor)
        {
            if (m_pSource[j]>=0)
            {
                if (m_pSource[j]>kfWaveshapeFactor*fPositive)
                    fPositive = m_pSource[j];
                else
                    fPositive = kfWaveshapeFactor*fPositive;
            }
            if (m_pSource[j]<=0)
            {
                if (m_pSource[j]<kfWaveshapeFactor*fNegative)
                    fNegative = m_pSource[j];
                else
                    fNegative = kfWaveshapeFactor*fNegative;
            }
        }
        if (even(i))
            m_pBuffer[i*3+1] = max(fPositive/32768.,0.05);
        else
            m_pBuffer[i*3+1] = fNegative/32768.;
    }

    // If updating start or end of buffer, update wrap buffer correspondingly
    if (iEnd>=m_iLen-1)
    {
        m_fWrapBuffer[ 1] = m_pBuffer[((m_iLen-2)*3)+1];
        m_fWrapBuffer[ 4] = m_pBuffer[((m_iLen-1)*3)+1];
    }
    if (iStart<2)
    {
        m_fWrapBuffer[ 7] = m_pBuffer[1];
        m_fWrapBuffer[10] = m_pBuffer[4];
    }
}

void VisualBufferSignal::draw(GLfloat *p, int iLen, float xscale)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    
    //
    // Draw waveform
    //    
    
    // If we draw from start of array, remember to draw two triangles using the coordinates from end of buffer
    if (p==m_pBuffer || p== m_pBuffer+sizeof(float)*3)
    {
        glVertexPointer(3, GL_FLOAT, 0, &m_fWrapBuffer);
        glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    }

    glVertexPointer(3, GL_FLOAT, 0, p);
    glDrawArrays(GL_TRIANGLE_STRIP,0,iLen);
}

