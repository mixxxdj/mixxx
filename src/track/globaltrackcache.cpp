#include "track/globaltrackcache.h"

#include <QApplication>
#include <QThread>

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

class EvictAndSaveFunctor {
  public:
    explicit EvictAndSaveFunctor(GlobalTrackCacheEntryPointer cacheEntryPtr)
        : m_cacheEntryPtr(std::move(cacheEntryPtr)) {
    }

    void operator()(Track* plainPtr) {
        Q_UNUSED(plainPtr); // only used in DEBUG_ASSERT
        DEBUG_ASSERT(plainPtr == m_cacheEntryPtr->getPlainPtr());
        // Here we move m_cacheEntryPtr and the owned track out of the
        // functor and the owning reference counting object.
        // This is required to break a cycle reference from the weak pointer
        // inside the cache entry to the same reference counting object.
        GlobalTrackCache::evictAndSaveCachedTrack(std::move(m_cacheEntryPtr));
    }

    const GlobalTrackCacheEntryPointer& getCacheEntryPointer() const {
        return m_cacheEntryPtr;
    }

  private:
    GlobalTrackCacheEntryPointer m_cacheEntryPtr;
};


void deleteTrack(Track* plainPtr) {
    DEBUG_ASSERT(plainPtr);

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
        if (traceLogEnabled()) {
            kLogger.trace() << "Unlocking cache";
        }
        if (kLogStats && debugLogEnabled()) {
            kLogger.debug()
                    << "#tracksById ="
                    << m_pInstance->m_tracksById.size()
                    << "/ #tracksByCanonicalLocation ="
                    << m_pInstance->m_tracksByCanonicalLocation.size();
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

void GlobalTrackCacheLocker::purgeTrackId(const TrackId& trackId) {
    DEBUG_ASSERT(m_pInstance);
    return m_pInstance->purgeTrackId(trackId);
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
        TrackFile fileInfo,
        SecurityTokenPointer pSecurityToken)
        : m_lookupResult(GlobalTrackCacheLookupResult::NONE) {
    DEBUG_ASSERT(m_pInstance);
    m_pInstance->resolve(this, std::move(fileInfo), TrackId(), std::move(pSecurityToken));
}

GlobalTrackCacheResolver::GlobalTrackCacheResolver(
        TrackFile fileInfo,
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
        DEBUG_ASSERT(m_trackRef.getId() == trackId);
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
    DEBUG_ASSERT(QThread::currentThread() == pInstance->thread());
    pInstance->deleteLater();
}

//static
void GlobalTrackCache::evictAndSaveCachedTrack(GlobalTrackCacheEntryPointer cacheEntryPtr) {
    // Any access to plainPtr before a validity check inside the
    // GlobalTrackCacheLocker scope is forbidden!! Due to race
    // conditions before locking the cache this pointer might
    // already have been either deleted or reused by a second
    // shared_ptr.
    if (s_pInstance) {
        QMetaObject::invokeMethod(
                s_pInstance,
                "evictAndSave",
                // Qt will choose either a direct or a queued connection
                // depending on the thread from which this method has
                // been invoked!
                Qt::AutoConnection,
                Q_ARG(GlobalTrackCacheEntryPointer, std::move(cacheEntryPtr)));
    } else {
        // After the singular instance has been destroyed we are
        // not able to save pending changes. The track is deleted
        // when deletingPtr falls out of scope at the end of this
        // function
        kLogger.warning()
                << "Cannot evict and save"
                << cacheEntryPtr->getPlainPtr()
                << "after singleton has already been destroyed!";
    }
}

GlobalTrackCache::GlobalTrackCache(GlobalTrackCacheSaver* pSaver)
    : m_mutex(QMutex::Recursive),
      m_pSaver(pSaver),
      m_tracksById(kUnorderedCollectionMinCapacity, DbId::hash_fun) {
    DEBUG_ASSERT(m_pSaver);
    qRegisterMetaType<GlobalTrackCacheEntryPointer>("GlobalTrackCacheEntryPointer");
}

GlobalTrackCache::~GlobalTrackCache() {
    deactivate();
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
        const QString oldCanonicalLocation = i->first;
        Track* plainPtr = i->second->getPlainPtr();
        auto fileInfo = plainPtr->getFileInfo();
        TrackRef trackRef = TrackRef::fromFileInfo(
                fileInfo,
                plainPtr->getId());
        if (!trackRef.hasCanonicalLocation() && trackRef.hasId() && pRelocator) {
            auto relocatedFileInfo = pRelocator->relocateCachedTrack(
                        trackRef.getId(),
                        fileInfo);
            if (fileInfo != relocatedFileInfo) {
                plainPtr->relocate(relocatedFileInfo);
                trackRef = TrackRef::fromFileInfo(
                        relocatedFileInfo,
                        trackRef.getId());
                fileInfo = std::move(relocatedFileInfo);
            }
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
                i->second));
    }
    m_tracksByCanonicalLocation = std::move(relocatedTracksByCanonicalLocation);
}

