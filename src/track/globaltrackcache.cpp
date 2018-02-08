#include "track/globaltrackcache.h"

#include <QApplication>
#include <QThread>

#include "util/assert.h"
#include "util/logger.h"


namespace {

const mixxx::Logger kLogger("GlobalTrackCache");

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

void deleteTrack(Track* plainPtr) {
    if (plainPtr) {
        if (traceLogEnabled()) {
            plainPtr->dumpObjectInfo();
        }
        plainPtr->deleteLater();
    }
}

} // anonymous namespace

GlobalTrackCacheLocker::GlobalTrackCacheLocker()
        : m_pCacheMutex(nullptr) {
    lockCache();
}

GlobalTrackCacheLocker::GlobalTrackCacheLocker(
        GlobalTrackCacheLocker&& moveable)
        : m_pCacheMutex(std::move(moveable.m_pCacheMutex)) {
    moveable.m_pCacheMutex = nullptr;
}

GlobalTrackCacheLocker::~GlobalTrackCacheLocker() {
    unlockCache();
}

void GlobalTrackCacheLocker::lockCache() {
    DEBUG_ASSERT(!m_pCacheMutex);
    QMutex* pCacheMutex = &GlobalTrackCache::instance().m_mutex;
    if (traceLogEnabled()) {
        kLogger.trace() << "Locking cache";
    }
    pCacheMutex->lock();
    if (traceLogEnabled()) {
        kLogger.trace() << "Cache is locked";
    }
    m_pCacheMutex = pCacheMutex;
}

void GlobalTrackCacheLocker::unlockCache() {
    if (m_pCacheMutex) {
        // Verify consistency before unlocking the cache
        DEBUG_ASSERT(GlobalTrackCache::instance().verifyConsistency());
        if (traceLogEnabled()) {
            kLogger.trace() << "Unlocking cache";
        }
        if (kLogStats && debugLogEnabled()) {
            kLogger.debug()
                    << "#tracksById ="
                    << GlobalTrackCache::s_pInstance->m_tracksById.size()
                    << "/ #tracksByCanonicalLocation ="
                    << GlobalTrackCache::s_pInstance->m_tracksByCanonicalLocation.size();
        }
        m_pCacheMutex->unlock();
        if (traceLogEnabled()) {
            kLogger.trace() << "Cache is unlocked";
        }
        m_pCacheMutex = nullptr;
    }
}

GlobalTrackCacheResolver::GlobalTrackCacheResolver()
        : m_lookupResult(GlobalTrackCacheLookupResult::NONE) {
    // Verify consistency after the cache has been locked
    DEBUG_ASSERT(GlobalTrackCache::instance().verifyConsistency());
}

void GlobalTrackCacheResolver::initLookupResult(
        GlobalTrackCacheLookupResult lookupResult,
        TrackPointer&& strongPtr,
        TrackRef&& trackRef) {
    DEBUG_ASSERT(m_pCacheMutex); // cache is still locked
    DEBUG_ASSERT(GlobalTrackCacheLookupResult::NONE == m_lookupResult);
    DEBUG_ASSERT(!m_strongPtr);
    m_lookupResult = lookupResult;
    m_strongPtr = std::move(strongPtr);
    m_trackRef = std::move(trackRef);
}

void GlobalTrackCacheResolver::initTrackIdAndUnlockCache(TrackId trackId) {
    DEBUG_ASSERT(m_pCacheMutex); // cache is still locked
    DEBUG_ASSERT(GlobalTrackCacheLookupResult::NONE != m_lookupResult);
    DEBUG_ASSERT(m_strongPtr);
    DEBUG_ASSERT(trackId.isValid());
    if (m_trackRef.getId().isValid()) {
        // Ignore setting the same id twice
        DEBUG_ASSERT(m_trackRef.getId() == trackId);
    } else {
        m_trackRef = GlobalTrackCache::instance().initTrackIdInternal(
                m_strongPtr,
                m_trackRef,
                trackId);
    }
    unlockCache();
    DEBUG_ASSERT(m_trackRef == createTrackRef(*m_strongPtr));
}


//static
GlobalTrackCache* volatile GlobalTrackCache::s_pInstance = nullptr;

//static
void GlobalTrackCache::createInstance(GlobalTrackCacheEvictor* pEvictor) {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());
    DEBUG_ASSERT(!s_pInstance);
    s_pInstance = new GlobalTrackCache(pEvictor);
}

//static
void GlobalTrackCache::destroyInstance() {
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());
    DEBUG_ASSERT(s_pInstance);
    GlobalTrackCache* pInstance = s_pInstance;
    // Reset the static/global pointer before entering the destructor
    s_pInstance = nullptr;
    // Delete the singular instance
    delete pInstance;
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

