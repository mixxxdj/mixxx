#include "track/globaltrackcache.h"

#include "util/assert.h"
#include "util/logger.h"
#include "util/stat.h"


namespace {

const mixxx::Logger kLogger("GlobalTrackCache");

const Stat::ComputeFlags kStatCounterFlags = Stat::experimentFlags(
    Stat::COUNT | Stat::SUM | Stat::AVERAGE |
    Stat::SAMPLE_VARIANCE |Stat::SAMPLE_MEDIAN |
    Stat::MIN | Stat::MAX);

const QString kInsertByIdCounter("GlobalTrackCache::insertById");
const QString kEraseByIdCounter("GlobalTrackCache::eraseById");

const QString kInsertByCanonicalLocationCounter("GlobalTrackCache::insertByCanonicalLocation");
const QString kEraseByCanonicalLocationCounter("GlobalTrackCache::eraseByCanonicalLocation");

const QString kEvictCounter("GlobalTrackCache::evict");

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
    DEBUG_ASSERT(nullptr == m_pCacheMutex);
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
    if (nullptr != m_pCacheMutex) {
        // Verify consistency before unlocking the cache
        DEBUG_ASSERT(GlobalTrackCache::instance().verifyConsistency());
        if (kLogger.traceEnabled()) {
            kLogger.trace() << "Unlocking cache";
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
    DEBUG_ASSERT(nullptr != m_pCacheMutex); // cache is still locked
    DEBUG_ASSERT(GlobalTrackCacheLookupResult::NONE != m_lookupResult);
    DEBUG_ASSERT(m_pTrack);
    DEBUG_ASSERT(trackId.isValid());
    if (m_trackRef.getId().isValid()) {
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
    DEBUG_ASSERT(s_pInstance == nullptr);
    s_pInstance = new GlobalTrackCache(pEvictor);
}

//static
void GlobalTrackCache::destroyInstance() {
    DEBUG_ASSERT(s_pInstance != nullptr);
    GlobalTrackCache* pInstance = s_pInstance;
    s_pInstance = nullptr;
    delete pInstance;
}

//static
void GlobalTrackCache::deleter(Track* pTrack) {
    DEBUG_ASSERT(pTrack);
    if (s_pInstance) {
        delete s_pInstance->evict(pTrack);
    } else {
        kLogger.warning()
                << "Deleting uncached track";
        delete pTrack;
    }
}

GlobalTrackCache::GlobalTrackCache(GlobalTrackCacheEvictor* pEvictor)
    : m_pEvictor(pEvictor),
      m_mutex(QMutex::Recursive) {
    DEBUG_ASSERT(m_pEvictor != nullptr);
    DEBUG_ASSERT(verifyConsistency());
}

GlobalTrackCache::~GlobalTrackCache() {
    DEBUG_ASSERT(verifyConsistency());
    // Verify that the cache is empty upon destruction
    DEBUG_ASSERT(m_tracksById.empty());
    DEBUG_ASSERT(m_tracksByCanonicalLocation.empty());
}

bool GlobalTrackCache::verifyConsistency() const {
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
    TrackPointer pTrack(item.weakPtr);
    if (pTrack) {
        return std::make_pair(item.ref, pTrack);
    }
    // Race condition: The deleter for the Track object has not
    // been invoked, but the object is still referenced within
    // the cache. We need to revive it and later the deleter will
    // leave the unexpired object in the cache.
    if (kLogger.debugEnabled()) {
        kLogger.debug()
                << "Reviving zombie track"
                << item.ref;
    }
    DEBUG_ASSERT(item.weakPtr.expired());
    DEBUG_ASSERT(item.plainPtr);
    pTrack = TrackPointer(item.plainPtr, deleter);
    Item revivedItem(item.ref, pTrack);
    DEBUG_ASSERT(!revivedItem.weakPtr.expired());
    if (revivedItem.ref.hasId()) {
        m_tracksById.insert(
                revivedItem.ref.getId(),
                revivedItem);
    }
    if (revivedItem.ref.hasCanonicalLocation()) {
        m_tracksByCanonicalLocation.insert(
                revivedItem.ref.getCanonicalLocation(),
                revivedItem);
    }
    return std::make_pair(revivedItem.ref, pTrack);
}

bool GlobalTrackCache::resolveInternal(
        GlobalTrackCacheResolver* /*in/out*/ pCacheResolver,
        TrackRef* /*out*/ pTrackRef,
        const TrackId& /*in*/ trackId,
        const QFileInfo& /*in*/ fileInfo) {
    DEBUG_ASSERT(nullptr != pCacheResolver);
    // Primary lookup by id (if available)
    if (trackId.isValid()) {
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Resolving track by id"
                    << trackId;
        }
        auto trackRefPointer = lookupInternal(trackId);
        if (trackRefPointer.second) {
            if (kLogger.debugEnabled()) {
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
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Resolving track by canonical location"
                    << trackRef.getCanonicalLocation();
        }
        auto trackRefPointer = lookupInternal(trackRef);
        if (trackRefPointer.second) {
            // Cache hit
            if (kLogger.debugEnabled()) {
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
    if (nullptr != pTrackRef) {
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
    if (kLogger.debugEnabled()) {
        kLogger.debug()
                << "Cache miss - inserting new track into cache"
                << trackRef;
    }
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
        Stat::track(kInsertByIdCounter, Stat::COUNTER, kStatCounterFlags, 1);
    }
    if (trackRef.hasCanonicalLocation()) {
        m_tracksByCanonicalLocation.insert(
                trackRef.getCanonicalLocation(),
                item);
        Stat::track(kInsertByCanonicalLocationCounter, Stat::COUNTER, kStatCounterFlags, 1);
    }
    return GlobalTrackCacheResolver(
            std::move(cacheResolver),
            GlobalTrackCacheLookupResult::MISS,
            std::move(trackRef),
            std::move(pTrack));
}

TrackRef GlobalTrackCache::initTrackIdInternal(
        const TrackPointer& pTrack,
        const TrackRef& trackRef,
        TrackId trackId) {
    DEBUG_ASSERT(pTrack);
    DEBUG_ASSERT(!trackRef.getId().isValid());
    DEBUG_ASSERT(trackId.isValid());
    DEBUG_ASSERT(m_tracksById.end() == m_tracksById.find(trackId));
    TrackRef trackRefWithId(trackRef, trackId);
    Item item(trackRefWithId, pTrack);
    m_tracksById.insert(
            item.ref.getId(),
            item);
    m_tracksByCanonicalLocation.insert(
            item.ref.getCanonicalLocation(),
            item);
    return trackRefWithId;
}

Track* GlobalTrackCache::evict(
        Track* pTrack,
        bool evictUnexpired) {
    DEBUG_ASSERT(pTrack);
    const auto trackRef = TrackRef::fromFileInfo(
            pTrack->m_fileInfo,
            pTrack->m_record.getId());
    GlobalTrackCacheLocker cacheLocker;
    Track* pEvictedTrack = evictInternal(
            &cacheLocker,
            trackRef,
            evictUnexpired);
    // The cache might have been unlocked during the callback!
    DEBUG_ASSERT(!pEvictedTrack || (pEvictedTrack == pTrack));
    return pEvictedTrack;
}

GlobalTrackCache::Item GlobalTrackCache::purgeInternal(
        const TrackRef& trackRef,
        bool purgeUnexpired) {
    if (kLogger.debugEnabled()) {
        kLogger.debug()
                << "Purging track"
                << trackRef;
    }

    Item purgeItem;
    DEBUG_ASSERT(!purgeItem.ref.isValid());
    if (trackRef.hasId()) {
        const auto trackById = m_tracksById.find(trackRef.getId());
        if (trackById != m_tracksById.end()) {
            purgeItem = *trackById;
            if (purgeUnexpired || purgeItem.weakPtr.expired()) {
                m_tracksById.erase(trackById);
                Stat::track(kEraseByIdCounter, Stat::COUNTER, kStatCounterFlags, 1);
            } else {
                kLogger.debug()
                    << "Skip purging of unexpired track"
                    << purgeItem.ref
                    << ": use_count ="
                    << purgeItem.weakPtr.use_count();
                return Item();
            }
        }
    }
    DEBUG_ASSERT(
            !trackRef.hasCanonicalLocation() ||
            !purgeItem.ref.hasCanonicalLocation() ||
            (trackRef.getCanonicalLocation() == purgeItem.ref.getCanonicalLocation()));
    const QString canonicalLocation(
            purgeItem.ref.hasCanonicalLocation() ?
                    purgeItem.ref.getCanonicalLocation() :
                    trackRef.getCanonicalLocation());
    const auto trackByCanonicalLocation(
            m_tracksByCanonicalLocation.find(canonicalLocation));
    if (m_tracksByCanonicalLocation.end() != trackByCanonicalLocation) {
        if (purgeItem.ref.hasCanonicalLocation()) {
            DEBUG_ASSERT(purgeItem == *trackByCanonicalLocation);
        } else {
            purgeItem = *trackByCanonicalLocation;
        }
        DEBUG_ASSERT(
            !trackRef.hasId() ||
            !purgeItem.ref.hasId() ||
            (trackRef.getId() == purgeItem.ref.getId()));
        if (purgeUnexpired || purgeItem.weakPtr.expired()) {
            // Even if given trackRef does not have an id the found
            // item might have one. The corresponding entry must be
            // removed, otherwise we end up with an inconsistent
            // cache!
            if (!trackRef.hasId() && purgeItem.ref.hasId()) {
                m_tracksById.remove(purgeItem.ref.getId());
            }
            m_tracksByCanonicalLocation.erase(trackByCanonicalLocation);
            Stat::track(kEraseByCanonicalLocationCounter, Stat::COUNTER, kStatCounterFlags, 1);
            return purgeItem;
        } else {
            // Even if given trackRef does not have an id the found
            // item might have one. The corresponding entry must be
            // re-inserted, otherwise we end up with an inconsistent
            // cache!
            if (!trackRef.hasId() && purgeItem.ref.hasId()) {
                m_tracksById.insert(
                        purgeItem.ref.getId(),
                        purgeItem);
            }
            kLogger.debug()
                << "Skip purging of unexpired track"
                << purgeItem.ref
                << ": use_count ="
                << purgeItem.weakPtr.use_count();
            return Item();
        }
    }
    return purgeItem;
}

Track* GlobalTrackCache::evictInternal(
        GlobalTrackCacheLocker* pCacheLocker,
        const TrackRef& trackRef,
        bool evictUnexpired) {
    if (kLogger.debugEnabled()) {
        kLogger.debug()
                << "Evicting track"
                << trackRef;
    }

    const Item purgedItem = purgeInternal(trackRef, evictUnexpired);
    DEBUG_ASSERT(verifyConsistency());
    if (purgedItem.plainPtr) {
        // It can produce dangerous signal loops if the track is still
        // sending signals while being saved! All references to this
        // track have been dropped at this point, so there is no need
        // to send any signals.
        // See: https://bugs.launchpad.net/mixxx/+bug/1365708
        purgedItem.plainPtr->blockSignals(true);

        // Keep the cache locked while evicting the track object!
        // The callback is given the chance to unlock the cache
        // after all operations that rely on managed track ownership
        // have been done, e.g. exporting track metadata into a file.
        m_pEvictor->onEvictingTrackFromCache(
                pCacheLocker,
                purgedItem.plainPtr);
        Stat::track(kEvictCounter, Stat::COUNTER, kStatCounterFlags, 1);
        // At this point the cache might have been unlocked already
    } else {
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Skip evicting of uncached/recached track"
                    << trackRef;
        }
    }
    return purgedItem.plainPtr;
}

QList<TrackPointer> GlobalTrackCache::lookupAll() {
    QList<TrackPointer> allTracks;
    GlobalTrackCacheLocker cacheLocker;
    QList<Item> cacheItems(
            m_tracksByCanonicalLocation.values());
    allTracks.reserve(cacheItems.size());
    for (Item cacheItem : cacheItems) {
        TrackPointer pTrack = reviveInternal(cacheItem).second;
        DEBUG_ASSERT(pTrack);
        allTracks.append(pTrack);
    }
    return allTracks;
}

void GlobalTrackCache::evictAll() {
    QList<TrackPointer> allTracks(lookupAll());
    for (const TrackPointer& pTrack : allTracks) {
        evict(pTrack.get(), true);
    }
}

bool GlobalTrackCache::isEmpty() const {
    GlobalTrackCacheLocker cacheLocker;
    return m_tracksById.isEmpty() && m_tracksByCanonicalLocation.isEmpty();
}
