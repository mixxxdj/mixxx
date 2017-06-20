#ifndef TRACKDAO_H
#define TRACKDAO_H

#include <QFileInfo>
#include <QObject>
#include <QSet>
#include <QHash>
#include <QList>
#include <QSqlDatabase>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QCache>
#include <QString>

#include "preferences/usersettings.h"
#include "library/dao/dao.h"
#include "track/track.h"
#include "util/class.h"
#include "util/memory.h"

class SqlTransaction;
class PlaylistDAO;
class AnalysisDao;
class CueDAO;
class LibraryHashDAO;

// Holds a strong reference to a track while it is in the "recent tracks"
// cache. Once it expires from the cache it signals to
// TrackDAO::saveTrack(TrackPointer) to save the track if it is dirty and then
// drops the strong reference. This prevents a race condition caused by caching
// TrackPointers themselves within the QCache by holding a strong reference to
// TrackPointer (and thereby serving it out of the weak pointer track cache) up
// until the track has been saved to the database.
class RecentTrackCacheItem : public QObject {
    Q_OBJECT
  public:
    explicit RecentTrackCacheItem(
            const TrackPointer& pTrack);
    virtual ~RecentTrackCacheItem();

    const TrackPointer& getTrack() const {
        return m_pTrack;
    }

  signals:
    void saveTrack(TrackPointer pTrack);

  private:
    TrackPointer m_pTrack;
};

class TrackDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    // The 'config object' is necessary because users decide ID3 tags get
    // synchronized on track metadata change
    TrackDAO(
            CueDAO& cueDao,
            PlaylistDAO& playlistDao,
            AnalysisDao& analysisDao,
            LibraryHashDAO& libraryHashDao,
            UserSettingsPointer pConfig);
    ~TrackDAO() override;

    void initialize(const QSqlDatabase& database) override {
        m_database = database;
    }
    void finish();

    TrackId getTrackId(const QString& absoluteFilePath);
    QList<TrackId> getTrackIds(const QList<QFileInfo>& files);
    QList<TrackId> getTrackIds(const QDir& dir);

    bool trackExistsInDatabase(const QString& absoluteFilePath);

    // WARNING: Only call this from the main thread instance of TrackDAO.
    TrackPointer getTrack(TrackId trackId, const bool cacheOnly=false) const;

    // Returns a set of all track locations in the library.
    QSet<QString> getTrackLocations();
    QString getTrackLocation(TrackId trackId);

    TrackPointer addSingleTrack(const QFileInfo& fileInfo, bool unremove);
    QList<TrackId> addMultipleTracks(const QList<QFileInfo>& fileInfoList, bool unremove);

    void addTracksPrepare();
    TrackPointer addTracksAddFile(const QFileInfo& fileInfo, bool unremove);
    TrackId addTracksAddTrack(const TrackPointer& pTrack, bool unremove);
    void addTracksFinish(bool rollback = false);

    bool onHidingTracks(
            const QList<TrackId>& trackIds);
    void afterHidingTracks(
            const QList<TrackId>& trackIds);

    bool onUnhidingTracks(
            const QList<TrackId>& trackIds);
    void afterUnhidingTracks(
            const QList<TrackId>& trackIds);

    bool onPurgingTracks(
            const QList<TrackId>& trackIds);
    void afterPurgingTracks(
            const QList<TrackId>& trackIds);

    // Fetches trackLocation from the database or adds it. If searchForCoverArt
    // is true, searches the track and its directory for cover art via
    // asynchronous request to CoverArtCache. If adding or fetching the track
    // fails, returns a transient TrackPointer for trackLocation. If
    // pAlreadyInLibrary is non-NULL, sets it to whether trackLocation was
    // already in the database.
    TrackPointer getOrAddTrack(const QString& trackLocation,
                               bool processCoverArt,
                               bool* pAlreadyInLibrary);

    void markTracksAsMixxxDeleted(const QString& dir);

    // Scanning related calls. Should be elsewhere or private somehow.
    void markTrackLocationsAsVerified(const QStringList& locations);
    void markTracksInDirectoriesAsVerified(const QStringList& directories);
    void invalidateTrackLocationsInLibrary();
    void markUnverifiedTracksAsDeleted();
    void markTrackLocationsAsDeleted(const QString& directory);
    bool detectMovedTracks(QSet<TrackId>* pTracksMovedSetOld,
                          QSet<TrackId>* pTracksMovedSetNew,
                          const QStringList& addedTracks,
                          volatile const bool* pCancel);

    bool verifyRemainingTracks(
            const QStringList& libraryRootDirs,
            volatile const bool* pCancel);

    void detectCoverArtForTracksWithoutCover(volatile const bool* pCancel,
                                        QSet<TrackId>* pTracksChanged);

  signals:
    void trackDirty(TrackId trackId) const;
    void trackClean(TrackId trackId) const;
    void trackChanged(TrackId trackId);
    void tracksAdded(QSet<TrackId> trackIds);
    void tracksRemoved(QSet<TrackId> trackIds);
    void dbTrackAdded(TrackPointer pTrack);
    void progressVerifyTracksOutside(QString path);
    void progressCoverArt(QString file);
    void forceModelUpdate();

  public slots:
    // The public interface to the TrackDAO requires a TrackPointer so that we
    // have a guarantee that the track will not be deleted while we are working
    // on it. However, private parts of TrackDAO can use the raw saveTrack(TIO*)
    // call.
    void saveTrack(const TrackPointer& pTrack);

    // Clears the cached Tracks, which can be useful when the
    // underlying database tables change (eg. during a library rescan,
    // we might detect that a track has been moved and modify the update
    // the tables directly.)
    void clearCache();

    void databaseTrackAdded(TrackPointer pTrack);
    void databaseTracksMoved(QSet<TrackId> tracksMovedSetOld, QSet<TrackId> tracksMovedSetNew);
    void databaseTracksChanged(QSet<TrackId> tracksChanged);

  private slots:
    void slotTrackDirty(Track* pTrack);
    void slotTrackChanged(Track* pTrack);
    void slotTrackClean(Track* pTrack);
    void slotTrackReferenceExpired(Track* pTrack);

  private:
    TrackPointer getTrackFromDB(TrackId trackId) const;

    void saveTrack(Track* pTrack);
    bool updateTrack(Track* pTrack);

    QSqlDatabase m_database;

    CueDAO& m_cueDao;
    PlaylistDAO& m_playlistDao;
    AnalysisDao& m_analysisDao;
    LibraryHashDAO& m_libraryHashDao;

    UserSettingsPointer m_pConfig;
    // Mutex that protects m_sTracks.
    static QMutex m_sTracksMutex;
    // Weak pointer cache of active tracks.
    static QHash<TrackId, TrackWeakPointer> m_sTracks;

    void cacheRecentTrack(
            TrackId trackId,
            const TrackPointer& pTrack) const;

    // "Recent tracks" cache -- holds strong references to recently used
    // tracks. When a track is expired, calls saveTrack(TrackPointer) without
    // dropping the strong reference to the track. This prevents a race
    // condition where the strong reference is dropped and therefore cache-only
    // getTrack calls made by BaseSqlTableModel return null and serve stale
    // results from BaseTrackCache before the newly expired TrackPointer has
    // been saved to the database.
    mutable QCache<TrackId, RecentTrackCacheItem> m_recentTracksCache;

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

#endif //TRACKDAO_H
