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
#include <QSharedPointer>
#include <QSqlDatabase>

#include "configobject.h"
#include "library/basetrackcache.h"
#include "library/dao/trackdao.h"
#include "library/dao/cratedao.h"
#include "library/dao/cuedao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/analysisdao.h"
#include "library/dao/directorydao.h"
#include "library/dao/libraryhashdao.h"

#ifdef __SQLITE3__
typedef struct sqlite3_context sqlite3_context;
typedef struct Mem sqlite3_value;
#endif

class TrackInfoObject;

#define AUTODJ_TABLE "Auto DJ"

class BpmDetector;

/**
   @author Albert Santoni
*/
class TrackCollection : public QObject {
    Q_OBJECT
  public:
    static const int kRequiredSchemaVersion;

    TrackCollection(ConfigObject<ConfigValue>* pConfig);
    virtual ~TrackCollection();
    bool checkForTables();

    void resetLibaryCancellation();
    QSqlDatabase& getDatabase();

    CrateDAO& getCrateDAO();
    TrackDAO& getTrackDAO();
    PlaylistDAO& getPlaylistDAO();
    DirectoryDAO& getDirectoryDAO();
    QSharedPointer<BaseTrackCache> getTrackSource();
    void setTrackSource(QSharedPointer<BaseTrackCache> trackSource);
    void cancelLibraryScan();

    ConfigObject<ConfigValue>* getConfig() {
        return m_pConfig;
    }

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
    ConfigObject<ConfigValue>* m_pConfig;
    QSqlDatabase m_db;
    QSharedPointer<BaseTrackCache> m_defaultTrackSource;
    PlaylistDAO m_playlistDao;
    CrateDAO m_crateDao;
    CueDAO m_cueDao;
    DirectoryDAO m_directoryDao;
    AnalysisDao m_analysisDao;
    LibraryHashDAO m_libraryHashDao;
    TrackDAO m_trackDao;
};

#endif // TRACKCOLLECTION_H
