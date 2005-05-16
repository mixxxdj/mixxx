/***************************************************************************
                          visualbuffertemporal.cpp  -  description
                             -------------------
    begin                : Tue Aug 31 2004
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

#include "visualbuffertemporal.h"
#include "../readerextract.h"
#include "../mathstuff.h"
#include "../configobject.h"
#include "../controlobject.h"
#include "../controlobjectthreadmain.h"
#include "../enginetemporal.h"

VisualBufferTemporal::VisualBufferTemporal(EngineBuffer *pEngineBuffer, const char *group) : VisualBuffer(pEngineBuffer, group)
{
    m_pControlPhase = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "temporalPhase")));
    m_pControlShape = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "temporalShape")));
    m_pControlBeatFirst = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "temporalBeatFirst")));
    m_pControlRate = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "rate")));
    m_pControlBpm = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "bpm_control")));
    
    // Ensure a horizontal line is visible
    //for (int i=0; i<m_iLen; i+=2)
    //    m_pBuffer[i*3+1]=0.05f;

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

    connect(m_pControlPhase, SIGNAL(valueChanged(double)), this, SLOT(slotTemporalChanged()));
    connect(m_pControlShape, SIGNAL(valueChanged(double)), this, SLOT(slotTemporalChanged()));

    ControlObjectThreadMain *p = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "playposition")));
    connect(p, SIGNAL(valueChanged(double)), this, SLOT(slotPlayposChanged()));
    connect(&m_qTimer, SIGNAL(timeout()), this, SLOT(update()));

    m_bPlayposChanged = false;
    m_bTemporalChanged = false;
    
    m_iTimesCalled = 0;
    m_qTimer.startTimer(50);
}

VisualBufferTemporal::~VisualBufferTemporal()
{
}

void VisualBufferTemporal::update()
{
    m_iTimesCalled++;
    if (m_bPlayposChanged || m_bTemporalChanged || m_iTimesCalled>10)
    {
        update(0, m_iLen, 0,0 );
        m_bPlayposChanged = false;
        m_bTemporalChanged = false;
        m_iTimesCalled = 0;
    }
}

void VisualBufferTemporal::slotPlayposChanged()
{
    m_bPlayposChanged = true;
    
}

void VisualBufferTemporal::slotTemporalChanged()
{
    m_bTemporalChanged = true;
}

void VisualBufferTemporal::update(int iPos, int iLen, long int, int)
{
    Q_ASSERT(m_fResampleFactor=1);
    Q_ASSERT(iPos==0);

    int iPlaypos = (int)(m_dBufferPlaypos/m_fReaderExtractFactor);

    // Find out file position of the sample at iStart
    float fStartFilePos = (m_dAbsPlaypos-m_dBufferPlaypos)/m_fReaderExtractFactor;
    
    float fPhaseOffset = m_pControlPhase->get();
    float fPeriod = 1./(44100./48000.)*m_pControlBpm->get()/(60.*2.); // *2 because it is rectified sinusoid
            
    // Update from m_dBufferPlaypos and iLen/2 forward
    float fPhaseInc = fPeriod/(float)MAXDISPLAYRATE;
    float fPhase = fPhaseOffset+(fStartFilePos+m_pControlBeatFirst->get()+(float)iPlaypos)*fPhaseInc;
    int i;
    for (i=iPlaypos; i<iPlaypos+iLen/2; ++i)
    {
        float temp = (fPhase+fPhaseInc)-floor((fPhase+fPhaseInc)/1.);
//          qDebug("temp %f",temp);
//         m_pBuffer[((i%m_iLen)*3+1)] = wndKaiserSample(256, fShape, temp*256.);
        m_pBuffer[((i%m_iLen)*3+1)] = EngineTemporal::temporalWindow(m_pControlShape->get(), temp);

        fPhase += fPhaseInc;
    }

    // Update from m_dBufferPlaypos and iLen/2 backward
    fPhase = fPhaseOffset+(fStartFilePos+m_pControlBeatFirst->get()+(float)iPlaypos)*fPhaseInc;
    for (i=iPlaypos-1; i>iPlaypos-iLen/2; --i)
    {
        float temp = (fPhase+fPhaseInc)-floor((fPhase+fPhaseInc)/1.);
//         m_pBuffer[(((i+m_iLen)%m_iLen)*3+1)] = wndKaiserSample(256, fShape, temp*256.);
        m_pBuffer[(((i+m_iLen)%m_iLen)*3+1)] = EngineTemporal::temporalWindow(m_pControlShape->get(), temp);
        
        fPhase -= fPhaseInc;
    }

    // If updating start or end of buffer, update wrap buffer correspondingly
//    if (iEnd>=m_iLen-1)
    {
        m_fWrapBuffer[ 1] = m_pBuffer[((m_iLen-2)*3)+1];
        m_fWrapBuffer[ 4] = m_pBuffer[((m_iLen-1)*3)+1];
    }    
//    if (iStart<2)
    {    
        m_fWrapBuffer[ 7] = m_pBuffer[1];
        m_fWrapBuffer[10] = m_pBuffer[4];
    }
}

void VisualBufferTemporal::draw(GLfloat *p, int iLen, float)
{
/*
    // Transpose using TemporalPhase
    float fBpm = m_pControlBpm->get();
    if (fBpm>60.)
    {
        float fPhaseOffset = m_pControlPhase->get()-0.5;
        float fPeriod = m_fDisplayRate/(fBpm/(60.)); // *2 because it is rectified sinusoid

        glTranslatef(fPhaseOffset*fPeriod,0.,0.);
    }
*/
    
    glEnableClientState(GL_VERTEX_ARRAY);
    //glTranslatef(0,0,0);
    
    glLineWidth(3.);
    
    // If we draw from start of array, remember to draw two triangles using the coordinates from end of buffer
    if (p==m_pBuffer || p== m_pBuffer+sizeof(float)*3)
    {
        glVertexPointer(3, GL_FLOAT, 0, &m_fWrapBuffer);
        glDrawArrays(GL_LINE_STRIP,0,4);
    }    
    glVertexPointer(3, GL_FLOAT, 0, p);
    glDrawArrays(GL_LINE_STRIP,0,iLen);

    // If we draw from start of array, remember to draw two triangles using the coordinates from end of buffer
    glRotatef(180.,1,0,0);
    if (p==m_pBuffer || p== m_pBuffer+sizeof(float)*3)
    {
        glVertexPointer(3, GL_FLOAT, 0, &m_fWrapBuffer);
        glDrawArrays(GL_LINE_STRIP,0,4);
    }    
    glVertexPointer(3, GL_FLOAT, 0, p);
    glDrawArrays(GL_LINE_STRIP,0,iLen);

}
