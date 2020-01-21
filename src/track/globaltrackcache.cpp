#include "track/globaltrackcache.h"

#include <QCoreApplication>

#include "moc_globaltrackcache.cpp"
#include "track/track.h"
#include "util/assert.h"
#include "util/logger.h"
#include "util/thread_affinity.h"

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

TrackRef validateAndCanonicalizeRequestedTrackRef(
        const TrackRef& requestedTrackRef,
        const Track& cachedTrack) {
    const auto cachedTrackRef = createTrackRef(cachedTrack);
    // If an id has been provided the caller expects that if a track
    // is found it is supposed to have the exact same id. This cannot
    // be guaranteed due to file system aliasing.
    // The found track may or may not have a valid id.
    if (requestedTrackRef.hasId() &&
            requestedTrackRef.getId() != cachedTrackRef.getId()) {
        DEBUG_ASSERT(
                requestedTrackRef.getLocation() !=
                cachedTrackRef.getLocation());
        DEBUG_ASSERT(
                requestedTrackRef.getCanonicalLocation() ==
                cachedTrackRef.getCanonicalLocation());
        kLogger.warning()
                << "Found a different track for the same canonical location:"
                << "requested =" << requestedTrackRef
                << "cached =" << cachedTrackRef;
        return cachedTrackRef;
    } else {
        // Regular case, i.e. no aliasing
        return requestedTrackRef;
    }
}

class EvictAndSaveFunctor {
  public:
    explicit EvictAndSaveFunctor(
            GlobalTrackCacheEntryPointer cacheEntryPtr)
        : m_cacheEntryPtr(std::move(cacheEntryPtr)) {
    }

    void operator()(Track* plainPtr) {
        Q_UNUSED(plainPtr); // only used in DEBUG_ASSERT
        DEBUG_ASSERT(m_cacheEntryPtr);
        DEBUG_ASSERT(plainPtr == m_cacheEntryPtr->getPlainPtr());
        // Here we move m_cacheEntryPtr and the owned track out of the
        // functor and the owning reference counting object.
        // This is required to break a cycle reference from the weak pointer
        // inside the cache entry to the same reference counting object.
        GlobalTrackCache::evictAndSaveCachedTrack(std::move(m_cacheEntryPtr));
        // Verify that this functor is only invoked once
        DEBUG_ASSERT(!m_cacheEntryPtr);
    }

    const GlobalTrackCacheEntryPointer& getCacheEntryPointer() const {
        return m_cacheEntryPtr;
    }

  private:
    GlobalTrackCacheEntryPointer m_cacheEntryPtr;
};

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
    m_pInstance->purgeTrackId(trackId);
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

QSet<TrackId> GlobalTrackCacheLocker::getCachedTrackIds() const {
    DEBUG_ASSERT(m_pInstance);
    return m_pInstance->getCachedTrackIds();
}

GlobalTrackCacheResolver::GlobalTrackCacheResolver(
        TrackFile fileInfo,
        SecurityTokenPointer pSecurityToken)
        : m_lookupResult(GlobalTrackCacheLookupResult::None) {
    DEBUG_ASSERT(m_pInstance);
    m_pInstance->resolve(this, std::move(fileInfo), TrackId(), std::move(pSecurityToken));
}

GlobalTrackCacheResolver::GlobalTrackCacheResolver(
        TrackFile fileInfo,
        TrackId trackId,
        SecurityTokenPointer pSecurityToken)
        : m_lookupResult(GlobalTrackCacheLookupResult::None) {
    DEBUG_ASSERT(m_pInstance);
    m_pInstance->resolve(this, std::move(fileInfo), std::move(trackId), std::move(pSecurityToken));
}

void GlobalTrackCacheResolver::initLookupResult(
        GlobalTrackCacheLookupResult lookupResult,
        TrackPointer&& strongPtr,
        TrackRef&& trackRef) {
    DEBUG_ASSERT(m_pInstance);
    DEBUG_ASSERT(GlobalTrackCacheLookupResult::None == m_lookupResult);
    DEBUG_ASSERT(!m_strongPtr);
    m_lookupResult = lookupResult;
    m_strongPtr = std::move(strongPtr);
    m_trackRef = std::move(trackRef);
}

