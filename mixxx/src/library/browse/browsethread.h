#ifndef BROWSETHREAD_H
#define BROWSETHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QStandardItem>
#include <QList>

class BrowseThread : public QThread {
    Q_OBJECT
  public:
    BrowseThread(QObject *parent = 0);
    ~BrowseThread();

    void setPath(QString& path);
    void run();

  signals:
    void rowsAppended(const QList< QList<QStandardItem*> >&);
    void clearModel();

  private:
    void populateModel();

    QMutex m_mutex;
    QWaitCondition m_locationUpdated;
    QList<int> m_searchColumns;
    QString m_path;
    volatile bool m_bStopThread;
};

#endif // BROWSETHREAD_H
