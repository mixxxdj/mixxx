#ifndef LIBRARYTEST_H
#define LIBRARYTEST_H

#include "test/mixxxtest.h"

#include "library/trackcollection.h"
#include "util/memory.h"


class LibraryTest : public MixxxTest {
  protected:
    LibraryTest() {
        m_pTrackCollection = std::make_unique<TrackCollection>(config());
    }
    ~LibraryTest() override {
    }

    TrackCollection* collection() const {
        return m_pTrackCollection.get();
    }

  private:
    std::unique_ptr<TrackCollection> m_pTrackCollection;
};


#endif /* LIBRARYTEST_H */