void GlobalTrackCache::deactivate() {
    // Ideally the cache should be empty when destroyed.
    // But since this is difficult to achieve all remaining
    // cached tracks will be evicted no matter if they are still
    // referenced or not. This ensures that the eviction
    // callback is triggered for all modified tracks before
    // exiting the application.
    auto i = m_tracksById.begin();
    while (i != m_tracksById.end()) {
        Track* plainPtr= i->second->getPlainPtr();
        m_pSaver->saveCachedTrack(plainPtr);
        m_tracksByCanonicalLocation.erase(plainPtr->getCanonicalLocation());
        i = m_tracksById.erase(i);
    }

    auto j = m_tracksByCanonicalLocation.begin();
    while (j != m_tracksByCanonicalLocation.end()) {
        Track* plainPtr= j->second->getPlainPtr();
        m_pSaver->saveCachedTrack(plainPtr);
        j = m_tracksByCanonicalLocation.erase(j);
    }

    // Verify that all cached tracks have been evicted
    DEBUG_ASSERT(m_tracksById.empty());
    DEBUG_ASSERT(m_tracksByCanonicalLocation.empty());

    // The singular cache instance is already unavailable and
    // all allocated tracks will simply be deleted when their
    // shared pointer goes out of scope. Unsaved modifications
    // will be lost.
    m_pSaver = nullptr;
}

bool GlobalTrackCache::isEmpty() const {
    return m_tracksById.empty() && m_tracksByCanonicalLocation.empty();
}

TrackPointer GlobalTrackCache::lookupById(
        const TrackId& trackId) {
    const auto trackById(m_tracksById.find(trackId));
    if (m_tracksById.end() != trackById) {
        // Cache hit
        if (traceLogEnabled()) {
            kLogger.trace()
                    << "Cache hit for"
                    << trackId
                    << trackById->second->getPlainPtr();
        }
        return revive(trackById->second);
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
            if (traceLogEnabled()) {
                kLogger.trace()
                        << "Cache hit for"
                        << canonicalLocation
                        << trackByCanonicalLocation->second->getPlainPtr();
            }
            return revive(trackByCanonicalLocation->second);
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
        GlobalTrackCacheEntryPointer entryPtr) {

    TrackPointer savingPtr = entryPtr->getSavingWeakPtr().lock();
    if (savingPtr) {
        if (traceLogEnabled()) {
            kLogger.trace()
                    << "Found alive track"
                    << entryPtr->getPlainPtr();
        }
        return savingPtr;
    }

    // We are here if another thread is preempted during the
    // destructor of the last savingPtr referencing this
    // track, after the reference counter drops to zero and
    // before locking the cache. We need to revive it to abort
    // the deleter in the other thread.
    if (debugLogEnabled()) {
        kLogger.debug()
                << "Reviving zombie track"
                << entryPtr->getPlainPtr();
    }
    DEBUG_ASSERT(entryPtr->getSavingWeakPtr().expired());

    savingPtr = TrackPointer(entryPtr->getPlainPtr(),
            EvictAndSaveFunctor(entryPtr));
    entryPtr->setSavingWeakPtr(savingPtr);
    return savingPtr;
}

void GlobalTrackCache::resolve(
        GlobalTrackCacheResolver* /*in/out*/ pCacheResolver,
        TrackFile /*in*/ fileInfo,
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
    auto deletingPtr = std::unique_ptr<Track, void (&)(Track*)>(
            new Track(
                    std::move(fileInfo),
                    std::move(pSecurityToken),
                    std::move(trackId)),
            deleteTrack);
    // Track objects live together with the cache on the main thread
    // and will be deleted later within the event loop. But this
    // function might be called from any thread, even from worker
    // threads without an event loop. We need to move the newly
    // created object to the target thread.
    deletingPtr->moveToThread(QApplication::instance()->thread());

    auto cacheEntryPtr = std::make_shared<GlobalTrackCacheEntry>(
            std::move(deletingPtr));
    auto savingPtr = TrackPointer(
            cacheEntryPtr->getPlainPtr(),
            EvictAndSaveFunctor(cacheEntryPtr));
    cacheEntryPtr->setSavingWeakPtr(savingPtr);

    if (debugLogEnabled()) {
        kLogger.debug()
                << "Cache miss - inserting new track into cache"
                << trackRef
                << deletingPtr.get();
    }

    if (trackRef.hasId()) {
        // Insert item by id
        DEBUG_ASSERT(m_tracksById.find(
                trackRef.getId()) == m_tracksById.end());
        m_tracksById.insert(std::make_pair(
                trackRef.getId(),
                cacheEntryPtr));
    }
    if (trackRef.hasCanonicalLocation()) {
        // Insert item by track location
        DEBUG_ASSERT(m_tracksByCanonicalLocation.find(
                trackRef.getCanonicalLocation()) == m_tracksByCanonicalLocation.end());
        m_tracksByCanonicalLocation.insert(std::make_pair(
                trackRef.getCanonicalLocation(),
                cacheEntryPtr));
    }
    pCacheResolver->initLookupResult(
            GlobalTrackCacheLookupResult::MISS,
            std::move(savingPtr),
            std::move(trackRef));
}

TrackRef GlobalTrackCache::initTrackId(
        const TrackPointer& strongPtr,
        TrackRef trackRef,
        TrackId trackId) {
    DEBUG_ASSERT(strongPtr);
    DEBUG_ASSERT(!trackRef.getId().isValid());
    DEBUG_ASSERT(trackId.isValid());

    TrackRef trackRefWithId(trackRef, trackId);

    EvictAndSaveFunctor* pDel = std::get_deleter<EvictAndSaveFunctor>(strongPtr);
    DEBUG_ASSERT(pDel);

    // Insert item by id
    DEBUG_ASSERT(m_tracksById.find(trackId) == m_tracksById.end());
    m_tracksById.insert(std::make_pair(
            trackId,
            pDel->getCacheEntryPointer()));

    strongPtr->initId(trackId);
    DEBUG_ASSERT(createTrackRef(*strongPtr) == trackRefWithId);
    DEBUG_ASSERT(m_tracksById.find(trackId) != m_tracksById.end());

    return trackRefWithId;
}

void GlobalTrackCache::purgeTrackId(TrackId trackId) {
    DEBUG_ASSERT(trackId.isValid());

    const auto trackById(m_tracksById.find(trackId));
    if (m_tracksById.end() != trackById) {
        Track* track = trackById->second->getPlainPtr();
        track->resetId();
        m_tracksById.erase(trackById);
    }
}

void GlobalTrackCache::evictAndSave(
        GlobalTrackCacheEntryPointer cacheEntryPtr) {
    DEBUG_ASSERT(cacheEntryPtr);

    // We need to be sure this is always called from the main thread
    // because we can only access the DB from it and we must not lose the
    // the lock until all changes are persistently stored in file and DB
    // to not hand out the track again with old metadata.
    DEBUG_ASSERT(QApplication::instance()->thread() == QThread::currentThread());

    GlobalTrackCacheLocker cacheLocker;

    if (!cacheEntryPtr->getSavingWeakPtr().expired()) {
        // We have handed out (revived) this track again after our reference count
        // drops to zero and before acquiring the lock at the beginning of this function
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Skip to evict and save a revived or reallocated track"
                    << cacheEntryPtr->getPlainPtr();
        }
        return;
    }

    if (!evict(cacheEntryPtr->getPlainPtr())) {
        // A second deleter has already evicted the track from cache after our
        // reference count drops to zero and before acquiring the lock at the
        // beginning of this function
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Skip to save an already evicted track"
                    << cacheEntryPtr->getPlainPtr();
        }
        return;
    }

    DEBUG_ASSERT(isEvicted(cacheEntryPtr->getPlainPtr()));
    m_pSaver->saveCachedTrack(cacheEntryPtr->getPlainPtr());

    // here the cacheEntryPtr goes out of scope, the cache is deleted
    // including the owned track
}

