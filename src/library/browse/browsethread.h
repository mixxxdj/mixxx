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
    void requestPopulateModel(mixxx::FileAccess path, BrowseTableModel* client);
    void run();
    static BrowseThreadPointer getInstanceRef();

  signals:
    void rowsAppended(const QList<QList<QStandardItem*>>&, BrowseTableModel*);
    void clearModel(BrowseTableModel*);

  private:
    BrowseThread(QObject *parent = 0);

    void populateModel(mixxx::FileAccess path, BrowseTableModel* client);

    QMutex m_mutex;
    QWaitCondition m_condition;

    // You must hold m_mutex to touch m_path, m_model_observer or m_bRun
    mixxx::FileAccess m_path;
    BrowseTableModel* m_model_observer;
    bool m_bRun;
};