GlobalTrackCache::GlobalTrackCache(GlobalTrackCacheEvictor* pEvictor)
    : m_pEvictor(pEvictor),
      m_mutex(QMutex::Recursive) {
    DEBUG_ASSERT(m_pEvictor);
    DEBUG_ASSERT(verifyConsistency());
    DEBUG_ASSERT(isActive());
}

GlobalTrackCache::~GlobalTrackCache() {
    deactivateInternal();
}

void GlobalTrackCache::deactivate() {
    GlobalTrackCacheLocker locker;
    deactivateInternal();
}

void GlobalTrackCache::deactivateInternal() {
    DEBUG_ASSERT(verifyConsistency());
    // Ideally the cache should be empty when destroyed.
    // But since this is difficult to achieve all remaining
    // cached tracks will evicted no matter if they are still
    // referenced or not. This ensures that the eviction
    // callback is triggered for all modified tracks before
    // exiting the application.
    while (!m_indexedTracks.empty()) {
        evictAndDeleteInternal(
                nullptr,
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
    m_pEvictor = nullptr;
    DEBUG_ASSERT(!isActive());
}

bool GlobalTrackCache::verifyConsistency() const {
    VERIFY_OR_DEBUG_ASSERT(m_indexedTracks.size() >= m_tracksById.size()) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_indexedTracks.size() >= m_tracksByCanonicalLocation.size()) {
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

TrackPointer GlobalTrackCache::lookupById(
        const TrackId& trackId) {
    if (trackId.isValid()) {
        GlobalTrackCacheLocker cacheLocker;
        return lookupByIdInternal(trackId);
    } else {
        return TrackPointer();
    }
}

TrackPointer GlobalTrackCache::lookupByIdInternal(
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
        return reviveInternal(plainPtr);
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

TrackPointer GlobalTrackCache::lookupByRefInternal(
        const TrackRef& trackRef) {
    if (trackRef.hasId()) {
        return lookupByIdInternal(trackRef.getId());
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
            return reviveInternal(plainPtr);
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

TrackPointer GlobalTrackCache::reviveInternal(
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

bool GlobalTrackCache::resolveInternal(
        GlobalTrackCacheResolver* /*in/out*/ pCacheResolver,
        TrackRef* /*out, optional*/ pTrackRef,
        const TrackId& /*in*/ trackId,
        const QFileInfo& /*in*/ fileInfo) {
    DEBUG_ASSERT(pCacheResolver);
    // Primary lookup by id (if available)
    if (trackId.isValid()) {
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Resolving track by id"
                    << trackId;
        }
        auto strongPtr = lookupByIdInternal(trackId);
        if (strongPtr) {
            if (debugLogEnabled()) {
                kLogger.debug()
                        << "Cache hit - found track by id"
                        << trackId
                        << strongPtr.get();
            }
            TrackRef trackRef = createTrackRef(*strongPtr);
            if (pTrackRef) {
                *pTrackRef = createTrackRef(*strongPtr);
            }
            pCacheResolver->initLookupResult(
                    GlobalTrackCacheLookupResult::HIT,
                    std::move(strongPtr),
                    std::move(trackRef));
            return true;
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
        auto strongPtr = lookupByRefInternal(trackRef);
        if (strongPtr) {
            // Cache hit
            if (debugLogEnabled()) {
                kLogger.debug()
                        << "Cache hit - found track by canonical location"
                        << trackRef.getCanonicalLocation()
                        << strongPtr.get();
            }
            if (pTrackRef) {
                *pTrackRef = trackRef;
            }
            pCacheResolver->initLookupResult(
                    GlobalTrackCacheLookupResult::HIT,
                    std::move(strongPtr),
                    std::move(trackRef));
            return true;
        }
    }
    if (debugLogEnabled()) {
        kLogger.debug()
                << "Cache miss - unresolved track"
                << trackRef;
    }
    if (pTrackRef) {
        *pTrackRef = std::move(trackRef);
    }
    return false;
}

GlobalTrackCacheResolver GlobalTrackCache::resolve(
        const TrackId& trackId,
        const QFileInfo& fileInfo,
        const SecurityTokenPointer& pSecurityToken) {
    GlobalTrackCacheResolver cacheResolver;
    TrackRef trackRef;
    if (resolveInternal(&cacheResolver, &trackRef, trackId, fileInfo)) {
        DEBUG_ASSERT(cacheResolver.getLookupResult() == GlobalTrackCacheLookupResult::HIT);
        return cacheResolver;
    }
    if (!isActive()) {
        // Do not allocate any new tracks once the cache
        // has been deactivated
        DEBUG_ASSERT(isEmptyInternal());
        kLogger.warning()
                << "Cache miss - caching has already been deactivated"
                << trackRef;
        return cacheResolver;
    }
    auto plainPtr =
            new Track(
                    std::move(fileInfo),
                    std::move(pSecurityToken),
                    std::move(trackId));
    auto strongPtr = TrackPointer(plainPtr, deleter);
    trackRef = createTrackRef(*plainPtr);
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
    cacheResolver.initLookupResult(
            GlobalTrackCacheLookupResult::MISS,
            std::move(strongPtr),
            std::move(trackRef));
    return cacheResolver;
}

TrackRef GlobalTrackCache::initTrackIdInternal(
        const TrackPointer& strongPtr,
        TrackRef trackRef,
        TrackId trackId) {
    DEBUG_ASSERT(strongPtr);
    DEBUG_ASSERT(!trackRef.getId().isValid());
    DEBUG_ASSERT(trackId.isValid());
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

void GlobalTrackCache::afterEvicted(
        GlobalTrackCacheLocker* pCacheLocker,
        Track* pEvictedTrack) {
    DEBUG_ASSERT(pEvictedTrack);

    // It can produce dangerous signal loops if the track is still
    // sending signals while being saved! All references to this
    // track have been dropped at this point, so there is no need
    // to send any signals.
    // See: https://bugs.launchpad.net/mixxx/+bug/136578
    // NOTE(uklotzde, 2018-02-03): Simply disconnecting all receivers
    // doesn't seem to work reliably. Emitting the clean() signal from
    // a track that is about to deleted may cause access violations!!
    pEvictedTrack->blockSignals(true);

    // Keep the cache locked while evicting the track object!
    // The callback is given the chance to unlock the cache
    // after all operations that rely on managed track ownership
    // have been done, e.g. exporting track metadata into a file.
    m_pEvictor->afterEvictedTrackFromCache(
            pCacheLocker,
            pEvictedTrack);

    // At this point the cache might have been unlocked by the
    // callback!!
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
    return evictAndDeleteInternal(
            &cacheLocker,
            indexedTrack,
            false);
}

bool GlobalTrackCache::evictAndDeleteInternal(
        GlobalTrackCacheLocker* pCacheLocker,
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

    const bool evicted = evictInternal(trackRef, indexedTrack, evictUnexpired);
    DEBUG_ASSERT(verifyConsistency());
    if (evicted) {
        // The evicted entry must not be accessible anymore!
        DEBUG_ASSERT(m_indexedTracks.find(plainPtr) == m_indexedTracks.end());
        DEBUG_ASSERT(!lookupByRefInternal(trackRef));
        afterEvicted(pCacheLocker, plainPtr);
        // After returning from the callback the lock might have
        // already been released!
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Deleting evicted track"
                    << trackRef
                    << plainPtr;
        }
        deleteTrack(plainPtr);
        return true;
    } else {
        // ...otherwise the given plainPtr is still referenced within
        // the cache and must not be deleted, yet!
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

bool GlobalTrackCache::evictInternal(
        const TrackRef& trackRef,
        IndexedTracks::iterator indexedTrack,
        bool evictUnexpired) {
    DEBUG_ASSERT(indexedTrack != m_indexedTracks.end());
    Track* plainPtr = (*indexedTrack).first;
    DEBUG_ASSERT(plainPtr);
    DEBUG_ASSERT(m_unindexedTracks.find(plainPtr) == m_unindexedTracks.end());
    TrackWeakPointer weakPtr = (*indexedTrack).second;
    if (trackRef.hasId()) {
        const auto trackById = m_tracksById.find(trackRef.getId());
        if (trackById != m_tracksById.end()) {
            DEBUG_ASSERT((*trackById).second == plainPtr);
            if (evictUnexpired || weakPtr.expired()) {
                // Evict expired track
                m_tracksById.erase(trackById);
            } else {
                // Keep unexpired track
                return false;
            }
        }
    }
    const auto trackByCanonicalLocation(
            m_tracksByCanonicalLocation.find(trackRef.getCanonicalLocation()));
    if (m_tracksByCanonicalLocation.end() != trackByCanonicalLocation) {
        if (evictUnexpired || weakPtr.expired()) {
            // Evict expired track
            m_tracksByCanonicalLocation.erase(
                    trackByCanonicalLocation);
        } else {
            // Keep unexpired track
            return false;
        }
    }
    m_indexedTracks.erase(indexedTrack);
    return true;
}

bool GlobalTrackCache::isEmpty() const {
    GlobalTrackCacheLocker cacheLocker;
    return isEmptyInternal();
}

bool GlobalTrackCache::isEmptyInternal() const {
    return m_indexedTracks.empty() && m_unindexedTracks.empty();
}

void GlobalTrackCache::resetIndices() {
    GlobalTrackCacheLocker cacheLocker;
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
