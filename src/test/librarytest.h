#ifndef LIBRARYTEST_H
#define LIBRARYTEST_H

#include "test/mixxxtest.h"

#include "repository/repository.h"
#include "library/trackcollection.h"


class LibraryTest : public MixxxTest {
  protected:
    LibraryTest()
        : m_repository(config()),
          m_dbConnectionScope(m_repository.dbConnectionPool()),
          m_trackCollection(config()) {
        QSqlDatabase sqlDatabase(m_dbConnectionScope);
        mixxx::Repository::initDatabaseSchema(sqlDatabase);
        m_trackCollection.connectDatabase(sqlDatabase);
    }
    ~LibraryTest() override {
        m_trackCollection.disconnectDatabase();
    }

    mixxx::DbConnectionPoolPtr dbConnectionPool() const {
        return m_repository.dbConnectionPool();
    }

    const mixxx::DbConnectionPool::ThreadLocalScope& dbConnectionScope() const {
        return m_dbConnectionScope;
    }

    TrackCollection* collection() {
        return &m_trackCollection;
    }

  private:
    mixxx::Repository m_repository;
    const mixxx::DbConnectionPool::ThreadLocalScope m_dbConnectionScope;
    TrackCollection m_trackCollection;
};


#endif /* LIBRARYTEST_H */
