/***************************************************************************
                          visualbuffer.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen and Kenny 
                                       Erleben
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "visualbuffer.h"
#include "../readerextract.h"
#include "controlpotmeter.h"
#include "../defs.h"

/**
 * Default Constructor.
 */
VisualBuffer::VisualBuffer(ReaderExtract *pReaderExtract, ControlPotmeter *pPlaypos)
{
    m_pPlaypos = pPlaypos;
    m_pReaderExtract = pReaderExtract;

    // Get length and pointer to buffer in ReaderExtract
    m_pSource = (CSAMPLE *)m_pReaderExtract->getBasePtr();
    m_iSourceLen = m_pReaderExtract->getBufferSize();

    // iPos and iLen in update()is given relative to the ReaderExtractWave buffer,
    // of length READBUFFERSIZE. iCpos and iClen is the relative position in the
    // current ReaderExtract object
    m_fPositionFactor = (CSAMPLE)READBUFFERSIZE/(CSAMPLE)m_iSourceLen;
}

/**
 * Deconstructor.
 */
VisualBuffer::~VisualBuffer()
{
    if (m_pBuffer)
    {
        delete [] m_pBuffer;
        m_pBuffer = 0;
    }
}

GLfloat *VisualBuffer::allocate(int iSize)
{
    //---
    //--- Memory is allocated according to the values
    //---
    //---            Read Freq       Write Freq      Priority
    //---  AGP       [0 - 0.25)      [0 - 0.25)      (0.25 - 0.75]
    //--- VIDEO      [0 - 0.25)      [0 - 0.25)      (0.75 -  1]
    //---
    //---
    float fPriority = 1.0f;
    float fReadFrequency = 0;
    float fWriteFrequency = 0;

    float fMegabytes = (iSize * sizeof(GL_FLOAT)/1000000.f);

    GLfloat *pArray = new GLfloat[iSize];

    return pArray;
}

void VisualBuffer::validate()
{
    GLenum errCode = glGetError();
    if (errCode!=GL_NO_ERROR)
    {
        const GLubyte* errmsg = gluErrorString(errCode);
        qDebug("Visuals: %s",errmsg);
    }
}

/**
 * Retrieve Vertex Array Pointer. The buffer can be wrapped, and
 * thus pointers to two arrays are returned as bufInfo types.
 *
 * @return          A bufInfo struct containing a pointer to the samples,
 *                  and the number of samples.
 */
bufInfo VisualBuffer::getVertexArray()
{
    // Conversion to DISPLAYRATE
    //GLfloat fResampleFactor = (GLfloat)READCHUNKSIZE/(GLfloat)vertex->getChunkSize();
    int iPos = (m_pPlaypos->getValue()/m_fResampleFactor)-m_iDisplayLen/2;
    while (iPos<0)
        iPos += m_iLen;

    bufInfo i;
    i.p1 = &m_pBuffer[iPos*3];
    i.len1 = min(iPos+m_iDisplayLen, m_iLen-iPos);
    i.p2 = m_pBuffer;
    i.len2 = m_iDisplayLen-i.len1;

//    qDebug("Total pos %i",i.len1+i.len2);
//    std::cout << "pos " << pos << ", len1 " << i.len1 << ", len2 " << i.len2 << ", displayLen " << displayLen << "\n";

    return i;
}

/**
 * Get Total Number of Samples in buffer.
 *
 * @return    The number of samples in buffer.
 */
int VisualBuffer::getBufferLength()
{
    return m_iLen;
}

/**
 * Get Total Number of display samples.
 *
 * @return    The number of samples to display.
 */
int VisualBuffer::getDisplayLength()
{
    return m_iDisplayLen;
}

GLfloat *VisualBuffer::getBasePtr()
{
    return m_pBuffer;
}


