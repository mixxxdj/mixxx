#include "track/globaltrackcache.h"

#include <QApplication>

#include "util/assert.h"
#include "util/logger.h"


namespace {

constexpr std::size_t kUnorderedCollectionMinCapacity = 1024;

const mixxx::Logger kLogger("GlobalTrackCache");

//static
GlobalTrackCache* volatile s_pInstance = nullptr;

// Enforce logging during tests
constexpr bool kLogEnabled = false;

inline bool debugLogEnabled() {
    return kLogEnabled || kLogger.debugEnabled();
}

inline bool traceLogEnabled() {
    return kLogEnabled || kLogger.traceEnabled();
}

constexpr bool kLogStats = false;

inline
TrackRef createTrackRef(const Track& track) {
    return TrackRef::fromFileInfo(track.getFileInfo(), track.getId());
}

} // anonymous namespace

GlobalTrackCacheLocker::GlobalTrackCacheLocker()
        : m_pInstance(nullptr) {
    lockCache();
}

GlobalTrackCacheLocker::GlobalTrackCacheLocker(
        GlobalTrackCacheLocker&& moveable)
        : m_pInstance(std::move(moveable.m_pInstance)) {
    moveable.m_pInstance = nullptr;
}

GlobalTrackCacheLocker::~GlobalTrackCacheLocker() {
    unlockCache();
}

void GlobalTrackCacheLocker::lockCache() {
    DEBUG_ASSERT(s_pInstance);
    DEBUG_ASSERT(!m_pInstance);
    if (traceLogEnabled()) {
        kLogger.trace() << "Locking cache";
    }
    s_pInstance->m_mutex.lock();
    if (traceLogEnabled()) {
        kLogger.trace() << "Cache is locked";
    }
    m_pInstance = s_pInstance;
}

void GlobalTrackCacheLocker::unlockCache() {
    if (m_pInstance) {
        // Verify consistency before unlocking the cache
        DEBUG_ASSERT(m_pInstance->verifyConsistency());
        if (traceLogEnabled()) {
            kLogger.trace() << "Unlocking cache";
        }
        if (kLogStats && debugLogEnabled()) {
            kLogger.debug()
                    << "#tracksById ="
                    << m_pInstance->m_tracksById.size()
                    << "/ #tracksByCanonicalLocation ="
                    << m_pInstance->m_tracksByCanonicalLocation.size()
                    << "/ #indexedTracks ="
                    << m_pInstance->m_indexedTracks.size();
        }
        m_pInstance->m_mutex.unlock();
        if (traceLogEnabled()) {
            kLogger.trace() << "Cache is unlocked";
        }
        m_pInstance = nullptr;
    }
}

void GlobalTrackCacheLocker::resetCache() const {
    DEBUG_ASSERT(m_pInstance);
    m_pInstance->reset();
}

void GlobalTrackCacheLocker::deactivateCache() const {
    DEBUG_ASSERT(m_pInstance);
    m_pInstance->deactivate();
}

bool GlobalTrackCacheLocker::isEmpty() const {
    DEBUG_ASSERT(m_pInstance);
    return m_pInstance->isEmpty();
}

TrackPointer GlobalTrackCacheLocker::lookupTrackById(
        const TrackId& trackId) const {
    DEBUG_ASSERT(m_pInstance);
    return m_pInstance->lookupById(trackId);
}

TrackPointer GlobalTrackCacheLocker::lookupTrackByRef(
        const TrackRef& trackRef) const {
    DEBUG_ASSERT(m_pInstance);
    return m_pInstance->lookupByRef(trackRef);
}

GlobalTrackCacheResolver::GlobalTrackCacheResolver(
        QFileInfo fileInfo,
        SecurityTokenPointer pSecurityToken)
        : m_lookupResult(GlobalTrackCacheLookupResult::NONE) {
    DEBUG_ASSERT(m_pInstance);
    m_pInstance->resolve(this, std::move(fileInfo), TrackId(), std::move(pSecurityToken));
}

GlobalTrackCacheResolver::GlobalTrackCacheResolver(
        QFileInfo fileInfo,
        TrackId trackId,
        SecurityTokenPointer pSecurityToken)
        : m_lookupResult(GlobalTrackCacheLookupResult::NONE) {
    DEBUG_ASSERT(m_pInstance);
    m_pInstance->resolve(this, std::move(fileInfo), std::move(trackId), std::move(pSecurityToken));
}

