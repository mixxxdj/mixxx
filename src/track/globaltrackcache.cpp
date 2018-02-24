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
                    << "/ #allocatedTracks ="
                    << m_pInstance->m_allocatedTracks.size();
        }
        m_pInstance->m_mutex.unlock();
        if (traceLogEnabled()) {
            kLogger.trace() << "Cache is unlocked";
        }
        m_pInstance = nullptr;
    }
}

void GlobalTrackCacheLocker::relocateCachedTracks(
        GlobalTrackCacheRelocator* pRelocator) const {
    DEBUG_ASSERT(m_pInstance);
    m_pInstance->relocateTracks(pRelocator);
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
void GlobalTrackCache::createInstance(GlobalTrackCacheSaver* pDeleter) {
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

//static
void GlobalTrackCache::deleter(Track* plainPtr) {
    DEBUG_ASSERT(plainPtr);
    // Any access to plainPtr before a validity check inside the
    // GlobalTrackCacheLocker scope is forbidden!! Due to race
    // conditions before locking the cache this pointer might
    // already have been either deleted or reused by a second
    // shared_ptr.
    if (s_pInstance) {
        s_pInstance->evictOrDelete(plainPtr);
    } else {
        // After the singular instance has been destroyed we are
        // no longer able to decide if the given pointer is still
        // valid or has already been deleted before!! This should
        // only happen during an unordered shutdown with pending
        // references. Those references should have been released
        // before destroying the cache, otherwise any modifications
        // and edits will also be lost.
        kLogger.critical()
                << "Skipping deletion of"
                << plainPtr
                << "after singleton has already been destroyed!";
    }
}

GlobalTrackCache::GlobalTrackCache(GlobalTrackCacheSaver* pSaver)
    : m_mutex(QMutex::Recursive),
      m_pSaver(pSaver),
      m_allocatedTracks(kUnorderedCollectionMinCapacity),
      m_tracksById(kUnorderedCollectionMinCapacity, DbId::hash_fun) {
    DEBUG_ASSERT(m_pSaver);
    DEBUG_ASSERT(verifyConsistency());
}

GlobalTrackCache::~GlobalTrackCache() {
    deactivate();
}

bool GlobalTrackCache::verifyConsistency() const {
    // All cached tracks by ID need to be also listed in m_allocatedTracks
    for (TracksById::const_iterator i = m_tracksById.begin(); i != m_tracksById.end(); ++i) {
        Track* plainPtr = (*i).second;
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

        const auto j = m_allocatedTracks.find(plainPtr);
        VERIFY_OR_DEBUG_ASSERT(j != m_allocatedTracks.end()) {
            return false;
        }
    }
    for (TracksByCanonicalLocation::const_iterator i = m_tracksByCanonicalLocation.begin();
            i != m_tracksByCanonicalLocation.end(); ++i) {
        Track* plainPtr = (*i).second;
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
        const auto k = m_allocatedTracks.find(plainPtr);
        VERIFY_OR_DEBUG_ASSERT(k != m_allocatedTracks.end()) {
            return false;
        }
    }
    return true;
}

void GlobalTrackCache::relocateTracks(
        GlobalTrackCacheRelocator* pRelocator) {
    if (debugLogEnabled()) {
        kLogger.debug()
                << "Relocating tracks";
    }
    TracksByCanonicalLocation relocatedTracksByCanonicalLocation;
    for (auto&&
            i = m_tracksByCanonicalLocation.begin();
            i != m_tracksByCanonicalLocation.end();
            ++i) {
        const QString oldCanonicalLocation = (*i).first;
        Track* plainPtr = (*i).second;
        QFileInfo fileInfo = plainPtr->getFileInfo();
        // The file info has to be refreshed, otherwise it might return
        // a cached and outdated absolute and canonical location!
        fileInfo.refresh();
        TrackRef trackRef = TrackRef::fromFileInfo(
                fileInfo,
                plainPtr->getId());
        if (!trackRef.hasCanonicalLocation() && trackRef.hasId() && pRelocator) {
            fileInfo = pRelocator->relocateCachedTrack(
                        trackRef.getId(),
                        fileInfo);
            if (fileInfo != plainPtr->getFileInfo()) {
                plainPtr->relocate(fileInfo);
            }
            trackRef = TrackRef::fromFileInfo(
                    fileInfo,
                    trackRef.getId());
        }
        if (!trackRef.hasCanonicalLocation()) {
            kLogger.warning()
                    << "Failed to relocate track"
                    << oldCanonicalLocation
                    << trackRef;
            continue;
        }
        QString newCanonicalLocation = trackRef.getCanonicalLocation();
        if (oldCanonicalLocation == newCanonicalLocation) {
            // Copy the entry unmodified into the new map
            relocatedTracksByCanonicalLocation.insert(*i);
            continue;
        }
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Relocating track"
                    << "from" << oldCanonicalLocation
                    << "to" << newCanonicalLocation;
        }
        relocatedTracksByCanonicalLocation.insert(std::make_pair(
                std::move(newCanonicalLocation),
                plainPtr));
    }
    m_tracksByCanonicalLocation = std::move(relocatedTracksByCanonicalLocation);
}

void GlobalTrackCache::deactivate() {
    DEBUG_ASSERT(verifyConsistency());
    // Ideally the cache should be empty when destroyed.
    // But since this is difficult to achieve all remaining
    // cached tracks will evicted no matter if they are still
    // referenced or not. This ensures that the eviction
    // callback is triggered for all modified tracks before
    // exiting the application.
    for (const auto& i: m_allocatedTracks) {
        auto strongPtr = i.second.lock();
        if (strongPtr) {
            evictAndSave(strongPtr);
        }
    }
    // Verify that all allocated tracks have been evicted
    DEBUG_ASSERT(m_tracksById.empty());
    DEBUG_ASSERT(m_tracksByCanonicalLocation.empty());
    // The singular cache instance is already unavailable and
    // all allocated tracks will simply be deleted when their
    // shared pointer goes out of scope. Unsaved modifications
    // will be lost.
    m_pSaver = nullptr;
}

bool GlobalTrackCache::isEmpty() const {
    DEBUG_ASSERT(m_tracksById.size() <= m_allocatedTracks.size());
    DEBUG_ASSERT(m_tracksByCanonicalLocation.size() <= m_allocatedTracks.size());
    return m_allocatedTracks.empty();
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
    const auto i = m_allocatedTracks.find(plainPtr);
    DEBUG_ASSERT(i != m_allocatedTracks.end());
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
    // We are here if the an other thread is preempted during
    // the destructor of the last shared_ptr referencing this
    // track, after the reference counter drops to zero and
    // before locking the cache. We need to revive it to abort
    // the deleter in the other thread.
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
    if (!m_pSaver) {
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
    DEBUG_ASSERT(m_allocatedTracks.find(plainPtr) == m_allocatedTracks.end());
    TrackWeakPointer weakPtr(strongPtr);
    m_allocatedTracks.insert(std::make_pair(
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

void GlobalTrackCache::evictOrDelete(
        Track* plainPtr) {
    DEBUG_ASSERT(plainPtr);
    GlobalTrackCacheLocker cacheLocker;

    const AllocatedTracks::iterator allocatedTrack =
            m_allocatedTracks.find(plainPtr);
    if (allocatedTrack == m_allocatedTracks.end()) {
        // We have handed out and already delete this track while waiting at
        // the lock at the beginning of this function.
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Skip deletion of already deleted track"
                    << plainPtr;
        }
        return;
    }

    if (!allocatedTrack->second.expired()) {
        // We have handed out this track again while waiting at
        // the lock at the beginning of this function.
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Skip deletion of revived track"
                    << plainPtr;
        }
        return;
    }

    TrackPointer strongPtr;

    strongPtr = lookupById(plainPtr->getId());
    if (!strongPtr) {
        const auto trackRef = createTrackRef(*plainPtr);
        strongPtr = lookupByRef(trackRef);
    }

    if (strongPtr) {
        evictAndSave(strongPtr);
    } else {
        // Track has already been evicted and can be deallocated now.
        m_allocatedTracks.erase(allocatedTrack);
        // We safely delete the object via the Qt event queue instead
        // of using operator delete! Otherwise the deleted track object
        // might be accessed when processing cross-thread signals that
        // are delayed within a queued connection and may arrive after
        // the object has already been deleted.
        if (traceLogEnabled()) {
            plainPtr->dumpObjectInfo();
        }
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Deleting"
                    << plainPtr;
        }
        plainPtr->deleteLater();
    }
}

bool GlobalTrackCache::evictAndSave(TrackPointer strongPtr) {
    DEBUG_ASSERT(strongPtr);
    evict(strongPtr);
    m_pSaver->saveCachedTrack(strongPtr);
    return true;
}

void GlobalTrackCache::evict(TrackPointer strongPtr) {
    // Make the cached track object invisible to avoid reusing
    // it before starting to save it. This is achieved by
    // removing it from both cache indices.
    const auto trackRef = createTrackRef(*strongPtr);
    if (debugLogEnabled()) {
        kLogger.debug()
                << "Evicting track"
                << trackRef
                << strongPtr.get();
    }
    if (trackRef.hasId()) {
        const auto trackById = m_tracksById.find(trackRef.getId());
        if (trackById != m_tracksById.end()) {
            DEBUG_ASSERT(trackById->second == strongPtr.get());
            m_tracksById.erase(trackById);
        }
    }
    if (trackRef.hasCanonicalLocation()) {
        const auto trackByCanonicalLocation(
                m_tracksByCanonicalLocation.find(trackRef.getCanonicalLocation()));
        if (m_tracksByCanonicalLocation.end() != trackByCanonicalLocation) {
            DEBUG_ASSERT(trackByCanonicalLocation->second == strongPtr.get());
            m_tracksByCanonicalLocation.erase(
                    trackByCanonicalLocation);
        }
    }
    DEBUG_ASSERT(verifyConsistency());
}
