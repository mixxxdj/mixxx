#ifndef TRACKCACHE_H_
#define TRACKCACHE_H_


#include <QObject>
#include <QHash>
#include <QList>
#include <QMap>

#include "track/track.h"
#include "track/trackref.h"


class /*interface*/ TrackCacheEvictor {
public:
    virtual void evictTrack(Track* pTrack) = 0;

protected:
    virtual ~TrackCacheEvictor() {}
};

enum class TrackCacheLookupResult {
    NONE,
    HIT,
    MISS
};

class TrackCacheLocker {
public:
    TrackCacheLocker(const TrackCacheLocker&) = delete;
    TrackCacheLocker(TrackCacheLocker&& moveable);
    virtual ~TrackCacheLocker();

    TrackCacheLookupResult getTrackCacheLookupResult() const {
        return m_lookupResult;
    }

    const TrackRef& getTrackRef() const {
        return m_trackRef;
    }

    const TrackPointer& getTrack() const {
        return m_pTrack;
    }

    void updateResolvedTrackId(TrackId trackId);

    void unlockCache();

    TrackCacheLocker& operator=(const TrackCacheLocker&) = delete;

private:
    friend class TrackCache;

    void lockCache();

protected:
    TrackCacheLocker();
    TrackCacheLocker(
            TrackCacheLocker&& moveable,
            TrackCacheLookupResult lookupResult,
            TrackRef trackRef,
            TrackPointer pTrack);

    TrackCacheLocker& operator=(TrackCacheLocker&&);

    QMutex* m_pCacheMutex;

    TrackCacheLookupResult m_lookupResult;

    TrackRef m_trackRef;
    TrackPointer m_pTrack;
};

class TrackCacheResolver final: public TrackCacheLocker {
public:
    TrackCacheResolver(const TrackCacheResolver&) = delete;
    TrackCacheResolver(TrackCacheResolver&&) = default;

    void updateTrackId(TrackId trackId);

    TrackCacheResolver& operator=(const TrackCacheResolver&) = delete;

private:
    friend class TrackCache;
    TrackCacheResolver();
    TrackCacheResolver(
            TrackCacheResolver&& moveable,
            TrackCacheLookupResult lookupResult,
            TrackRef trackRef,
            TrackPointer pTrack);

    TrackCacheResolver& operator=(TrackCacheResolver&&) = default;
};

class TrackCache {
public:
    static void createInstance(TrackCacheEvictor* pEvictor);
    static void destroyInstance();

    // Access the singular instance (singleton)
    static TrackCache& instance() {
        DEBUG_ASSERT(s_pInstance != nullptr);
        return *s_pInstance;
    }

    // Lookup an existing Track object in the cache.
    TrackCacheLocker lookupById(
            const TrackId& trackId) const;

    // Lookup an existing Track object in the cache or create
    // a temporary object on cache miss. The temporary object for
    // the file should be released before the cache is unlocked
    // to prevent concurrent file access.
    TrackCacheLocker lookupOrCreateTemporaryForFile(
            const QFileInfo& fileInfo,
            const SecurityTokenPointer& pSecurityToken = SecurityTokenPointer()) const;

    QList<TrackPointer> lookupAll() const;

    // Lookup an existing or create a new Track object
    TrackCacheResolver resolve(
            const QFileInfo& fileInfo,
            const SecurityTokenPointer& pSecurityToken = SecurityTokenPointer()) {
        return resolve(TrackId(), fileInfo, pSecurityToken);
    }
    TrackCacheResolver resolve(
            const TrackId& trackId,
            const QFileInfo& fileInfo,
            const SecurityTokenPointer& pSecurityToken = SecurityTokenPointer());

    void evictAll();

private:
    friend class TrackCacheLocker;
    friend class TrackCacheResolver;

    static TrackCache* volatile s_pInstance;

    static void deleter(Track* pTrack);

    class Item final {
    public:
        Item()
            : plainPtr(nullptr) {
        }
        Item(const TrackRef trackRef,
                const TrackPointer& pTrack)
            : ref(trackRef),
              weakPtr(pTrack),
              plainPtr(pTrack.get()) {
        }

        friend bool operator==(
            const Item& lhs,
            const Item& rhs) {
            return (lhs.ref.getId() == rhs.ref.getId()) &&
                    (lhs.ref.getCanonicalLocation() == rhs.ref.getCanonicalLocation()) &&
                    (lhs.plainPtr == rhs.plainPtr) &&
                    // std::weak_ptr does not provide operator==() so we need
                    // to implement it in terms of bidirectional owner_before()
                    // comparisons
                    !lhs.weakPtr.owner_before(rhs.weakPtr) &&
                    !rhs.weakPtr.owner_before(lhs.weakPtr);
        }
        friend bool operator!=(
            const Item& lhs,
            const Item& rhs) {
            return !(lhs == rhs);
        }

        TrackRef         ref;
        TrackWeakPointer weakPtr;
        Track* plainPtr;
    };

    explicit TrackCache(TrackCacheEvictor* pEvictor);
    ~TrackCache();

    // This function should only be called DEBUG_ASSERT statements
    // to verify the class invariants during development.
    bool verifyConsistency() const;

    TrackPointer lookupInternal(
            const TrackId& trackId) const;
    TrackPointer lookupInternal(
            const TrackRef& trackRef) const;

    bool resolveInternal(
            TrackCacheResolver* pCacheResolver,
            TrackRef* pTrackRef,
            const TrackId& trackId,
            const QFileInfo& fileInfo);

    TrackRef updateTrackIdInternal(
            const TrackPointer& pTrack,
            const TrackRef& trackRef,
            TrackId trackId);

    Item purgeInternal(
            const TrackRef& trackRef);

    void evict(
            Track* pTrack);
    Track* evictInternal(
            const TrackRef& trackRef);

    TrackCacheEvictor* m_pEvictor;

    mutable QMutex m_mutex;

    typedef QHash<TrackId, Item> TracksById;
    TracksById m_tracksById;
    typedef QMap<QString, Item> TracksByCanonicalLocation;
    TracksByCanonicalLocation m_tracksByCanonicalLocation;
};


#endif // TRACKCACHE_H_
