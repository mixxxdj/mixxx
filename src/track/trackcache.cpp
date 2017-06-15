#include "track/trackcache.h"

#include "util/assert.h"


namespace {

inline
TrackRef createTrackRef(const Track& track) {
    return TrackRef::fromFileInfo(track.getFileInfo(), track.getId());
}

} // anonymous namespace

TrackCacheLocker::TrackCacheLocker()
        : m_pCacheMutex(nullptr),
          m_lookupResult(TrackCacheLookupResult::NONE) {
    lockCache();
    // Verify consistency after the cache has been locked
    DEBUG_ASSERT(TrackCache::instance().verifyConsistency());
}

TrackCacheLocker::TrackCacheLocker(
        TrackCacheLocker&& moveable)
        : m_pCacheMutex(std::move(moveable.m_pCacheMutex)),
          m_lookupResult(std::move(moveable.m_lookupResult)),
          m_trackRef(std::move(moveable.m_trackRef)),
          m_pTrack(std::move(moveable.m_pTrack)) {
    moveable.m_pCacheMutex = nullptr;
}

TrackCacheLocker& TrackCacheLocker::operator=(
        TrackCacheLocker&& moveable) {
    if (this != &moveable) {
        m_pCacheMutex = std::move(moveable.m_pCacheMutex);
        moveable.m_pCacheMutex = nullptr;
        m_lookupResult = std::move(moveable.m_lookupResult);
        m_trackRef = std::move(moveable.m_trackRef);
        m_pTrack = std::move(moveable.m_pTrack);
    }
    return *this;
}

TrackCacheLocker::TrackCacheLocker(
        TrackCacheLocker&& moveable,
        TrackCacheLookupResult lookupResult,
        TrackRef trackRef,
        TrackPointer pTrack)
        : m_pCacheMutex(moveable.m_pCacheMutex),
          m_lookupResult(lookupResult),
          m_trackRef(std::move(trackRef)),
          m_pTrack(std::move(pTrack)) {
    moveable.m_pCacheMutex = nullptr;
    // Class invariants
    DEBUG_ASSERT((TrackCacheLookupResult::NONE != m_lookupResult) || !m_pTrack);
}

TrackCacheLocker::~TrackCacheLocker() {
    unlockCache();
}

void TrackCacheLocker::lockCache() {
    DEBUG_ASSERT(nullptr == m_pCacheMutex);
    QMutex* pCacheMutex = &TrackCache::instance().m_mutex;
    pCacheMutex->lock();
    m_pCacheMutex = pCacheMutex;
}

void TrackCacheLocker::unlockCache() {
    if (nullptr != m_pCacheMutex) {
        // Verify consistency before unlocking the cache
        DEBUG_ASSERT(TrackCache::instance().verifyConsistency());
        m_pCacheMutex->unlock();
        m_pCacheMutex = nullptr;
    }
}

TrackCacheResolver::TrackCacheResolver() {
}

TrackCacheResolver::TrackCacheResolver(
        TrackCacheResolver&& moveable,
        TrackCacheLookupResult lookupResult,
        TrackRef trackRef,
        TrackPointer pTrack)
        : TrackCacheLocker(
            std::move(moveable),
            std::move(lookupResult),
            std::move(trackRef),
            std::move(pTrack)) {
}

void TrackCacheResolver::updateTrackId(TrackId trackId) {
    DEBUG_ASSERT(nullptr != m_pCacheMutex); // cache is still locked
    DEBUG_ASSERT(TrackCacheLookupResult::NONE != m_lookupResult);
    DEBUG_ASSERT(m_pTrack);
    DEBUG_ASSERT(trackId.isValid());
    m_trackRef = TrackCache::instance().updateTrackIdInternal(
            m_pTrack,
            m_trackRef,
            trackId);
    DEBUG_ASSERT(m_trackRef.getId() == trackId);
    m_pTrack->initId(trackId);
    DEBUG_ASSERT(createTrackRef(*m_pTrack) == m_trackRef);
}


//static
TrackCache* volatile TrackCache::s_pInstance = nullptr;

//static
void TrackCache::createInstance(TrackCacheEvictor* pEvictor) {
    DEBUG_ASSERT(s_pInstance == nullptr);
    s_pInstance = new TrackCache(pEvictor);
}

//static
void TrackCache::destroyInstance() {
    DEBUG_ASSERT(s_pInstance != nullptr);
    TrackCache* pInstance = s_pInstance;
    s_pInstance = nullptr;
    delete pInstance;
}

