#pragma once


#include <set>

#include <QObject>
#include <QHash>
#include <QList>
#include <QMap>

#include "track/track.h"
#include "track/trackref.h"


class TrackRefPtr: public TrackPointer {
  public:
    TrackRefPtr() = default;
    TrackRefPtr(const TrackRefPtr&) = default;
#if !defined(_MSC_VER) || _MSC_VER > 1900
    TrackRefPtr(TrackRefPtr&&) = default;
#else
    // Workaround for Visual Studio 2015 (and before)
    TrackRefPtr(TrackRefPtr&& that)
        : TrackPointer(std::move(that)),
          m_ref(std::move(that.m_ref)) {
    }
#endif
    TrackRefPtr(TrackPointer ptr, TrackRef ref)
        : TrackPointer(std::move(ptr)),
          m_ref(std::move(ref)) {
    }

    TrackRefPtr& operator=(const TrackRefPtr&) = default;
#if !defined(_MSC_VER) || _MSC_VER > 1900
    TrackRefPtr& operator=(TrackRefPtr&&) = default;
#else
    // Workaround for Visual Studio 2015 (and before)
    TrackRefPtr& operator=(TrackRefPtr&& that) {
        TrackPointer::operator=(std::move(that));
        m_ref = std::move(that.m_ref);
        return *this;
    }
#endif

    const TrackRef& ref() const {
        return m_ref;
    }

  private:
    TrackRef m_ref;
};

enum class GlobalTrackCacheLookupResult {
    NONE,
    HIT,
    MISS
};

class GlobalTrackCacheLocker {
public:
    GlobalTrackCacheLocker(const GlobalTrackCacheLocker&) = delete;
    GlobalTrackCacheLocker(GlobalTrackCacheLocker&&);
    virtual ~GlobalTrackCacheLocker();

    GlobalTrackCacheLocker& operator=(const GlobalTrackCacheLocker&) = delete;
    GlobalTrackCacheLocker& operator=(GlobalTrackCacheLocker&&) = delete;

    void unlockCache();

private:
    friend class GlobalTrackCache;

    void lockCache();

protected:
    GlobalTrackCacheLocker();
    GlobalTrackCacheLocker(
            GlobalTrackCacheLocker&& moveable,
            GlobalTrackCacheLookupResult lookupResult,
            TrackRefPtr trackRefPtr);

    QMutex* m_pCacheMutex;
};

class GlobalTrackCacheResolver final: public GlobalTrackCacheLocker {
public:
    GlobalTrackCacheResolver(const GlobalTrackCacheResolver&) = delete;
#if !defined(_MSC_VER) && (_MSC_VER > 1900)
    GlobalTrackCacheResolver(GlobalTrackCacheResolver&&) = default;
#else
    // Visual Studio 2015 does not support default generated move constructors
    GlobalTrackCacheResolver(GlobalTrackCacheResolver&& moveable)
        : GlobalTrackCacheLocker(std::move(moveable)),
          m_lookupResult(std::move(moveable.m_lookupResult)),
          m_trackRefPtr(std::move(moveable.m_trackRefPtr)) {
    }
#endif

    GlobalTrackCacheLookupResult getLookupResult() const {
        return m_lookupResult;
    }

    operator const TrackRefPtr&() const {
        return m_trackRefPtr;
    }

    void initTrackIdAndUnlockCache(TrackId trackId);

    GlobalTrackCacheResolver& operator=(const GlobalTrackCacheResolver&) = delete;
    GlobalTrackCacheResolver& operator=(GlobalTrackCacheResolver&&) = delete;

private:
    friend class GlobalTrackCache;
    GlobalTrackCacheResolver();

    void initLookupResult(
            GlobalTrackCacheLookupResult lookupResult,
            TrackRefPtr trackRefPtr);

    GlobalTrackCacheLookupResult m_lookupResult;

    TrackRefPtr m_trackRefPtr;
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
        DEBUG_ASSERT(s_pInstance);
        return *s_pInstance;
    }

    bool isActive() {
        return m_pEvictor != nullptr;
    }
    void deactivate();

    // Lookup an existing Track object in the cache.
    TrackRefPtr lookupById(
            const TrackId& trackId);

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

    bool isEmpty() const;

    // Reset all indices but keep the currently allocated tracks
    // to prevent memory leaks.
    void resetIndices();

private:
    friend class GlobalTrackCacheLocker;
    friend class GlobalTrackCacheResolver;

    static GlobalTrackCache* volatile s_pInstance;

    // Callback for the smart-pointer
    static void deleter(Track* pTrack);

    class Item final {
    public:
        Item()
            : plainPtr(nullptr) {
        }
        Item(TrackRef trackRef,
                const TrackPointer& pTrack)
            : ref(std::move(trackRef)),
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

    TrackRefPtr lookupByIdInternal(
            const TrackId& trackId);
    TrackRefPtr lookupByRefInternal(
            const TrackRef& trackRef);

    TrackRefPtr reviveInternal(
            const Item& item);

    bool resolveInternal(
            GlobalTrackCacheResolver* pCacheResolver,
            TrackRef* pTrackRef,
            const TrackId& trackId,
            const QFileInfo& fileInfo);

    TrackRefPtr initTrackIdInternal(
            TrackRefPtr pTrackRef,
            TrackId trackId);

    bool evictAndDelete(
            Track* pTrack);

    typedef std::set<Track*> AllocatedTracks;

    bool evictAndDeleteInternal(
            GlobalTrackCacheLocker* /*nullable*/ pCacheLocker,
            AllocatedTracks::iterator ipIndexedTrack,
            bool evictUnexpired);
    bool evictInternal(
            const TrackRef& trackRef,
            AllocatedTracks::iterator ipIndexedTrack,
            bool evictUnexpired);

    void afterEvicted(
            GlobalTrackCacheLocker* /*nullable*/ pCacheLocker,
            Track* pEvictedTrack);

    bool isEmptyInternal() const;

    void deactivateInternal();

    GlobalTrackCacheEvictor* m_pEvictor;

    mutable QMutex m_mutex;

    AllocatedTracks m_indexedTracks;
    AllocatedTracks m_unindexedTracks;

    typedef QHash<TrackId, Item> TracksById;
    TracksById m_tracksById;
    typedef QMap<QString, Item> TracksByCanonicalLocation;
    TracksByCanonicalLocation m_tracksByCanonicalLocation;
};
