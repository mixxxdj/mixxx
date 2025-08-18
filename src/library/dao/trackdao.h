#pragma once

#include <QList>
#include <QObject>
#include <QSet>
#include <QString>
#include <memory>

#include "library/dao/dao.h"
#include "library/relocatedtrack.h"
#include "preferences/usersettings.h"
#include "track/globaltrackcache.h"
#include "util/class.h"

class SqlTransaction;
class PlaylistDAO;
class AnalysisDao;
class GenreDao;
class CueDAO;
class LibraryHashDAO;

namespace mixxx {
class FileInfo;
class TrackRecord;

} // namespace mixxx

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
            GenreDao& genreDao,
            LibraryHashDAO& libraryHashDao,
            UserSettingsPointer pConfig);
    ~TrackDAO() override;

    void finish();

    QList<TrackId> resolveTrackIds(
            const QList<QUrl>& urls,
            ResolveTrackIdFlags flags = ResolveTrackIdFlag::ResolveOnly);
    QList<TrackId> resolveTrackIds(
            const QList<mixxx::FileInfo>& fileInfos,
            ResolveTrackIdFlags flags = ResolveTrackIdFlag::ResolveOnly);

    QList<TrackRef> getAllTrackRefs(
            const QDir& rootDir) const;

    TrackPointer getTrackByRef(
            const TrackRef& trackRef) const;

    // Returns a set of all track locations in the library,
    // incl. locations of tracks currently marked as missing.
    QSet<QString> getAllTrackLocations() const;
    // Return only tracks that are reported to exist during last scan.
    QSet<QString> getAllExistingTrackLocations() const;
    // Return all tracks reported missing during last scan.
    QSet<QString> getAllMissingTrackLocations() const;
    QString getTrackLocation(TrackId trackId) const;

    // Only used by friend class LibraryScanner, but public for testing!
    bool detectMovedTracks(
            QList<RelocatedTrack>* pRelocatedTracks,
            const QStringList& addedTracks,
            volatile const bool* pCancel) const;

    // Only used by friend class TrackCollection, but public for testing!
    bool saveTrack(Track* pTrack) const;

    /// Update the play counter properties according to the corresponding
    /// aggregated properties obtained from the played history.
    bool updatePlayCounterFromPlayedHistory(
            const QSet<TrackId>& trackIds) const;

    /// Don't use even if public!!! Ugly workaround for C++ visibility restrictions.
    /// This method is invoked by a free function that needs to access
    /// a private Track member that only TrackDAO is allowed to access
    /// as a friend.
    static void setTrackGenreInternal(Track* pTrack, const QString& genre);
    /// Don't use even if public!!! Ugly workaround for C++ visibility restrictions.
    /// This method is invoked by a free function that needs to access
    /// a private TrackRecord member that only TrackDAO is allowed to
    /// access as a friend.
    static void setTrackHeaderParsedInternal(Track* pTrack, bool headerParsed);
    /// Don't use even if public!!! Ugly workaround for C++ visibility restrictions.
    /// This method is invoked by a free function that needs to access
    /// private TrackRecord member that only TrackDAO is allowed to
    /// access as a friend.
    static bool getTrackHeaderParsedInternal(const mixxx::TrackRecord& trackRecord);

    /// Lookup and load a track by URL.
    ///
    /// Only local file URLs are supported.
    ///
    /// Returns `nullptr` if no track matches the given URL.
    TrackPointer getTrackByUrl(const QUrl& url) const {
        return getTrackByRef(TrackRef::fromUrl(url));
    }

  signals:
    // Forwarded from Track object
    void trackDirty(TrackId trackId);
    void trackClean(TrackId trackId);

    // Multiple tracks
    void tracksAdded(const QSet<TrackId>& trackIds);
    void tracksChanged(const QSet<TrackId>& trackIds);
    void tracksRemoved(const QSet<TrackId>& trackIds);
    void waveformSummaryUpdated(const TrackId trackId);

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
    friend class TrackAnalysisScheduler;

    QList<TrackId> resolveTrackIds(
            const QStringList& pathList,
            ResolveTrackIdFlags flags = ResolveTrackIdFlag::ResolveOnly);

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
            const QString& filePath,
            bool unremove);
    void addTracksFinish(bool rollback = false);

    bool updateTrack(const Track& track) const;

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
    void cleanupTrackLocationsDirectory() const;
    void invalidateTrackLocationsInLibrary() const;
    void markUnverifiedTracksAsDeleted();

    bool verifyRemainingTracks(
            const QList<mixxx::FileInfo>& libraryRootDirs,
            volatile const bool* pCancel);

    void detectCoverArtForTracksWithoutCover(volatile const bool* pCancel,
                                        QSet<TrackId>* pTracksChanged);

    // Callback for GlobalTrackCache
    mixxx::FileAccess relocateCachedTrack(TrackId trackId) override;

    CueDAO& m_cueDao;
    PlaylistDAO& m_playlistDao;
    AnalysisDao& m_analysisDao;
    GenreDao& m_genreDao;
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
