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
        m_repository.initDatabaseSchema();
        m_trackCollection.setDatabase(m_repository.database());
    }
    ~LibraryTest() override {
    }

    TrackCollection* collection() {
        return &m_trackCollection;
    }

  private:
    mixxx::Repository m_repository;
    TrackCollection m_trackCollection;
};


#endif /* LIBRARYTEST_H */
