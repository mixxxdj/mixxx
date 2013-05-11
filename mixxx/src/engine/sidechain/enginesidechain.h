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

#include "configobject.h"
#include "defs.h"
#include "engine/sidechain/sidechainworker.h"
#include "util/fifo.h"

class EngineSideChain : public QThread {
    Q_OBJECT
  public:
    EngineSideChain(ConfigObject<ConfigValue> * pConfig);
    virtual ~EngineSideChain();

    // Not thread-safe, wait-free. Submit buffer of samples to the sidechain for
    // processing. Should only be called from a single writer thread (typically
    // the engine callback).
    void writeSamples(const CSAMPLE* buffer, int buffer_size);

    // Thread-safe, blocking.
    void addSideChainWorker(SideChainWorker* pWorker);

  private:
    void run();

    ConfigObject<ConfigValue>* m_pConfig;
    // Indicates that the thread should exit.
    volatile bool m_bStopThread;

    FIFO<CSAMPLE> m_sampleFifo;
    CSAMPLE* m_pWorkBuffer;

    // Provides thread safety around the wait condition below.
    QMutex m_waitLock;
    // Allows sleeping until we have samples to process.
    QWaitCondition m_waitForSamples;

    // Sidechain workers registered with EngineSideChain.
    QMutex m_workerLock;
    QList<SideChainWorker*> m_workers;
};

#endif
