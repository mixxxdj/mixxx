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
#include "../readerevent.h"
#include "../controlobject.h"
#include "../defs.h"
#include "../engineobject.h"
#include "player.h"
#include "../enginebuffer.h"

/**
 * Default Constructor.
 */
VisualBuffer::VisualBuffer(ReaderExtract *pReaderExtract, EngineBuffer *pEngineBuffer, const char *group)
{
    m_pReaderExtract = pReaderExtract;
    m_pEngineBuffer = pEngineBuffer;
    
    m_pRate = ControlObject::getControl(ConfigKey(group, "rateEngine"));
    
    Q_ASSERT(m_pReaderExtract);
    
    // Get length and pointer to buffer in ReaderExtract
    m_pSource = (CSAMPLE *)m_pReaderExtract->getBasePtr();
    Q_ASSERT(m_pSource);
    m_iSourceLen = m_pReaderExtract->getBufferSize();

    // Set resample factor and display rate and display factor
    if (m_pReaderExtract->getRate()>MAXDISPLAYRATE)
    {
        m_fResampleFactor = (float)m_pReaderExtract->getRate()/(float)MAXDISPLAYRATE;
        m_fDisplayRate = MAXDISPLAYRATE;
    }
    else
    {
        m_fResampleFactor = 1.;
        m_fDisplayRate = m_pReaderExtract->getRate();
    }
    m_fDisplayFactor = (float)MAXDISPLAYRATE/m_fDisplayRate;

    // Determine conversion factor between ReaderExtractWave and the m_pReaderExtract object
    m_fReaderExtractFactor = READBUFFERSIZE/m_iSourceLen;
    
    // Length of this buffer
    m_iLen = (int)((float)m_iSourceLen/m_fResampleFactor);

    // Number of samples from this buffer to display
    m_iDisplayLen = m_iLen-(2*m_iLen/READCHUNK_NO);

    // Allocate buffer in video memory
    m_pBuffer = allocate(3*m_iLen);

    // Reset buffer
    GLfloat *p = m_pBuffer;
    for (int i=0; i<m_iLen; i++)
    {
        *p++ = (float)i; //*m_fDisplayFactor;
        *p++ = 0.;
        *p++ = 0.;
    }

    installEventFilter(this);
}

/**
 * Deconstructor.
 */
VisualBuffer::~VisualBuffer()
{
    delete [] m_pBuffer;
}

bool VisualBuffer::eventFilter(QObject *o, QEvent *e)
{
    // Update buffers
    // If a user events are received, update containers
    if (e->type() == (QEvent::Type)10002)
    {
        ReaderEvent *re = (ReaderEvent *)e;
        update(re->pos(), re->len());
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(o,e);
    }
    return true;
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
//    float fPriority = 1.0f;
//    float fReadFrequency = 0;
//    float fWriteFrequency = 0;
//    float fMegabytes = (iSize * sizeof(GL_FLOAT)/1000000.f);

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
    // Update abs and buffer playpos
    m_pEngineBuffer->lockPlayposVars();
    m_dAbsPlaypos = m_pEngineBuffer->getAbsPlaypos();
    m_dBufferPlaypos = m_pEngineBuffer->getBufferPlaypos();
    m_pEngineBuffer->unlockPlayposVars();
    
    // Convert playpos (minus latency) to DISPLAYRATE
    float fPos = ((((m_dBufferPlaypos-Player::getBufferSize())/m_fReaderExtractFactor)/m_fResampleFactor)-(float)m_iDisplayLen/2.f);
    
    //qDebug("pos %f, corrected %f", m_pPlaypos->getValue(), getCorrectedPlaypos());
        
    //
    // Add to fPos based on current rate and time since m_pPlaypos was updated
    //
    
    while (fPos<0)
        fPos += (float)m_iLen;
    int iPos = (int)fPos;

    bufInfo i;
    i.p1 = &m_pBuffer[iPos*3];
    i.len1 = min(m_iDisplayLen, m_iLen-iPos);
    i.p2 = m_pBuffer;
    i.len2 = m_iDisplayLen-i.len1;
    i.corr = fPos-(float)iPos;

//    qDebug("Total pos %i",i.len1+i.len2);
//    std::cout << "playpos " << m_pPlaypos->getValue() << ", pos " << iPos << "\n"; //", len1 " << i.len1 << ", len2 " << i.len2 << ", displayLen " << m_iDisplayLen << "\n";

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

void VisualBuffer::setColorFg(float r, float g, float b)
{
    m_materialFg.ambient[0] = r;
    m_materialFg.ambient[1] = g;
    m_materialFg.ambient[2] = b;
    m_materialFg.ambient[3] = 1.0f;

    m_materialFg.diffuse[0] = r;
    m_materialFg.diffuse[1] = g;
    m_materialFg.diffuse[2] = b;
    m_materialFg.diffuse[3] = 1.0f;

    m_materialFg.specular[0] = r;
    m_materialFg.specular[1] = g;
    m_materialFg.specular[2] = b;
    m_materialFg.specular[3] = 1.0f;

    m_materialFg.shininess = 128;
}

void VisualBuffer::setColorBg(float r, float g, float b)
{
    m_materialBg.ambient[0] = r;
    m_materialBg.ambient[1] = g;
    m_materialBg.ambient[2] = b;
    m_materialBg.ambient[3] = 1.0f;

    m_materialBg.diffuse[0] = r;
    m_materialBg.diffuse[1] = g;
    m_materialBg.diffuse[2] = b;
    m_materialBg.diffuse[3] = 1.0f;

    m_materialBg.specular[0] = r;
    m_materialBg.specular[1] = g;
    m_materialBg.specular[2] = b;
    m_materialBg.specular[3] = 1.0f;

    m_materialBg.shininess = 128;
}

