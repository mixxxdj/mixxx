/***************************************************************************
                          signalvertexbuffer.cpp  -  description
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

#include "signalvertexbuffer.h"
#include "../enginebuffer.h"
#include "../soundbuffer.h"
#include "fastvertexarray.h"

/**
 * Default Constructor.
 */
SignalVertexBuffer::SignalVertexBuffer(int _len, int _resampleFactor, EngineBuffer *_enginebuffer, FastVertexArray *_vertex)
{
    installEventFilter(this);

    vertex = _vertex;
    enginebuffer = _enginebuffer;
    soundbuffer = enginebuffer->getSoundBuffer();
    resampleFactor = _resampleFactor;
    len = _len/resampleFactor;
    displayLen = len-(4*READCHUNKSIZE/resampleFactor);
    std::cout << "displayLen: " << displayLen << "\n";
    playpos = 0;
    time.start();

    buffer = vertex->getStartPtr(READCHUNK_NO);

    // Reset buffer
    GLfloat *p = buffer;
    for (int i=0; i<len; i++)
    {
        *p++ = (float)i;
        *p++ = 0.; //(float)cos((i/(1.0f*len))*6.28*5+1.5707963267);
        *p++ = 0.;
    }   
};

/**
 * Deconstructor.
 */
SignalVertexBuffer::~SignalVertexBuffer()
{
    //delete [] buffer;
};

bool SignalVertexBuffer::eventFilter(QObject *o, QEvent *e)
{
    // If a user events are received, update either playpos or buffer
    if (e->type() == (QEvent::Type)1001)
    {
        // Update visual buffer 1
        updateBuffer(soundbuffer->read_buffer,soundbuffer->visualPos1, 
                     soundbuffer->visualLen1,soundbuffer->visualPos2, soundbuffer->visualLen2);
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(o,e);
    }
    return TRUE;
}


/**
 * Updates buffer.
 *
 * Input: source     - Pointer to samples which is to be copied to this buffer
 *        pos1, len1 - Position and length of samples relative to source
 *        pos2, len2 - Same. Is used if buffer is circular and the samples are wrapped
 *
 */ 
void SignalVertexBuffer::updateBuffer(float *source, int pos1, int len1, int pos2, int len2)
{
    //std::cout << "pos1: " << pos1 << ", len1: " << len1 <<", pos2: " << pos2 << ", len2: " << len2 << ", len: " << len1+len2 << "\n";

    int pos, len;
    if (len1>0) 
    { 
        len=len1; 
        pos=pos1; 
    }
    else 
    { 
        len=len2; 
        pos=pos2;
    }
    
    float *copySource = source+pos;
    GLfloat *copyDest = &buffer[(int)(pos/resampleFactor)*3];
    for (int i=0; i<len; i+=resampleFactor)
    {
        *copyDest++;
        *copyDest++ = copySource[i]*(1./32768.);
        *copyDest++;
    }
}

/**
 * Retrieve Vertex Array Pointer. The buffer can be wrapped, and
 * thus pointers to two arrays are returned as bufInfo types.
 *
 * @return          A bufInfo struct containing a pointer to the samples,
 *                  and the number of samples.
 */
bufInfo SignalVertexBuffer::getVertexArray()
{
	// Calculate new playpos based on playpos, rate and time since 
	int dt = time.elapsed();
	time.restart();
	int newPlaypos = playpos + (dt*enginebuffer->visualRate*enginebuffer->SRATE/1000);
    
	playpos = enginebuffer->visualPlaypos/resampleFactor;

	int pos = playpos-(displayLen/2);
    while (pos<0)
        pos += len;

    bufInfo i;
    i.p1   = &buffer[pos*3];
    i.len1 = min(pos+displayLen,len)-pos;
    i.p2   = buffer;
    i.len2 = displayLen-i.len1;
    
//    std::cout << "pos " << pos << ", len1 " << i.len1 << ", len2 " << i.len2 << ", displayLen " << displayLen << "\n";

    return i;
};

/**
 * Get Total Number of Samples in buffer.
 *
 * @return    The number of samples in buffer.
 */
int SignalVertexBuffer::getBufferLength()
{
    return len;
};

/**
 * Get Total Number of display samples.
 *
 * @return    The number of samples to display.
 */
int SignalVertexBuffer::getDisplayLength()
{
    return displayLen;
};

