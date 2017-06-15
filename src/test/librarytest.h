#ifndef LIBRARYTEST_H
#define LIBRARYTEST_H

#include "test/mixxxtest.h"

#include "database/mixxxdb.h"
#include "library/trackcollection.h"
#include "util/db/dbconnectionpooler.h"
#include "util/db/dbconnectionpooled.h"


class LibraryTest : public MixxxTest {
  protected:
    LibraryTest()
        : m_mixxxDb(config()),
          m_dbConnectionPooler(m_mixxxDb.connectionPool()),
          m_dbConnectionPooled(m_dbConnectionPooler),
          m_trackCollection(config()) {
        QSqlDatabase dbConnection(m_dbConnectionPooled);
        MixxxDb::initDatabaseSchema(dbConnection);
        m_trackCollection.connectDatabase(dbConnection);
    }
    ~LibraryTest() override {
        m_trackCollection.disconnectDatabase();
    }

    mixxx::DbConnectionPoolPtr dbConnectionPool() const {
        return m_mixxxDb.connectionPool();
    }

    QSqlDatabase dbConnection() const {
        return static_cast<QSqlDatabase>(m_dbConnectionPooled);
    }

    TrackCollection* collection() {
        return &m_trackCollection;
    }

  private:
    const MixxxDb m_mixxxDb;
    const mixxx::DbConnectionPooler m_dbConnectionPooler;
    const mixxx::DbConnectionPooled m_dbConnectionPooled;
    TrackCollection m_trackCollection;
};


#endif /* LIBRARYTEST_H */
