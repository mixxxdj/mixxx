#pragma once


#include <map>
#include <unordered_map>

#include "track/track.h"
#include "track/trackref.h"


// forward declaration(s)
class GlobalTrackCache;

enum class GlobalTrackCacheLookupResult {
    NONE,
    HIT,
    MISS
};

// Find the updated location of a track in the database when
// the canonical location is no longer valid or accessible.
class /*interface*/ GlobalTrackCacheRelocator {
private:
    friend class GlobalTrackCache;
    // Try to determine and return the relocated file info
    // or otherwise return just the provided file info.
    virtual QFileInfo relocateCachedTrack(
            TrackId trackId,
            QFileInfo fileInfo) = 0;

protected:
    virtual ~GlobalTrackCacheRelocator() {}
};

class GlobalTrackCacheLocker {
public:
    GlobalTrackCacheLocker();
    GlobalTrackCacheLocker(const GlobalTrackCacheLocker&) = delete;
    GlobalTrackCacheLocker(GlobalTrackCacheLocker&&);
    virtual ~GlobalTrackCacheLocker();

    GlobalTrackCacheLocker& operator=(const GlobalTrackCacheLocker&) = delete;
    GlobalTrackCacheLocker& operator=(GlobalTrackCacheLocker&&) = delete;

    void unlockCache();

    void relocateCachedTracks(
            GlobalTrackCacheRelocator* /*nullable*/ pRelocator) const;

    // Enforces the eviction of all cached tracks including invocation
    // of the callback and disables the cache permanently.
    void deactivateCache() const;

    bool isEmpty() const;

    // Lookup an existing Track object in the cache
    TrackPointer lookupTrackById(
            const TrackId& trackId) const;
    TrackPointer lookupTrackByRef(
            const TrackRef& trackRef) const;

private:
    friend class GlobalTrackCache;

    void lockCache();

protected:
    GlobalTrackCacheLocker(
            GlobalTrackCacheLocker&& moveable,
            GlobalTrackCacheLookupResult lookupResult,
            TrackPointer&& strongPtr,
            TrackRef&& trackRef);

    GlobalTrackCache* m_pInstance;
};

class GlobalTrackCacheResolver final: public GlobalTrackCacheLocker {
public:
    GlobalTrackCacheResolver(
                QFileInfo fileInfo,
                SecurityTokenPointer pSecurityToken = SecurityTokenPointer());
    GlobalTrackCacheResolver(
                QFileInfo fileInfo,
                TrackId trackId,
                SecurityTokenPointer pSecurityToken = SecurityTokenPointer());
    GlobalTrackCacheResolver(const GlobalTrackCacheResolver&) = delete;
#if !defined(_MSC_VER) && (_MSC_VER > 1900)
    GlobalTrackCacheResolver(GlobalTrackCacheResolver&&) = default;
#else
    // Visual Studio 2015 does not support default generated move constructors
    GlobalTrackCacheResolver(GlobalTrackCacheResolver&& moveable)
        : GlobalTrackCacheLocker(std::move(moveable)),
          m_lookupResult(std::move(moveable.m_lookupResult)),
          m_strongPtr(std::move(moveable.m_strongPtr)),
          m_trackRef(std::move(moveable.m_trackRef)) {
    }
#endif

    GlobalTrackCacheLookupResult getLookupResult() const {
        return m_lookupResult;
    }

    const TrackPointer& getTrack() const {
        return m_strongPtr;
    }

    const TrackRef& getTrackRef() const {
        return m_trackRef;
    }

    void initTrackIdAndUnlockCache(TrackId trackId);

    GlobalTrackCacheResolver& operator=(const GlobalTrackCacheResolver&) = delete;
    GlobalTrackCacheResolver& operator=(GlobalTrackCacheResolver&&) = delete;

private:
    friend class GlobalTrackCache;
    GlobalTrackCacheResolver();

    void initLookupResult(
            GlobalTrackCacheLookupResult lookupResult,
            TrackPointer&& strongPtr,
            TrackRef&& trackRef);

    GlobalTrackCacheLookupResult m_lookupResult;

    TrackPointer m_strongPtr;

    TrackRef m_trackRef;
};

class /*interface*/ GlobalTrackCacheSaver {
private:
    friend class GlobalTrackCache;
    virtual void saveCachedTrack(
            TrackPointer pTrack) noexcept = 0;

protected:
    virtual ~GlobalTrackCacheSaver() {}
};

class GlobalTrackCache {
public:
    static void createInstance(GlobalTrackCacheSaver* pDeleter);
    // NOTE(uklotzde, 2018-02-20): We decided not to destroy the singular
    // instance during shutdown, because we are not able to guarantee that
    // all track references have been released before. Instead the singular
    // instance is only deactivated. The following function has only been
    // preserved for completeness.
    // See also: GlobalTrackCacheLocker::deactivateCache()
    static void destroyInstance();

private:
    friend class GlobalTrackCacheLocker;
    friend class GlobalTrackCacheResolver;

    // Callback for the smart-pointer
    static void deleter(Track* plainPtr);

    explicit GlobalTrackCache(GlobalTrackCacheSaver* pDeleter);
    ~GlobalTrackCache();

    // This function should only be called DEBUG_ASSERT statements
    // to verify the class invariants during development.
    bool verifyConsistency() const;

    void relocateTracks(
            GlobalTrackCacheRelocator* /*nullable*/ pRelocator);

    TrackPointer lookupById(
            const TrackId& trackId);
    TrackPointer lookupByRef(
            const TrackRef& trackRef);

    TrackPointer revive(
            Track* plainPtr);

    void resolve(
            GlobalTrackCacheResolver* /*in/out*/ pCacheResolver,
            QFileInfo /*in*/ fileInfo,
            TrackId /*in*/ trackId,
            SecurityTokenPointer /*in*/ pSecurityToken);

    TrackRef initTrackId(
            const TrackPointer& strongPtr,
            TrackRef trackRef,
            TrackId trackId);

    void evictOrDelete(Track* plainPtr);

    typedef std::unordered_map<Track*, TrackWeakPointer> AllocatedTracks;

    bool evictAndSave(TrackPointer strongPtr);

    void evict(TrackPointer strongPtr);

    bool isEmpty() const;

    void deactivate();

    // Managed by GlobalTrackCacheLocker
    mutable QMutex m_mutex;

    GlobalTrackCacheSaver* m_pSaver;

    // This is the owner of the Track objects.
    // The tracks are saved back to database and file metatdata if the first
    // shared_ptr expires. This time the Track is still cached.
    // Than the track is copied into a second shared_ptr used while saving the
    // track. If this also expires, the track is finally deleted and removed
    // from the index.
    AllocatedTracks m_allocatedTracks;

    // This caches the unsaved Tracks by ID
    typedef std::unordered_map<TrackId, Track*, TrackId::hash_fun_t> TracksById;
    TracksById m_tracksById;

    // This caches the unsaved Tracks by location
    typedef std::map<QString, Track*> TracksByCanonicalLocation;
    TracksByCanonicalLocation m_tracksByCanonicalLocation;
};
