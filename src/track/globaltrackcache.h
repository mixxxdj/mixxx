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
    virtual TrackFile relocateCachedTrack(
            TrackId trackId,
            TrackFile fileInfo) = 0;

protected:
    virtual ~GlobalTrackCacheRelocator() {}
};

class GlobalTrackCacheEntry final {
    // We need to hold two shared pointers, the deletingPtr is
    // responsible for the lifetime of the Track object itself.
    // The second one counts the references outside Mixxx, if it
    // is not longer referenced, the track is saved and evicted
    // from the cache.
  public:
    explicit GlobalTrackCacheEntry(
            std::unique_ptr<Track, void (&)(Track*)> deletingPtr)
        : m_deletingPtr(std::move(deletingPtr)) {
    }

    GlobalTrackCacheEntry(const GlobalTrackCacheEntry& other) = delete;

    Track* getPlainPtr() const {
        return m_deletingPtr.get();
    }
    const TrackWeakPointer& getSavingWeakPtr() const {
        return m_savingWeakPtr;
    }
    void setSavingWeakPtr(TrackWeakPointer savingWeakPtr) {
        m_savingWeakPtr = std::move(savingWeakPtr);
    }

  private:
    std::unique_ptr<Track, void (&)(Track*)> m_deletingPtr;
    TrackWeakPointer m_savingWeakPtr;
};

typedef std::shared_ptr<GlobalTrackCacheEntry> GlobalTrackCacheEntryPointer;

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

    void purgeTrackId(const TrackId& trackId);

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
                TrackFile fileInfo,
                SecurityTokenPointer pSecurityToken = SecurityTokenPointer());
    GlobalTrackCacheResolver(
                TrackFile fileInfo,
                TrackId trackId,
                SecurityTokenPointer pSecurityToken = SecurityTokenPointer());
    GlobalTrackCacheResolver(const GlobalTrackCacheResolver&) = delete;
    GlobalTrackCacheResolver(GlobalTrackCacheResolver&&) = default;

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
    virtual void saveCachedTrack(Track* pTrack) noexcept = 0;

protected:
    virtual ~GlobalTrackCacheSaver() {}
};

class GlobalTrackCache : public QObject {
    Q_OBJECT

public:
    static void createInstance(GlobalTrackCacheSaver* pDeleter);
    // NOTE(uklotzde, 2018-02-20): We decided not to destroy the singular
    // instance during shutdown, because we are not able to guarantee that
    // all track references have been released before. Instead the singular
    // instance is only deactivated. The following function has only been
    // preserved for completeness.
    // See also: GlobalTrackCacheLocker::deactivateCache()
    static void destroyInstance();

    // Deleter callbacks for the smart-pointer
    static void evictAndSaveCachedTrack(GlobalTrackCacheEntryPointer cacheEntryPtr);

private slots:
    void evictAndSave(GlobalTrackCacheEntryPointer cacheEntryPtr);

private:
    friend class GlobalTrackCacheLocker;
    friend class GlobalTrackCacheResolver;

    explicit GlobalTrackCache(GlobalTrackCacheSaver* pDeleter);
    ~GlobalTrackCache();

    void relocateTracks(
            GlobalTrackCacheRelocator* /*nullable*/ pRelocator);

    TrackPointer lookupById(
            const TrackId& trackId);
    TrackPointer lookupByRef(
            const TrackRef& trackRef);

    TrackPointer revive(GlobalTrackCacheEntryPointer entryPtr);

    void resolve(
            GlobalTrackCacheResolver* /*in/out*/ pCacheResolver,
            TrackFile /*in*/ fileInfo,
            TrackId /*in*/ trackId,
            SecurityTokenPointer /*in*/ pSecurityToken);

    TrackRef initTrackId(
            const TrackPointer& strongPtr,
            TrackRef trackRef,
            TrackId trackId);

    void purgeTrackId(TrackId trackId);

    bool evict(Track* plainPtr);
    bool isEvicted(Track* plainPtr) const;

    bool isEmpty() const;

    void deactivate();

    // Managed by GlobalTrackCacheLocker
    mutable QMutex m_mutex;

    GlobalTrackCacheSaver* m_pSaver;

    // This caches the unsaved Tracks by ID
    typedef std::unordered_map<TrackId, GlobalTrackCacheEntryPointer, TrackId::hash_fun_t> TracksById;
    TracksById m_tracksById;

    // This caches the unsaved Tracks by location
    typedef std::map<QString, GlobalTrackCacheEntryPointer> TracksByCanonicalLocation;
    TracksByCanonicalLocation m_tracksByCanonicalLocation;
};
