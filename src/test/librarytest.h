#ifndef LIBRARYTEST_H
#define LIBRARYTEST_H

#include "test/mixxxtest.h"

#include "database/mixxxdb.h"
#include "library/trackcollection.h"


class LibraryTest : public MixxxTest {
  protected:
    LibraryTest()
        : m_mixxxDb(config()),
          m_dbConnectionScope(m_mixxxDb.connectionPool()),
          m_trackCollection(config()) {
        QSqlDatabase dbConnection(m_dbConnectionScope);
        MixxxDb::initDatabaseSchema(dbConnection);
        m_trackCollection.connectDatabase(dbConnection);
    }
    ~LibraryTest() override {
        m_trackCollection.disconnectDatabase();
    }

    mixxx::DbConnectionPoolPtr dbConnectionPool() const {
        return m_mixxxDb.connectionPool();
    }

    const mixxx::DbConnectionPool::ThreadLocalScope& dbConnectionScope() const {
        return m_dbConnectionScope;
    }

    TrackCollection* collection() {
        return &m_trackCollection;
    }

  private:
    MixxxDb m_mixxxDb;
    const mixxx::DbConnectionPool::ThreadLocalScope m_dbConnectionScope;
    TrackCollection m_trackCollection;
};


#endif /* LIBRARYTEST_H */