void GlobalTrackCacheResolver::initLookupResult(
        GlobalTrackCacheLookupResult lookupResult,
        TrackPointer&& strongPtr,
        TrackRef&& trackRef) {
    DEBUG_ASSERT(m_pInstance);
    DEBUG_ASSERT(GlobalTrackCacheLookupResult::NONE == m_lookupResult);
    DEBUG_ASSERT(!m_strongPtr);
    m_lookupResult = lookupResult;
    m_strongPtr = std::move(strongPtr);
    m_trackRef = std::move(trackRef);
}

void GlobalTrackCacheResolver::initTrackIdAndUnlockCache(TrackId trackId) {
    DEBUG_ASSERT(m_pInstance);
    DEBUG_ASSERT(GlobalTrackCacheLookupResult::NONE != m_lookupResult);
    DEBUG_ASSERT(m_strongPtr);
    DEBUG_ASSERT(trackId.isValid());
    if (m_trackRef.getId().isValid()) {
        // Ignore initializing the same id twice
        DEBUG_ASSERT(m_trackRef.getId() == trackId);
    } else {
        m_trackRef = m_pInstance->initTrackId(
                m_strongPtr,
                m_trackRef,
                trackId);
    }
    unlockCache();
    DEBUG_ASSERT(m_trackRef == createTrackRef(*m_strongPtr));
}

//static
void GlobalTrackCache::createInstance(GlobalTrackCacheDeleter* pDeleter) {
    DEBUG_ASSERT(!s_pInstance);
    s_pInstance = new GlobalTrackCache(pDeleter);
}

//static
void GlobalTrackCache::destroyInstance() {
    DEBUG_ASSERT(s_pInstance);
    GlobalTrackCache* pInstance = s_pInstance;
    // Reset the static/global pointer before entering the destructor
    s_pInstance = nullptr;
    // Delete the singular instance
    delete pInstance;
}

void GlobalTrackCache::deleteTrack(Track* plainPtr) {
    VERIFY_OR_DEBUG_ASSERT(plainPtr) {
        kLogger.warning()
                << "Cannot delete null track pointer";
        return;
    }
    if (traceLogEnabled()) {
        plainPtr->dumpObjectInfo();
    }
    // Track object must not be deleted by operator new!
    // Otherwise the deleted track object might be accessed
    // when processing cross-thread signals that are delayed
    // within a queued connection and may arrive after the
    // object has already been deleted.
    if (debugLogEnabled()) {
        kLogger.debug()
                << "Deleting track"
                << plainPtr;
    }
    plainPtr->deleteLater();
}

//static
void GlobalTrackCache::deleter(Track* plainPtr) {
    DEBUG_ASSERT(plainPtr);
    // Any access to plainPtr is forbidden!! Due to race condition
    // this pointer might already have been deleted! This happens
    // when a previous invocation is outpaced by a following
    // invocation that will already delete the object behind the
    // pointer. Tracks might be revived before being deleted and
    // as a consequence the reference count might drop to 0 multiple
    // times. The order in which those competing invocations finally
    // lock and enter the thread-safe cache is undefined!!!
    if (s_pInstance) {
        s_pInstance->evictAndDelete(plainPtr);
    } else {
        // Simply delete unreferenced tracks when the cache is no
        // longer available. This might but should not happen.
        kLogger.warning()
                << "Deleting uncached track";
        deleteTrack(plainPtr);
    }
}

GlobalTrackCache::GlobalTrackCache(GlobalTrackCacheDeleter* pDeleter)
    : m_mutex(QMutex::Recursive),
      m_pDeleter(pDeleter),
      m_indexedTracks(kUnorderedCollectionMinCapacity),
      m_unindexedTracks(kUnorderedCollectionMinCapacity),
      m_tracksById(kUnorderedCollectionMinCapacity, DbId::hash_fun) {
    DEBUG_ASSERT(m_pDeleter);
    DEBUG_ASSERT(verifyConsistency());
}

GlobalTrackCache::~GlobalTrackCache() {
    deactivate();
}

