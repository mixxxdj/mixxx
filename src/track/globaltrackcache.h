#pragma once


#include <map>
#include <set>

#include "track/track.h"
#include "track/trackref.h"


// forward declaration(s)
class GlobalTrackCache;

enum class GlobalTrackCacheLookupResult {
    NONE,
    HIT,
    MISS
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

    // Reset all indices but keep the currently allocated tracks
    // to prevent memory leaks.
    void resetCache() const;

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

class /*interface*/ GlobalTrackCacheEvictor {
public:
    /**
     * This function will be called when evicting a track from the
     * cache to perform operations before the track object is deleted.
     *
     * The parameter pCacheLocker is optional and might be null. It
     * allows the callee to explicitly unlock the cache before performing
     * any long running operations that don't require that the cache is
     * kept locked. The unlockCache() operation is the only operation that
     * should be called on pCacheLocker! The second pointer is accessible
     * and valid even if after the cache has been unlocked.
     */
    virtual void afterEvictedTrackFromCache(
            GlobalTrackCacheLocker* /*nullable*/ pCacheLocker,
            Track* /*not null*/ plainPtr) = 0;

protected:
    virtual ~GlobalTrackCacheEvictor() {}
};

class GlobalTrackCache {
public:
    static void createInstance(GlobalTrackCacheEvictor* pEvictor);
    static void destroyInstance();

private:
    friend class GlobalTrackCacheLocker;
    friend class GlobalTrackCacheResolver;

    // Callback for the smart-pointer
    static void deleter(Track* plainPtr);

    explicit GlobalTrackCache(GlobalTrackCacheEvictor* pEvictor);
    ~GlobalTrackCache();

    // This function should only be called DEBUG_ASSERT statements
    // to verify the class invariants during development.
    bool verifyConsistency() const;

    void reset();

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

    bool evictAndDelete(
            Track* plainPtr);

    typedef std::map<Track*, TrackWeakPointer> IndexedTracks;

    bool evictAndDelete(
            GlobalTrackCacheLocker* /*nullable*/ pCacheLocker,
            IndexedTracks::iterator indexedTrack,
            bool evictUnexpired);
    bool evict(
            const TrackRef& trackRef,
            IndexedTracks::iterator indexedTrack,
            bool evictUnexpired);

    void afterEvicted(
            GlobalTrackCacheLocker* /*nullable*/ pCacheLocker,
            Track* plainPtr);

    bool isEmpty() const;

    void deactivate();

    // Managed by GlobalTrackCacheLocker
    mutable QMutex m_mutex;

    GlobalTrackCacheEvictor* m_pEvictor;

    IndexedTracks m_indexedTracks;

    typedef std::set<Track*> UnindexedTracks;
    UnindexedTracks m_unindexedTracks;

    typedef std::map<TrackId, Track*> TracksById;
    TracksById m_tracksById;

    typedef std::map<QString, Track*> TracksByCanonicalLocation;
    TracksByCanonicalLocation m_tracksByCanonicalLocation;
};
