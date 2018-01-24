#include "track/globaltrackcache.h"

#include "util/assert.h"
#include "util/logger.h"


namespace {

const mixxx::Logger kLogger("GlobalTrackCache");

// Enforce logging during tests
constexpr bool kLogEnabled = false;

inline bool debugLogEnabled() {
    return kLogEnabled || kLogger.debugEnabled();
}

constexpr bool kLogStats = true;

inline
TrackRef createTrackRef(const Track& track) {
    return TrackRef::fromFileInfo(track.getFileInfo(), track.getId());
}

} // anonymous namespace

GlobalTrackCacheLocker::GlobalTrackCacheLocker()
        : m_pCacheMutex(nullptr),
          m_lookupResult(GlobalTrackCacheLookupResult::NONE) {
    lockCache();
    // Verify consistency after the cache has been locked
    DEBUG_ASSERT(GlobalTrackCache::instance().verifyConsistency());
}

GlobalTrackCacheLocker::GlobalTrackCacheLocker(
        GlobalTrackCacheLocker&& moveable)
        : m_pCacheMutex(std::move(moveable.m_pCacheMutex)),
          m_lookupResult(std::move(moveable.m_lookupResult)),
          m_trackRef(std::move(moveable.m_trackRef)),
          m_pTrack(std::move(moveable.m_pTrack)) {
    moveable.m_pCacheMutex = nullptr;
}

GlobalTrackCacheLocker& GlobalTrackCacheLocker::operator=(
        GlobalTrackCacheLocker&& moveable) {
    if (this != &moveable) {
        m_pCacheMutex = std::move(moveable.m_pCacheMutex);
        moveable.m_pCacheMutex = nullptr;
        m_lookupResult = std::move(moveable.m_lookupResult);
        m_trackRef = std::move(moveable.m_trackRef);
        m_pTrack = std::move(moveable.m_pTrack);
    }
    return *this;
}

GlobalTrackCacheLocker::GlobalTrackCacheLocker(
        GlobalTrackCacheLocker&& moveable,
        GlobalTrackCacheLookupResult lookupResult,
        TrackRef trackRef,
        TrackPointer pTrack)
        : m_pCacheMutex(moveable.m_pCacheMutex),
          m_lookupResult(lookupResult),
          m_trackRef(std::move(trackRef)),
          m_pTrack(std::move(pTrack)) {
    moveable.m_pCacheMutex = nullptr;
    // Class invariants
    DEBUG_ASSERT((GlobalTrackCacheLookupResult::NONE != m_lookupResult) || !m_pTrack);
}

GlobalTrackCacheLocker::~GlobalTrackCacheLocker() {
    unlockCache();
}

void GlobalTrackCacheLocker::lockCache() {
    DEBUG_ASSERT(!m_pCacheMutex);
    QMutex* pCacheMutex = &GlobalTrackCache::instance().m_mutex;
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "Locking cache";
    }
    pCacheMutex->lock();
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "Cache is locked";
    }
    m_pCacheMutex = pCacheMutex;
}