bool GlobalTrackCache::verifyConsistency() const {
    VERIFY_OR_DEBUG_ASSERT(m_tracksById.size() <= m_indexedTracks.size()) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_tracksByCanonicalLocation.size() <= m_indexedTracks.size()) {
        return false;
    }
    for (IndexedTracks::const_iterator i = m_indexedTracks.begin(); i != m_indexedTracks.end(); ++i) {
        const Track* plainPtr = (*i).first;
        VERIFY_OR_DEBUG_ASSERT(plainPtr) {
            return false;
        }
        TrackRef trackRef = createTrackRef(*plainPtr);
        if (trackRef.hasId()) {
            TracksById::const_iterator trackById =
                    m_tracksById.find(trackRef.getId());
            VERIFY_OR_DEBUG_ASSERT(trackById != m_tracksById.end()) {
                return false;
            }
            VERIFY_OR_DEBUG_ASSERT((*trackById).second == plainPtr) {
                return false;
            }
        }
        if (trackRef.hasCanonicalLocation()) {
            TracksByCanonicalLocation::const_iterator trackByCanonicalLocation =
                    m_tracksByCanonicalLocation.find(trackRef.getCanonicalLocation());
            VERIFY_OR_DEBUG_ASSERT(trackByCanonicalLocation != m_tracksByCanonicalLocation.end()) {
                return false;
            }
            VERIFY_OR_DEBUG_ASSERT((*trackByCanonicalLocation).second == plainPtr) {
                return false;
            }
        }
    }
    for (TracksById::const_iterator i = m_tracksById.begin(); i != m_tracksById.end(); ++i) {
        const Track* plainPtr = (*i).second;
        VERIFY_OR_DEBUG_ASSERT(plainPtr) {
            return false;
        }
        const TrackRef trackRef = createTrackRef(*plainPtr);
        VERIFY_OR_DEBUG_ASSERT(trackRef.getId().isValid()) {
            return false;
        }
        VERIFY_OR_DEBUG_ASSERT(trackRef.getId() == (*i).first) {
            return false;
        }
        const QString canonicalLocation = trackRef.getCanonicalLocation();
        if (!canonicalLocation.isEmpty()) {
            VERIFY_OR_DEBUG_ASSERT(
                    1 == m_tracksByCanonicalLocation.count(canonicalLocation)) {
                return false;
            }
            TracksByCanonicalLocation::const_iterator j =
                    m_tracksByCanonicalLocation.find(canonicalLocation);
            VERIFY_OR_DEBUG_ASSERT(m_tracksByCanonicalLocation.end() != j) {
                return false;
            }
            VERIFY_OR_DEBUG_ASSERT((*j).second == plainPtr) {
                return false;
            }
        }
    }
    for (TracksByCanonicalLocation::const_iterator i = m_tracksByCanonicalLocation.begin(); i != m_tracksByCanonicalLocation.end(); ++i) {
        const Track* plainPtr = (*i).second;
        VERIFY_OR_DEBUG_ASSERT(plainPtr) {
            return false;
        }
        const TrackRef trackRef = createTrackRef(*plainPtr);
        VERIFY_OR_DEBUG_ASSERT(!trackRef.getCanonicalLocation().isEmpty()) {
            return false;
        }
        VERIFY_OR_DEBUG_ASSERT(trackRef.getCanonicalLocation() == (*i).first) {
            return false;
        }
        VERIFY_OR_DEBUG_ASSERT(1 == m_tracksByCanonicalLocation.count(trackRef.getCanonicalLocation())) {
            return false;
        }
        TracksById::const_iterator j = m_tracksById.find(trackRef.getId());
        VERIFY_OR_DEBUG_ASSERT(
                (m_tracksById.end() == j) || ((*j).second == plainPtr)) {
            return false;
        }
    }
    return true;
}

void GlobalTrackCache::deactivate() {
    DEBUG_ASSERT(verifyConsistency());
    // Ideally the cache should be empty when destroyed.
    // But since this is difficult to achieve all remaining
    // cached tracks will evicted no matter if they are still
    // referenced or not. This ensures that the eviction
    // callback is triggered for all modified tracks before
    // exiting the application.
    while (!m_indexedTracks.empty()) {
        evictAndDelete(
                m_indexedTracks.begin(),
                true);
    }
    // Verify that all indexed tracks have been evicted
    DEBUG_ASSERT(m_tracksById.empty());
    DEBUG_ASSERT(m_tracksByCanonicalLocation.empty());
    // The singular cache instance is already unavailable and
    // all unindexed tracks will simply be deleted when their
    // shared pointer goes out of scope. Their modifications
    // will be lost.
    m_unindexedTracks.clear();
    m_pDeleter = nullptr;
}