void GlobalTrackCacheResolver::initTrackIdAndUnlockCache(TrackId trackId) {
    DEBUG_ASSERT(m_pInstance);
    DEBUG_ASSERT(GlobalTrackCacheLookupResult::None != m_lookupResult);
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
void GlobalTrackCache::createInstance(
        GlobalTrackCacheSaver* pSaver,
        deleteTrackFn_t deleteTrackFn) {
    DEBUG_ASSERT(!s_pInstance);
    kLogger.info() << "Creating instance";
    s_pInstance = new GlobalTrackCache(pSaver, deleteTrackFn);
}

//static
void GlobalTrackCache::destroyInstance() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(s_pInstance);

    kLogger.info() << "Destroying instance";
    // Processing all pending events is required to evict all
    // remaining references from the cache.
    QCoreApplication::processEvents();
    // Now the cache should be empty
    DEBUG_ASSERT(GlobalTrackCacheLocker().isEmpty());
    GlobalTrackCache* pInstance = s_pInstance;
    // Reset the static/global pointer before entering the destructor
    s_pInstance = nullptr;
    // Delete the singular instance
    pInstance->deleteLater();
}

void GlobalTrackCacheEntry::TrackDeleter::operator()(Track* pTrack) const {
    DEBUG_ASSERT(pTrack);

    // We safely delete the object via the Qt event queue instead
    // of using operator delete! Otherwise the deleted track object
    // might be accessed when processing cross-thread signals that
    // are delayed within a queued connection and may arrive after
    // the object has already been deleted.
    if (traceLogEnabled()) {
        pTrack->dumpObjectInfo();
    }
    if (debugLogEnabled()) {
        kLogger.debug()
                << "Deleting"
                << pTrack;
    }

    if (m_deleteTrackFn) {
        // Custom delete function
        (*m_deleteTrackFn)(pTrack);
    } else {
        // Default delete function
        pTrack->deleteLater();
    }
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
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
                "slotEvictAndSave",
#else
                [cacheEntryPtr = std::move(cacheEntryPtr)] {
                    s_pInstance->slotEvictAndSave(cacheEntryPtr);
                },
#endif
                // Qt will choose either a direct or a queued connection
                // depending on the thread from which this method has
                // been invoked!
                Qt::AutoConnection
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
                ,
                Q_ARG(GlobalTrackCacheEntryPointer, std::move(cacheEntryPtr))
#endif
        );
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

GlobalTrackCache::GlobalTrackCache(
        GlobalTrackCacheSaver* pSaver,
        deleteTrackFn_t deleteTrackFn)
    : m_mutex(QMutex::Recursive),
      m_pSaver(pSaver),
      m_deleteTrackFn(deleteTrackFn),
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

void GlobalTrackCache::saveEvictedTrack(Track* pEvictedTrack) const {
    DEBUG_ASSERT(pEvictedTrack);
    // Disconnect all receivers and block signals before saving the
    // track. Accessing an object-under-destruction in signal handlers
    // could cause undefined behavior!
    // NOTE(uklotzde, 2018-02-03): Simply disconnecting all receivers
    // doesn't seem to work reliably. Emitting the clean() signal from
    // a track that is about to deleted may cause access violations!!
    pEvictedTrack->disconnect();
    pEvictedTrack->blockSignals(true);
    m_pSaver->saveEvictedTrack(pEvictedTrack);
}

void GlobalTrackCache::deactivate() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    if (isEmpty()) {
        return;
    }

    // Ideally the cache should be empty when destroyed.
    // But since this is difficult to achieve all remaining
    // cached tracks will be evicted no matter if they are still
    // referenced or not. This ensures that the eviction
    // callback is triggered for all modified tracks before
    // exiting the application.
    kLogger.warning()
            << "Evicting all remaining"
            << m_tracksById.size()
            << '/'
            << m_tracksByCanonicalLocation.size()
            << "tracks from cache";

    while (!m_tracksById.empty()) {
        auto i = m_tracksById.begin();
        Track* plainPtr= i->second->getPlainPtr();
        saveEvictedTrack(plainPtr);
        m_tracksByCanonicalLocation.erase(plainPtr->getCanonicalLocation());
        m_tracksById.erase(i);
    }

    while (!m_tracksByCanonicalLocation.empty()) {
        auto i = m_tracksByCanonicalLocation.begin();
        Track* plainPtr= i->second->getPlainPtr();
        saveEvictedTrack(plainPtr);
        m_tracksByCanonicalLocation.erase(i);
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
    TrackPointer trackPtr;
    const auto trackById(m_tracksById.find(trackId));
    if (m_tracksById.end() != trackById) {
        // Cache hit
        if (traceLogEnabled()) {
            kLogger.trace()
                    << "Cache hit for"
                    << trackId
                    << trackById->second->getPlainPtr();
        }
        trackPtr = revive(trackById->second);
        DEBUG_ASSERT(trackPtr);
    } else {
        // Cache miss
        if (traceLogEnabled()) {
            kLogger.trace()
                    << "Cache miss for"
                    << trackId;
        }
    }
    return trackPtr;
}

