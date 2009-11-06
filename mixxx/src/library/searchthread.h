// searchthread.h
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#ifndef SEARCHTHREAD_H
#define SEARCHTHREAD_H

#include <QMutex>
#include <QThread>
#include <QQueue>
#include <QPair>
#include <QWaitCondition>
#include <QString>

class TrackModel;

class SearchThread : public QThread {
    Q_OBJECT
  public:
    SearchThread(QObject* parent = NULL);
    virtual ~SearchThread();
    void enqueueSearch(TrackModel* model, QString search);
    void stop();

  protected:
    void run();

  private:
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    QWaitCondition m_stopWaitCondition;
    QQueue<QPair<TrackModel*, QString> > m_searchQueue;
    bool m_bQuit;
};

#endif /* SEARCHTHREAD_H */