void GlobalTrackCache::reset() {
    if (!m_indexedTracks.empty()) {
        kLogger.warning()
                << "Resetting indices while"
                << m_indexedTracks.size()
                << "tracks are still cached and indexed";
        for (auto i = m_indexedTracks.begin(); i != m_indexedTracks.end(); ++i) {
            Track* plainPtr = (*i).first;
            DEBUG_ASSERT(m_unindexedTracks.find(plainPtr) == m_unindexedTracks.end());
        }
        m_indexedTracks.clear();
    }
    m_tracksById.clear();
    m_tracksByCanonicalLocation.clear();
}

bool GlobalTrackCache::isEmpty() const {
    DEBUG_ASSERT(m_tracksById.size() <= m_indexedTracks.size());
    DEBUG_ASSERT(m_tracksByCanonicalLocation.size() <= m_indexedTracks.size());
    return m_indexedTracks.empty() && m_unindexedTracks.empty();
}

TrackPointer GlobalTrackCache::lookupById(
        const TrackId& trackId) {
    const auto trackById(m_tracksById.find(trackId));
    if (m_tracksById.end() != trackById) {
        // Cache hit
        Track* plainPtr = (*trackById).second;
        if (traceLogEnabled()) {
            kLogger.trace()
                    << "Cache hit for"
                    << trackId
                    << plainPtr;
        }
        return revive(plainPtr);
    } else {
        // Cache miss
        if (traceLogEnabled()) {
            kLogger.trace()
                    << "Cache miss for"
                    << trackId;
        }
        return TrackPointer();
    }
}

TrackPointer GlobalTrackCache::lookupByRef(
        const TrackRef& trackRef) {
    if (trackRef.hasId()) {
        return lookupById(trackRef.getId());
    } else {
        const auto canonicalLocation = trackRef.getCanonicalLocation();
        const auto trackByCanonicalLocation(
                m_tracksByCanonicalLocation.find(canonicalLocation));
        if (m_tracksByCanonicalLocation.end() != trackByCanonicalLocation) {
            // Cache hit
            Track* plainPtr = (*trackByCanonicalLocation).second;
            if (traceLogEnabled()) {
                kLogger.trace()
                        << "Cache hit for"
                        << canonicalLocation
                        << plainPtr;
            }
            return revive(plainPtr);
        } else {
            // Cache miss
            if (traceLogEnabled()) {
                kLogger.trace()
                        << "Cache miss for"
                        << canonicalLocation;
            }
            return TrackPointer();
        }
    }
}

TrackPointer GlobalTrackCache::revive(
        Track* plainPtr) {
    DEBUG_ASSERT(plainPtr);
    const auto i = m_indexedTracks.find(plainPtr);
    DEBUG_ASSERT(i != m_indexedTracks.end());
    TrackWeakPointer weakPtr = (*i).second;
    TrackPointer strongPtr = weakPtr.lock();
    if (strongPtr) {
        if (traceLogEnabled()) {
            kLogger.trace()
                    << "Found alive track"
                    << plainPtr;
        }
        return strongPtr;
    }
    // Race condition: The deleter for the Track object has not
    // been invoked, but the object is still referenced within
    // the cache. We need to revive it and later the deleter will
    // leave the unexpired object in the cache.
    if (debugLogEnabled()) {
        kLogger.debug()
                << "Reviving zombie track"
                << plainPtr;
    }
    DEBUG_ASSERT(weakPtr.expired());
    strongPtr = TrackPointer(plainPtr, deleter);
    weakPtr = strongPtr;
    DEBUG_ASSERT(!weakPtr.expired());
    (*i).second = weakPtr;
    const TrackRef trackRef = createTrackRef(*plainPtr);
    if (trackRef.hasId()) {
        auto i = m_tracksById.find(trackRef.getId());
        if (i != m_tracksById.end()) {
            (*i).second = plainPtr;
        } else {
            m_tracksById.insert(std::make_pair(
                    trackRef.getId(),
                    plainPtr));
        }
    }
    if (trackRef.hasCanonicalLocation()) {
        auto i = m_tracksByCanonicalLocation.find(
                trackRef.getCanonicalLocation());
        if (i != m_tracksByCanonicalLocation.end()) {
            (*i).second = plainPtr;
        } else {
            m_tracksByCanonicalLocation.insert(std::make_pair(
                    trackRef.getCanonicalLocation(),
                    plainPtr));
        }
    }
    return strongPtr;
}

