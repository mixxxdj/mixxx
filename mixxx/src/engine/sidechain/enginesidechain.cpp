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
 * to be executed in real-time. For example, m_shoutcast encoding/broadcasting
 * and recording encoding can be done here. This class uses double-buffering
 * to increase the amount of time the CPU has to do whatever work needs to
 * be done, and that work is executed in a separate thread. (Threading
 * allows the next buffer to be filled while processing a buffer that's is
 * already full.)
 *
 */

#include <QtCore>
#include <QtDebug>
#include <QMutexLocker>

#include "engine/sidechain/enginesidechain.h"
#include "engine/sidechain/sidechainworker.h"
#include "util/timer.h"
#include "util/counter.h"
#include "sampleutil.h"

#define SIDECHAIN_BUFFER_SIZE 65536

EngineSideChain::EngineSideChain(ConfigObject<ConfigValue> * pConfig)
        : m_pConfig(pConfig),
          m_bStopThread(false),
          m_sampleFifo(SIDECHAIN_BUFFER_SIZE),
          m_pWorkBuffer(SampleUtil::alloc(SIDECHAIN_BUFFER_SIZE)) {
    // Starts the thread and goes to the "run()" function below.
   	start(QThread::LowPriority);
}

EngineSideChain::~EngineSideChain() {
    m_waitLock.lock();
    m_bStopThread = true;
    m_waitForSamples.wakeAll();
    m_waitLock.unlock();

    // Wait until the thread has finished.
    wait();

    QMutexLocker locker(&m_workerLock);
    while (!m_workers.empty()) {
        SideChainWorker* pWorker = m_workers.takeLast();
        pWorker->shutdown();
        delete pWorker;
    }
    locker.unlock();

    SampleUtil::free(m_pWorkBuffer);
}

void EngineSideChain::addSideChainWorker(SideChainWorker* pWorker) {
    QMutexLocker locker(&m_workerLock);
    m_workers.append(pWorker);
}

void EngineSideChain::writeSamples(const CSAMPLE* newBuffer, int buffer_size) {
    ScopedTimer t("EngineSideChain:writeSamples");
    int samples_written = m_sampleFifo.write(newBuffer, buffer_size);

    if (samples_written != buffer_size) {
        Counter("EngineSideChain::writeSamples buffer overrun").increment();
    }

    if (m_sampleFifo.writeAvailable() < SIDECHAIN_BUFFER_SIZE/5) {
        // Signal to the sidechain that samples are available.
        m_waitForSamples.wakeAll();
    }
}

void EngineSideChain::run() {
    // the id of this thread, for debugging purposes //XXX copypasta (should
    // factor this out somehow), -kousu 2/2009
    unsigned static id = 0;
    QThread::currentThread()->setObjectName(QString("EngineSideChain %1").arg(++id));

    while (true) {
        // Check to see if we're supposed to exit/stop this thread.
        if (m_bStopThread) {
            return;
        }

        int samples_read;
        while ((samples_read = m_sampleFifo.read(
            m_pWorkBuffer, SIDECHAIN_BUFFER_SIZE))) {
            QMutexLocker locker(&m_workerLock);
            foreach (SideChainWorker* pWorker, m_workers) {
                pWorker->process(m_pWorkBuffer, samples_read);
            }
        }

        // Check to see if we're supposed to exit/stop this thread.
        if (m_bStopThread) {
            return;
        }

        // Sleep until samples are available.
        m_waitLock.lock();
        m_waitForSamples.wait(&m_waitLock);
        m_waitLock.unlock();
    }
}
