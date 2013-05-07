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

#define SIDECHAIN_BUFFER_SIZE 65536

#include "engine/sidechain/sidechainworker.h"

class EngineSideChain : public QThread {
    Q_OBJECT
  public:
    EngineSideChain(ConfigObject<ConfigValue> * pConfig);
    virtual ~EngineSideChain();
    void submitSamples(CSAMPLE* buffer, int buffer_size);

    void addSideChainWorker(SideChainWorker* pWorker);

  private:
    // Swaps the buffers in the double-buffering mechanism we use.
    void swapBuffers();
    void run();

    ConfigObject<ConfigValue> * m_pConfig;
    // Indicates that the thread should exit.
    volatile bool m_bStopThread;
    // Index of the last valid sample in the buffer.
    unsigned long m_iBufferEnd;
    // Giant buffer to store audio.
    CSAMPLE* m_bufferFront;
    // Another giant buffer to store audio.
    CSAMPLE* m_bufferBack;
    // Pointer to the fillable giant buffer (for double-buffering)
    CSAMPLE* m_buffer;
    // Pointer to the filled giant buffer (after swapping).
    CSAMPLE* m_filledBuffer;

    // Provides thread safety for the back buffer.
    QMutex m_backBufferLock;
    // Provides thread safety around the wait condition below.
    QMutex m_waitLock;
    // Allows sleeping until we have a full buffer.
    QWaitCondition m_waitForFullBuffer;

    // Sidechain workers registered with EngineSideChain.
    QMutex m_workerLock;
    QList<SideChainWorker*> m_workers;
};

#endif
