#include "trackinfocache.h"

#include "util/assert.h"


TrackInfoCacheLocker::TrackInfoCacheLocker()
        : m_pCacheMutex(nullptr),
          m_resolveResult(RESOLVE_NONE) {
    lockCache();
    // Verify consistency after the cache has been locked
    DEBUG_ASSERT(TrackInfoCache::instance().verifyConsistency());
}

TrackInfoCacheLocker::TrackInfoCacheLocker(
        TrackInfoCacheLocker&& moveable)
        : m_pCacheMutex(std::move(moveable.m_pCacheMutex)),
          m_trackRef(std::move(moveable.m_trackRef)),
          m_resolveResult(std::move(moveable.m_resolveResult)),
          m_pResolvedTrack(std::move(moveable.m_pResolvedTrack)) {
    moveable.m_pCacheMutex = nullptr;
}

TrackInfoCacheLocker& TrackInfoCacheLocker::operator=(
        TrackInfoCacheLocker&& moveable) {
    if (this != &moveable) {
        m_pCacheMutex = std::move(moveable.m_pCacheMutex);
        moveable.m_pCacheMutex = nullptr;
        m_trackRef = std::move(moveable.m_trackRef);
        m_resolveResult = std::move(moveable.m_resolveResult);
        m_pResolvedTrack = std::move(moveable.m_pResolvedTrack);
    }
    return *this;
}

TrackInfoCacheLocker::TrackInfoCacheLocker(
        TrackInfoCacheLocker&& moveable,
        TrackRef trackRef,
        ResolveResult resolveResult,
        TrackPointer pResolvedTrack)
        : m_pCacheMutex(moveable.m_pCacheMutex),
          m_trackRef(std::move(trackRef)),
          m_resolveResult(resolveResult),
          m_pResolvedTrack(std::move(pResolvedTrack)) {
    moveable.m_pCacheMutex = nullptr;
    // Class invariants
    DEBUG_ASSERT((RESOLVE_NONE != m_resolveResult) || m_pResolvedTrack.isNull());
}

TrackInfoCacheLocker::~TrackInfoCacheLocker() {
    unlockCache();
}

void TrackInfoCacheLocker::updateResolvedTrackId(TrackId trackId) {
    DEBUG_ASSERT(nullptr != m_pCacheMutex); // cache is still locked
    DEBUG_ASSERT(RESOLVE_NONE != m_resolveResult);
    DEBUG_ASSERT(!m_pResolvedTrack.isNull());
    DEBUG_ASSERT(m_pResolvedTrack->getId() == trackId);
    if (trackId.isValid()) {
        TrackInfoCache::instance().resolveInternal(
                this,
                TrackRef(m_trackRef, trackId));
    } else {
        TrackInfoCache::instance().m_tracksById.remove(trackId);
    }
    DEBUG_ASSERT(nullptr != m_pCacheMutex); // cache is still locked
    DEBUG_ASSERT(RESOLVE_NONE != m_resolveResult);
    DEBUG_ASSERT(!m_pResolvedTrack.isNull());
    DEBUG_ASSERT(m_pResolvedTrack->getId() == m_trackRef.getId());
}

void TrackInfoCacheLocker::lockCache() {
    DEBUG_ASSERT(nullptr == m_pCacheMutex);
    QMutex* pCacheMutex = &TrackInfoCache::instance().m_mutex;
    pCacheMutex->lock();
    m_pCacheMutex = pCacheMutex;
}

void TrackInfoCacheLocker::unlockCache() {
    if (nullptr != m_pCacheMutex) {
        // Verify consistency before unlocking the cache
        DEBUG_ASSERT(TrackInfoCache::instance().verifyConsistency());
        m_pCacheMutex->unlock();
        m_pCacheMutex = nullptr;
    }
}


//static
TrackInfoCache* volatile TrackInfoCache::s_pInstance = nullptr;

//static
void TrackInfoCache::createInstance(TrackInfoCacheEvictor* pEvictor) {
    DEBUG_ASSERT(s_pInstance == nullptr);
    s_pInstance = new TrackInfoCache(pEvictor);
}

//static
void TrackInfoCache::destroyInstance() {
    DEBUG_ASSERT(s_pInstance != nullptr);
    TrackInfoCache* pInstance = s_pInstance;
    s_pInstance = nullptr;
    delete pInstance;
}

//static
void TrackInfoCache::deleter(TrackInfoObject* pTrack) {
    if (s_pInstance != nullptr) {
        s_pInstance->evict(pTrack);
    }
    delete pTrack;
}

TrackInfoCache::TrackInfoCache(TrackInfoCacheEvictor* pEvictor)
    : m_pEvictor(pEvictor),
      m_mutex(QMutex::Recursive) {
    DEBUG_ASSERT(m_pEvictor != nullptr);
    DEBUG_ASSERT(verifyConsistency());
}

