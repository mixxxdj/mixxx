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

VisualBufferSignal::VisualBufferSignal(ReaderExtract *pReaderExtract, const char *group) : VisualBuffer(pReaderExtract, group)
{
//    qDebug("signal: resampleFactor %f, displayRate %f, displayFactor %f, readerExtractFactor %f", m_fResampleFactor, m_fDisplayRate,m_fDisplayFactor, m_fReaderExtractFactor);
    m_fLastNegative = 0.;
    m_fLastPositive = 0.;
}

VisualBufferSignal::~VisualBufferSignal()
{
}

void VisualBufferSignal::update(int iPos, int iLen)
{
//    qDebug("signal upd pos %i, len %i, total len %i",iPos,iLen,m_iSourceLen);
    
    CSAMPLE *pSource = &m_pSource[iPos];
    GLfloat *pDest = &m_pBuffer[(int)(iPos/m_fResampleFactor)*3];

    float temp = min(iLen, m_iSourceLen-iPos-1);
    
    //bool bEven = true;
    for (float i=0; i<=temp-m_fResampleFactor; i+=m_fResampleFactor)
    {
        GLfloat fVal = 0;
        for (int j=(int)i; j<(int)(i+m_fResampleFactor); j+=32)
            fVal += pSource[j];
        
/*
        if (fVal>=0.)
            m_fLastPositive = 0.75*m_fLastPositive+0.25*fVal;
        if (fVal<=0.)
            m_fLastNegative = 0.75*m_fLastNegative+0.25*fVal;
        
        if (bEven)
        {
            fVal = m_fLastPositive;
            bEven = false;
        }
        else
        {
            fVal = m_fLastNegative; 
            bEven = true;
        }
*/            
        *pDest++;
        *pDest++ = 32.*(1./32768.)*fVal/m_fResampleFactor;
        *pDest++;
        
    }

    if (temp<iLen)
    {
//        qDebug("upd2: %i-%i, len: %i",0,iLen-temp, m_iSourceLen);
        pSource = &m_pSource[0];
        pDest = &m_pBuffer[0];
        for (int i=0; i<=iLen-temp; ++i)
        {
            GLfloat fVal = 0;
            for (int j=(int)i; j<(int)(i+m_fResampleFactor); j+=32)
                fVal += pSource[j];

            *pDest++;
            *pDest++ = 32.*(1./32768.)*fVal/m_fResampleFactor;
            *pDest++;
        }
    }
}

void VisualBufferSignal::draw(GLfloat *p, int iLen, float)
{
    glVertexPointer(3, GL_FLOAT, 0, p);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP,0,iLen);
}
