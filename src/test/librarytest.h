#ifndef LIBRARYTEST_H
#define LIBRARYTEST_H

#include "test/mixxxtest.h"

#include "repository/repository.h"
#include "library/trackcollection.h"


class LibraryTest : public MixxxTest {
  protected:
    LibraryTest()
        : m_repository(config()),
          m_trackCollection(config()) {
        m_repository.openDatabaseConnection();
        QSqlDatabase database = m_repository.database();
        mixxx::Repository::initDatabaseSchema(database);
        m_trackCollection.connectDatabase(database);
    }
    ~LibraryTest() override {
        m_trackCollection.disconnectDatabase();
    }

    TrackCollection* collection() {
        return &m_trackCollection;
    }

  private:
    mixxx::Repository m_repository;
    TrackCollection m_trackCollection;
};


#endif /* LIBRARYTEST_H */