TrackInfoCache::~TrackInfoCache() {
    DEBUG_ASSERT(verifyConsistency());
    // Verify that the cache is empty upon destruction
    DEBUG_ASSERT(m_tracksById.empty());
    DEBUG_ASSERT(m_tracksByCanonicalLocation.empty());
}

bool TrackInfoCache::verifyConsistency() const {
    DEBUG_ASSERT_AND_HANDLE(m_tracksById.keys().size() == m_tracksById.uniqueKeys().size()) {
        return false;
    }
    for (TracksById::const_iterator i(m_tracksById.begin()); i != m_tracksById.end(); ++i) {
        const TrackRef trackRef(i.value().plainPtr->createRef());
        const TrackId trackId(trackRef.getId());
        DEBUG_ASSERT_AND_HANDLE(trackId == i.key()) {
            return false;
        }
        DEBUG_ASSERT_AND_HANDLE(1 == m_tracksById.count(trackId)) {
            return false;
        }
        const QString canonicalLocation(trackRef.getCanonicalLocation());
        DEBUG_ASSERT_AND_HANDLE(canonicalLocation.isEmpty() ||
                (1 == m_tracksByCanonicalLocation.count(canonicalLocation))) {
            return false;
        }
    }
    for (TracksByCanonicalLocation::const_iterator i(m_tracksByCanonicalLocation.begin()); i != m_tracksByCanonicalLocation.end(); ++i) {
        const QString canonicalLocation(i.value().plainPtr->getCanonicalLocation());
        DEBUG_ASSERT_AND_HANDLE(canonicalLocation == i.key()) {
            return false;
        }
        DEBUG_ASSERT_AND_HANDLE(!canonicalLocation.isEmpty()) {
            return false;
        }
    }
    return true;
}

TrackPointer TrackInfoCache::lookup(
        const TrackId& trackId) const {
    if (trackId.isValid()) {
        TrackInfoCacheLocker cacheLocker;
        auto trackById(m_tracksById.find(trackId));
        if (m_tracksById.end() != trackById) {
            // Cache hit
            DEBUG_ASSERT(trackById.value().plainPtr != nullptr);
            DEBUG_ASSERT(trackById.value().plainPtr->getId() == trackId);
            return trackById.value().weakPtr.toStrongRef();
        }
    }
    return TrackPointer();
}

void TrackInfoCache::resolveInternal(
        TrackInfoCacheLocker* pCacheLocker,
        const TrackRef& trackRef) {
    DEBUG_ASSERT(nullptr != pCacheLocker);
    DEBUG_ASSERT(trackRef.isValid());
    // Primary lookup by id
    if (trackRef.hasId()) {
        qDebug() << "TrackInfoCache:"
                << "Resolving track by id"
                << trackRef;
        auto trackById(m_tracksById.find(trackRef.getId()));
        if (m_tracksById.end() != trackById) {
            // Cache hit
            TrackPointer pTrack(trackById.value().weakPtr.toStrongRef());
            if (pTrack.isNull()) {
                // Explicitly evict the cached track before the deleter does it
                qDebug() << "TrackInfoCache:"
                        << "Cache hit - evicting zombie track"
                        << trackRef;
                evictInternal(
                        trackById.value().plainPtr,
                        trackRef);
            } else {
                TrackRef trackHit(pTrack->createRef());
                DEBUG_ASSERT(trackHit.getId() == trackRef.getId());
                qDebug() << "TrackInfoCache:"
                        << "Cache hit - found track by id"
                        << trackHit;
                // Reinsert by canonical location
                if (trackHit.hasCanonicalLocation()) {
                    m_tracksByCanonicalLocation.insert(
                            trackHit.getCanonicalLocation(),
                            TrackInfoCacheItem(pTrack));
                }
                *pCacheLocker = TrackInfoCacheLocker(
                        std::move(*pCacheLocker),
                        std::move(trackHit),
                        TrackInfoCacheLocker::RESOLVE_HIT,
                        std::move(pTrack));
                return;
            }
        }
    }
    // Secondary lookup by canonical location
    if (trackRef.hasCanonicalLocation()) {
        qDebug() << "TrackInfoCache:"
                << "Resolving track by canonical location"
                << trackRef;
        auto trackByCanonicalLocation(
                m_tracksByCanonicalLocation.find(
                        trackRef.getCanonicalLocation()));
        if (m_tracksByCanonicalLocation.end() != trackByCanonicalLocation) {
            // Cache hit
            TrackPointer pTrack(trackByCanonicalLocation.value().weakPtr.toStrongRef());
            if (pTrack.isNull()) {
                // Explicitly evict the cached track before the deleter does it
                qDebug() << "TrackInfoCache:"
                        << "Cache hit - evicting zombie track"
                        << trackRef;
                evictInternal(
                        trackByCanonicalLocation.value().plainPtr,
                        trackRef);
            } else {
                TrackRef trackHit(pTrack->createRef());
                qDebug() << "TrackInfoCache:"
                        << "Cache hit - found track by canonical location"
                        << trackHit;
                // Reinsert by id
                if (trackHit.hasId()) {
                    m_tracksById.insert(
                            trackHit.getId(),
                            TrackInfoCacheItem(pTrack));
                }
                *pCacheLocker = TrackInfoCacheLocker(
                        std::move(*pCacheLocker),
                        std::move(trackHit),
                        TrackInfoCacheLocker::RESOLVE_HIT,
                        std::move(pTrack));
                return;
            }
        }
    }
}