void GlobalTrackCacheLocker::unlockCache() {
    if (m_pCacheMutex) {
        // Verify consistency before unlocking the cache
        DEBUG_ASSERT(GlobalTrackCache::instance().verifyConsistency());
        if (kLogger.traceEnabled()) {
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
        if (kLogger.traceEnabled()) {
            kLogger.trace() << "Cache is unlocked";
        }
        m_pCacheMutex = nullptr;
    }
}

GlobalTrackCacheResolver::GlobalTrackCacheResolver() {
}

GlobalTrackCacheResolver::GlobalTrackCacheResolver(
        GlobalTrackCacheResolver&& moveable,
        GlobalTrackCacheLookupResult lookupResult,
        TrackRef trackRef,
        TrackPointer pTrack)
        : GlobalTrackCacheLocker(
            std::move(moveable),
            std::move(lookupResult),
            std::move(trackRef),
            std::move(pTrack)) {
}

void GlobalTrackCacheResolver::initTrackId(TrackId trackId) {
    DEBUG_ASSERT(m_pCacheMutex); // cache is still locked
    DEBUG_ASSERT(GlobalTrackCacheLookupResult::NONE != m_lookupResult);
    DEBUG_ASSERT(m_pTrack);
    DEBUG_ASSERT(trackId.isValid());
    if (m_trackRef.getId().isValid()) {
        // Ignore setting the same id twice
        DEBUG_ASSERT(m_trackRef.getId() == trackId);
    } else {
        m_trackRef = GlobalTrackCache::instance().initTrackIdInternal(
                m_pTrack,
                m_trackRef,
                trackId);
        DEBUG_ASSERT(m_trackRef.getId() == trackId);
        m_pTrack->initId(trackId);
    }
    DEBUG_ASSERT(createTrackRef(*m_pTrack) == m_trackRef);
}


//static
GlobalTrackCache* volatile GlobalTrackCache::s_pInstance = nullptr;

//static
void GlobalTrackCache::createInstance(GlobalTrackCacheEvictor* pEvictor) {
    DEBUG_ASSERT(!s_pInstance);
    s_pInstance = new GlobalTrackCache(pEvictor);
}

//static
void GlobalTrackCache::destroyInstance() {
    DEBUG_ASSERT(s_pInstance);
    GlobalTrackCache* pInstance = s_pInstance;
    s_pInstance = nullptr;
    delete pInstance;
}

//static
void GlobalTrackCache::deleter(Track* pTrack) {
    DEBUG_ASSERT(pTrack);
    // Any access to pTrack is forbidden!! Due to race condition
    // this pointer might already have been deleted! Only the
    // cache knows this.
    if (s_pInstance) {
        s_pInstance->evict(pTrack);
    } else {
        // Simply delete unreferenced tracks when the cache is
        // no longer available
        kLogger.warning()
                << "Deleting uncached track";
        delete pTrack;
    }
}

GlobalTrackCache::GlobalTrackCache(GlobalTrackCacheEvictor* pEvictor)
    : m_pEvictor(pEvictor),
      m_mutex(QMutex::Recursive) {
    DEBUG_ASSERT(m_pEvictor);
    DEBUG_ASSERT(verifyConsistency());
}

GlobalTrackCache::~GlobalTrackCache() {
    DEBUG_ASSERT(!s_pInstance);
    DEBUG_ASSERT(verifyConsistency());
    // Ideally the cache should be empty when destroyed.
    // But since this is difficult to achieve all remaining
    // cached tracks will evicted no matter if they are still
    // refereced or not. This ensures that the eviction
    // callback is triggered for all modified tracks before
    // exiting the application.
    for (auto pTrack: m_indexedTracks) {
        evictInternal(
                createTrackRef(*pTrack),
                pTrack,
                true);
    }
    m_unindexedTracks.clear();
    // Verify that the cache is empty upon destruction
    DEBUG_ASSERT(m_indexedTracks.empty());
    DEBUG_ASSERT(m_tracksById.empty());
    DEBUG_ASSERT(m_tracksByCanonicalLocation.empty());
}

bool GlobalTrackCache::verifyConsistency() const {
    VERIFY_OR_DEBUG_ASSERT(m_indexedTracks.size() >=
            AllocatedTracks::size_type(m_tracksById.keys().size())) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_indexedTracks.size() >=
            AllocatedTracks::size_type(m_tracksByCanonicalLocation.keys().size())) {
        return false;
    }
    for (AllocatedTracks::const_iterator i = m_indexedTracks.begin(); i != m_indexedTracks.end(); ++i) {
        Track* pTrack = *i;
        VERIFY_OR_DEBUG_ASSERT(*i) {
            return false;
        }
        TrackRef trackRef = createTrackRef(*pTrack);
        if (trackRef.hasId()) {
            TracksById::const_iterator trackById =
                    m_tracksById.find(trackRef.getId());
            VERIFY_OR_DEBUG_ASSERT(trackById != m_tracksById.end()) {
                return false;
            }
            VERIFY_OR_DEBUG_ASSERT(trackById.value().ref == trackRef) {
                return false;
            }
            VERIFY_OR_DEBUG_ASSERT(trackById.value().plainPtr == pTrack) {
                return false;
            }
        }
        if (trackRef.hasCanonicalLocation()) {
            TracksByCanonicalLocation::const_iterator trackByCanonicalLocation =
                    m_tracksByCanonicalLocation.find(trackRef.getCanonicalLocation());
            VERIFY_OR_DEBUG_ASSERT(trackByCanonicalLocation != m_tracksByCanonicalLocation.end()) {
                return false;
            }
            VERIFY_OR_DEBUG_ASSERT(trackByCanonicalLocation.value().ref == trackRef) {
                return false;
            }
            VERIFY_OR_DEBUG_ASSERT(trackByCanonicalLocation.value().plainPtr == pTrack) {
                return false;
            }
        }
    }
    VERIFY_OR_DEBUG_ASSERT(m_tracksById.keys().size() == m_tracksById.uniqueKeys().size()) {
        return false;
    }
    for (TracksById::const_iterator i(m_tracksById.begin()); i != m_tracksById.end(); ++i) {
        const TrackRef trackRef((*i).ref);
        const TrackId trackId(trackRef.getId());
        VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
            return false;
        }
        VERIFY_OR_DEBUG_ASSERT(i.key() == trackId) {
            return false;
        }
        VERIFY_OR_DEBUG_ASSERT(1 == m_tracksById.count(trackId)) {
            return false;
        }
        const QString canonicalLocation(trackRef.getCanonicalLocation());
        if (!canonicalLocation.isEmpty()) {
            VERIFY_OR_DEBUG_ASSERT(
                    1 == m_tracksByCanonicalLocation.count(canonicalLocation)) {
                return false;
            }
            TracksByCanonicalLocation::const_iterator j(
                    m_tracksByCanonicalLocation.find(canonicalLocation));
            VERIFY_OR_DEBUG_ASSERT(m_tracksByCanonicalLocation.end() != j) {
                return false;
            }
            VERIFY_OR_DEBUG_ASSERT((*j).ref == trackRef) {
                return false;
            }
        }
    }
    for (TracksByCanonicalLocation::const_iterator i(m_tracksByCanonicalLocation.begin()); i != m_tracksByCanonicalLocation.end(); ++i) {
        const TrackRef trackRef((*i).ref);
        const TrackId trackId(trackRef.getId());
        const QString canonicalLocation(trackRef.getCanonicalLocation());
        VERIFY_OR_DEBUG_ASSERT(!canonicalLocation.isEmpty()) {
            return false;
        }
        VERIFY_OR_DEBUG_ASSERT(i.key() == canonicalLocation) {
            return false;
        }
        VERIFY_OR_DEBUG_ASSERT(1 == m_tracksByCanonicalLocation.count(canonicalLocation)) {
            return false;
        }
        TracksById::const_iterator j(
                m_tracksById.find(trackId));
        VERIFY_OR_DEBUG_ASSERT(
                (m_tracksById.end() == j) || ((*j).ref == trackRef)) {
            return false;
        }
    }
    return true;
}

