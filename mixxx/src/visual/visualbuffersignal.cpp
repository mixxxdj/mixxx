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
#include "../controlobject.h"
#include "../enginebuffer.h"

VisualBufferSignal::VisualBufferSignal(ReaderExtract *pReaderExtract, EngineBuffer *pEngineBuffer, const char *group) : VisualBuffer(pReaderExtract, pEngineBuffer, group)
{
//    qDebug("signal: resampleFactor %f, displayRate %f, displayFactor %f, readerExtractFactor %f", m_fResampleFactor, m_fDisplayRate,m_fDisplayFactor, m_fReaderExtractFactor);
    m_fLastNegative = 0.;
    m_fLastPositive = 0.;

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
    
    m_fWrapBuffer2[ 0] = -2;
    m_fWrapBuffer2[ 1] = 0;
    m_fWrapBuffer2[ 2] = 0;
    m_fWrapBuffer2[ 3] = -1;
    m_fWrapBuffer2[ 4] = 0;
    m_fWrapBuffer2[ 5] = 0; 
    m_fWrapBuffer2[ 6] = 0;
    m_fWrapBuffer2[ 7] = 0;
    m_fWrapBuffer2[ 8] = 0;
    m_fWrapBuffer2[ 9] = 1;
    m_fWrapBuffer2[10] = 0;
    m_fWrapBuffer2[11] = 0; 
    
    // Allocate secondary buffer in video memory
    m_pBuffer2 = new GLfloat[3*m_iLen];

    // Reset buffer
    GLfloat *p = m_pBuffer2;
    for (i=0; i<m_iLen; i++)
    {
        *p++ = (float)i;
        *p++ = 0.;
        *p++ = 0.;
    }
    
    // Used for temporal (secondary buffer)
    m_pControlPhase = ControlObject::getControl(ConfigKey(group, "temporalPhase"));
    m_pControlShape = ControlObject::getControl(ConfigKey(group, "temporalShape"));
    m_pControlBeatFirst = ControlObject::getControl(ConfigKey(group, "temporalBeatFirst"));
    m_pControlRate = ControlObject::getControl(ConfigKey(group, "rate"));
    m_pControlBpm = ControlObject::getControl(ConfigKey(group, "bpm"));
}

VisualBufferSignal::~VisualBufferSignal()
{
}

