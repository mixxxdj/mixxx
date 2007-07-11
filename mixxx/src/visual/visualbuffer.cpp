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

#include <qgl.h>
#include "visualbuffer.h"
#include "../readerextract.h"
#include "../readerevent.h"
#include "../controlobject.h"
#include "../controlobjectthreadmain.h"
#include "../defs.h"
#include "../engineobject.h"
#include "player.h"
#include "../enginebuffer.h"
#include "../mathstuff.h"


VisualBuffer::VisualBuffer(EngineBuffer *pEngineBuffer, const char *group)
{
    Q_ASSERT(pEngineBuffer);

    m_pReaderExtract = 0;
    m_pEngineBuffer = pEngineBuffer;
    

    m_pSampleRate = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Master]","samplerate")));
    m_pLatency = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Master]","latency")));
    
    m_pSource = 0;

    m_fResampleFactor = 1.f;
    //m_fDisplayRate = (float)MAXDISPLAYRATE;

    m_fDisplayFactor = 1.;
    
    // Length of this buffer. 
    m_iLen = MAXDISPLAYRATE*READBUFFERSIZE/(int)m_pSampleRate->get();
    if (!even(m_iLen))
        m_iLen--;
    m_iSourceLen = m_iLen;
        
    // Determine conversion factor between ReaderExtractWave and the m_pReaderExtract object
    m_fReaderExtractFactor = READBUFFERSIZE/m_iSourceLen;

    // Number of samples from this buffer to display
    m_iDisplayLen = m_iLen-(2*m_iLen/READCHUNK_NO);

    // Allocate buffer in video memory
    m_pBuffer = new GLfloat[3*m_iLen];

    // Reset buffer
    GLfloat *p = m_pBuffer;
    for (int i=0; i<m_iLen; i++)
    {
        *p++ = (float)i;
        *p++ = 0.;
        *p++ = 0.;
    }
    
    installEventFilter(this);
}

VisualBuffer::VisualBuffer(ReaderExtract *pReaderExtract, EngineBuffer *pEngineBuffer, const char *group)
{
    m_pReaderExtract = pReaderExtract;
    m_pEngineBuffer = pEngineBuffer;

    m_pSampleRate = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Master]","samplerate")));
    m_pLatency = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Master]","latency")));

    Q_ASSERT(m_pReaderExtract);

    // Get length and pointer to buffer in ReaderExtract
    m_pSource = (CSAMPLE *)m_pReaderExtract->getBasePtr();
    Q_ASSERT(m_pSource);

    // Set resample factor, length of this buffer etc.
    m_pBuffer = 0;
    m_iLen = 0;
    slotSetupBuffer(m_pReaderExtract->getBufferSize(), m_pReaderExtract->getRate());

    installEventFilter(this);
}

/**
 * Deconstructor.
 */
VisualBuffer::~VisualBuffer()
{
    delete m_pSampleRate;
    delete m_pEngineBuffer;
    delete [] m_pBuffer;
}

bool VisualBuffer::eventFilter(QObject *o, QEvent *e)
{
    // Update buffers
    // If a user events are received, update containers
    if (e->type() == (QEvent::Type)10002)
    {
        ReaderEvent *re = (ReaderEvent *)e;
        update(re->bufferPos(), re->bufferLen(), re->fileStartPos(), re->bufferStartPos());
        slotSetupBuffer(re->fileLen(), re->srate());
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(o,e);
    }
    return true;
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

    Q_ASSERT(m_dBufferPlaypos>=0.);
    
    // Convert playpos (minus latency) to DISPLAYRATE
    float fPos = ((((m_dBufferPlaypos/m_fReaderExtractFactor)/m_fResampleFactor)-m_pLatency->get()*MAXDISPLAYRATE)-(float)m_iDisplayLen/2.f);

    //qDebug("pos %f, corrected %f", m_pPlaypos->getValue(), getCorrectedPlaypos());

    //
    // Add to fPos based on current rate and time since m_pPlaypos was updated
    //

    while (fPos<0)
        fPos += (float)m_iLen;
    int iPos = (int)fPos;

    // Ensure the position is even
    if (!even(iPos))
        iPos--;

    bufInfo i;
    i.p1 = &m_pBuffer[iPos*3];
    i.len1 = math_min(m_iDisplayLen, m_iLen-iPos);
    i.p2 = m_pBuffer;
    i.len2 = m_iDisplayLen-i.len1;
    i.corr = fPos-(float)iPos;

	Q_ASSERT(i.len1>=0);
	Q_ASSERT(i.len2>=0);

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

void VisualBuffer::slotSetupBuffer(int bufferlen, int srate)
{
    //qDebug("srate %i",srate);

    m_iSourceLen = bufferlen;

    // Set resample factor and display rate and display factor
    //Q_ASSERT(srate<MAXDISPLAYRATE);
    if (srate>MAXDISPLAYRATE)
        m_fResampleFactor = (float)srate/(float)MAXDISPLAYRATE;
    else
        m_fResampleFactor = 1.;

    // Determine conversion factor between ReaderExtractWave and the m_pReaderExtract object
    m_fReaderExtractFactor = READBUFFERSIZE/m_iSourceLen;

//bufferlen/(srate/maxdisprate)


    // Length of this buffer.
    int iLenOld = m_iLen;
    m_iLen = (int)(floorf((float)m_iSourceLen/m_fResampleFactor));
    if (!even(m_iLen))
        m_iLen--;

    // Readjust resample factor to actual length of display buffer
    m_fResampleFactor = (float)bufferlen/(float)m_iLen;

//    float fDisplayRate = srate/m_fResampleFactor;


//qDebug("resample %f, srclen %i, len %i",m_fResampleFactor, m_iSourceLen, m_iLen);

    // Number of samples from this buffer to display
    m_iDisplayLen = m_iLen-(2*m_iLen/READCHUNK_NO);
    //int m_iDisplayLen2 = fDisplayRate*((READCHUNKSIZE*(READCHUNK_NO-2))/m_fReaderExtractFactor)/srate;

    //qDebug("len %i, len2 %i", m_iDisplayLen, m_iDisplayLen2);

    // Allocate buffer if necessary
    if (m_pBuffer && m_iLen != iLenOld)
    {
        delete [] m_pBuffer;
        m_pBuffer = 0;
    }
    if (!m_pBuffer)
    {
        m_pBuffer = new GLfloat[3*m_iLen];

        // Reset buffer
        GLfloat *p = m_pBuffer;
        for (int i=0; i<m_iLen; i++)
        {
            *p++ = (float)i;
            *p++ = 0.;
            *p++ = 0.;
        }
    }
}