TrackInfoCacheLocker TrackInfoCache::resolve(
        TrackRef trackRef,
        const SecurityTokenPointer& pSecurityToken) {
    TrackInfoCacheLocker cacheLocker;
    resolveInternal(&cacheLocker, trackRef);
    if (cacheLocker.getResolveResult() == TrackInfoCacheLocker::RESOLVE_HIT) {
        return cacheLocker;
    } else if (trackRef.isValid()) {
        qDebug() << "TrackInfoCache:"
                << "Cache miss - inserting track into cache"
                << trackRef;
        TrackPointer pTrack(
                new TrackInfoObject(
                        QFileInfo(trackRef.getLocation()),
                        pSecurityToken,
                        trackRef.getId()),
                deleter);
        if (trackRef.hasId()) {
            m_tracksById.insert(
                    trackRef.getId(),
                    TrackInfoCacheItem(pTrack));
        }
        if (trackRef.hasCanonicalLocation()) {
            m_tracksByCanonicalLocation.insert(
                    trackRef.getCanonicalLocation(),
                    TrackInfoCacheItem(pTrack));
        }
        return TrackInfoCacheLocker(
                std::move(cacheLocker),
                std::move(trackRef),
                TrackInfoCacheLocker::RESOLVE_MISS,
                std::move(pTrack));
    } else {
        return cacheLocker;
    }
}

void TrackInfoCache::resetIds(
        const QList<TrackId> trackIds) {
    const TrackInfoCacheLocker cacheLocker;
    for (const auto& trackId: trackIds) {
        qDebug() << "TrackInfoCache:"
                << "Resetting track id"
                << trackId;
        auto trackById(m_tracksById.find(trackId));
        if (m_tracksById.end() != trackById) {
            trackById.value().plainPtr->setId(TrackId());
            m_tracksById.erase(trackById);
        }
    }
}

void TrackInfoCache::evict(
        TrackInfoObject* pTrack) {
    DEBUG_ASSERT(pTrack != nullptr);
    const TrackInfoCacheLocker cacheLocker;
    evictInternal(
        pTrack,
        TrackRef(pTrack->m_fileInfo, pTrack->m_id));
}

bool TrackInfoCache::evictInternal(
        TrackInfoObject* pTrack,
        const TrackRef& trackRef) {
    DEBUG_ASSERT(pTrack != nullptr);

    qDebug() << "TrackInfoCache:"
            << "Evicting track"
            << trackRef;
    TrackInfoCacheItem cacheItem;
    auto trackById(m_tracksById.find(trackRef.getId()));
    auto trackByCanonicalLocation(m_tracksByCanonicalLocation.find(trackRef.getCanonicalLocation()));
    if (m_tracksById.end() != trackById) {
        cacheItem = *trackById;
    } else {
        if (m_tracksByCanonicalLocation.end() != trackByCanonicalLocation) {
            cacheItem = *trackByCanonicalLocation;
        }
    }
    if (cacheItem.plainPtr != pTrack) {
        qDebug() << "TrackInfoCache:"
                << "Uncached track cannot be evicted"
                << trackRef;
        return false;
    } else {
        m_tracksById.erase(trackById);
        m_tracksByCanonicalLocation.erase(trackByCanonicalLocation);
    }

    // It can produce dangerous signal loops if the TIO is still
    // sending signals while being saved! All references to this
    // TIO have been dropped at this point, so there is no need
    // to send any signals.
    // See: https://bugs.launchpad.net/mixxx/+bug/1365708
    pTrack->blockSignals(true);

    // Keep the cache locked while evicting the track object!
    m_pEvictor->evictTrack(pTrack);
    return true;
}

QList<TrackPointer> TrackInfoCache::lookupAll() const {
    QList<TrackPointer> allTracks;
    TrackInfoCacheLocker cacheLocker;
    QList<TrackInfoCacheItem> cacheItems(
            m_tracksByCanonicalLocation.values());
    allTracks.reserve(cacheItems.size());
    for (TrackInfoCacheItem cacheItem : cacheItems) {
        TrackPointer pTrack(cacheItem.weakPtr.toStrongRef());
        if (!pTrack.isNull()) {
            allTracks.append(pTrack);
        }
    }
    return allTracks;
}

void TrackInfoCache::evictAll() {
    QList<TrackPointer> allTracks(lookupAll());
    for (const TrackPointer& pTrack : allTracks) {
        evict(pTrack.data());
    }
}
