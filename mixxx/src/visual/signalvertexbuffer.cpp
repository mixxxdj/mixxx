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
#include "../readerextract.h"
#include "fastvertexarray.h"
#include "controlpotmeter.h"
/**
 * Default Constructor.
 */
SignalVertexBuffer::SignalVertexBuffer(ReaderExtract *_readerExtract, ControlPotmeter *_playpos, FastVertexArray *_vertex)
{
    vertex = _vertex;
    playpos = _playpos;
    readerExtract = _readerExtract;

    len = vertex->getSize();
    buffer = vertex->getStartPtr();
    
    // Should match update boundary used in EngineBuffer when calling Reader::wake()
    displaylen = len-2*vertex->getChunkSize();
    
    qDebug("len: %i, READCHUNKSIZE: %i, chunkSize: %i",len,READCHUNKSIZE,vertex->getChunkSize());
    
    // Reset buffer
    GLfloat *p = buffer;
    for (int i=0; i<len; i++)
    {
        *p++ = (float)i;
        *p++ = 0.; //(float)cos((i/(1.0f*len))*6.28*5+1.5707963267);
        *p++ = 0.;
    }
}

/**
 * Deconstructor.
 */
SignalVertexBuffer::~SignalVertexBuffer()
{
};


/**
 * Updates buffer.
 *
 * Input: playpos - Play position relative to the vertex buffer
 *
 */ 
void SignalVertexBuffer::update(int pos, int len)
{
//   qDebug("SignalVertexBuffer::update: pos %i, len %i",pos,len); 
    int buffersize = readerExtract->getBufferSize();
    
    // Get pos and len for this ReaderExtract buffer
    CSAMPLE convertFactor = (CSAMPLE)READBUFFERSIZE/(CSAMPLE)buffersize;
    int cpos = (int)((CSAMPLE)pos/(CSAMPLE)convertFactor);
    int clen = (int)((CSAMPLE)len/(CSAMPLE)convertFactor);
//    qDebug("pos %i, cpos %i",pos,cpos);
        
    CSAMPLE *source = &((CSAMPLE *)readerExtract->getBasePtr())[cpos];

    GLfloat resampleFactor = ((GLfloat)buffersize/(GLfloat)(vertex->getChunkSize()*READCHUNK_NO));
    GLfloat *dest = &buffer[(int)(cpos/resampleFactor)*3];

    for (int i=0; i<readerExtract->getBufferSize()/READCHUNK_NO; i+=resampleFactor)
    {
        GLfloat val = 0;
        for (int j=i; j<i+resampleFactor; j++)
        {
            val += source[j]*(1./32768.);
        }
//        if (resampleFactor ==1.)
//            qDebug("idx: %i, val: %f, resamplefactor: %f",i,val/resampleFactor,resampleFactor);
        *dest++;
        *dest++ = val/resampleFactor;
        *dest++;
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
    //******************************* CONVERT TO DISPLAYRATE!!!!!
    GLfloat resampleFactor = (GLfloat)READCHUNKSIZE/(GLfloat)vertex->getChunkSize();
    int pos = (playpos->getValue()/resampleFactor)-displaylen/2;
    while (pos<0)
        pos += len;

    bufInfo i;
    i.p1 = &buffer[pos*3];
    i.len1 = min(pos+displaylen, len-pos);
    i.p2 = buffer;
    i.len2 = displaylen-i.len1;

//    qDebug("Total pos %i",i.len1+i.len2);
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
    return displaylen;
};

