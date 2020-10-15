#pragma once

#include <memory>

#include "control/controlobject.h"
#include "database/mixxxdb.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "test/mixxxtest.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"

class LibraryTest : public MixxxTest {
  protected:
    LibraryTest();
    ~LibraryTest() override = default;

    const mixxx::DbConnectionPoolPtr& dbConnectionPool() const {
        return m_dbConnectionPooler;
    }

    QSqlDatabase dbConnection() const {
        return mixxx::DbConnectionPooled(m_dbConnectionPooler);
    }

    TrackCollectionManager* trackCollections() const {
        return m_pTrackCollectionManager.get();
    }

    TrackCollection* internalCollection() const {
        return trackCollections()->internalCollection();
    }

    TrackPointer getOrAddTrackByLocation(
            const QString& trackLocation) const;

  private:
    const MixxxDb m_mixxxDb;
    const mixxx::DbConnectionPooler m_dbConnectionPooler;
    const std::unique_ptr<TrackCollectionManager> m_pTrackCollectionManager;
    ControlObject m_keyNotationCO;
};