TrackPointer GlobalTrackCache::lookupByRef(
        const TrackRef& trackRef) {
    TrackPointer trackPtr;
    if (trackRef.hasId()) {
        trackPtr = lookupById(trackRef.getId());
        if (trackPtr) {
            return trackPtr;
        }
    }
    if (trackRef.hasCanonicalLocation()) {
        trackPtr = lookupByCanonicalLocation(trackRef.getCanonicalLocation());
        if (trackPtr) {
            const auto cachedTrackRef =
                    validateAndCanonicalizeRequestedTrackRef(trackRef, *trackPtr);
            // Multiple tracks may reference the same physical file on disk
            if (!trackRef.hasId() || trackRef.getId() == cachedTrackRef.getId()) {
                return trackPtr;
            }
        }
    }
    return trackPtr;
}

TrackPointer GlobalTrackCache::lookupByCanonicalLocation(
        const QString& canonicalLocation) {
    TrackPointer trackPtr;
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
        trackPtr = revive(trackByCanonicalLocation->second);
        DEBUG_ASSERT(trackPtr);
    } else {
        // Cache miss
        if (traceLogEnabled()) {
            kLogger.trace()
                    << "Cache miss for"
                    << canonicalLocation;
        }
    }
    return trackPtr;
}

QSet<TrackId> GlobalTrackCache::getCachedTrackIds() const {
    QSet<TrackId> trackIds;
    for (const auto& entry : m_tracksById) {
        trackIds << entry.first;
    }
    return trackIds;
}

