#pragma once

#include <memory>

#include "test/mixxxtest.h"

#include "database/mixxxdb.h"
#include "library/trackcollection.h"
#include "util/db/dbconnectionpooler.h"
#include "util/db/dbconnectionpooled.h"
#include "track/globaltrackcache.h"

class LibraryTest : public MixxxTest,
    public virtual /*implements*/ GlobalTrackCacheSaver {

  public:
    void saveEvictedTrack(Track* pTrack) noexcept override;

  protected:
    LibraryTest();
    ~LibraryTest() override;

    mixxx::DbConnectionPoolPtr dbConnectionPool() const {
        return m_mixxxDb.connectionPool();
    }

    QSqlDatabase dbConnection() const {
        return m_dbConnection;
    }

    TrackCollection* collection() {
        return m_pTrackCollection.get();
    }

  private:
    const MixxxDb m_mixxxDb;
    const mixxx::DbConnectionPooler m_dbConnectionPooler;
    QSqlDatabase m_dbConnection;
    std::unique_ptr<TrackCollection> m_pTrackCollection;
};
