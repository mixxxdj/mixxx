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

class MainExecuter : public QObject {
    Q_OBJECT
public:
    ~MainExecuter() {}

    static MainExecuter& getInstance() {
        static MainExecuter instance;
        return instance;
    }
    QMutex lambdaMutex;
    func lambda;

    static void callAsync(func lambda) {
//        MainExecuter* me = new MainExecuter(lambda);
//        me->moveToThread(qApp->thread());
//        connect(me, SIGNAL(runOnMainThread()),
//                me, SLOT(call()), Qt::QueuedConnection);
//        emit(me->runOnMainThread());
        MainExecuter* instance = &getInstance();
        instance->setLambda(lambda);
        instance->moveToThread(qApp->thread());

        connect(instance, SIGNAL(runOnMainThread()),
                instance, SLOT(call()), Qt::QueuedConnection);
        emit(instance->runOnMainThread());
    }

    static void callSync(func lambda) {
//        MainExecuter* me = new MainExecuter(lambda);
//        me->moveToThread(qApp->thread());
//        connect(me, SIGNAL(runOnMainThread()),
//                me, SLOT(call()), Qt::QueuedConnection);
//        me->m_lambdaMutex.lock();
//        emit(me->runOnMainThread());
//        me->m_lambdaMutex.lock();
//        me->m_lambdaMutex.unlock();
        MainExecuter* instance = &getInstance();
        instance->setLambda(lambda);
        instance->moveToThread(qApp->thread());

        connect(instance, SIGNAL(runOnMainThread()),
                instance, SLOT(call()), Qt::QueuedConnection);
        instance->lambdaMutex.lock();
        emit(instance->runOnMainThread());
        instance->lambdaMutex.lock();
        instance->lambdaMutex.unlock();
    }

signals:
    void runOnMainThread();

private slots:
    void call() {
        DBG() << "calling lambda in " << QThread::currentThread()->objectName();
        MainExecuter* instance = &getInstance();
        instance->lambda();
        instance->lambdaMutex.unlock();
        disconnect(instance, 0, 0, 0);
    }

private:
//    MainExecuter(func lambda) : m_lambda(lambda) {  }
    MainExecuter() : lambda(NULL) { static short count = 0; DBG() << ++count; }
    void setLambda(func newLambda) { lambda = newLambda; }
};



class TrackCollection : public QThread {
    Q_OBJECT
  public:
    TrackCollection(ConfigObject<ConfigValue>* pConfig);
    ~TrackCollection();
    void run();

    void callAsync(func lambda);
    void callSync(func lambda);

    void stopThread();
    void addLambdaToQueue(func lambda);

    bool checkForTables();

    /** Import the files in a given diretory, without recursing into subdirectories */
    bool importDirectory(const QString& directory, TrackDAO& trackDao,
                         const QStringList& nameFilters, volatile bool* cancel);

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
};

#endif
