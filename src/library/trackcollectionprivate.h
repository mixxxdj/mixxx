// trackcollection.h
// Created 10/27/2008 by Albert Santoni <gamegod \a\t users.sf.net>
// Lambda scheme introduced 27/09/2013 by Nazar Gerasymchuk <troyan3 @ gmail.com>

#ifndef TRACKCOLLECTIONPRIVATE_H
#define TRACKCOLLECTIONPRIVATE_H

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
#include "library/dao/autodjcratesdao.h"
#include "library/queryutil.h"

#define AUTODJ_TABLE "Auto DJ"

class TrackCollectionPrivate;

typedef std::function <void (TrackCollectionPrivate*)> func;

class TrackInfoObject;
class ControlObjectThread;
class BpmDetector;

// Separate thread providing database access. Holds all DAO. To make access to DB
// see callAsync/callSync methods.
class TrackCollectionPrivate : public QObject {
    Q_OBJECT
  public:
    TrackCollectionPrivate(ConfigObject<ConfigValue>* pConfig);
    ~TrackCollectionPrivate();

    void initialize();

    bool checkForTables();

    QSqlDatabase& getDatabase();

    CrateDAO& getCrateDAO();
    TrackDAO& getTrackDAO();
    PlaylistDAO& getPlaylistDAO();
#ifdef __AUTODJCRATES__
    AutoDJCratesDAO& getAutoDJCratesDAO();
#endif

    ConfigObject<ConfigValue>* getConfig() {
        return m_pConfig;
    }

  signals:
    void initialized();

  private:
    void createAndPopulateDbConnection();
    ConfigObject<ConfigValue>* m_pConfig;
    QSqlDatabase* m_pDatabase;
    PlaylistDAO* m_pPlaylistDao;
    CrateDAO* m_pCrateDao;
    CueDAO* m_pCueDao;
    AnalysisDao* m_pAnalysisDao;
    TrackDAO* m_pTrackDao;
    AutoDJCratesDAO* m_pAutoDjCratesDao;

    QQueue<func> m_lambdas;
    volatile bool m_stop;
    QMutex m_lambdasQueueMutex; // mutex for accessing queue of lambdas
    QSemaphore m_semLambdasReadyToCall; // lambdas in queue ready to call
    QSemaphore m_semLambdasFree; // count of vacant places for lambdas in queue
    ControlObjectThread* m_pCOTPlaylistIsBusy;

    const QRegExp m_supportedFileExtensionsRegex;
    int m_inCallSyncCount;
};

#endif