void VisualBufferSignal::update(int iPos, int iLen)
{
    // Update resample factor
    //m_fResampleFactor = (float)m_pReaderExtract->getRate()/(float)MAXDISPLAYRATE;

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
/*    
    //
    // Fill secondary buffer with temporal curve * buffer    int iPlaypos = (int)(m_dBufferPlaypos/m_fReaderExtractFactor);
    //
    
    //int iPlaypos = (int)(m_dBufferPlaypos/m_fReaderExtractFactor);
    int iPlaypos = iStart;

    // Update abs and buffer playpos
    m_pEngineBuffer->lockPlayposVars();
    m_dAbsPlaypos = m_pEngineBuffer->getAbsPlaypos();
    m_dBufferPlaypos = m_pEngineBuffer->getBufferPlaypos();
    m_dAbsStartpos = m_pEngineBuffer->getAbsStartpos();
    m_pEngineBuffer->unlockPlayposVars();
        
    // Find out file position of the sample at iStart
    qDebug("start abs %f, abs play %f",m_dAbsStartpos, m_dAbsPlaypos);
    float bufferPlayposDist = m_dAbsPlaypos-m_dAbsStartpos;
            
    float fStartFilePos = (m_dAbsPlaypos-bufferPlayposDist)/m_fReaderExtractFactor;
    
    float fPhaseOffset = m_pControlPhase->getValue();
    float fPeriod = m_pControlBpm->getValue()/(60.*2.); // *2 because it is rectified sinusoid
    float fShape = m_pControlShape->getValue();

    qDebug("abs %f, dist %f, startfilepos %f, iPlaypos %i, total %f", m_dAbsPlaypos, bufferPlayposDist, fStartFilePos, iPlaypos, fStartFilePos+iPlaypos);
                
    // Update from m_dBufferPlaypos and iLen/2 forward
    float fPhaseInc = fPeriod/m_fDisplayRate;
    
    
    int iAbsStartpos = m_dAbsStartpos/m_fResampleFactor;
    int iStartAbs;
    if (iAbsStartpos<iStart)
        iStartAbs = iStart-iAbsStartpos;
    else
        iStartAbs = iStart+iLen-iAbsStartpos;
    
    float fPhase = (iStartAbs+(m_pControlBeatFirst->getValue()/m_fReaderExtractFactor))*fPhaseInc;
    
    // Difference between iPos and iPlaypos
    int i;
    for (i=iStart; i<iEnd; ++i)
    {
        float temp = fPhase-floor((fPhase)/1.);
        m_pBuffer2[((i)*3+1)] = m_pBuffer[((i)*3+1)] *  temp; //wndKaiserSample(256, fShape, temp*256.);

        fPhase += fPhaseInc;
    }
    
    */
    /*
    for (i=iPlaypos; i<iPlaypos+iLen/2; ++i)
    {
        float temp = (fPhase+fPhaseInc)-floor((fPhase+fPhaseInc)/1.);
        m_pBuffer2[((i%m_iLen)*3+1)] = m_pBuffer[((i%m_iLen)*3+1)] * wndKaiserSample(256, fShape, temp*256.);

        fPhase += fPhaseInc;
    }

    // Update from m_dBufferPlaypos and iLen/2 backward
    fPhase = (fStartFilePos+m_pControlBeatFirst->getValue()+(float)iPlaypos)*fPhaseInc;
    for (i=iPlaypos-1; i>iPlaypos-iLen/2; --i)
    {
        float temp = (fPhase+fPhaseInc)-floor((fPhase+fPhaseInc)/1.);
        m_pBuffer2[(((i+m_iLen)%m_iLen)*3+1)] = m_pBuffer[(((i+m_iLen)%m_iLen)*3+1)] * wndKaiserSample(256, fShape, temp*256.);
        
        fPhase -= fPhaseInc;
    }
*/
/*
    // Wrap stuff
    if (iEnd>=m_iLen-1)
    {
        m_fWrapBuffer2[ 1] = m_pBuffer2[((m_iLen-2)*3)+1];
        m_fWrapBuffer2[ 4] = m_pBuffer2[((m_iLen-1)*3)+1];
    }    
    if (iStart<2)
    {    
        m_fWrapBuffer2[ 7] = m_pBuffer2[1];
        m_fWrapBuffer2[10] = m_pBuffer2[4];
    }
*/
}

void VisualBufferSignal::draw(GLfloat *p, int iLen, float)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    
/*
    if (p==m_pBuffer || p== m_pBuffer+sizeof(float)*3)
    {
        glVertexPointer(3, GL_FLOAT, 0, &m_fWrapBuffer2);
        glDrawArrays(GL_LINE_STRIP,0,4);
    }
    
    GLfloat *p2 = m_pBuffer2 + (p-m_pBuffer);
    glVertexPointer(3, GL_FLOAT, 0, p2);
    glDrawArrays(GL_LINE_STRIP,0,iLen);
    
    // Draw secondary, using another color
    float a[4];
    a[0] = 0.5;
    a[1] = 0.5;
    a[2] = 0.5;
    a[3] = 0.5;
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,a);
*/    
    

    // If we draw from start of array, remember to draw two triangles using the coordinates from end of buffer
    if (p==m_pBuffer || p== m_pBuffer+sizeof(float)*3)
    {
        glVertexPointer(3, GL_FLOAT, 0, &m_fWrapBuffer);
        glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    }
//     for (int i=0; i<4; ++i)
//         qDebug("i %i, idx %f, p %f", i, m_fWrapBuffer[i*3], m_fWrapBuffer[(i*3)+1]);

    glVertexPointer(3, GL_FLOAT, 0, p);
    glDrawArrays(GL_TRIANGLE_STRIP,0,iLen);
      
}
