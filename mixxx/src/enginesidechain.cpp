/***************************************************************************
                          enginesidechain.cpp
                             -------------------
    copyright            : (C) 2008 Albert Santoni
    email                : gamegod \a\t users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

/* This class provides a way to do audio processing that does not need
 * to be executed in real-time. For example, shoutcast encoding/broadcasting
 * and recording encoding can be done here. This class uses double-buffering
 * to increase the amount of time the CPU has to do whatever work needs to
 * be done, and that work is executed in a separate thread. (Threading
 * allows the next buffer to be filled while processing a buffer that's is
 * already full.)
 *
 */

#include <QtCore>
#include <QtDebug>
#include "enginesidechain.h"
#include "enginebuffer.h"

#ifdef __SHOUTCAST__
#include "engineshoutcast.h"
#endif

EngineSideChain::EngineSideChain(ConfigObject<ConfigValue> * pConfig)
{
    m_pConfig = pConfig;
    
    m_bufferFront = new CSAMPLE[SIDECHAIN_BUFFER_SIZE];
    m_bufferBack  = new CSAMPLE[SIDECHAIN_BUFFER_SIZE];
    m_buffer = m_bufferFront;
    
    m_iBufferEnd = 0;
    
#ifdef __SHOUTCAST__
    // Shoutcast
    shoutcast = new EngineShoutcast(m_pConfig);
#endif    
    
    
    start();    //Starts the thread and goes to the "run()" function below.
}

EngineSideChain::~EngineSideChain()
{
    m_backBufferLock.lock();
    m_waitLock.lock();
    
    terminate(); //FIXME: Nasty

    //Free up memory
    delete m_bufferFront;
    delete m_bufferBack;
    
#ifdef __SHOUTCAST__
    delete shoutcast;
#endif

    m_waitLock.unlock();
    m_backBufferLock.unlock();
}

/** Submit a buffer of samples to be processed in the sidechain*/ 
void EngineSideChain::submitSamples(CSAMPLE* newBuffer, int buffer_size)
{       
    //Copy samples into m_buffer.
    if (m_iBufferEnd + buffer_size <= SIDECHAIN_BUFFER_SIZE)    //FIXME: is <= correct?
    {
        memcpy(&m_buffer[m_iBufferEnd], newBuffer, buffer_size * sizeof(CSAMPLE));
        m_iBufferEnd += buffer_size;
    }
    else //If the new buffer won't fit, copy as much of it as we can over and then copy the rest after swapping.
    {
        memcpy(&m_buffer[m_iBufferEnd], newBuffer, (SIDECHAIN_BUFFER_SIZE - m_iBufferEnd)*sizeof(CSAMPLE));
        //Save the number of samples written because m_iBufferEnd gets reset in swapBuffers:
        int iNumSamplesWritten = SIDECHAIN_BUFFER_SIZE - m_iBufferEnd; 
        
        //m_backBufferLock.lock(); //This will block the callback thread if the buffering overflows.
        swapBuffers(); //Swaps buffers and resets m_iBufferEnd to zero.
        //m_backBufferLock.unlock();

        //Since we swapped buffers, we now have a full buffer that needs processing.
        m_waitLock.lock();
        m_waitForFullBuffer.wakeAll(); //... so wake the thread up and get processing. :)
        m_waitLock.unlock();  

        //Calculate how many leftover samples need to be written to the other buffer.
        int iNumSamplesStillToWrite = buffer_size - iNumSamplesWritten;
        
        //Check to see if the remaining samples will fit in the other empty buffer.
        if (iNumSamplesStillToWrite > SIDECHAIN_BUFFER_SIZE)
        {
            iNumSamplesStillToWrite = SIDECHAIN_BUFFER_SIZE; //Drop samples if they won't fit.
            qDebug() << "EngineSideChain warning: dropped samples";
        }
        memcpy(&m_buffer[m_iBufferEnd], &newBuffer[iNumSamplesWritten], iNumSamplesStillToWrite*sizeof(CSAMPLE));
        m_iBufferEnd += iNumSamplesStillToWrite;
     
    }
}

/* Swaps the buffers in the double-buffering mechanism we use */ 
void EngineSideChain::swapBuffers()
{
    if (m_buffer == m_bufferFront)
    {
        m_buffer = m_bufferBack;
        m_filledBuffer = m_bufferFront;
    }
    else
    {
        m_buffer = m_bufferFront;
        m_filledBuffer = m_bufferBack;
    }
    
    m_iBufferEnd = 0;
}

void EngineSideChain::run()
{
    while (true)
    {
        m_waitLock.lock();
        m_waitForFullBuffer.wait(&m_waitLock);  //Sleep until the buffer has been filled.
        m_waitLock.unlock();
        
        //This portion of the code should be able to touch the buffer without having to use
        //the m_bufferLock mutex, because the buffers should have been swapped.
        
        //IMPORTANT: The filled buffer is "m_filledBuffer" - that's the audio we need to process here.
        
        //Do CPU intensive and non-realtime processing here.

        //m_backBufferLock.lock(); //This will cause the audio/callback thread to block if the buffers overflow.
        
#ifdef __SHOUTCAST__
        shoutcast->process(m_filledBuffer, m_filledBuffer, SIDECHAIN_BUFFER_SIZE);
#endif   
 
        //m_backBufferLock.unlock();
    
    }

}
