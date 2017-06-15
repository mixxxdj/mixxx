#ifndef LIBRARYTEST_H
#define LIBRARYTEST_H

#include "test/mixxxtest.h"

#include "database/mixxxdb.h"
#include "library/trackcollection.h"
#include "util/db/dbconnectionpooler.h"
#include "util/db/dbconnectionpooled.h"
#include "track/trackcache.h"


class LibraryTest : public MixxxTest,
    public virtual /*implements*/ TrackCacheEvictor {

  public:
    void evictTrack(Track* pTrack) override {
        m_trackCollection.saveTrack(pTrack);
    }

  protected:
    LibraryTest()
        : m_mixxxDb(config()),
          m_dbConnectionPooler(m_mixxxDb.connectionPool()),
          m_dbConnection(mixxx::DbConnectionPooled(m_mixxxDb.connectionPool())),
          m_trackCollection(config()) {
        MixxxDb::initDatabaseSchema(m_dbConnection);
        m_trackCollection.connectDatabase(m_dbConnection);
        TrackCache::createInstance(this);
    }
    ~LibraryTest() override {
        TrackCache::instance().evictAll();
        TrackCache::destroyInstance();
        m_trackCollection.disconnectDatabase();
    }

    mixxx::DbConnectionPoolPtr dbConnectionPool() const {
        return m_mixxxDb.connectionPool();
    }

    QSqlDatabase dbConnection() const {
        return m_dbConnection;
    }

    TrackCollection* collection() {
        return &m_trackCollection;
    }

  private:
    const MixxxDb m_mixxxDb;
    const mixxx::DbConnectionPooler m_dbConnectionPooler;
    QSqlDatabase m_dbConnection;
    TrackCollection m_trackCollection;
};


#endif /* LIBRARYTEST_H */
