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
    qDebug("pos %i-%i",iPos,(iPos+iLen)%m_iSourceLen);
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

void VisualBufferMarks::draw(GLfloat *p, int iLen)
{
    for (int i=0; i<iLen*3; i+=3)
    {
        if (p[i+1]==1)
        {
            glBegin(GL_POLYGON);
            glVertex3f(p[i]-1,-1.,0.);
            glVertex3f(p[i]+1,-1.,0.);
            glVertex3f(p[i]+1, 1.,0.);
            glVertex3f(p[i]-1, 1.,0.);
            glEnd();
        }
    }
}
