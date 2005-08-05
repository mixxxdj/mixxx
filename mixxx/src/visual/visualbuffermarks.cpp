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

VisualBufferMarks::VisualBufferMarks(ReaderExtract *pReaderExtract, EngineBuffer *pEngineBuffer, const char *group) : VisualBuffer(pReaderExtract, pEngineBuffer, group)
{
    m_pCuePoint = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "cue_point")));
//    qDebug("marks: resampleFactor %f, displayRate %f, displayFactor %f, readerExtractFactor %f", m_fResampleFactor, m_fDisplayRate,m_fDisplayFactor, m_fReaderExtractFactor);
    m_iCuePosition = -1;

    connect(m_pCuePoint, SIGNAL(valueChanged(double)), this, SLOT(slotUpdateCuePoint(double)));
}

VisualBufferMarks::~VisualBufferMarks()
{
}

void VisualBufferMarks::update(int iPos, int iLen, long int, int)
{
//    qDebug("mark upd pos %i, len %i",iPos,iLen);

    CSAMPLE *pSource = &m_pSource[iPos];
    GLfloat *pDest = &m_pBuffer[iPos*3];

    int temp = min(iLen, m_iSourceLen-iPos-1);
    for (int i=0; i<=temp; ++i)
    {
        *pDest++;
        *pDest++ = pSource[i];
        *pDest++;
    }

    if (temp<iLen)
    {
        pSource = &m_pSource[0];
        pDest = &m_pBuffer[0];
        for (int i=0; i<=iLen-temp; ++i)
        {
            *pDest++;
            *pDest++ = pSource[i];
            *pDest++;
        }
    }

    slotUpdateCuePoint(m_pCuePoint->get());
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

void VisualBufferMarks::draw(GLfloat *p, int iLen, float xscale)
{
//    glDrawArrays(GL_LINE_STRIP,0,iLen);

    // Ensures constant width of beat marks regardles for scaling
    float kfWidthBeat = 0.05*(1./xscale);
    float kfWidthCue =  0.15*(1./xscale);

    for (int i=0; i<iLen*3; i+=3)
    {
        if (m_iCuePosition!=-1 && p[i]==m_iCuePosition)
        {
            // Cue point
            float a[4];
            a[0] = 0.; //m_materialFg.ambient[0];
            a[1] = 0.; //m_materialFg.ambient[1];
            a[2] = 0.; //m_materialFg.ambient[2];
            a[3] = 1.; //m_materialFg.ambient[3];
            glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,a);

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
        else if (p[i+1]!=0.)
        {
            // Color is defined from confidence (between -0.2 and 0.3)
            //float v = 1.-(0.1+max(-0.2,min(p[i+1],0.3)))/0.5;

            //qDebug("v %f, c %f",p[i+1], v);

            // Interpolate ambient of fg and bg using the value at p[i+1]
            float a[4];
            a[0] = m_materialBg.ambient[0]-(m_materialBg.ambient[0]-m_materialFg.ambient[0])*p[i+1];
            a[1] = m_materialBg.ambient[1]-(m_materialBg.ambient[1]-m_materialFg.ambient[1])*p[i+1];
            a[2] = m_materialBg.ambient[2]-(m_materialBg.ambient[2]-m_materialFg.ambient[2])*p[i+1];
            a[3] = m_materialBg.ambient[3]-(m_materialBg.ambient[3]-m_materialFg.ambient[3])*p[i+1];
            glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,a);

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

void VisualBufferMarks::setMaterialCue(Material *pMaterial)
{
    m_pMaterialCue = pMaterial;
};
