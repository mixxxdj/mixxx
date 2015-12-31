#ifndef LIBRARYTEST_H
#define LIBRARYTEST_H

#include <QDir>
#include <QScopedPointer>

#include "test/mixxxtest.h"
#include "library/trackcollection.h"

class LibraryTest : public MixxxTest {
  protected:
    LibraryTest() {
        m_pTrackCollection.reset(new TrackCollection(config()));
    }

    TrackCollection* collection() {
        return m_pTrackCollection.data();
    }

  private:
    QScopedPointer<TrackCollection> m_pTrackCollection;
};


#endif /* LIBRARYTEST_H */
