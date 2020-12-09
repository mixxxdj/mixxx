#pragma once

#include <QFileInfo>
#include <QObject>
#include <QSet>
#include <QList>
#include <QSqlDatabase>
#include <QString>

#include "preferences/usersettings.h"
#include "library/dao/dao.h"
#include "library/relocatedtrack.h"
#include "track/globaltrackcache.h"
#include "util/class.h"
#include "util/memory.h"

class FwdSqlQuery;
class SqlTransaction;
class PlaylistDAO;
class AnalysisDao;
class CueDAO;
class LibraryHashDAO;

class TrackDAO : public QObject, public virtual DAO, public virtual GlobalTrackCacheRelocator {
    Q_OBJECT
  public:

    enum class ResolveTrackIdFlag : int {
        ResolveOnly = 0,
        UnhideHidden = 1,
        AddMissing = 2
    };
    Q_DECLARE_FLAGS(ResolveTrackIdFlags, ResolveTrackIdFlag)

    // The 'config object' is necessary because users decide ID3 tags get
    // synchronized on track metadata change
    TrackDAO(
            CueDAO& cueDao,
            PlaylistDAO& playlistDao,
            AnalysisDao& analysisDao,
            LibraryHashDAO& libraryHashDao,
            UserSettingsPointer pConfig);
    ~TrackDAO() override;

    void finish();

    QList<TrackId> resolveTrackIds(
            const QList<TrackFile> &trackFiles,
            ResolveTrackIdFlags flags = ResolveTrackIdFlag::ResolveOnly);

    TrackId getTrackIdByRef(
            const TrackRef& trackRef) const;
    QList<TrackRef> getAllTrackRefs(
            const QDir& rootDir) const;

    TrackPointer getTrackByRef(
            const TrackRef& trackRef) const;

    // Returns a set of all track locations in the library.
    QSet<QString> getAllTrackLocations() const;
    QString getTrackLocation(TrackId trackId) const;

    // Only used by friend class LibraryScanner, but public for testing!
    bool detectMovedTracks(
            QList<RelocatedTrack>* pRelocatedTracks,
            const QStringList& addedTracks,
            volatile const bool* pCancel) const;

    // Only used by friend class TrackCollection, but public for testing!
    void saveTrack(Track* pTrack) const;

    /// Update the play counter properties according to the corresponding
    /// aggregated properties obtained from the played history.
    bool updatePlayCounterFromPlayedHistory(
            const QSet<TrackId>& trackIds) const;

  signals:
    // Forwarded from Track object
    void trackDirty(TrackId trackId);
    void trackClean(TrackId trackId);

    // Multiple tracks
    void tracksAdded(const QSet<TrackId>& trackIds);
    void tracksChanged(const QSet<TrackId>& trackIds);
    void tracksRemoved(const QSet<TrackId>& trackIds);

    void progressVerifyTracksOutside(const QString& path);
    void progressCoverArt(const QString& file);
    void forceModelUpdate();

  public slots:
    // Slots to inform the TrackDAO about changes that
    // have been applied directly to the database.
    void slotDatabaseTracksChanged(
            const QSet<TrackId>& changedTrackIds);
    void slotDatabaseTracksRelocated(
            const QList<RelocatedTrack>& relocatedTracks);

  private:
    friend class LibraryScanner;
    friend class TrackCollection;

    TrackId getTrackIdByLocation(
            const QString& location) const;
    TrackPointer getTrackById(
            TrackId trackId) const;

    // Loads a track from the database (by id if available, otherwise by location)
    // or adds it if not found in case the location is known. The (optional) out
    // parameter is set if the track has been found (-> true) or added (-> false).
    // Asynchronously imports cover art for newly added tracks. On failure a nullptr
    // is returned and pAlreadyInLibrary is left untouched.
    TrackPointer getOrAddTrack(
            const TrackRef& trackRef,
            bool* pAlreadyInLibrary = nullptr);

    void addTracksPrepare();
    TrackId addTracksAddTrack(
            const TrackPointer& pTrack,
            bool unremove);
    TrackPointer addTracksAddFile(
            const TrackFile& trackFile,
            bool unremove);
    void addTracksFinish(bool rollback = false);

    bool updateTrack(Track* pTrack) const;

    void hideAllTracks(const QDir& rootDir) const;

    bool hideTracks(
            const QList<TrackId>& trackIds) const;
    void afterHidingTracks(
            const QList<TrackId>& trackIds);

    bool unhideTracks(
            const QList<TrackId>& trackIds) const;
    void afterUnhidingTracks(
            const QList<TrackId>& trackIds);

    bool onPurgingTracks(
            const QList<TrackId>& trackIds) const;
    void afterPurgingTracks(
            const QList<TrackId>& trackIds);

    // Scanning related calls.
    void markTrackLocationsAsVerified(const QStringList& locations) const;
    void markTracksInDirectoriesAsVerified(const QStringList& directories) const;
    void invalidateTrackLocationsInLibrary() const;
    void markUnverifiedTracksAsDeleted();

    bool verifyRemainingTracks(
            const QStringList& libraryRootDirs,
            volatile const bool* pCancel);

    void detectCoverArtForTracksWithoutCover(volatile const bool* pCancel,
                                        QSet<TrackId>* pTracksChanged);

    // Callback for GlobalTrackCache
    TrackFile relocateCachedTrack(
            TrackId trackId,
            TrackFile fileInfo) override;

    CueDAO& m_cueDao;
    PlaylistDAO& m_playlistDao;
    AnalysisDao& m_analysisDao;
    LibraryHashDAO& m_libraryHashDao;

    const UserSettingsPointer m_pConfig;

    std::unique_ptr<QSqlQuery> m_pQueryTrackLocationInsert;
    std::unique_ptr<QSqlQuery> m_pQueryTrackLocationSelect;
    std::unique_ptr<QSqlQuery> m_pQueryLibraryInsert;
    std::unique_ptr<QSqlQuery> m_pQueryLibraryUpdate;
    std::unique_ptr<QSqlQuery> m_pQueryLibrarySelect;
    std::unique_ptr<SqlTransaction> m_pTransaction;
    int m_trackLocationIdColumn;
    int m_queryLibraryIdColumn;
    int m_queryLibraryMixxxDeletedColumn;

    QSet<TrackId> m_tracksAddedSet;

    DISALLOW_COPY_AND_ASSIGN(TrackDAO);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TrackDAO::ResolveTrackIdFlags)
