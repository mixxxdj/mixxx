#ifndef TRACKINFOCACHE_H_
#define TRACKINFOCACHE_H_


#include <QObject>
#include <QHash>
#include <QList>
#include <QMap>

#include "trackinfoobject.h"
#include "track/trackref.h"


class /*interface*/ TrackInfoCacheEvictor {
public:
    virtual void evictTrack(TrackInfoObject* pTrack) = 0;

protected:
    virtual ~TrackInfoCacheEvictor() {}
};

class TrackInfoCacheLocker final {
public:
    TrackInfoCacheLocker(const TrackInfoCacheLocker&) = delete;
    TrackInfoCacheLocker(TrackInfoCacheLocker&& moveable);
    ~TrackInfoCacheLocker();

    enum ResolveResult {
        RESOLVE_NONE,
        RESOLVE_HIT,
        RESOLVE_MISS
    };

    ResolveResult getResolveResult() const {
        return m_resolveResult;
    }

    const TrackPointer& getResolvedTrack() const {
        return m_pResolvedTrack;
    }

    void updateResolvedTrackId(TrackId trackId);

    void unlockCache();

    TrackInfoCacheLocker& operator=(const TrackInfoCacheLocker&) = delete;
    TrackInfoCacheLocker& operator=(TrackInfoCacheLocker&&);

private:
    friend class TrackInfoCache;
    TrackInfoCacheLocker();
    TrackInfoCacheLocker(
            TrackInfoCacheLocker&& moveable,
            TrackRef trackRef,
            ResolveResult resolveResult,
            TrackPointer pResolvedTrack);

    void lockCache();

    QMutex* m_pCacheMutex;

    TrackRef m_trackRef;
    ResolveResult m_resolveResult;
    TrackPointer m_pResolvedTrack;
};

class TrackInfoCacheItem final {
public:
    TrackInfoCacheItem()
        : plainPtr(nullptr) {
}
    explicit TrackInfoCacheItem(const TrackPointer& pTrack)
        : weakPtr(pTrack.toWeakRef()),
          plainPtr(pTrack.data()) {
    }

    friend bool operator==(
            const TrackInfoCacheItem& lhs,
            const TrackInfoCacheItem& rhs) {
        return (lhs.plainPtr == rhs.plainPtr) &&
                (lhs.weakPtr == rhs.weakPtr);
    }
    friend bool operator!=(
            const TrackInfoCacheItem& lhs,
            const TrackInfoCacheItem& rhs) {
        return !(lhs == rhs);
    }

    TrackWeakPointer weakPtr;
    TrackInfoObject* plainPtr;
};

class TrackInfoCache {
public:
    static void createInstance(TrackInfoCacheEvictor* pEvictor);
    static void destroyInstance();

    // Access the singular instance (singleton)
    static TrackInfoCache& instance() {
        DEBUG_ASSERT(s_pInstance != nullptr);
        return *s_pInstance;
    }

    // Lookup an existing TrackInfoObject in the cache
    TrackPointer lookup(
            const TrackId& trackId) const;
    QList<TrackPointer> lookupAll() const;

    // Lookup an existing or create a new TrackInfoObject
    TrackInfoCacheLocker resolve(
            TrackRef trackRef,
            const SecurityTokenPointer& pSecurityToken = SecurityTokenPointer());

    // Reset the ids of tracks that have been purged from the library
    void resetIds(const QList<TrackId> trackIds);

    void evictAll();

private:
    friend class TrackInfoCacheLocker;

    static TrackInfoCache* volatile s_pInstance;

    static void deleter(TrackInfoObject* pTrack);

    explicit TrackInfoCache(TrackInfoCacheEvictor* pEvictor);
    ~TrackInfoCache();

    // This function should only be called DEBUG_ASSERT statements
    // to verify the class invariants during development.
    bool verifyConsistency() const;

    void resolveInternal(
            TrackInfoCacheLocker* pCacheLocker,
            const TrackRef& trackRef);

    void evict(
            TrackInfoObject* pTrack);
    bool evictInternal(
            TrackInfoObject* pTrack,
            const TrackRef& trackRef);

    TrackInfoCacheEvictor* m_pEvictor;

    mutable QMutex m_mutex;

    typedef QHash<TrackId, TrackInfoCacheItem> TracksById;
    TracksById m_tracksById;
    typedef QMap<QString, TrackInfoCacheItem> TracksByCanonicalLocation;
    TracksByCanonicalLocation m_tracksByCanonicalLocation;
};


#endif // TRACKINFOCACHE_H_
