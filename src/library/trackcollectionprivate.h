// trackcollection.h
// Created 10/27/2008 by Albert Santoni <gamegod \a\t users.sf.net>
// Lambda scheme introduced 27/09/2013 by Nazar Gerasymchuk <troyan3 @ gmail.com>

#ifndef TRACKCOLLECTIONPRIVATE_H
#define TRACKCOLLECTIONPRIVATE_H

#ifdef __SQLITE3__
#include <sqlite3.h>
#endif

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
#include <functional>

#include "configobject.h"
#include "library/basetrackcache.h"
#include "library/dao/trackdao.h"
#include "library/dao/directorydao.h"
#include "library/dao/cratedao.h"
#include "library/dao/cuedao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/analysisdao.h"
#include "library/dao/libraryhashdao.h"
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
    DirectoryDAO& getDirectoryDAO();
#ifdef __AUTODJCRATES__
    AutoDJCratesDAO& getAutoDJCratesDAO();
#endif

    /** Import the files in a given diretory, without recursing into subdirectories */
    bool importDirectory(const QString& directory, TrackDAO& trackDao,
                         const QStringList& nameFilters, volatile bool* cancel);

    ConfigObject<ConfigValue>* getConfig() {
        return m_pConfig;
    }

  signals:
    void initialized();

  protected:
#ifdef __SQLITE3__
    void installSorting(QSqlDatabase &db);
    static int sqliteLocaleAwareCompare(void* pArg,
                                        int len1, const void* data1,
                                        int len2, const void* data2);
    static void sqliteLike(sqlite3_context *p,
                          int aArgc,
                          sqlite3_value **aArgv);
    static void makeLatinLow(QChar* c, int count);
    static int likeCompareLatinLow(
            QString* pattern,
            QString* string,
            const QChar esc);
    static int likeCompareInner(
            const QChar* pattern,
            int patterenSize,
            const QChar* string,
            int stringSize,
            const QChar esc);
#endif // __SQLITE3__

  private:
    void createAndPopulateDbConnection();
    ConfigObject<ConfigValue>* m_pConfig;
    QSqlDatabase* m_pDatabase;
    PlaylistDAO* m_pPlaylistDao;
    CrateDAO* m_pCrateDao;
    DirectoryDAO* m_pDirectoryDao;
    CueDAO* m_pCueDao;
    LibraryHashDAO* m_pLibraryHashDao;
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
