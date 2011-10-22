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

#include <QtCore>
#include "defs.h"
#include "configobject.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "../recording/defs_recording.h"
#include "errordialoghandler.h"
#include "recording/recordingmanager.h"

#ifdef __SHOUTCAST__
class EngineShoutcast;
#endif

class EngineRecord;

#define SIDECHAIN_BUFFER_SIZE 65536

class EngineSideChain : public QThread {
Q_OBJECT

public:
    EngineSideChain(ConfigObject<ConfigValue> * pConfig);
    virtual ~EngineSideChain();
    void submitSamples(CSAMPLE* buffer, int buffer_size);

  signals:
    void bytesRecorded(int);
    void isRecording(bool);

  private:
    void swapBuffers();
    void run();

    ConfigObject<ConfigValue> * m_pConfig;
    const char* m_group;
    volatile bool m_bStopThread;                     //Indicates that the thread should exit.
    unsigned long m_iBufferEnd;             //Index of the last valid sample in the buffer.
    CSAMPLE* m_buffer;                      //Pointer to the fillable giant buffer (for double-buffering)
    CSAMPLE* m_filledBuffer;                //Pointer to the filled giant buffer (after swapping).
    CSAMPLE* m_bufferFront;                 //Giant buffer to store audio.
    CSAMPLE* m_bufferBack;                  //Another giant buffer to store audio.
    QMutex m_backBufferLock;                //Provides thread safety for the back buffer.
    QMutex m_waitLock;                      //Provides thread safety around the wait condition below.
    QWaitCondition m_waitForFullBuffer;     //Allows sleeping until we have a full buffer.

#ifdef __SHOUTCAST__
    EngineShoutcast *m_shoutcast;
#endif
    EngineRecord* m_rec;
};

#endif