GlobalTrackCacheLocker GlobalTrackCache::lookupById(
        const TrackId& trackId) {
    GlobalTrackCacheLocker cacheLocker;
    if (trackId.isValid()) {
        auto trackRefPointer = lookupInternal(trackId);
        if (trackRefPointer.second) {
            cacheLocker.m_lookupResult = GlobalTrackCacheLookupResult::HIT;
            cacheLocker.m_trackRef = std::move(trackRefPointer.first);
            cacheLocker.m_pTrack = std::move(trackRefPointer.second);
        } else {
            cacheLocker.m_lookupResult = GlobalTrackCacheLookupResult::MISS;
        }
    }
    return std::move(cacheLocker);
}

std::pair<TrackRef, TrackPointer> GlobalTrackCache::lookupInternal(
        const TrackId& trackId) {
    const auto trackById(m_tracksById.find(trackId));
    if (m_tracksById.end() != trackById) {
        // Cache hit
        if (kLogger.traceEnabled()) {
            kLogger.trace()
                    << "Cache hit for"
                    << trackId;
        }
        return reviveInternal(*trackById);
    } else {
        // Cache miss
        if (kLogger.traceEnabled()) {
            kLogger.trace()
                    << "Cache miss for"
                    << trackId;
        }
        return std::make_pair(TrackRef(), TrackPointer());
    }
}

std::pair<TrackRef, TrackPointer> GlobalTrackCache::lookupInternal(
        const TrackRef& trackRef) {
    if (trackRef.hasId()) {
        return lookupInternal(trackRef.getId());
    } else {
        const auto canonicalLocation = trackRef.getCanonicalLocation();
        const auto trackByCanonicalLocation(
                m_tracksByCanonicalLocation.find(canonicalLocation));
        if (m_tracksByCanonicalLocation.end() != trackByCanonicalLocation) {
            // Cache hit
            if (kLogger.traceEnabled()) {
                kLogger.trace()
                        << "Cache hit for"
                        << canonicalLocation;
            }
            return reviveInternal(*trackByCanonicalLocation);
        } else {
            // Cache miss
            if (kLogger.traceEnabled()) {
                kLogger.trace()
                        << "Cache miss for"
                        << canonicalLocation;
            }
            return std::make_pair(TrackRef(), TrackPointer());
        }
    }
}

