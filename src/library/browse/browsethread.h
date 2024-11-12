/*
 * browsethread.h         (C) 2011 Tobias Rafreider
 */

#pragma once

#include <QList>
#include <QMutex>
#include <QSharedPointer>
#include <QThread>
#include <QWaitCondition>
#include <QWeakPointer>

#include "util/fileaccess.h"

// This class is a singleton and represents a thread
// that is used to read ID3 metadata
// from a particular folder.
//
// The BrowseTableModel uses this class.
// Note: Don't call getInstance() from places
// other than the GUI thread.
class BrowseTableModel;
class BrowseThread;
class QStandardItem;

typedef QSharedPointer<BrowseThread> BrowseThreadPointer;

class BrowseThread : public QThread {
    Q_OBJECT
  public:
    virtual ~BrowseThread();
    void requestPopulateModel(mixxx::FileAccess path, BrowseTableModel* pModelObserver);
    void run();
    static BrowseThreadPointer getInstanceRef();

  signals:
    void rowsAppended(const QList<QList<QStandardItem*>>&, BrowseTableModel*);
    void clearModel(BrowseTableModel*);

  private:
    BrowseThread(QObject *parent = 0);

    void waitUntilThreadIsRunning();
    void requestThreadToStop();
    void populateModel(const mixxx::FileAccess& path, BrowseTableModel* pModelObserver);

    QMutex m_requestMutex;
    QWaitCondition m_requestCondition;

    // You must hold m_requestMutex to touch m_requestedPath, m_pModelObserver
    // or m_runState
    mixxx::FileAccess m_requestedPath;
    BrowseTableModel* m_pModelObserver;
    bool m_runState;
};
