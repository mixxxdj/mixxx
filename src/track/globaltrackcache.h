#pragma once


#include <QObject>
#include <QHash>
#include <QList>
#include <QMap>

#include "track/track.h"
#include "track/trackref.h"


enum class GlobalTrackCacheLookupResult {
    NONE,
    HIT,
    MISS
};

class GlobalTrackCacheLocker {
public:
    GlobalTrackCacheLocker(const GlobalTrackCacheLocker&) = delete;
    GlobalTrackCacheLocker(GlobalTrackCacheLocker&& moveable);
    virtual ~GlobalTrackCacheLocker();

    GlobalTrackCacheLookupResult getGlobalTrackCacheLookupResult() const {
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

    GlobalTrackCacheLocker& operator=(const GlobalTrackCacheLocker&) = delete;

private:
    friend class GlobalTrackCache;

    void lockCache();

protected:
    GlobalTrackCacheLocker();
    GlobalTrackCacheLocker(
            GlobalTrackCacheLocker&& moveable,
            GlobalTrackCacheLookupResult lookupResult,
            TrackRef trackRef,
            TrackPointer pTrack);

    GlobalTrackCacheLocker& operator=(GlobalTrackCacheLocker&&);

    QMutex* m_pCacheMutex;

    GlobalTrackCacheLookupResult m_lookupResult;

    TrackRef m_trackRef;
    TrackPointer m_pTrack;
};

class GlobalTrackCacheResolver final: public GlobalTrackCacheLocker {
public:
    GlobalTrackCacheResolver(const GlobalTrackCacheResolver&) = delete;
#if defined(_MSC_VER) && (_MSC_VER <= 1900)
    // Visual Studio 2015 does not support default generated move constructors
    GlobalTrackCacheResolver(GlobalTrackCacheResolver&& moveable)
        : GlobalTrackCacheLocker(std::move(moveable)) {
    }
#else
    GlobalTrackCacheResolver(GlobalTrackCacheResolver&&) = default;
#endif

    void updateTrackId(TrackId trackId);

    GlobalTrackCacheResolver& operator=(const GlobalTrackCacheResolver&) = delete;

private:
    friend class GlobalTrackCache;
    GlobalTrackCacheResolver();
    GlobalTrackCacheResolver(
            GlobalTrackCacheResolver&& moveable,
            GlobalTrackCacheLookupResult lookupResult,
            TrackRef trackRef,
            TrackPointer pTrack);

#if defined(_MSC_VER) && (_MSC_VER <= 1900)
    // Visual Studio 2015 does not support default generated move assignment operators
    GlobalTrackCacheResolver& operator=(GlobalTrackCacheResolver&& moveable) {
        GlobalTrackCacheLocker::operator=(std::move(moveable));
        return *this;
    }
#else
    GlobalTrackCacheResolver& operator=(GlobalTrackCacheResolver&&) = default;
#endif
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
    virtual void onEvictingTrackFromCache(
            GlobalTrackCacheLocker* /*nullable*/ pCacheLocker,
            Track* pTrack) = 0; // not null

protected:
    virtual ~GlobalTrackCacheEvictor() {}
};

class GlobalTrackCache {
public:
    static void createInstance(GlobalTrackCacheEvictor* pEvictor);
    static void destroyInstance();

    // Access the singular instance (singleton)
    static GlobalTrackCache& instance() {
        DEBUG_ASSERT(s_pInstance != nullptr);
        return *s_pInstance;
    }

    // Lookup an existing Track object in the cache.
    //
    // NOTE: The GlobalTrackCache is locked during the lifetime of the
    // result object. It should be destroyed ASAP to reduce lock
    // contention!
    GlobalTrackCacheLocker lookupById(
            const TrackId& trackId) const;

    QList<TrackPointer> lookupAll() const;

    // Lookup an existing or create a new Track object.
    //
    // NOTE: The GlobalTrackCache is locked during the lifetime of the
    // result object. It should be destroyed ASAP to reduce lock
    // contention!
    GlobalTrackCacheResolver resolve(
            const QFileInfo& fileInfo,
            const SecurityTokenPointer& pSecurityToken = SecurityTokenPointer()) {
        return resolve(TrackId(), fileInfo, pSecurityToken);
    }
    GlobalTrackCacheResolver resolve(
            const TrackId& trackId,
            const QFileInfo& fileInfo,
            const SecurityTokenPointer& pSecurityToken = SecurityTokenPointer());

    void evictAll();

private:
    friend class GlobalTrackCacheLocker;
    friend class GlobalTrackCacheResolver;

    static GlobalTrackCache* volatile s_pInstance;

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

    explicit GlobalTrackCache(GlobalTrackCacheEvictor* pEvictor);
    ~GlobalTrackCache();

    // This function should only be called DEBUG_ASSERT statements
    // to verify the class invariants during development.
    bool verifyConsistency() const;

    TrackPointer lookupInternal(
            const TrackId& trackId) const;
    TrackPointer lookupInternal(
            const TrackRef& trackRef) const;

    bool resolveInternal(
            GlobalTrackCacheResolver* pCacheResolver,
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
            GlobalTrackCacheLocker* /*nullable*/ pCacheLocker,
            const TrackRef& trackRef);

    GlobalTrackCacheEvictor* m_pEvictor;

    mutable QMutex m_mutex;

    typedef QHash<TrackId, Item> TracksById;
    TracksById m_tracksById;
    typedef QMap<QString, Item> TracksByCanonicalLocation;
    TracksByCanonicalLocation m_tracksByCanonicalLocation;
};
