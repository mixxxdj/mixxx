#pragma once

#include <QDir>
#include <QList>
#include <QSet>
#include <memory>

#include "library/relocatedtrack.h"
#include "preferences/usersettings.h"
#include "track/globaltrackcache.h"
#include "util/db/dbconnectionpool.h"
#include "util/parented_ptr.h"
#include "util/thread_affinity.h"

class LibraryScanner;
class TrackCollection;
class ExternalTrackCollection;

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

    TrackCollection* internalCollection() {
        DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
        return m_pInternalCollection;
    }

    const QList<ExternalTrackCollection*>& externalCollections() const {
        DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
        return m_externalCollections;
    }

    bool hideTracks(const QList<TrackId>& trackIds);
    bool unhideTracks(const QList<TrackId>& trackIds);
    void hideAllTracks(const QDir& rootDir);

    void purgeTracks(const QList<TrackRef>& trackRefs);
    void purgeAllTracks(const QDir& rootDir);

    bool addDirectory(const QString& dir);
    bool removeDirectory(const QString& dir);
    void relocateDirectory(QString oldDir, QString newDir);

    TrackPointer getOrAddTrack(
            const TrackRef& trackRef,
            bool* pAlreadyInLibrary = nullptr);

    // Save the track in both the internal database and external collections.
    // Export of metadata is deferred until the track is evicted from the
    // cache to prevent file corruption due to concurrent access.
    // Returns true if the track was dirty and has been saved, otherwise
    // false.
    bool saveTrack(const TrackPointer& pTrack);

  signals:
    void libraryScanStarted();
    void libraryScanFinished();

  public slots:
    void startLibraryScan();
    void stopLibraryScan();

    void slotScanTrackAdded(TrackPointer pTrack);
    void slotScanTracksUpdated(QSet<TrackId> updatedTrackIds);
    void slotScanTracksRelocated(QList<RelocatedTrack> relocatedTracks);

  private:
    // Callback for GlobalTrackCache
    void saveEvictedTrack(Track* pTrack) noexcept override;

    // Might be called from any thread
    enum class TrackMetadataExportMode {
        Immediate,
        Deferred,
    };
    void saveTrack(
            Track* pTrack,
            TrackMetadataExportMode mode);
    void exportTrackMetadata(
            Track* pTrack,
            TrackMetadataExportMode mode) const;

    const UserSettingsPointer m_pConfig;

    const parented_ptr<TrackCollection> m_pInternalCollection;

    QList<ExternalTrackCollection*> m_externalCollections;

    // TODO: Extract and decouple LibraryScanner from TrackCollectionManager
    std::unique_ptr<LibraryScanner> m_pScanner;
};