//static
void TrackCache::deleter(Track* pTrack) {
    if (s_pInstance != nullptr) {
        s_pInstance->evict(pTrack);
    }
    delete pTrack;
}

TrackCache::TrackCache(TrackCacheEvictor* pEvictor)
    : m_pEvictor(pEvictor),
      m_mutex(QMutex::Recursive) {
    DEBUG_ASSERT(m_pEvictor != nullptr);
    DEBUG_ASSERT(verifyConsistency());
}

TrackCache::~TrackCache() {
    DEBUG_ASSERT(verifyConsistency());
    // Verify that the cache is empty upon destruction
    DEBUG_ASSERT(m_tracksById.empty());
    DEBUG_ASSERT(m_tracksByCanonicalLocation.empty());
}

bool TrackCache::verifyConsistency() const {
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
        VERIFY_OR_DEBUG_ASSERT(createTrackRef(*(*i).plainPtr) == trackRef) {
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
        VERIFY_OR_DEBUG_ASSERT(createTrackRef(*(*i).plainPtr) == trackRef) {
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

TrackCacheLocker TrackCache::lookupById(
        const TrackId& trackId) const {
    TrackCacheLocker cacheLocker;
    if (trackId.isValid()) {
        const TrackPointer pTrack(lookupInternal(trackId));
        if (pTrack) {
            cacheLocker.m_lookupResult = TrackCacheLookupResult::HIT;
            cacheLocker.m_trackRef = createTrackRef(*pTrack);
            cacheLocker.m_pTrack = pTrack;
        } else {
            cacheLocker.m_lookupResult = TrackCacheLookupResult::MISS;
        }
    }
    return std::move(cacheLocker);
}

TrackCacheLocker TrackCache::lookupOrCreateTemporaryForFile(
        const QFileInfo& fileInfo,
        const SecurityTokenPointer& pSecurityToken) const {
    const TrackRef trackRef(TrackRef::fromFileInfo(fileInfo));
    TrackCacheLocker cacheLocker;
    if (trackRef.isValid()) {
        const TrackPointer pTrack(lookupInternal(trackRef));
        if (pTrack) {
            cacheLocker.m_lookupResult = TrackCacheLookupResult::HIT;
            cacheLocker.m_trackRef = createTrackRef(*pTrack);
            cacheLocker.m_pTrack = pTrack;
        } else {
            cacheLocker.m_lookupResult = TrackCacheLookupResult::MISS;
            cacheLocker.m_trackRef = trackRef;
            cacheLocker.m_pTrack =
                    Track::newTemporary(fileInfo, pSecurityToken);
        }
    }
    return std::move(cacheLocker);
}

TrackPointer TrackCache::lookupInternal(
        const TrackId& trackId) const {
    const auto trackById(m_tracksById.find(trackId));
    if (m_tracksById.end() != trackById) {
        // Cache hit
        return TrackPointer((*trackById).weakPtr);
    } else {
        // Cache miss
        return TrackPointer();
    }
}

TrackPointer TrackCache::lookupInternal(
        const TrackRef& trackRef) const {
    if (trackRef.hasId()) {
        return lookupInternal(trackRef.getId());
    } else {
        const auto trackByCanonicalLocation(
                m_tracksByCanonicalLocation.find(trackRef.getCanonicalLocation()));
        if (m_tracksByCanonicalLocation.end() != trackByCanonicalLocation) {
            // Cache hit
            return TrackPointer((*trackByCanonicalLocation).weakPtr);
        } else {
            // Cache miss
            return TrackPointer();
        }
    }
}

bool TrackCache::resolveInternal(
        TrackCacheResolver* /*in/out*/ pCacheResolver,
        TrackRef* /*out*/ pTrackRef,
        const TrackId& /*in*/ trackId,
        const QFileInfo& /*in*/ fileInfo) {
    DEBUG_ASSERT(nullptr != pCacheResolver);
    // Primary lookup by id (if available)
    if (trackId.isValid()) {
        qDebug() << "TrackCache:"
                << "Resolving track by id"
                << trackId;
        const auto trackById(m_tracksById.find(trackId));
        if (m_tracksById.end() != trackById) {
            // Cache hit
            TrackRef resolvedTrackRef((*trackById).ref);
            TrackPointer pResolvedTrack((*trackById).weakPtr);
            if (pResolvedTrack) {
                qDebug() << "TrackCache:"
                        << "Cache hit - found track by id"
                        << resolvedTrackRef;
                *pCacheResolver = TrackCacheResolver(
                        std::move(*pCacheResolver),
                        TrackCacheLookupResult::HIT,
                        resolvedTrackRef,
                        pResolvedTrack);
                return true;
            } else {
                // Explicitly evict the cached track before the deleter does it
                qDebug() << "TrackCache:"
                        << "Cache hit - evicting zombie track"
                        << resolvedTrackRef;
                Track* pEvictedTrack = evictInternal(resolvedTrackRef);
                DEBUG_ASSERT((nullptr == pEvictedTrack) ||
                        (pEvictedTrack == (*trackById).plainPtr));
            }
        }
    }
    // Secondary lookup by canonical location
    // The TrackRef is constructed now after the lookup by ID failed to
    // avoid calculating the canonical file path if it is not needed.
    TrackRef trackRef(TrackRef::fromFileInfo(fileInfo, trackId));
    if (trackRef.hasCanonicalLocation()) {
        qDebug() << "TrackCache:"
                << "Resolving track by canonical location"
                << trackRef.getCanonicalLocation();
        const auto trackByCanonicalLocation(
                m_tracksByCanonicalLocation.find(
                        trackRef.getCanonicalLocation()));
        if (m_tracksByCanonicalLocation.end() != trackByCanonicalLocation) {
            // Cache hit
            TrackRef resolvedTrackRef((*trackByCanonicalLocation).ref);
            TrackPointer pResolvedTrack((*trackByCanonicalLocation).weakPtr);
            if (pResolvedTrack) {
                qDebug() << "TrackCache:"
                        << "Cache hit - found track by canonical location"
                        << resolvedTrackRef;
                // Consistency: Resolving by id  must return the same result!
                DEBUG_ASSERT(!resolvedTrackRef.hasId() ||
                        (lookupInternal(resolvedTrackRef.getId()) == pResolvedTrack));
                *pCacheResolver = TrackCacheResolver(
                        std::move(*pCacheResolver),
                        TrackCacheLookupResult::HIT,
                        resolvedTrackRef,
                        pResolvedTrack);
                return true;
            } else {
                // Explicitly evict the cached track before the deleter does it
                qDebug() << "TrackCache:"
                        << "Cache hit - evicting zombie track"
                        << resolvedTrackRef;
                Track* pEvictedTrack = evictInternal(resolvedTrackRef);
                DEBUG_ASSERT((nullptr == pEvictedTrack) ||
                        (pEvictedTrack == (*trackByCanonicalLocation).plainPtr));
            }
        }
    }
    if (nullptr != pTrackRef) {
        *pTrackRef = trackRef;
    }
    return false;
}

TrackCacheResolver TrackCache::resolve(
        const TrackId& trackId,
        const QFileInfo& fileInfo,
        const SecurityTokenPointer& pSecurityToken) {
    TrackRef trackRef;
    TrackCacheResolver cacheResolver;
    if (resolveInternal(&cacheResolver, &trackRef, trackId, fileInfo)) {
        DEBUG_ASSERT(cacheResolver.getTrackCacheLookupResult() == TrackCacheLookupResult::HIT);
        return cacheResolver;
    }
    if (!trackRef.isValid()) {
        DEBUG_ASSERT(cacheResolver.getTrackCacheLookupResult() == TrackCacheLookupResult::NONE);
        qWarning() << "TrackCache:"
                << "Cache miss - ignoring invalid track"
                << trackRef;
        return cacheResolver;
    }
    qDebug() << "TrackCache:"
            << "Cache miss - inserting new track into cache"
            << trackRef;
    TrackPointer pTrack(
            new Track(
                    std::move(fileInfo),
                    std::move(pSecurityToken),
                    std::move(trackId)),
            deleter);
    DEBUG_ASSERT(createTrackRef(*pTrack) == trackRef);
    const Item item(trackRef, pTrack);
    if (trackRef.hasId()) {
        m_tracksById.insert(
                trackRef.getId(),
                item);
    }
    if (trackRef.hasCanonicalLocation()) {
        m_tracksByCanonicalLocation.insert(
                trackRef.getCanonicalLocation(),
                item);
    }
    return TrackCacheResolver(
            std::move(cacheResolver),
            TrackCacheLookupResult::MISS,
            std::move(trackRef),
            std::move(pTrack));
}

TrackRef TrackCache::updateTrackIdInternal(
        const TrackPointer& pTrack,
        const TrackRef& trackRef,
        TrackId trackId) {
    DEBUG_ASSERT(trackId.isValid());
    if (trackRef.getId() != trackId) {
        DEBUG_ASSERT(m_tracksById.end() == m_tracksById.find(trackId));
        TrackRef trackRefWithId(trackRef, trackId);
        Item item(trackRefWithId, pTrack);
        if (trackRef.hasId())
        m_tracksById.insert(
                item.ref.getId(),
                item);
        m_tracksByCanonicalLocation.insert(
                item.ref.getCanonicalLocation(),
                item);
        return trackRefWithId;
    }
    return trackRef;
}

void TrackCache::evict(
        Track* pTrack) {
    DEBUG_ASSERT(pTrack != nullptr);
    const TrackCacheLocker cacheLocker;
    Track* pEvictedTrack = evictInternal(
        TrackRef::fromFileInfo(pTrack->m_fileInfo, pTrack->m_record.getId()));
    DEBUG_ASSERT((nullptr == pEvictedTrack) || (pEvictedTrack == pTrack));
    DEBUG_ASSERT(verifyConsistency());
}

TrackCache::Item TrackCache::purgeInternal(
        const TrackRef& trackRef) {
    qDebug() << "TrackCache:"
            << "Purging track"
            << trackRef;

    Item purgedItem;
    DEBUG_ASSERT(!purgedItem.ref.isValid());
    if (trackRef.hasId()) {
        const auto trackById(m_tracksById.find(trackRef.getId()));
        if (m_tracksById.end() != trackById) {
            purgedItem = *trackById;
            m_tracksById.erase(trackById);
        }
    }
    DEBUG_ASSERT(
            !trackRef.hasCanonicalLocation() ||
            !purgedItem.ref.hasCanonicalLocation() ||
            (trackRef.getCanonicalLocation() == purgedItem.ref.getCanonicalLocation()));
    const QString canonicalLocation(
            purgedItem.ref.hasCanonicalLocation() ?
                    purgedItem.ref.getCanonicalLocation() :
                    trackRef.getCanonicalLocation());
    const auto trackByCanonicalLocation(
            m_tracksByCanonicalLocation.find(canonicalLocation));
    if (m_tracksByCanonicalLocation.end() != trackByCanonicalLocation) {
        if (purgedItem.ref.hasCanonicalLocation()) {
            DEBUG_ASSERT(purgedItem == *trackByCanonicalLocation);
        } else {
            purgedItem = *trackByCanonicalLocation;
            // Even if given trackRef does not have an id the found
            // item might have one. The corresponding entry must be
            // removed, otherwise we end up with an inconsistent
            // cache!
            DEBUG_ASSERT(
                !trackRef.hasId() ||
                !purgedItem.ref.hasId() ||
                (trackRef.getId() == purgedItem.ref.getId()));
            if (!trackRef.hasId() && purgedItem.ref.hasId()) {
                m_tracksById.remove(purgedItem.ref.getId());
            }
        }
        m_tracksByCanonicalLocation.erase(trackByCanonicalLocation);
    }
    return purgedItem;
}

Track* TrackCache::evictInternal(
        const TrackRef& trackRef) {
    qDebug() << "TrackCache:"
            << "Evicting track"
            << trackRef;

    const Item purgedItem(purgeInternal(trackRef));
    if (nullptr != purgedItem.plainPtr) {
        // It can produce dangerous signal loops if the track is still
        // sending signals while being saved! All references to this
        // track have been dropped at this point, so there is no need
        // to send any signals.
        // See: https://bugs.launchpad.net/mixxx/+bug/1365708
        purgedItem.plainPtr->blockSignals(true);

        // Keep the cache locked while evicting the track object!
        m_pEvictor->evictTrack(purgedItem.plainPtr);
    } else {
        qDebug() << "TrackCache:"
                << "Uncached track cannot be evicted"
                << trackRef;
    }
    return purgedItem.plainPtr;
}

QList<TrackPointer> TrackCache::lookupAll() const {
    QList<TrackPointer> allTracks;
    TrackCacheLocker cacheLocker;
    QList<Item> cacheItems(
            m_tracksByCanonicalLocation.values());
    allTracks.reserve(cacheItems.size());
    for (Item cacheItem : cacheItems) {
        TrackPointer pTrack(cacheItem.weakPtr);
        if (pTrack) {
            allTracks.append(pTrack);
        }
    }
    return allTracks;
}

void TrackCache::evictAll() {
    QList<TrackPointer> allTracks(lookupAll());
    for (const TrackPointer& pTrack : allTracks) {
        evict(pTrack.get());
    }
}
