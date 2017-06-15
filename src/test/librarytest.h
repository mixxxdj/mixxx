#ifndef LIBRARYTEST_H
#define LIBRARYTEST_H

#include "test/mixxxtest.h"

#include "database/mixxxdb.h"
#include "library/trackcollection.h"
#include "util/db/dbconnectionpooled.h"


class LibraryTest : public MixxxTest {
  protected:
    LibraryTest()
        : m_mixxxDb(config()),
          m_dbConnection(m_mixxxDb.connectionPool()),
          m_trackCollection(config()) {
        QSqlDatabase dbConnection(m_dbConnection);
        MixxxDb::initDatabaseSchema(dbConnection);
        m_trackCollection.connectDatabase(dbConnection);
    }
    ~LibraryTest() override {
        m_trackCollection.disconnectDatabase();
    }

    mixxx::DbConnectionPoolPtr dbConnectionPool() const {
        return m_mixxxDb.connectionPool();
    }

    const mixxx::DbConnectionPooled& dbConnection() const {
        return m_dbConnection;
    }

    TrackCollection* collection() {
        return &m_trackCollection;
    }

  private:
    MixxxDb m_mixxxDb;
    const mixxx::DbConnectionPooled m_dbConnection;
    TrackCollection m_trackCollection;
};


#endif /* LIBRARYTEST_H */