void GlobalTrackCache::resolve(
        GlobalTrackCacheResolver* /*in/out*/ pCacheResolver,
        QFileInfo /*in*/ fileInfo,
        TrackId trackId,
        SecurityTokenPointer pSecurityToken) {
    DEBUG_ASSERT(pCacheResolver);
    // Primary lookup by id (if available)
    if (trackId.isValid()) {
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Resolving track by id"
                    << trackId;
        }
        auto strongPtr = lookupById(trackId);
        if (strongPtr) {
            if (debugLogEnabled()) {
                kLogger.debug()
                        << "Cache hit - found track by id"
                        << trackId
                        << strongPtr.get();
            }
            TrackRef trackRef = createTrackRef(*strongPtr);
            pCacheResolver->initLookupResult(
                    GlobalTrackCacheLookupResult::HIT,
                    std::move(strongPtr),
                    std::move(trackRef));
            return;
        }
    }
    // Secondary lookup by canonical location
    // The TrackRef is constructed now after the lookup by ID failed to
    // avoid calculating the canonical file path if it is not needed.
    TrackRef trackRef = TrackRef::fromFileInfo(fileInfo, trackId);
    if (trackRef.hasCanonicalLocation()) {
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Resolving track by canonical location"
                    << trackRef.getCanonicalLocation();
        }
        auto strongPtr = lookupByRef(trackRef);
        if (strongPtr) {
            // Cache hit
            if (debugLogEnabled()) {
                kLogger.debug()
                        << "Cache hit - found track by canonical location"
                        << trackRef.getCanonicalLocation()
                        << strongPtr.get();
            }
            pCacheResolver->initLookupResult(
                    GlobalTrackCacheLookupResult::HIT,
                    std::move(strongPtr),
                    std::move(trackRef));
            return;
        }
    }
    if (!m_pDeleter) {
        // Do not allocate any new tracks once the cache
        // has been deactivated
        DEBUG_ASSERT(isEmpty());
        kLogger.warning()
                << "Cache miss - caching has already been deactivated"
                << trackRef;
        return;
    }
    if (debugLogEnabled()) {
        kLogger.debug()
                << "Cache miss - allocating track"
                << trackRef;
    }
    auto plainPtr =
            new Track(
                    std::move(fileInfo),
                    std::move(pSecurityToken),
                    std::move(trackId));
    auto strongPtr = TrackPointer(plainPtr, deleter);
    // Track objects live together with the cache on the main thread
    // and will be deleted later within the event loop. But this
    // function might be called from any thread, even from worker
    // threads without an event loop. We need to move the newly
    // created object to the target thread.
    plainPtr->moveToThread(QApplication::instance()->thread());
    if (debugLogEnabled()) {
        kLogger.debug()
                << "Cache miss - inserting new track into cache"
                << trackRef
                << plainPtr;
    }
    DEBUG_ASSERT(m_indexedTracks.find(plainPtr) == m_indexedTracks.end());
    DEBUG_ASSERT(m_unindexedTracks.find(plainPtr) == m_unindexedTracks.end());
    TrackWeakPointer weakPtr(strongPtr);
    m_indexedTracks.insert(std::make_pair(
            plainPtr,
            weakPtr));
    if (trackRef.hasId()) {
        // Insert item by id
        DEBUG_ASSERT(m_tracksById.find(
                trackRef.getId()) == m_tracksById.end());
        m_tracksById.insert(std::make_pair(
                trackRef.getId(),
                plainPtr));
    }
    if (trackRef.hasCanonicalLocation()) {
        // Insert item by track location
        DEBUG_ASSERT(m_tracksByCanonicalLocation.find(
                trackRef.getCanonicalLocation()) == m_tracksByCanonicalLocation.end());
        m_tracksByCanonicalLocation.insert(std::make_pair(
                trackRef.getCanonicalLocation(),
                plainPtr));
    }
    pCacheResolver->initLookupResult(
            GlobalTrackCacheLookupResult::MISS,
            std::move(strongPtr),
            std::move(trackRef));
}

TrackRef GlobalTrackCache::initTrackId(
        const TrackPointer& strongPtr,
        TrackRef trackRef,
        TrackId trackId) {
    DEBUG_ASSERT(strongPtr);
    DEBUG_ASSERT(!trackRef.getId().isValid());
    DEBUG_ASSERT(trackId.isValid());
    DEBUG_ASSERT(m_tracksByCanonicalLocation.find(
            trackRef.getCanonicalLocation()) != m_tracksByCanonicalLocation.end());
    TrackRef trackRefWithId(trackRef, trackId);

    // Insert item by id
    DEBUG_ASSERT(m_tracksById.find(trackId) == m_tracksById.end());
    m_tracksById.insert(std::make_pair(
            trackId,
            strongPtr.get()));

    strongPtr->initId(trackId);
    DEBUG_ASSERT(createTrackRef(*strongPtr) == trackRefWithId);

    return trackRefWithId;
}

