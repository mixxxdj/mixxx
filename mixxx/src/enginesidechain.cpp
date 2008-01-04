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
    m_bufferLock.lock();
    m_waitLock.lock();
    //Free up memory
    delete m_bufferFront;
    delete m_bufferBack;
    
#ifdef __SHOUTCAST__
    delete shoutcast;
#endif
    terminate(); //FIXME: Nasty

    m_waitLock.unlock();
    m_bufferLock.unlock();

}

void EngineSideChain::submitSamples(CSAMPLE* newBuffer, int buffer_size)
{
    m_bufferLock.lock();
        
    //Copy samples into m_buffer.
    if (m_iBufferEnd + buffer_size < SIDECHAIN_BUFFER_SIZE)
    {
        memcpy(&m_buffer[m_iBufferEnd], newBuffer, buffer_size * sizeof(*newBuffer));
        m_iBufferEnd += buffer_size;
    }
    else //If the new buffer won't fit, copy as much of it as we can over and then copy the rest after swapping.
    {
        memcpy(&m_buffer[m_iBufferEnd], newBuffer, (SIDECHAIN_BUFFER_SIZE - m_iBufferEnd)*sizeof(*newBuffer));
        //Save the number of samples written because m_iBufferEnd gets reset in swapBuffers:
        int iNumSamplesWritten = SIDECHAIN_BUFFER_SIZE - m_iBufferEnd; 
        swapBuffers(); //Swaps buffers and resets m_iBufferEnd to zero.
        int iNumSamplesStillToWrite = buffer_size - iNumSamplesWritten;
        
        //Check to see if the remaining samples will fit in the other empty buffer.
        if (iNumSamplesStillToWrite > SIDECHAIN_BUFFER_SIZE)
        {
            iNumSamplesStillToWrite = SIDECHAIN_BUFFER_SIZE; //Drop samples if they won't fit.
            qDebug() << "EngineSideChain warning: dropped samples";
        }
        memcpy(&m_buffer[m_iBufferEnd], newBuffer, iNumSamplesStillToWrite*sizeof(*newBuffer));
        
        //Since we swapped buffers, we now have a full buffer that needs processing.
        m_waitLock.lock();
        m_waitForFullBuffer.wakeAll(); //... so wake the thread up and get processing. :)
        m_waitLock.unlock();       
    }
    
    m_bufferLock.unlock();
}

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
        //a mutex, because the buffers should have been swapped.
        
        //IMPORTANT: The filled buffer is "m_filledBuffer" - that's the audio we need to process here.
        
        //Do CPU intensive and non-realtime processing here.
    
        //Eg. Use EngineShoutcast...

#ifdef __SHOUTCAST__
        shoutcast->process(m_filledBuffer, m_filledBuffer, SIDECHAIN_BUFFER_SIZE);
#endif    
    
    }

}
