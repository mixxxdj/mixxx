/***************************************************************************
                           trackcollection.h
                              -------------------
     begin                : 10/27/2008
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

#ifndef TRACKCOLLECTION_H
#define TRACKCOLLECTION_H

#include <QtSql>
#include <QList>
#include <QQueue>
#include <QRegExp>
#include <QSemaphore>
#include <QMutex>
#include <QSharedPointer>
#include <QSqlDatabase>
#include <QApplication>
#include <QThread>
#include <QAtomicInt>

#include "configobject.h"
#include "library/basetrackcache.h"
#include "library/dao/trackdao.h"
#include "library/dao/cratedao.h"
#include "library/dao/cuedao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/analysisdao.h"
#include "library/queryutil.h"

#define AUTODJ_TABLE "Auto DJ"

// Lambda function
typedef std::function <void ()> func;

class TrackInfoObject;
class ControlObjectThread;
class BpmDetector;

/**
   @author Albert Santoni
*/

// Helper class for calling some code in Main thread
class MainExecuter : public QObject {
    Q_OBJECT
public:
    ~MainExecuter() {}

    // singletone
    static MainExecuter& getInstance() {
        static MainExecuter instance;
        return instance;
    }

    static void callAsync(func lambda, QString where = QString()) {
        DBG() << where;
        MainExecuter& instance = getInstance();
        instance.m_lambdaMutex.lock();
        instance.setLambda(lambda);
        emit(instance.runOnMainThread());
    }

    static void callSync(func lambda, QString where = QString()) {
        DBG() << where;
        if (QThread::currentThread() == qApp->thread()) {
            // We are already on Main thread
            lambda();
        } else {
            MainExecuter& instance = getInstance();
            instance.m_lambdaMutex.lock();
            instance.setLambda(lambda);
            emit(instance.runOnMainThread());
            instance.m_lambdaMutex.lock();
            instance.m_lambdaMutex.unlock();
        }
    }

signals:
    void runOnMainThread();

public slots:
    void call() {
        if (m_lamdaCount.testAndSetAcquire(1, 0)) {;
            DBG() << "calling m_lambda in " << QThread::currentThread()->objectName();
            m_lambda();
            m_lambdaMutex.unlock();
        }
    }

private:
    MainExecuter()
            : m_lambda(NULL) {
        moveToThread(qApp->thread());
        connect(this, SIGNAL(runOnMainThread()),
                this, SLOT(call()), Qt::QueuedConnection);
    }

    void setLambda(func newLambda) {
        m_lambda = newLambda;
        m_lamdaCount = 1;
    }

    QMutex m_lambdaMutex;
    func m_lambda;
    QAtomicInt m_lamdaCount;
};


// Separate thread providing database access. Holds all DAO. To make access to DB
// see callAsync/callSync methods.
class TrackCollection : public QThread {
    Q_OBJECT
  public:
    TrackCollection(ConfigObject<ConfigValue>* pConfig);
    ~TrackCollection();
    void run();

    void callAsync(func lambda, QString where = QString());
    void callSync(func lambda, QString where = QString());

    void stopThread();
    void addLambdaToQueue(func lambda);

    bool checkForTables();

    // Import the files in a given diretory, without recursing into subdirectories
    bool importDirectory(const QString& directory, TrackDAO& trackDao,
                         const QStringList& nameFilters, volatile bool* cancel, volatile bool* pause);

    void resetLibaryCancellation();
    QSqlDatabase& getDatabase();

    CrateDAO& getCrateDAO();
    TrackDAO& getTrackDAO();
    PlaylistDAO& getPlaylistDAO();
    QSharedPointer<BaseTrackCache> getTrackSource(const QString& name);
    void addTrackSource(const QString& name, QSharedPointer<BaseTrackCache> trackSource);
    void cancelLibraryScan();

    ConfigObject<ConfigValue>* getConfig() {
        return m_pConfig;
    }

  signals:
    void startedLoading();
    void progressLoading(QString path);
    void finishedLoading();
    void initialized();

  private:
    void createAndPopulateDbConnection();
    ConfigObject<ConfigValue>* m_pConfig;
    QSqlDatabase* m_database;
    QHash<QString, QSharedPointer<BaseTrackCache> > m_trackSources;
    PlaylistDAO* m_playlistDao;
    CrateDAO* m_crateDao;
    CueDAO* m_cueDao;
    AnalysisDao* m_analysisDao;
    TrackDAO* m_trackDao;

    QQueue<func> m_lambdas;
    volatile bool m_stop;
    QMutex m_lambdasQueueMutex; // mutex for accessing queue of lambdas
    QSemaphore m_semLambdasReadyToCall; // lambdas in queue ready to call
    QSemaphore m_semLambdasFree; // count of vacant places for lambdas in queue
    ControlObjectThread* m_pCOTPlaylistIsBusy;

    const QRegExp m_supportedFileExtensionsRegex;
    bool m_inCallSync;
    QStringList m_tracksListInCnunk;

    void addTrackToChunk(const QString filePath, TrackDAO &trackDao);
    void addChunkToDatabase(TrackDAO &trackDao);

};

#endif