bool GlobalTrackCache::evictAndDelete(
        Track* plainPtr) {
    DEBUG_ASSERT(plainPtr);
    GlobalTrackCacheLocker cacheLocker;

    const IndexedTracks::iterator indexedTrack =
            m_indexedTracks.find(plainPtr);
    if (indexedTrack == m_indexedTracks.end()) {
        if (m_unindexedTracks.erase(plainPtr)) {
            // Unindexed tracks are directly deleted without
            // invoking the post-evict hook! This only happens
            // when resetting the indices while some tracks
            // are still cached and indexed.
            if (debugLogEnabled()) {
                kLogger.debug()
                        << "Deleting unindexed track"
                        << createTrackRef(*plainPtr)
                        << plainPtr;
            }
            deleteTrack(plainPtr);
            return true;
        } else {
            // Due to a rare but expected race condition the track
            // has already been deleted and must not be deleted
            // again!
            if (debugLogEnabled()) {
                kLogger.debug()
                        << "Skip deletion of already deleted track"
                        << plainPtr;
            }
            return false;
        }
    }

    // Now we know that the pointer has not been deleted before
    // and we can safely access it!
    return evictAndDelete(
            indexedTrack,
            false);
}

bool GlobalTrackCache::evictAndDelete(
        IndexedTracks::iterator indexedTrack,
        bool evictUnexpired) {
    Track* plainPtr = (*indexedTrack).first;
    DEBUG_ASSERT(plainPtr);
    DEBUG_ASSERT(m_unindexedTracks.find(plainPtr) == m_unindexedTracks.end());
    const auto trackRef = createTrackRef(*plainPtr);
    if (debugLogEnabled()) {
        kLogger.debug()
                << "Evicting indexed track"
                << trackRef
                << plainPtr;
    }

    const bool evicted = evict(trackRef, indexedTrack, evictUnexpired);
    DEBUG_ASSERT(verifyConsistency());
    if (evicted) {
        // The evicted entry must not be accessible anymore!
        DEBUG_ASSERT(m_indexedTracks.find(plainPtr) == m_indexedTracks.end());
        DEBUG_ASSERT(!lookupByRef(trackRef));
        m_pDeleter->deleteCachedTrack(plainPtr);
        return true;
    } else {
        // ...otherwise the given plainPtr is still referenced within
        // the cache and must not be deleted yet!
        DEBUG_ASSERT(m_indexedTracks.find(plainPtr) != m_indexedTracks.end());
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Keeping unevicted track"
                    << trackRef
                    << plainPtr;
        }
        return false;
    }
}

bool GlobalTrackCache::evict(
        const TrackRef& trackRef,
        IndexedTracks::iterator indexedTrack,
        bool evictUnexpired) {
    DEBUG_ASSERT(indexedTrack != m_indexedTracks.end());
    Track* plainPtr = (*indexedTrack).first;
    DEBUG_ASSERT(plainPtr);
    DEBUG_ASSERT(m_unindexedTracks.find(plainPtr) == m_unindexedTracks.end());
    TrackWeakPointer weakPtr = (*indexedTrack).second;
    if (!(evictUnexpired || weakPtr.expired())) {
        return false;
    }
    if (trackRef.hasId()) {
        const auto trackById = m_tracksById.find(trackRef.getId());
        if (trackById != m_tracksById.end()) {
            DEBUG_ASSERT((*trackById).second == plainPtr);
            m_tracksById.erase(trackById);
        }
    }
    if (trackRef.hasCanonicalLocation()) {
        const auto trackByCanonicalLocation(
                m_tracksByCanonicalLocation.find(trackRef.getCanonicalLocation()));
        if (m_tracksByCanonicalLocation.end() != trackByCanonicalLocation) {
            DEBUG_ASSERT((*trackByCanonicalLocation).second == plainPtr);
            m_tracksByCanonicalLocation.erase(
                    trackByCanonicalLocation);
        }
    }
    m_indexedTracks.erase(indexedTrack);
    return true;
}
