#pragma once

#include <memory>

#include "control/controlobject.h"
#include "database/mixxxdb.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "test/mixxxdbtest.h"
#include "test/soundsourceproviderregistration.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"

class LibraryTest : public MixxxDbTest, SoundSourceProviderRegistration {
  protected:
    LibraryTest();
    ~LibraryTest() override = default;

    TrackCollectionManager* trackCollectionManager() const {
        return m_pTrackCollectionManager.get();
    }

    TrackCollection* internalCollection() const {
        return trackCollectionManager()->internalCollection();
    }

    TrackPointer getOrAddTrackByLocation(
            const QString& trackLocation) const;

  private:
    const std::unique_ptr<TrackCollectionManager> m_pTrackCollectionManager;
    ControlObject m_keyNotationCO;
};
