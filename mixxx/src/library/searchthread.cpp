// searchthread.cpp
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#include <QMutexLocker>
#include <QSet>

#include "library/searchthread.h"

#include "library/trackmodel.h"

SearchThread::SearchThread(QObject* parent)
        : QThread(parent),
          m_bQuit(false) {
    start();
}

SearchThread::~SearchThread() {
    stop();
}

void SearchThread::enqueueSearch(TrackModel* pModel, QString search) {
    if (pModel == NULL)
        return;

    QMutexLocker lock(&m_mutex);
    m_searchQueue.enqueue(QPair<TrackModel*,QString>(pModel, search));
    m_waitCondition.wakeAll();
}

void SearchThread::stop() {
    QMutexLocker lock(&m_mutex);
    m_bQuit = true;
    m_waitCondition.wakeAll();
}

void SearchThread::run() {
    while (!m_bQuit) {
        m_mutex.lock();
        if (m_bQuit)
            break;
        if (m_searchQueue.isEmpty()) {
            m_waitCondition.wait(&m_mutex);
        }

        QQueue<QPair<TrackModel*, QString> > searches;
        QSet<TrackModel*> processed;
        while (!m_searchQueue.isEmpty()) {
            QPair<TrackModel*, QString> pair = m_searchQueue.takeLast();
            // If we already executed a more recent search than this one, ignore
            // this one.
            if (processed.contains(pair.first))
                continue;
            searches.enqueue(pair);
            processed.insert(pair.first);

        }
        // Don't need to hold the lock while doing each search.
        m_mutex.unlock();

        while (!searches.isEmpty()) {
            QPair<TrackModel*, QString> pair = searches.takeFirst();
            pair.first->search(pair.second);
        }
    }
}