TrackPointer GlobalTrackCache::revive(
        GlobalTrackCacheEntryPointer entryPtr) {

    TrackPointer savingPtr = entryPtr->lock();
    if (savingPtr) {
        if (traceLogEnabled()) {
            kLogger.trace()
                    << "Found alive track"
                    << entryPtr->getPlainPtr();
        }
        DEBUG_ASSERT(!savingPtr->signalsBlocked());
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
    DEBUG_ASSERT(entryPtr->expired());

    savingPtr = TrackPointer(entryPtr->getPlainPtr(),
            EvictAndSaveFunctor(entryPtr));
    entryPtr->init(savingPtr);
    DEBUG_ASSERT(!savingPtr->signalsBlocked());
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
                    GlobalTrackCacheLookupResult::Hit,
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
        auto strongPtr = lookupByCanonicalLocation(
                trackRef.getCanonicalLocation());
        if (strongPtr) {
            // Cache hit
            if (debugLogEnabled()) {
                kLogger.debug()
                        << "Cache hit - found track by canonical location"
                        << trackRef.getCanonicalLocation()
                        << strongPtr.get();
            }
            auto cachedTrackRef = validateAndCanonicalizeRequestedTrackRef(
                    trackRef,
                    *strongPtr);
            // Multiple tracks may reference the same physical file on disk
            if (!trackRef.hasId() || trackRef.getId() == cachedTrackRef.getId()) {
                pCacheResolver->initLookupResult(
                        GlobalTrackCacheLookupResult::Hit,
                        std::move(strongPtr),
                        std::move(trackRef));
            } else {
                pCacheResolver->initLookupResult(
                        GlobalTrackCacheLookupResult::ConflictCanonicalLocation,
                        TrackPointer(),
                        std::move(cachedTrackRef));
            }
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
    auto deletingPtr = std::unique_ptr<Track, GlobalTrackCacheEntry::TrackDeleter>(
            new Track(
                    std::move(fileInfo),
                    std::move(pSecurityToken),
                    std::move(trackId)),
            GlobalTrackCacheEntry::TrackDeleter(m_deleteTrackFn));

    auto cacheEntryPtr = std::make_shared<GlobalTrackCacheEntry>(
            std::move(deletingPtr));
    auto savingPtr = TrackPointer(
            cacheEntryPtr->getPlainPtr(),
            EvictAndSaveFunctor(cacheEntryPtr));
    cacheEntryPtr->init(savingPtr);

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

    // Track objects live together with the cache on the main thread
    // and will be deleted later within the event loop. But this
    // function might be called from any thread, even from worker
    // threads without an event loop. We need to move the newly
    // created object to the main thread.
    savingPtr->moveToThread(QCoreApplication::instance()->thread());

    pCacheResolver->initLookupResult(
            GlobalTrackCacheLookupResult::Miss,
            std::move(savingPtr),
            std::move(trackRef));
}

TrackRef GlobalTrackCache::initTrackId(
        const TrackPointer& strongPtr,
        const TrackRef& trackRef,
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

void GlobalTrackCache::slotEvictAndSave(
        GlobalTrackCacheEntryPointer cacheEntryPtr) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(cacheEntryPtr);

    // GlobalTrackCacheSaver::saveEvictedTrack() requires that
    // exclusive access is guaranteed for the duration of the
    // whole invocation!
    GlobalTrackCacheLocker cacheLocker;

    if (!cacheEntryPtr->expired()) {
        // We have handed out (revived) this track again after our reference count
        // drops to zero and before acquiring the lock at the beginning of this function
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Skip to evict and save a revived or reallocated track"
                    << cacheEntryPtr->getPlainPtr();
        }
        return;
    }

    if (!tryEvict(cacheEntryPtr->getPlainPtr())) {
        // A second deleter has already evicted the track from cache after our
        // reference count drops to zero and before acquiring the lock at the
        // beginning of this function
        if (debugLogEnabled()) {
            kLogger.debug()
                    << "Skip to save an already evicted track again"
                    << cacheEntryPtr->getPlainPtr();
        }
        return;
    }

    DEBUG_ASSERT(!isCached(cacheEntryPtr->getPlainPtr()));
    saveEvictedTrack(cacheEntryPtr->getPlainPtr());

    // Explicitly release the cacheEntryPtr including the owned
    // track object while the cache is still locked.
    cacheEntryPtr.reset();

    // Finally the exclusive lock on the cache is released implicitly
    // when exiting the scope of this method.
}

bool GlobalTrackCache::tryEvict(Track* plainPtr) {
    DEBUG_ASSERT(plainPtr);
    // Make the cached track object invisible to avoid reusing
    // it before starting to save it. This is achieved by
    // removing it from both cache indices.
    // Due to expected race conditions pointers might be evicted
    // multiple times. Therefore we need to check the stored
    // pointers to avoid evicting a new cached track object instead
    // of the given plainPtr!!
    bool evicted = false;
    bool notEvicted = false;
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
            if (trackById->second->getPlainPtr() == plainPtr) {
                m_tracksById.erase(trackById);
                evicted = true;
            } else {
                notEvicted = true;
            }
        }
    }
    if (trackRef.hasCanonicalLocation()) {
        const auto trackByCanonicalLocation(
                m_tracksByCanonicalLocation.find(trackRef.getCanonicalLocation()));
        if (m_tracksByCanonicalLocation.end() != trackByCanonicalLocation) {
            if (trackByCanonicalLocation->second->getPlainPtr() == plainPtr) {
                m_tracksByCanonicalLocation.erase(
                        trackByCanonicalLocation);
                evicted = true;
            } else {
                notEvicted = true;
            }
        }
    }
    DEBUG_ASSERT(!isCached(plainPtr));
    Q_UNUSED(notEvicted); // only used in debug assertion
    DEBUG_ASSERT(!(evicted && notEvicted));
    return evicted;
}

bool GlobalTrackCache::isCached(Track* plainPtr) const {
    for (auto&& entry: m_tracksById) {
        if (entry.second->getPlainPtr() == plainPtr) {
            return true;
        }
    }
    for (auto&& entry: m_tracksByCanonicalLocation) {
        if (entry.second->getPlainPtr() == plainPtr) {
              return true;
        }
    }
    return false;
}
