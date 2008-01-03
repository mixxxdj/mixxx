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
#include "enginebuffer.h"

EngineSideChain::EngineSideChain(ConfigObject<ConfigValue> * pConfig, const char * _group)
{
    m_pConfig = pConfig;
    m_group = _group;
    
    m_bufferFront = new CSAMPLE[SIDECHAIN_BUFFER_SIZE];
    m_bufferBack  = new CSAMPLE[SIDECHAIN_BUFFER_SIZE];
    m_buffer = m_bufferFront;
    
    start();    //Starts the thread and goes to the "run()" function below.
}

EngineSideChain::submitSamples(CSAMPLE* buffer)
{
    m_bufferLock.lock();
    
    //Copy samples into m_buffer.
    
  
    //Pseudocode:
    //if buffer is full:
    //  call swapBuffers()
    //  and then wake the run thread (below):
    m_waitLock.lock();
    m_waitForFullBuffer.wakeAll();
    m_waitLock.unlock();
    
    m_bufferLock.unlock();
}

EngineSideChain::swapBuffers()
{
    if (m_buffer == m_bufferFront)
        m_buffer = m_bufferBack;
    else
        m_buffer = m_bufferFront;
}

EngineSideChain::run()
{
    while (true)
    {
        m_waitLock.lock();
        m_waitForFullBuffer.wait(&m_waitLock);  //Sleep until the buffer has been filled.
        m_waitLock.unlock();
        
        //This portion of the code should be able to touch the buffer without having to use
        //a mutex, because the buffers should have been swapped.
        
        //Do CPU intensive and non-realtime processing here.
    
        //Eg. Use EngineShoutcast...
    
    
    }

}