std::pair<TrackRef, TrackPointer> GlobalTrackCache::reviveInternal(
        const Item& item) {
    DEBUG_ASSERT(m_indexedTracks.find(item.plainPtr) != m_indexedTracks.end());
    TrackPointer pTrack(item.weakPtr);
    if (pTrack) {
        DEBUG_ASSERT(pTrack.get() == item.plainPtr);
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Found alive track"
                    << item.ref
                    << item.plainPtr;
        }
        return std::make_pair(item.ref, pTrack);
    }
    // Race condition: The deleter for the Track object has not
    // been invoked, but the object is still referenced within
    // the cache. We need to revive it and later the deleter will
    // leave the unexpired object in the cache.
    if (debugLogEnabled()) {
        kLogger.debug()
                << "Reviving zombie track"
                << item.ref
                << item.plainPtr;
    }
    DEBUG_ASSERT(item.weakPtr.expired());
    DEBUG_ASSERT(item.plainPtr);
    pTrack = TrackPointer(item.plainPtr, deleter);
    Item revivedItem(item.ref, pTrack);
    DEBUG_ASSERT(revivedItem.ref == item.ref);
    DEBUG_ASSERT(revivedItem.plainPtr == pTrack.get());
    DEBUG_ASSERT(!revivedItem.weakPtr.expired());
    if (revivedItem.ref.hasId()) {
        auto i = m_tracksById.find(
                revivedItem.ref.getId());
        if (i != m_tracksById.end()) {
            i.value().weakPtr = revivedItem.weakPtr;
            // Only the smart pointer should have changed
            DEBUG_ASSERT(i.value() == revivedItem);
        } else {
            m_tracksById.insert(
                    revivedItem.ref.getId(),
                    revivedItem);
        }
    }
    if (revivedItem.ref.hasCanonicalLocation()) {
        auto i = m_tracksByCanonicalLocation.find(
                revivedItem.ref.getCanonicalLocation());
        if (i != m_tracksByCanonicalLocation.end()) {
            i.value().weakPtr = revivedItem.weakPtr;
            // Only the smart pointer should have changed
            DEBUG_ASSERT(i.value() == revivedItem);
        } else {
            m_tracksByCanonicalLocation.insert(
                    revivedItem.ref.getCanonicalLocation(),
                    revivedItem);
        }
    }
    return std::make_pair(revivedItem.ref, pTrack);
}

bool GlobalTrackCache::resolveInternal(
        GlobalTrackCacheResolver* /*in/out*/ pCacheResolver,
        TrackRef* /*out*/ pTrackRef,
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
        auto trackRefPointer = lookupInternal(trackId);
        if (trackRefPointer.second) {
            if (debugLogEnabled()) {
                kLogger.debug()
                        << "Cache hit - found track by id"
                        << trackRefPointer.first;
            }
            *pCacheResolver = GlobalTrackCacheResolver(
                    std::move(*pCacheResolver),
                    GlobalTrackCacheLookupResult::HIT,
                    std::move(trackRefPointer.first),
                    std::move(trackRefPointer.second));
            return true;
        }
    }
    // Secondary lookup by canonical location
    // The TrackRef is constructed now after the lookup by ID failed to
    // avoid calculating the canonical file path if it is not needed.
    TrackRef trackRef(TrackRef::fromFileInfo(fileInfo, trackId));
    if (trackRef.hasCanonicalLocation()) {
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Resolving track by canonical location"
                    << trackRef.getCanonicalLocation();
        }
        auto trackRefPointer = lookupInternal(trackRef);
        if (trackRefPointer.second) {
            // Cache hit
            if (debugLogEnabled()) {
                kLogger.debug()
                        << "Cache hit - found track by canonical location"
                        << trackRefPointer.first;
            }
            *pCacheResolver = GlobalTrackCacheResolver(
                    std::move(*pCacheResolver),
                    GlobalTrackCacheLookupResult::HIT,
                    std::move(trackRefPointer.first),
                    std::move(trackRefPointer.second));
            return true;
        }
    }
    if (pTrackRef) {
        *pTrackRef = trackRef;
    }
    return false;
}

