#ifndef BROWSETHREAD_H
#define BROWSETHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QStandardItem>
#include <QList>

#include "library/browse/browsetablemodel.h"

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
    QList<int> m_searchColumns;
    QString m_path;
    bool m_bStopThread;

    static BrowseThread* m_instance;
    BrowseTableModel* m_model_observer;
};

#endif // BROWSETHREAD_H
