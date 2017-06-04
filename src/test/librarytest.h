#ifndef LIBRARYTEST_H
#define LIBRARYTEST_H

#include "test/mixxxtest.h"

#include "library/trackcollection.h"


class LibraryTest : public MixxxTest {
  protected:
    LibraryTest()
        : m_trackCollection(config()) {
        m_trackCollection.initDatabaseSchema();
    }
    ~LibraryTest() override {
    }

    TrackCollection* collection() {
        return &m_trackCollection;
    }

  private:
    TrackCollection m_trackCollection;
};


#endif /* LIBRARYTEST_H */