GlobalTrackCacheResolver GlobalTrackCache::resolve(
        const TrackId& trackId,
        const QFileInfo& fileInfo,
        const SecurityTokenPointer& pSecurityToken) {
    TrackRef trackRef;
    GlobalTrackCacheResolver cacheResolver;
    if (resolveInternal(&cacheResolver, &trackRef, trackId, fileInfo)) {
        DEBUG_ASSERT(cacheResolver.getGlobalTrackCacheLookupResult() == GlobalTrackCacheLookupResult::HIT);
        return cacheResolver;
    }
    if (!trackRef.isValid()) {
        DEBUG_ASSERT(cacheResolver.getGlobalTrackCacheLookupResult() == GlobalTrackCacheLookupResult::NONE);
        kLogger.warning()
                << "Cache miss - ignoring invalid track"
                << trackRef;
        return cacheResolver;
    }
    auto pTrack = TrackPointer(
            new Track(
                    std::move(fileInfo),
                    std::move(pSecurityToken),
                    std::move(trackId)),
            deleter);
    DEBUG_ASSERT(createTrackRef(*pTrack) == trackRef);
    const Item item(trackRef, pTrack);
    if (debugLogEnabled()) {
        kLogger.debug()
                << "Cache miss - inserting new track into cache"
                << trackRef
                << item.plainPtr;
    }
    DEBUG_ASSERT(m_indexedTracks.find(item.plainPtr) == m_indexedTracks.end());
    m_indexedTracks.insert(item.plainPtr);
    if (trackRef.hasId()) {
        // Insert item by id
        DEBUG_ASSERT(m_tracksById.find(
                trackRef.getId()) == m_tracksById.end());
        m_tracksById.insert(
                trackRef.getId(),
                item);
    }
    if (trackRef.hasCanonicalLocation()) {
        // Insert item by track location
        DEBUG_ASSERT(m_tracksByCanonicalLocation.find(
                trackRef.getCanonicalLocation()) == m_tracksByCanonicalLocation.end());
        m_tracksByCanonicalLocation.insert(
                trackRef.getCanonicalLocation(),
                item);
    }
    return GlobalTrackCacheResolver(
            std::move(cacheResolver),
            GlobalTrackCacheLookupResult::MISS,
            std::move(trackRef),
            std::move(pTrack));
}

TrackRef GlobalTrackCache::initTrackIdInternal(
        TrackPointer pTrack,
        const TrackRef& trackRef,
        TrackId trackId) {

    DEBUG_ASSERT(!trackRef.getId().isValid());
    DEBUG_ASSERT(trackId.isValid());
    TrackRef trackRefWithId(trackRef, trackId);

    // Insert item by id
    DEBUG_ASSERT(m_tracksById.find(trackId) == m_tracksById.end());
    m_tracksById.insert(
            trackId,
            Item(trackRefWithId, std::move(pTrack)));

    // Update item by track location
    auto i = m_tracksByCanonicalLocation.find(
            trackRef.getCanonicalLocation());
    DEBUG_ASSERT(i != m_tracksByCanonicalLocation.end());
    i.value().ref = trackRefWithId;

    DEBUG_ASSERT(m_tracksById.find(trackId).value() ==
            m_tracksByCanonicalLocation.find(trackRef.getCanonicalLocation()).value());
    return trackRefWithId;
}

void GlobalTrackCache::afterEvicted(
        GlobalTrackCacheLocker* pCacheLocker,
        Track* pEvictedTrack) {
    // It can produce dangerous signal loops if the track is still
    // sending signals while being saved! All references to this
    // track have been dropped at this point, so there is no need
    // to send any signals.
    // See: https://bugs.launchpad.net/mixxx/+bug/136578
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

void GlobalTrackCache::evict(
        Track* pTrack,
        bool evictUnexpired) {
    DEBUG_ASSERT(pTrack);
    GlobalTrackCacheLocker cacheLocker;

    if (m_indexedTracks.find(pTrack) == m_indexedTracks.end()) {
        if (m_unindexedTracks.erase(pTrack)) {
            // Unindexed tracks are directly deleted without
            // invoking the post-evict hook! This only happens
            // while resetting the indices while some tracks
            // are still cached and indexed.
            if (debugLogEnabled()) {
                kLogger.debug()
                        << "Deleting unindexed track"
                        << createTrackRef(*pTrack)
                        << pTrack;
            }
            delete pTrack;
        } else {
            // Due to a rare but expected race condition the track
            // has already been deleted and must not be deleted
            // again!
            if (debugLogEnabled()) {
                kLogger.debug()
                        << "Skip deletion of dead track"
                        << pTrack;
            }
            return;
        }
    }
    // Now we know that the pointer has not been deleted before
    // and we can safely access it!s
    const auto trackRef = createTrackRef(*pTrack);
    if (debugLogEnabled()) {
        kLogger.debug()
                << "Evicting indexed track"
                << trackRef
                << pTrack;
    }

    Track* pEvictedTrack = evictInternal(trackRef, pTrack, evictUnexpired);
    DEBUG_ASSERT(verifyConsistency());
    if (pEvictedTrack) {
        DEBUG_ASSERT(pEvictedTrack == pTrack);
        afterEvicted(&cacheLocker, pEvictedTrack);
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Deleting evicted track"
                    << trackRef
                    << pTrack;
        }
        delete pEvictedTrack;
    } else {
        // If pEvictedTrack == nullptr then given pTrack is still
        // referenced within the cache and must not be deleted, yet!
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Keeping unevicted track"
                    << trackRef
                    << pTrack;
        }
    }
}

