/*
 * browsethread.h         (C) 2011 Tobias Rafreider
 */

#ifndef BROWSETHREAD_H
#define BROWSETHREAD_H

#include <QByteArrayData>
#include <QList>
#include <QMutex>
#include <QSharedPointer>
#include <QStandardItem>
#include <QString>
#include <QThread>
#include <QWaitCondition>
#include <QWeakPointer>

#include "util/file.h"

// This class is a singleton and represents a thread
// that is used to read ID3 metadata
// from a particular folder.
//
// The BrowseTableModel uses this class.
// Note: Don't call getInstance() from places
// other than the GUI thread.
class BrowseTableModel;
class BrowseThread;
class QObject;
class QStandardItem;
template<class T>
class QWeakPointer;

typedef QSharedPointer<BrowseThread> BrowseThreadPointer;

class BrowseThread : public QThread {
    Q_OBJECT
  public:
    virtual ~BrowseThread();
    void executePopulation(const MDir& path, BrowseTableModel* client);
    void run();
    static BrowseThreadPointer getInstanceRef();

  signals:
    void rowsAppended(const QList< QList<QStandardItem*> >&, BrowseTableModel*);
    void clearModel(BrowseTableModel*);

  private:
    BrowseThread(QObject *parent = 0);

    void populateModel();

    QMutex m_mutex;
    QWaitCondition m_locationUpdated;
    volatile bool m_bStopThread;

    // You must hold m_path_mutex to touch m_path or m_model_observer
    QMutex m_path_mutex;
    MDir m_path;
    BrowseTableModel* m_model_observer;

    static QWeakPointer<BrowseThread> m_weakInstanceRef;
};

#endif // BROWSETHREAD_H
