#ifndef LIBRARYTEST_H
#define LIBRARYTEST_H

#include "test/mixxxtest.h"

#include "database/mixxxdb.h"
#include "library/trackcollection.h"


class LibraryTest : public MixxxTest {
  protected:
    LibraryTest()
        : m_dataSource(config()),
          m_trackCollection(config()) {
        m_dataSource.initDatabaseSchema();
        m_trackCollection.setDatabase(m_dataSource.database());
    }
    ~LibraryTest() override {
    }

    TrackCollection* collection() {
        return &m_trackCollection;
    }

  private:
    MixxxDB m_dataSource;
    TrackCollection m_trackCollection;
};


#endif /* LIBRARYTEST_H */
