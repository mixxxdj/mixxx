/***************************************************************************
                          enginesidechain.h
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


#ifndef ENGINESIDECHAIN_H
#define ENGINESIDECHAIN_H

#define SIDECHAIN_BUFFER_SIZE 4096



class EngineSideChain : public QThread
{
    public:
        EngineSideChain(ConfigObject<ConfigValue> * pConfig, const char * _group);
        submitSamples(CSAMPLE* buffer);
        run();
        
    private:
        ConfigObject<ConfigValue> * m_pConfig;
        char* m_group;
        CSAMPLE* m_buffer;                      //Pointer to current giant buffer (for double-buffering)
        CSAMPLE* m_bufferFront;                 //Giant buffer to store audio.
        CSAMPLE* m_bufferBack;                  //Another giant buffer to store audio.
        QMutex m_bufferLock;                    //Provides thread safety for the buffer.
        QMutex m_waitLock;                      //Provides thread safety around the wait condition below.
        QWaitCondition m_waitForFullBuffer;     //Allows sleeping until we have a full buffer.
};

#endif
