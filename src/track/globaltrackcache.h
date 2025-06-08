#pragma once

#include <QWaitCondition>
#include <map>
#include <unordered_map>

#include "track/track_decl.h"
#include "track/trackref.h"
#include "util/compatibility/qmutex.h"
#include "util/fileaccess.h"

// forward declaration(s)
class GlobalTrackCache;

enum class GlobalTrackCacheLookupResult {
    None,
    Hit,
    Miss,
    ConflictCanonicalLocation
};

// Find the updated location of a track in the database when
// the canonical location is no longer valid or accessible.
class /*interface*/ GlobalTrackCacheRelocator {
private:
    friend class GlobalTrackCache;
    // Try to determine and return the relocated file info
    // or otherwise return just the provided file info.
    virtual mixxx::FileAccess relocateCachedTrack(TrackId trackId) = 0;

  protected:
    virtual ~GlobalTrackCacheRelocator() = default;
};

typedef void (*deleteTrackFn_t)(Track*);

class GlobalTrackCacheEntry final {
    // We need to hold two shared pointers, the deletingPtr is
    // responsible for the lifetime of the Track object itself.
    // The second one counts the references outside Mixxx, if it
    // is not longer referenced, the track is saved and evicted
    // from the cache.
  public:
    class TrackDeleter {
    public:
        explicit TrackDeleter(deleteTrackFn_t deleteTrackFn = nullptr)
                : m_deleteTrackFn(deleteTrackFn) {
        }

        void operator()(Track* pTrack) const;

    private:
        deleteTrackFn_t m_deleteTrackFn;
    };

    explicit GlobalTrackCacheEntry(
            std::unique_ptr<Track, TrackDeleter> deletingPtr)
        : m_deletingPtr(std::move(deletingPtr)) {
    }
    GlobalTrackCacheEntry(const GlobalTrackCacheEntry& other) = delete;
    GlobalTrackCacheEntry(GlobalTrackCacheEntry&&) = default;

    void init(TrackWeakPointer savingWeakPtr) {
        // Uninitialized or expired
        DEBUG_ASSERT(!m_savingWeakPtr.lock());
        m_savingWeakPtr = std::move(savingWeakPtr);
    }

    Track* getPlainPtr() const {
        return m_deletingPtr.get();
    }

    TrackPointer lock() const {
        return m_savingWeakPtr.lock();
    }
    bool expired() const {
        return m_savingWeakPtr.expired();
    }

  private:
    std::unique_ptr<Track, TrackDeleter> m_deletingPtr;
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
    QSet<TrackId> getCachedTrackIds() const;

  private:
    void lockCache();

  protected:
    GlobalTrackCache* m_pInstance;
};

class GlobalTrackCacheResolver final: public GlobalTrackCacheLocker {
public:
  GlobalTrackCacheResolver(
          mixxx::FileAccess fileAccess,
          bool temporary = false);
  GlobalTrackCacheResolver(
          mixxx::FileAccess fileAccess,
          TrackId trackId);
  GlobalTrackCacheResolver(const GlobalTrackCacheResolver&) = delete;
  GlobalTrackCacheResolver() = delete;
  GlobalTrackCacheResolver(GlobalTrackCacheResolver&&) = default;
  ~GlobalTrackCacheResolver() override;

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

    void initLookupResult(
            GlobalTrackCacheLookupResult lookupResult,
            TrackPointer&& strongPtr,
            TrackRef&& trackRef);

    GlobalTrackCacheLookupResult m_lookupResult;

    TrackPointer m_strongPtr;

    TrackRef m_trackRef;
};

/// Callback interface for pre-delete actions
class /*interface*/ GlobalTrackCacheSaver {
private:
    friend class GlobalTrackCache;

    /// Perform actions that are necessary to save any pending
    /// modifications of a Track object before it finally gets
    /// deleted.
    ///
    /// GlobalTrackCache ensures that the given pointer is valid
    /// and the last and only reference to this Track object.
    /// While invoked the GlobalTrackCache is locked to ensure
    /// that this particular track is not accessible while
    /// saving the Track object, e.g. by updating the database
    /// and exporting file tags.
    ///
    /// This callback method will always be invoked from the
    /// event loop thread of the owning GlobalTrackCache instance.
    /// Typically the GlobalTrackCache lives on the main thread
    /// that also controls access to the database.
    /// NOTE(2020-06-06): If these assumptions about thread affinity
    /// are no longer valid the design decisions need to be revisited
    /// carefully!
    virtual void saveEvictedTrack(
            Track* pEvictedTrack) noexcept = 0;

  protected:
    virtual ~GlobalTrackCacheSaver() = default;
};

class GlobalTrackCache : public QObject {
    Q_OBJECT

  public:
    static void createInstance(
            GlobalTrackCacheSaver* pSaver,
            // A custom deleter is only needed for tests without an event loop!
            deleteTrackFn_t deleteTrackFn = nullptr);
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
    void slotEvictAndSave(GlobalTrackCacheEntryPointer cacheEntryPtr);

  private:
    friend class GlobalTrackCacheLocker;
    friend class GlobalTrackCacheResolver;

    GlobalTrackCache(
            GlobalTrackCacheSaver* pSaver,
            deleteTrackFn_t deleteTrackFn);
    ~GlobalTrackCache() override;

    void relocateTracks(
            GlobalTrackCacheRelocator* /*nullable*/ pRelocator);

    TrackPointer lookupById(
            const TrackId& trackId);
    TrackPointer lookupByCanonicalLocation(
            const QString& canonicalLocation);

    /// Lookup the track either first by id or afterwards by canonical location.
    /// If a track with a different ID and the same canonical location
    /// is already cached, a nullptr is returned.
    TrackPointer lookupByRef(
            const TrackRef& trackRef);

    QSet<TrackId> getCachedTrackIds() const;

    TrackPointer revive(GlobalTrackCacheEntryPointer entryPtr);

    void resolve(
            GlobalTrackCacheResolver* /*in/out*/ pCacheResolver,
            mixxx::FileAccess fileAccess,
            TrackId trackId);

    void resolveTemporary(
            GlobalTrackCacheResolver* /*in/out*/ pCacheResolver,
            mixxx::FileAccess fileAccess);

    TrackRef initTrackId(
            const TrackPointer& strongPtr,
            const TrackRef& trackRef,
            TrackId trackId);

    void discardIncompleteTrack();

    void purgeTrackId(TrackId trackId);

    bool tryEvict(Track* plainPtr);
    bool isCached(Track* plainPtr) const;

    bool isEmpty() const;

    void deactivate();

    void saveEvictedTrack(Track* pEvictedTrack) const;

    // Managed by GlobalTrackCacheLocker
    mutable QMutex m_mutex;

    GlobalTrackCacheSaver* m_pSaver;

    deleteTrackFn_t m_deleteTrackFn;

    // Managed by GlobalTrackCacheResolver.
    // The track that is currently locked by the asynchronous metadata loader background thread.
    // m_isTrackCompleted will be signaled once the asynchronous loading has been completed.
    TrackPointer m_incompleteTrack;
    QWaitCondition m_isTrackCompleted;

    // This caches the unsaved Tracks by ID
    typedef std::unordered_map<TrackId, GlobalTrackCacheEntryPointer, TrackId::hash_fun_t> TracksById;
    TracksById m_tracksById;

    // This caches the unsaved Tracks by location
    typedef std::map<QString, GlobalTrackCacheEntryPointer> TracksByCanonicalLocation;
    TracksByCanonicalLocation m_tracksByCanonicalLocation;
};
