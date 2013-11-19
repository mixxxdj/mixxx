/*
 * browsethread.h         (C) 2011 Tobias Rafreider
 */

#ifndef BROWSETHREAD_H
#define BROWSETHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QStandardItem>
#include <QList>

// This class is a singleton and represents a thread
// that is used to read ID3 metadata
// from a particular folder.
//
// The BroseTableModel uses this class.
// Note: Don't call getInstance() from places
// other than the GUI thread.
class BrowseTableModel;

class BrowseThread : public QThread {
    Q_OBJECT
  public:
    void executePopulation(QString& path, BrowseTableModel* client);
    void run();
    static BrowseThread* getInstance();
    static void destroyInstance();

  signals:
    void rowsAppended(const QList< QList<QStandardItem*> >&, BrowseTableModel*);
    void clearModel(BrowseTableModel*);

  private:
    BrowseThread(QObject *parent = 0);
    virtual ~BrowseThread();

    void populateModel();

    QMutex m_mutex;
    QWaitCondition m_locationUpdated;
    volatile bool m_bStopThread;

    // You must hold m_path_mutex to touch m_path or m_model_observer
    QMutex m_path_mutex;
    QString m_path;
    BrowseTableModel* m_model_observer;

    static BrowseThread* m_instance;
};

#endif // BROWSETHREAD_H
