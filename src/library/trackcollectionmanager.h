#pragma once

#include <QDir>
#include <QList>
#include <QSet>
#include <memory>

#include "library/dao/directorydao.h"
#include "library/dao/genredao.h" // Added support for multiple genres
#include "preferences/usersettings.h"
#include "track/globaltrackcache.h"
#include "util/db/dbconnectionpool.h"
#include "util/parented_ptr.h"
#include "util/thread_affinity.h"

class LibraryScanner;
class TrackCollection;
class ExternalTrackCollection;
class RelocatedTrack;
struct LibraryScanResultSummary;

// Manages Mixxx's internal database of tracks as well as external track collections.
//
// All modifying operations that might affect external collections
// must be invoked through this class to keep all track collections
// synchronized!
//
// Both crates and playlists are currently only supported by the internal
// collection, which needs to be modified directly.
class TrackCollectionManager: public QObject,
    public virtual /*implements*/ GlobalTrackCacheSaver {
    Q_OBJECT

  public:
    TrackCollectionManager(
            QObject* parent,
            UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            deleteTrackFn_t deleteTrackForTestingFn = nullptr);
    ~TrackCollectionManager() override;

    TrackCollection* internalCollection() const {
        DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
        return m_pInternalCollection;
    }

    const QList<ExternalTrackCollection*>& externalCollections() const {
        DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
        return m_externalCollections;
    }

    // Access to GenreDAO for managing multiple genres
    GenreDAO* genreDAO() const {
        DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
        return m_pGenreDAO.get();
    }

    TrackPointer getTrackById(
            TrackId trackId) const;
    TrackPointer getTrackByRef(
            const TrackRef& trackRef) const;
    QList<TrackId> resolveTrackIds(
            const QList<mixxx::FileInfo>& fileInfos,
            QObject* pSource) const;
    QList<TrackId> resolveTrackIdsFromUrls(
            const QList<QUrl>& urls,
            bool addMissing) const;
    QList<TrackId> resolveTrackIdsFromLocations(
            const QList<QString>& locations) const;

    bool updateTrackGenre(
            Track* pTrack,
            const QString& genre) const;

    /// New methods for handling multiple genres
    /// Set multiple genres for a track (replaces previous ones)
    bool updateTrackGenres(
            Track* pTrack,
            const QStringList& genres) const;

    /// Adds genres to a track (keeps existing ones)
    bool addTrackGenres(
            Track* pTrack,
            const QStringList& genres) const;

    /// Removes specific genres from a track
    bool removeTrackGenres(
            Track* pTrack,
            const QStringList& genres) const;

    /// Gets all genres associated with a track
    QList<Genre> getTrackGenres(TrackId trackId) const;

    /// Ottiene tutti i generi disponibili nel database
    QList<Genre> getAllGenres() const;

    /// Cleans unused genres from the database
    int cleanupUnusedGenres() const;

#if defined(__EXTRA_METADATA__)
    bool updateTrackMood(
            Track* pTrack,
            const QString& mood) const;
#endif // __EXTRA_METADATA__

    bool hideTracks(const QList<TrackId>& trackIds) const;
    bool unhideTracks(const QList<TrackId>& trackIds) const;
    void hideAllTracks(const QDir& rootDir) const;

    void purgeTracks(const QList<TrackRef>& trackRefs) const;
    void purgeAllTracks(const QDir& rootDir) const;

    DirectoryDAO::AddResult addDirectory(const mixxx::FileInfo& newDir) const;
    DirectoryDAO::RemoveResult removeDirectory(const mixxx::FileInfo& oldDir) const;
    DirectoryDAO::RelocateResult relocateDirectory(
            const QString& oldDir, const QString& newDir) const;

    TrackPointer getOrAddTrack(
            const TrackRef& trackRef,
            bool* pAlreadyInLibrary = nullptr) const;

    // Save the track in both the internal database and external collections.
    // Export of metadata is deferred until the track is evicted from the
    // cache to prevent file corruption due to concurrent access.
    enum class SaveTrackResult {
        Saved,
        Skipped, // e.g. unmodified or missing/deleted tracks
        Failed,
    };
    SaveTrackResult saveTrack(const TrackPointer& pTrack) const;
    // Same as startLibraryScan() but don't emit the scan summary.
    void startLibraryAutoScan();

  signals:
    void libraryScanStarted();
    void libraryScanFinished();
    void libraryScanSummary(const LibraryScanResultSummary& result);

    /// New signals for genre management
    /// Emitted when a track's genres are changed
    void trackGenresUpdated(TrackId trackId, const QList<Genre>& genres);

    /// Fired when a new genre is added to the database
    void genreAdded(const Genre& genre);

    /// Emitted when unused genres are deleted
    void unusedGenresRemoved(int count);

  public slots:
    void startLibraryScan();
    void stopLibraryScan();

  private:
    void afterTrackAdded(const TrackPointer& pTrack) const;
    void afterTracksUpdated(const QSet<TrackId>& updatedTrackIds) const;
    void afterTracksRelocated(const QList<RelocatedTrack>& relocatedTracks) const;

    // Callback for GlobalTrackCache
    void saveEvictedTrack(Track* pTrack) noexcept override;

    // Might be called from any thread
    enum class TrackMetadataExportMode {
        Immediate,
        Deferred,
    };
    SaveTrackResult saveTrack(
            Track* pTrack,
            TrackMetadataExportMode mode) const;
    ExportTrackMetadataResult exportTrackMetadataBeforeSaving(
            Track* pTrack,
            TrackMetadataExportMode mode) const;

    /// Helper methods for genre management
    /// Converts a list of genre names to a list of IDs
    /// Creates new genres if necessary
    QList<DbId> convertGenreNamesToIds(const QStringList& genreNames) const;

    /// Converts a list of Genre structures into a list of names
    QStringList convertGenresToNames(const QList<Genre>& genres) const;

    const UserSettingsPointer m_pConfig;

    const parented_ptr<TrackCollection> m_pInternalCollection;

    // Added GenreDAO for multiple genre management
    std::unique_ptr<GenreDAO> m_pGenreDAO;

    QList<ExternalTrackCollection*> m_externalCollections;

    std::unique_ptr<LibraryScanner> m_pScanner;
};