Track* GlobalTrackCache::evictInternal(
        const TrackRef& trackRef,
        Track* pTrack,
        bool evictUnexpired) {
    Item evictItem;
    DEBUG_ASSERT(!evictItem.ref.isValid());
    if (trackRef.hasId()) {
        const auto trackById = m_tracksById.find(trackRef.getId());
        if (trackById != m_tracksById.end()) {
            evictItem = *trackById;
            DEBUG_ASSERT(evictItem.plainPtr == pTrack);
            if (evictUnexpired || evictItem.weakPtr.expired()) {
                // Evict expired track
                m_tracksById.erase(trackById);
            } else {
                // Keep unexpired track
                return nullptr;
            }
        }
    }
    DEBUG_ASSERT(
            !trackRef.hasCanonicalLocation() ||
            !evictItem.ref.hasCanonicalLocation() ||
            (trackRef.getCanonicalLocation() == evictItem.ref.getCanonicalLocation()));
    const QString canonicalLocation(
            evictItem.ref.hasCanonicalLocation() ?
                    evictItem.ref.getCanonicalLocation() :
                    trackRef.getCanonicalLocation());
    const auto trackByCanonicalLocation(
            m_tracksByCanonicalLocation.find(canonicalLocation));
    if (m_tracksByCanonicalLocation.end() != trackByCanonicalLocation) {
        if (evictItem.ref.hasCanonicalLocation()) {
            DEBUG_ASSERT(evictItem == *trackByCanonicalLocation);
        } else {
            evictItem = *trackByCanonicalLocation;
            DEBUG_ASSERT(evictItem.plainPtr == pTrack);
        }
        DEBUG_ASSERT(
            !trackRef.hasId() ||
            !evictItem.ref.hasId() ||
            (trackRef.getId() == evictItem.ref.getId()));
        if (evictUnexpired || evictItem.weakPtr.expired()) {
            // Evict expired track
            VERIFY_OR_DEBUG_ASSERT(trackRef.hasId() || !evictItem.ref.hasId()) {
                m_tracksById.remove(
                        evictItem.ref.getId());
            }
            m_tracksByCanonicalLocation.erase(
                    trackByCanonicalLocation);
        } else {
            VERIFY_OR_DEBUG_ASSERT(trackRef.hasId() || !evictItem.ref.hasId()) {
                m_tracksById.insert(
                        evictItem.ref.getId(),
                        evictItem);
            }
            // Keep unexpired track
            return nullptr;
        }
    }
    DEBUG_ASSERT(m_indexedTracks.find(pTrack) != m_indexedTracks.end());
    m_indexedTracks.erase(pTrack);
    return pTrack;
}

bool GlobalTrackCache::isEmpty() const {
    GlobalTrackCacheLocker cacheLocker;
    return m_indexedTracks.empty() && m_unindexedTracks.empty();
}

void GlobalTrackCache::resetIndices() {
    GlobalTrackCacheLocker cacheLocker;
    if (!m_indexedTracks.empty()) {
        kLogger.warning()
                << "Resetting indices while"
                << m_indexedTracks.size()
                << "tracks are still cached and indexed";
        if (m_unindexedTracks.empty()) {
            m_unindexedTracks.swap(m_indexedTracks);
        } else {
            m_unindexedTracks.insert(m_indexedTracks.begin(), m_indexedTracks.end());
            m_indexedTracks.clear();
        }
    }
    m_tracksById.clear();
    m_tracksByCanonicalLocation.clear();
}
