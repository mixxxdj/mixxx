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
#include "../readerextract.h"

VisualBufferMarks::VisualBufferMarks(ReaderExtract *pReaderExtract, ControlPotmeter *pPlaypos) : VisualBuffer(pReaderExtract, pPlaypos)
{
//    qDebug("marks: resampleFactor %f, displayRate %f, displayFactor %f, readerExtractFactor %f", m_fResampleFactor, m_fDisplayRate,m_fDisplayFactor, m_fReaderExtractFactor);
}

VisualBufferMarks::~VisualBufferMarks()
{
}
                                                                                   
void VisualBufferMarks::update(int iPos, int iLen)
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
}

void VisualBufferMarks::draw(GLfloat *p, int iLen, float xscale)
{
//    glDrawArrays(GL_LINE_STRIP,0,iLen);

    // Ensures constant width of beat marks regardles for scaling
    float kfWidth = 1.*(1./xscale);

    for (int i=0; i<iLen*3; i+=3)
    {
        if (p[i+1]>0.)
        {
/*
            float array[12];
            array[ 0] = p[i]+p[i+1]-kfWidth;
            array[ 1] = -1.;
            array[ 2] = p[i+2];
            array[ 3] = p[i]+p[i+1]+kfWidth;
            array[ 4] = -1.;
            array[ 5] = p[i+2];
            array[ 6] = p[i]+p[i+1]+kfWidth;
            array[ 7] = 1.;
            array[ 8] = p[i+2];
            array[ 9] = p[i]+p[i+1]-kfWidth;
            array[10] = 1.;
            array[11] = p[i+2];

            glVertexPointer(3, GL_FLOAT, 0, array);
            glEnableClientState(GL_VERTEX_ARRAY);
            glDrawArrays(GL_POLYGON, 0,4);
*/
/*
            glVertex3f(array[ 0], array[ 1], array[ 2]);
            glVertex3f(array[ 3], array[ 4], array[ 5]);
            glVertex3f(array[ 6], array[ 7], array[ 8]);
            glVertex3f(array[ 9], array[10], array[11]);
*/

            glBegin(GL_POLYGON);
            glVertex3f(p[i]-kfWidth,-1.  ,0.);
            glVertex3f(p[i]+kfWidth,-1.  ,0.);
            glVertex3f(p[i]+kfWidth,-0.8,0.);
            glVertex3f(p[i]-kfWidth,-0.8,0.);
            glEnd();
            glBegin(GL_POLYGON);
            glVertex3f(p[i]-kfWidth, 0.8 ,0.);
            glVertex3f(p[i]+kfWidth, 0.8 ,0.);
            glVertex3f(p[i]+kfWidth, 1.0 ,0.);
            glVertex3f(p[i]-kfWidth, 1.0 ,0.);
            glEnd();
        }
    }
}
