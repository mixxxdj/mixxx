//
// C++ Implementation: visualbuffersignalhfc
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "visualbuffersignalhfc.h"

VisualBufferSignalHFC::VisualBufferSignalHFC(ReaderExtract *pReaderExtract, EngineBuffer *pEngineBuffer, const char *group) : VisualBufferSignal(pReaderExtract, pEngineBuffer, group)
{
}

VisualBufferSignalHFC::~VisualBufferSignalHFC()
{
}

void VisualBufferSignalHFC::update(int iPos, int iLen, long int, int)
{
    //qDebug("signal upd pos %i, len %i, total len %i",iPos,iLen,m_iSourceLen);

    CSAMPLE *pSource = &m_pSource[iPos];
    GLfloat *pDest = &m_pBuffer[(int)(iPos/m_fResampleFactor)*3];

    float temp = min(iLen, m_iSourceLen-iPos-1);
//    qDebug("upd1: %i-%f, temp: %f",iPos,iPos+temp, temp);
    for (float i=0; i<=temp-m_fResampleFactor; i+=m_fResampleFactor)
    {
        GLfloat fVal = 0;
        for (int j=(int)i; j<(int)(i+m_fResampleFactor); j+=32)
            fVal += pSource[j]*(1./(4.*32768.));

        *pDest++;
        *pDest++ = 32.*fVal/m_fResampleFactor;
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
                fVal += pSource[j]*(1./(4.*32768.));

            *pDest++;
            *pDest++ = 32.*fVal/m_fResampleFactor;
            *pDest++;
        }
    }
}

void VisualBufferSignalHFC::draw(GLfloat *p, int iLen, float)
{
    glVertexPointer(3, GL_FLOAT, 0, p);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP,0,iLen);
}
