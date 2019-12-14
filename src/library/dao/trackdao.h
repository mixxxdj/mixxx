#ifndef TRACKDAO_H
#define TRACKDAO_H

#include <QFileInfo>
#include <QObject>
#include <QSet>
#include <QList>
#include <QSqlDatabase>
#include <QString>

#include "preferences/usersettings.h"
#include "library/dao/dao.h"
#include "track/globaltrackcache.h"
#include "util/class.h"
#include "util/memory.h"

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
            const UserSettingsPointer& pConfig);
    ~TrackDAO() override;

    void initialize(const QSqlDatabase& database) override {
        m_database = database;
    }
    void finish();

    TrackId getTrackId(const QString& absoluteFilePath) const;
    QList<TrackId> resolveTrackIds(const QList<QFileInfo> &files,
            ResolveTrackIdFlags flags);
    QList<TrackId> getAllTrackIds(const QDir& rootDir);

    // Returns a set of all track locations in the library.
    QSet<QString> getTrackLocations();
    QString getTrackLocation(TrackId trackId);
    QStringList getTrackLocations(const QList<TrackId>& trackIds);

    // Only used by friend class LibraryScanner, but public for testing!
    bool detectMovedTracks(QList<QPair<TrackRef, TrackRef>>* pReplacedTracks,
                          const QStringList& addedTracks,
                          volatile const bool* pCancel);

    // Only used by friend class TrackCollection, but public for testing!
    void saveTrack(Track* pTrack);

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
    void databaseTrackAdded(TrackPointer pTrack);
    void databaseTracksChanged(QSet<TrackId> tracksChanged);
    void databaseTracksReplaced(QList<QPair<TrackRef, TrackRef>> replacedTracks);

  private slots:
    void slotTrackDirty(Track* pTrack);
    void slotTrackChanged(Track* pTrack);
    void slotTrackClean(Track* pTrack);

  private:
    friend class LibraryScanner;
    friend class TrackCollection;

    TrackPointer getTrackById(TrackId trackId) const;

    // Fetches trackLocation from the database or adds it. If searchForCoverArt
    // is true, searches the track and its directory for cover art via
    // asynchronous request to CoverArtCache. If adding or fetching the track
    // fails, returns a transient TrackPointer for trackLocation. If
    // pAlreadyInLibrary is non-NULL, sets it to whether trackLocation was
    // already in the database.
    TrackPointer getOrAddTrackByLocation(
            const QString& trackLocation,
            bool* pAlreadyInLibrary = nullptr);

    void addTracksPrepare();
    TrackId addTracksAddTrack(
            const TrackPointer& pTrack,
            bool unremove);
    TrackPointer addTracksAddFile(
            const TrackFile& trackFile,
            bool unremove);
    void addTracksFinish(bool rollback = false);

    bool updateTrack(Track* pTrack);

    void hideAllTracks(const QDir& rootDir);

    bool hideTracks(
            const QList<TrackId>& trackIds);
    void afterHidingTracks(
            const QList<TrackId>& trackIds);

    bool unhideTracks(
            const QList<TrackId>& trackIds);
    void afterUnhidingTracks(
            const QList<TrackId>& trackIds);

    bool onPurgingTracks(
            const QList<TrackId>& trackIds);
    void afterPurgingTracks(
            const QList<TrackId>& trackIds);

    // Scanning related calls.
    void markTrackLocationsAsVerified(const QStringList& locations);
    void markTracksInDirectoriesAsVerified(const QStringList& directories);
    void invalidateTrackLocationsInLibrary();
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

    QSqlDatabase m_database;

    CueDAO& m_cueDao;
    PlaylistDAO& m_playlistDao;
    AnalysisDao& m_analysisDao;
    LibraryHashDAO& m_libraryHashDao;

    UserSettingsPointer m_pConfig;

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

#endif //TRACKDAO_H
