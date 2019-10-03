#include "test/librarytest.h"

namespace {

const bool kInMemoryDbConnection = true;

void deleteTrack(Track* pTrack) {
    // Delete track objects directly in unit tests with
    // no main event loop
    delete pTrack;
};

}

LibraryTest::LibraryTest()
    : m_mixxxDb(config(), kInMemoryDbConnection),
      m_dbConnectionPooler(m_mixxxDb.connectionPool()),
      m_dbConnection(mixxx::DbConnectionPooled(m_mixxxDb.connectionPool())),
      m_pTrackCollection(std::make_unique<TrackCollection>(config())) {
    MixxxDb::initDatabaseSchema(m_dbConnection);
    m_pTrackCollection->connectDatabase(m_dbConnection);
    GlobalTrackCache::createInstance(this, deleteTrack);
}

LibraryTest::~LibraryTest() {
    m_pTrackCollection->disconnectDatabase();
    m_pTrackCollection.reset();
    // With the track collection all remaining track references
    // should have been dropped before destroying the cache.
    GlobalTrackCache::destroyInstance();
}

void LibraryTest::saveEvictedTrack(Track* pTrack) noexcept {
    m_pTrackCollection->exportTrackMetadata(pTrack);
    m_pTrackCollection->saveTrack(pTrack);
}