bool GlobalTrackCache::evict(Track* plainPtr) {
    DEBUG_ASSERT(plainPtr);
    // Make the cached track object invisible to avoid reusing
    // it before starting to save it. This is achieved by
    // removing it from both cache indices.
    bool evicted = false;
    const auto trackRef = createTrackRef(*plainPtr);
    if (debugLogEnabled()) {
        kLogger.debug()
                << "Evicting track"
                << trackRef
                << plainPtr;
    }
    if (trackRef.hasId()) {
        const auto trackById = m_tracksById.find(trackRef.getId());
        if (trackById != m_tracksById.end()) {
            DEBUG_ASSERT(trackById->second->getPlainPtr() == plainPtr);
            m_tracksById.erase(trackById);
            evicted = true;
        }
    }
    if (trackRef.hasCanonicalLocation()) {
        const auto trackByCanonicalLocation(
                m_tracksByCanonicalLocation.find(trackRef.getCanonicalLocation()));
        if (m_tracksByCanonicalLocation.end() != trackByCanonicalLocation) {
            DEBUG_ASSERT(trackByCanonicalLocation->second->getPlainPtr() == plainPtr);
            m_tracksByCanonicalLocation.erase(
                    trackByCanonicalLocation);
            evicted = true;
        }
    }
    DEBUG_ASSERT(isEvicted(plainPtr));
    // Don't erase the pointer from m_cachedTracks here, because
    // this function is invoked from 2 different contexts. The
    // caller is responsible for doing this. Until then the cache
    // is inconsistent and verifyConsistency() is expected to fail.
    return evicted;
}

bool GlobalTrackCache::isEvicted(Track* plainPtr) const {
    for (auto&& entry: m_tracksById) {
        if (entry.second->getPlainPtr() == plainPtr) {
            return false;
        }
    }
    for (auto&& entry: m_tracksByCanonicalLocation) {
        if (entry.second->getPlainPtr() == plainPtr) {
              return false;
        }
    }
    return true;
}
