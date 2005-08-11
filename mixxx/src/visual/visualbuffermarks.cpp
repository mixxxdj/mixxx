/***************************************************************************
                          visualbuffermarks.cpp  -  description
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

#include "visualbuffermarks.h"
#include "readerextract.h"
#include "controlobject.h"
#include "configobject.h"
#include "controlobjectthreadmain.h"
#include "player.h"
#include "mathstuff.h"
#include <qgl.h>

VisualBufferMarks::VisualBufferMarks(ReaderExtract *pReaderExtract, EngineBuffer *pEngineBuffer, const char *group) : VisualBuffer(pReaderExtract, pEngineBuffer, group)
{
    m_iCuePosition = -1;
    m_dBeatDistance = 0.;
    m_dBeatFirst = 0.;
    m_dBpm = 0.;

    m_pCuePoint = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "cue_point")));
    connect(m_pCuePoint, SIGNAL(valueChanged(double)), this, SLOT(slotUpdateCuePoint(double)));

    // Control objects for beat info
    m_pControlBpm = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "file_bpm")));
//    m_pControlBeatFirst = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "BeatFirst")));

    connect(m_pControlBpm, SIGNAL(valueChanged(double)), this, SLOT(slotUpdateBpm(double)));
//    connect(m_pControlBeatFirst, SIGNAL(valueChanged(double)), this, SLOT(slotUpdateBeatFirst(double)));
    
    //    qDebug("marks: resampleFactor %f, displayRate %f, displayFactor %f, readerExtractFactor %f", m_fResampleFactor, m_fDisplayRate,m_fDisplayFactor, m_fReaderExtractFactor);
}

VisualBufferMarks::~VisualBufferMarks()
{
}

void VisualBufferMarks::update(int iPos, int iLen, long int liFileStartPos, int iBufferStartPos)
{
    slotUpdateCuePoint(m_pCuePoint->get());

    // Store start positions
    m_liFileStartPos = (int)(floorf((float)liFileStartPos)/m_fResampleFactor);
    m_iBufferStartPos = (int)(floorf((float)iBufferStartPos)/m_fResampleFactor);
}

void VisualBufferMarks::slotUpdateCuePoint(double v)
{
    // Find index in buffer where cue point should be displayed. Is set to -1 if cue point is
    // currently not in the visible buffer
    m_iCuePosition = -1;

    if (v>=0 && ((fabs(v-m_dAbsPlaypos)/m_fReaderExtractFactor)/m_fResampleFactor)<(float)m_iDisplayLen/2.f)
    {
        //qDebug("cue %f, play %f",m_pCuePoint->get(),m_pAbsPlaypos->get());
        float fCuediff = m_dAbsPlaypos-v;
        float fCuePos = m_dBufferPlaypos-fCuediff;
        fCuePos = ((fCuePos/m_fReaderExtractFactor)/m_fResampleFactor)-m_pLatency->get()*MAXDISPLAYRATE;
        while (fCuePos<0)
            fCuePos += (float)m_iLen;
        m_iCuePosition = (int)fCuePos;
    }
}

void VisualBufferMarks::slotUpdateBpm(double v)
{
    m_dBpm = v;
    m_dBeatDistance = (float)MAXDISPLAYRATE/(m_dBpm/(60.*2.)); // *2 because it is rectified sinusoid

    // Update internal buffer

//     qDebug("display rate %f",m_fDisplayRate);
}

void VisualBufferMarks::slotUpdateBeatFirst(double v)
{
    m_dBeatFirst = v;
}

void VisualBufferMarks::draw(GLfloat *p, int iLen, float xscale)
{
    // Ensures constant width of beat marks regardles for scaling
    float kfWidthBeat = 0.05*(1./xscale);
    float kfWidthCue =  0.15*(1./xscale);

    // Set color
    float a[4];
    a[0] = m_materialFg.ambient[0];
    a[1] = m_materialFg.ambient[1];
    a[2] = m_materialFg.ambient[2];
    a[3] = m_materialFg.ambient[3];
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,a);

    // Draw marks
    for (int i=0; i<iLen*3; i+=3)
    {
        if (m_iCuePosition!=-1 && p[i]==m_iCuePosition)
        {
            //
            // Cue point
            //
            
            glBegin(GL_POLYGON);
            glVertex3f(p[i], 0.8f,0.);
            glVertex3f(p[i]+kfWidthCue, 1.0f,0.);
            glVertex3f(p[i]-kfWidthCue, 1.0f,0.);
            glEnd();
            glBegin(GL_POLYGON);
            glVertex3f(p[i]-kfWidthCue,-1.  ,0.);
            glVertex3f(p[i]+kfWidthCue,-1.  ,0.);
            glVertex3f(p[i], -0.8f,0.);
            glEnd();
        }
        else if (m_dBpm>0.)
        {
            //
            // Draw beat marks
            //
            
            double fpos = m_liFileStartPos;
            if (p[i]<m_iBufferStartPos)
                fpos += m_iLen-m_iBufferStartPos+p[i];
            else
                fpos += p[i]-m_iBufferStartPos;

            // Is it time to draw a beat mark?
            double pos = fpos/m_dBeatDistance;
            if (fabs(pos-round(pos))<0.01)
            {
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
}

void VisualBufferMarks::setMaterialCue(Material *pMaterial)
{
    m_pMaterialCue = pMaterial;
};
