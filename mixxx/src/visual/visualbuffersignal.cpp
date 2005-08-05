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
    
    // Control objects for beat info
    m_pControlBpm = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "file_bpm")));
//     m_pControlBeatFirst = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "BeatFirst")));

    connect(m_pControlBpm, SIGNAL(valueChanged(double)), this, SLOT(slotUpdateBpm(double)));
//     connect(m_pControlBeatFirst, SIGNAL(valueChanged(double)), this, SLOT(slotUpdateBeatFirst(double)));
}

VisualBufferSignal::~VisualBufferSignal()
{
}

void VisualBufferSignal::slotUpdateBpm(double v)
{
    m_dBpm = v;
    m_dBeatDistance = (float)MAXDISPLAYRATE/(m_dBpm/(60.*2.)); // *2 because it is rectified sinusoid

//     qDebug("display rate %f",m_fDisplayRate);
}

void VisualBufferSignal::slotUpdateBeatFirst(double v)
{
    m_dBeatFirst = v;
}

void VisualBufferSignal::update(int iPos, int iLen, long int liFileStartPos, int iBufferStartPos)
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
    
    // Store start positions
    m_liFileStartPos = (int)(floorf((float)liFileStartPos)/m_fResampleFactor);
    m_iBufferStartPos = (int)(floorf((float)iBufferStartPos)/m_fResampleFactor);
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

    // The following code for drawing beat marks is currently very slow and thus disabled 
    return;

    //
    // Draw beat marks
    //
    
    // Ensures constant width of beat marks regardles for scaling
    float kfWidthBeat = 0.05*(1./xscale);

    if (m_dBpm==0.)
        return;
        
    for (int i=0; i<iLen*3; i+=3)
    {
        double fpos = m_liFileStartPos;
        if (p[i]<m_iBufferStartPos)
            fpos += m_iLen-m_iBufferStartPos+p[i];
        else
            fpos += p[i]-m_iBufferStartPos;

        // Is it time to draw a beat mark?
        double pos = fpos/m_dBeatDistance;
        if (fabs(pos-round(pos))<0.01)
        {
            // Color is defined from confidence (between -0.2 and 0.3)
            //float v = 1.-(0.1+max(-0.2,min(p[i+1],0.3)))/0.5;

            //qDebug("v %f, c %f",p[i+1], v);

            // Interpolate ambient of fg and bg using the value at p[i+1]
/*
            float a[4];
            a[0] = m_materialBg.ambient[0]-(m_materialBg.ambient[0]-m_materialFg.ambient[0])*p[i+1];
            a[1] = m_materialBg.ambient[1]-(m_materialBg.ambient[1]-m_materialFg.ambient[1])*p[i+1];
            a[2] = m_materialBg.ambient[2]-(m_materialBg.ambient[2]-m_materialFg.ambient[2])*p[i+1];
            a[3] = m_materialBg.ambient[3]-(m_materialBg.ambient[3]-m_materialFg.ambient[3])*p[i+1];
            glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,a);
*/

            glBegin(GL_POLYGON);
            glVertex3f(p[i]-kfWidthBeat,-1.  ,0.);
            glVertex3f(p[i]+kfWidthBeat,-1.  ,0.);
            glVertex3f(p[i]+kfWidthBeat,-0.8f,0.);
            glVertex3f(p[i]-kfWidthBeat,-0.8f,0.);
            glEnd();
            glBegin(GL_POLYGON);
            glVertex3f(p[i]-kfWidthBeat, 0.8f,0.);
            glVertex3f(p[i]+kfWidthBeat, 0.8f,0.);
            glVertex3f(p[i]+kfWidthBeat, 1.0f,0.);
            glVertex3f(p[i]-kfWidthBeat, 1.0f,0.);
            glEnd();
        }
    }
          
}

