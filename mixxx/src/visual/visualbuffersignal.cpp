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
//    qDebug("signal: resampleFactor %f, displayRate %f, displayFactor %f, readerExtractFactor %f", m_fResampleFactor, m_fDisplayRate,m_fDisplayFactor, m_fReaderExtractFactor);

}

VisualBufferSignal::~VisualBufferSignal()
{
}

void VisualBufferSignal::update(int iPos, int iLen)
{
    int iCpos = (int)(CSAMPLE)iPos;

    CSAMPLE *pSource = &m_pSource[iCpos];
    GLfloat *pDest = &m_pBuffer[(int)(iCpos/m_fResampleFactor)*3];

    for (int i=0; i<m_iSourceLen/READCHUNK_NO; i+=m_fResampleFactor)
    {
        GLfloat fVal = 0;
        for (int j=i; j<i+m_fResampleFactor; j++)
            fVal += pSource[j]*(1./32768.);

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
